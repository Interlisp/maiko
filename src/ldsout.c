/* $Id: ldsout.c,v 1.4 2001/12/26 22:17:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*                                                                      */
/*  (C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1998 Venue. */
/*  All Rights Reserved.                                                */
/*  Manufactured in the United States of America.                       */
/*                                                                      */
/************************************************************************/

#include "version.h"

#include <fcntl.h>        // for open, O_RDONLY
#include <stdio.h>        // for perror, fprintf, printf, stderr, sprintf
#include <stdlib.h>       // for exit, free, malloc
#include <string.h>       // for memset
#include <sys/mman.h>     // for mmap, MAP_FAILED
#ifndef MAP_FAILED
#define MAP_FAILED	((void *)-1)
#endif
#include <sys/stat.h>     // for stat, fstat
#include <sys/types.h>    // for off_t
#include <unistd.h>       // for lseek, read, close, getpagesize
#include "adr68k.h"       // for NativeAligned2FromLAddr
#ifdef BYTESWAP
#include "byteswapdefs.h" // for word_swap_page
#endif
#include "dbprint.h"      // for DBPRINT, TPRINT
#include "devif.h"        // for DspInterface
#include "ifpage.h"       // for IFPAGE, IFPAGE_KEYVAL
#include "initdspdefs.h"  // for flush_display_buffer, init_display2
#include "ldsoutdefs.h"   // for sysout_loader
#include "lispemul.h"     // for BYTESPER_PAGE, GETFPTOVP, DLword, GETPAGEOK
#include "lispmap.h"      // for DISPLAY_OFFSET
#include "lspglob.h"      // for Lisp_world, native_load_address
#include "lsptypes.h"

#define IFPAGE_ADDRESS 512
#define DEFAULT_MAX_SYSOUTSIZE 64 /* in Mbyte */
#define DEFAULT_PRIME_SYSOUTSIZE 8
#define MAX_EXPLICIT_SYSOUTSIZE 256 /* Max possible sysout size is 64Mb */
#define MBYTE 0x100000              /* 1 Mbyte */

/* Flag for indication whether process space is going to expand or not */
int Storage_expanded; /*  T or NIL */

/************************************************************************/
/*                                                                      */
/*                       s y s o u t _ l o a d e r                      */
/*                                                                      */
/*      Load the sysout file into memory.                               */
/*                                                                      */
/************************************************************************/
#if defined(DOS) || defined(XWINDOW)
#include "devif.h"
extern DspInterface currentdsp;
#endif /* DOS || XWINDOW */

/* sys_size is sysout size in megabytes */
unsigned sysout_loader(const char *sysout_file_name, unsigned sys_size) {
  int sysout; /* SysoutFile descriptor */

  IFPAGE ifpage; /* IFPAGE */

#ifdef BIGVM
  /* 1 "cell" per page for 256MB in 512 byte pages */
  unsigned int *fptovp;
#else
  DLword *fptovp; /* FPTOVP */
#endif                /* BIGVM */
  off_t fptovp_offset; /* FPTOVP start offset */

  char *lispworld_scratch; /* scratch area for lispworld */
  size_t lispworld_offset;   /* lispworld offset */

  unsigned sysout_size; /* sysout size in page */
  struct stat stat_buf; /* file stat buf */

  char errmsg[255];
  off_t cfp = -1;  /* tracks current file position in sysout, or -1 */

  /* Checks for specifying the process size (phase I) */
  /* If sys_size == 0 figure out the proper size later */
  if ((sys_size != 0) && (sys_size < DEFAULT_PRIME_SYSOUTSIZE)) {
    perror("You have to specify more than 8MB for process size");
    exit(-1);
  } else if (sys_size > MAX_EXPLICIT_SYSOUTSIZE) {
    perror("256Mb is the maximum legal sysout size.");
    exit(-1);
  }

  /*
   * first read the IFPAGE(InterfacePage)
   */

  /* open SysoutFile */
  sysout = open(sysout_file_name, O_RDONLY);
  if (sysout == -1) {
    sprintf(errmsg, "sysout_loader: can't open sysout file: %s", sysout_file_name);
    perror(errmsg);
    exit(-1);
  }

  /* seek to IFPAGE */
  if (lseek(sysout, IFPAGE_ADDRESS, SEEK_SET) == -1) {
    perror("sysout_loader: can't seek to IFPAGE");
    exit(-1);
  }

  /* reads IFPAGE into scratch_page */
  if (read(sysout, &ifpage, sizeof(IFPAGE)) == -1) {
    perror("sysout_loader: can't read IFPAGE");
    exit(-1);
  }

#ifdef BYTESWAP
  word_swap_page((unsigned short *)&ifpage, (3 + sizeof(IFPAGE)) / 4);
#endif

/* Check the sysout and emulator for compatibility:
       The sysout's ifpage.lversion must be >= LVERSION, and
       the sysout's ifpage.minbversion must be <= MINBVERSION
*/
#ifndef NOVERSION
  if (ifpage.lversion < LVERSION) {
    (void)fprintf(stderr, "Lisp VM is too old for this emulator.\n");
    (void)fprintf(stderr, "(version is %d, must be at least %d.)\n", ifpage.lversion, LVERSION);
    exit(-1);
  }

  if (ifpage.minbversion > MINBVERSION) {
    (void)fprintf(stderr, "Emulator is too old for this Lisp VM.\n");
    (void)fprintf(stderr, "(version is %d, must be at least %d.)\n", MINBVERSION, ifpage.minbversion);
    exit(-1);
  }
#endif /* NOVERSION */

  /* use default or the previous one */
  if (sys_size == 0) {
    if (ifpage.process_size == 0)        /* Pure LISP.SYSOUT */
      sys_size = DEFAULT_MAX_SYSOUTSIZE; /* default for pure SYSOUT */
    else
      sys_size = ifpage.process_size; /* use previous one */
  }

  /* Checks for specifying the process size (phase II) */
  if ((ifpage.storagefullstate == SFS_ARRAYSWITCHED) ||
      (ifpage.storagefullstate == SFS_FULLYSWITCHED)) {
    /* Storage may be allocated from Secondary space */
    /* Therefore you can not change \\DefaultSecondMDSPage */
    if (ifpage.process_size != sys_size) {
      char tmp[200];
      sprintf(tmp,
              "\nsysout loader: Error, secondary space in use. You can't specify size.\nProcess "
              "size = %d\nSys size = %d\n",
              ifpage.process_size, sys_size);
#ifdef DOS
      /* Note that we have an initialized display by now. */
      /* Hence we have to observe the display protocol. */
      VESA_errorexit(tmp);
#else
      (void)fprintf(stderr, "sysout_loader: You can't specify the process size.\n");
      (void)fprintf(stderr, "Because, secondary space is already used.\n");
      (void)fprintf(stderr, "(size is %d, you specified %d.)\n", ifpage.process_size, sys_size);
      exit(-1);
#endif /* DOS */
    }
    /*Can use this sys_size as the process size */
    /* The sys_size should be same as the previous one */
    Storage_expanded = NIL;
  } else { /* assumes that we can expand the process space */
    Storage_expanded = T;
    /* You can use secondary space , though it was STORAGEFULL
       So, STORAGEFULL may be set to NIL later  */
  }

  /* allocate Virtual Memory Space */

  lispworld_scratch = mmap(0, sys_size * MBYTE, PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (lispworld_scratch == MAP_FAILED) {
    (void)fprintf(stderr, "sysout_loader: can't allocate Lisp %dMBytes VM \n", sys_size);
    exit(-1);
  }

  /* now you can access lispworld */
  Lisp_world = (DLword *)lispworld_scratch;

  DBPRINT(("VM allocated = 0x%x at %p\n", sys_size * MBYTE, (void *)Lisp_world));
  DBPRINT(("Native Load Address = 0x%x\n", native_load_address));

  /*
   * get FPTOVP location and SysoutFile size
   */

  /* get FPTOVP location from IFPAGE */
  fptovp_offset = ifpage.fptovpstart;

  DBPRINT(("FPTOVP Location %ld \n", (long)fptovp_offset));

  /* get sysout file size in halfpage(256) */
  if (fstat(sysout, &stat_buf) == -1) {
    perror("sysout_loader: can't get sysout file size");
    exit(-1);
  }
  sysout_size = (unsigned)(stat_buf.st_size / BYTESPER_PAGE * 2);

  DBPRINT(("sysout size / 2 = 0x%x\n", sysout_size / 2));
  DBPRINT(("vmem size = 0x%x\n", ifpage.nactivepages));

  /* do some simple checks to see if this is really a sysout */
  if (ifpage.key != IFPAGE_KEYVAL) {
    printf("sysout_loader: %s isn't a sysout:\nkey is %x, should be %x\n", sysout_file_name,
           ifpage.key, IFPAGE_KEYVAL);
    exit(1);
  }

  if ((stat_buf.st_size & (BYTESPER_PAGE - 1)) != 0)
    printf("CAUTION::not an integral number of pages.  sysout & 0x1ff = 0x%x\n",
           (int)(stat_buf.st_size & (BYTESPER_PAGE - 1)));

  if (ifpage.nactivepages != (sysout_size / 2)) {
    printf("sysout_loader:IFPAGE says sysout size is %d\n", ifpage.nactivepages);
    printf("But, sysout size from UNIX is %d\n", sysout_size / 2);
    exit(-1);
  }

/*
 * Now get FPTOVP
 */

/* seek to FPTOVP */
#ifdef BIGVM
  fptovp_offset = (fptovp_offset - 1) * BYTESPER_PAGE + 4;
#else
  fptovp_offset = (fptovp_offset - 1) * BYTESPER_PAGE + 2;
#endif
  if (lseek(sysout, fptovp_offset, SEEK_SET) == -1) {
    perror("sysout_loader: can't seek to FPTOVP");
    exit(-1);
  }

  /* read FPTOVP */

#ifdef BIGVM
  /* fptovp is now in cells, not words */
  fptovp = malloc(sysout_size * 2 + 4);
  if (read(sysout, fptovp, sysout_size * 2) == -1) {
    perror("sysout_loader: can't read FPTOVP");
    free(fptovp);
    exit(-1);
  }

#ifdef BYTESWAP
  /* So space to swap is twice as big, too. */
  word_swap_page((unsigned short *)fptovp, (sysout_size / 2) + 1);
#endif /* BYTESWAP */

#else

  fptovp = malloc(sysout_size + 2);
  if (read(sysout, fptovp, sysout_size) == -1) {
    perror("sysout_loader: can't read FPTOVP");
    free(fptovp);
    exit(-1);
  }

#ifdef BYTESWAP
  word_swap_page((unsigned short *)fptovp, (sysout_size / 4) + 1);
#endif /* BYTESWAP */

#endif /* BIGVM */

  /*
   * Initialize the display (note now passing 68k address!!!)
   */
  init_display2(NativeAligned2FromLAddr(DISPLAY_OFFSET), 65536 * 16 * 2);

  /* read sysout file to lispworld */

  for (unsigned i = 0; i < (sysout_size / 2); i++) {
#ifdef DOS
    /* Dial that floats from left to right on the top line of the */
    /* displaty. Dial shows % of sysout loaded by digits and */
    /* position. */
    int columns;
    switch (currentdsp->graphicsmode) {
      case 0x104:
        columns = 120; /* 131 - 10 */
        break;
      case 0x102:
        columns = 69; /* 79 - 10 */
        break;
      default:
        columns = 69; /* 79 - 10 */
        break;
    }
    _settextposition((short)0, (short)0);
    if ((i & 0xf) == 0) {
      for (int j = 0; j < (columns * i) / (sysout_size >> 1); j++) putchar(' ');
      printf("-=(%2d%%)=-\n", (100 * i) / (sysout_size >> 1));
    }
#endif /* DOS */
    if (GETPAGEOK(fptovp, i) != 0177777) {
      /* only seek if not already at desired position */
      if (i * BYTESPER_PAGE != cfp) {
	if (lseek(sysout, i * BYTESPER_PAGE, SEEK_SET) == -1) {
	  perror("sysout_loader: can't seek sysout file");
	  free(fptovp);
	  exit(-1);
	}
	cfp = i * BYTESPER_PAGE; /* now at known position */
      }
      lispworld_offset = GETFPTOVP(fptovp, i) * BYTESPER_PAGE;
      if (read(sysout, lispworld_scratch + lispworld_offset, BYTESPER_PAGE) == -1) {
        printf("sysout_loader: can't read sysout file at %d\n", i);
        printf("               offset was 0x%zx (0x%x pages).\n", lispworld_offset,
               GETFPTOVP(fptovp, i));
        perror("read() error was");
        for (int j = 0; j < 10; j++) {
          printf(" %d: 0x%x  ", j, GETFPTOVP(fptovp, j));
        }
        free(fptovp);
        exit(-1);
      }
      cfp += BYTESPER_PAGE; /* new known position */
#ifdef BYTESWAP
      word_swap_page((DLword *)(lispworld_scratch + lispworld_offset), 128);
#endif
    }
  }
  free(fptovp);
  DBPRINT(("sysout file is read completely.\n"));

#if (defined(DISPLAYBUFFER) || defined(XWINDOW) || defined(DOS))
  TPRINT(("Flushing display buffer...\n"));
  flush_display_buffer();
  TPRINT(("After Flushing display buffer\n"));
#endif /* DISPLAYBUFFER || XWINDOW || DOS */

  close(sysout);
  return (sys_size);
}
