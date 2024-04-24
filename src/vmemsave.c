/* $Id: vmemsave.c,v 1.2 1999/01/03 02:07:45 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
*	vmemsave.c
*
*
*/

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>	// for ptrdiff_t
#include <stdio.h>
#include <stdlib.h>
#ifndef DOS
#include <dirent.h>
#include <string.h>
#include <sys/param.h>
#include <pwd.h>
#include <unistd.h>
#else
#include <direct.h>
#include <stdlib.h>
#define MAXPATHLEN _MAX_PATH
#define MAXNAMLEN _MAX_PATH
#define alarm(x) 1
#endif /* DOS */


#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "vmemsave.h"
#include "timeout.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "locfile.h"
#include "dbprint.h"
#include "devif.h"
#include "ifpage.h"       // for MACHINETYPE_MAIKO

#include "vmemsavedefs.h"
#include "byteswapdefs.h"
#include "commondefs.h"
#include "dskdefs.h"
#include "initkbddefs.h"
#include "perrnodefs.h"
#include "ufsdefs.h"


/* Error return values from VMEMSAVE */
#define COMPLETESYSOUT NIL
#define BADFILENAME (S_POSITIVE | 1)
#define NOFILESPACE (S_POSITIVE | 2)
#define FILECANNOTOPEN (S_POSITIVE | 3)
#define FILECANNOTSEEK (S_POSITIVE | 4)
#define FILECANNOTWRITE (S_POSITIVE | 5)
#define FILETIMEOUT (S_POSITIVE | 6)

extern struct pixrect *CursorBitMap, *InvisibleCursorBitMap;
extern struct cursor CurrentCursor, InvisibleCursor;
#ifdef DOS
extern DspInterface currentdsp;
#endif /* DOS */

extern int *Lisp_errno;
extern int Dummy_errno; /* Used if errno cell isn't provided by Lisp.*/
extern int please_fork;

/************************************************************************/
/*									*/
/*			l i s p s t r i n g p				*/
/*									*/
/*	Predicate:  Is the argument (which must be a Lisp 1-d array)	*/
/*		    a lisp string?  i.e., are its elements char's?	*/
/*									*/
/************************************************************************/

int lispstringP(LispPTR Lisp)
{
  switch (((OneDArray *)(NativeAligned4FromLAddr(Lisp)))->typenumber) {
    case THIN_CHAR_TYPENUMBER:
    case FAT_CHAR_TYPENUMBER: return (1);

    default: return (0);
  }
}

/************************************************************************/
/*									*/
/*			v m e m _ s a v e 0				*/
/*									*/
/*	Implements the VMEMSAVE subr.  Write out the current lisp	*/
/*	lisp image to the specified file.  If the sysout file-name	*/
/*	is explicitly specified, the directory on which the file	*/
/*	resides is exactly (?) an existing directory.  This is		*/
/*	guaranteed by the Lisp code, \MAIKO.CHECKFREEPAGE in LLFAULT.	*/
/*									*/
/*	If the file argument is nill, the default sysout-file name,	*/
/*	"~/lisp.virtualmem", is used, subject to override by the	*/
/*	LDEDESTSYSOUT UNIX-environment variable.			*/
/*									*/
/*									*/
/*									*/
/*									*/
/* Argument:	LispPTR	*args	args[0]					*/
/*			 The file name in Lisp format specifying a	*/
/*			 file to which the current Lisp image should	*/
/*			 be flushed or Lisp NIL.			*/
/*									*/
/* Value:	If succeed, returns NIL, otherwise one of the		*/
/*		following Lisp integers, indicating the reason.		*/
/*									*/
/*			1	BADFILENAME				*/
/*			2	NOFILESPACE				*/
/*			3	FILECANNOTOPEN				*/
/*			4	FILECANNOTSEEK				*/
/*			5	FILECANNOTWRITE				*/
/*			6	FILETIMEOUT				*/
/*									*/
/* Side Effect:	None.							*/
/*									*/
/************************************************************************/

LispPTR vmem_save0(LispPTR *args)
{
  char *def;
  char pathname[MAXPATHLEN], sysout[MAXPATHLEN], host[MAXNAMLEN];
#ifdef DOS
  char pwd[MAXNAMLEN];
  char drive[1];
#else
  struct passwd *pwd;
#endif /* DOS */

  Lisp_errno = &Dummy_errno;

  if ((args[0] != NIL) && lispstringP(args[0])) {
    /* Check of lispstringP is safer for LispStringToCString */
    LispStringToCString(args[0], pathname, MAXPATHLEN);
    separate_host(pathname, host);
#ifdef DOS
    if (!unixpathname(pathname, sysout, 0, 0, drive, 0, 0)) return (BADFILENAME);
#else
    if (!unixpathname(pathname, sysout, 0, 0)) return (BADFILENAME);
#endif /* DOS */
    return (vmem_save(sysout));
  } else {
    if ((def = getenv("LDEDESTSYSOUT")) == 0) {
#ifdef DOS
      if (getcwd(pwd, MAXNAMLEN) == NULL) return (FILETIMEOUT);
      strcpy(sysout, pwd);
      strcat(sysout, "/lisp.vm");
#else
      pwd = getpwuid(getuid()); /* NEED TIMEOUT */
      if (pwd == (struct passwd *)NULL) return (FILETIMEOUT);
      strcpy(sysout, pwd->pw_dir);
      strcat(sysout, "/lisp.virtualmem");
#endif /* DOS */
    } else {
      if (*def == '~' && (*(def + 1) == '/' || *(def + 1) == '\0')) {
#ifdef DOS
        if (getcwd(pwd, MAXNAMLEN) == NULL) return (FILETIMEOUT);
        strcpy(sysout, pwd);
#else
        pwd = getpwuid(getuid()); /* NEED TIMEOUT */
        if (pwd == (struct passwd *)NULL) return (FILETIMEOUT);
        strcpy(sysout, pwd->pw_dir);
#endif /* DOS */
        strcat(sysout, def + 1);
      } else {
        strcpy(sysout, def);
      }
    }
    return (vmem_save(sysout));
  }
}

/************************************************************************/
/*									*/
/*			s o r t _ f p t o v p				*/
/*									*/
/*	Sort the entries in the file-page-to-virtual-page table,	*/
/*	to try to make a sysout file that has contiguous runs of	*/
/*	virtual pages in it, for speed.					*/
/*									*/
/************************************************************************/
#ifndef BIGVM
#ifndef BYTESWAP
static int twowords(const void *i, const void *j) /* the difference between two  DLwords. */
{ return (*(const DLword *)i - *(const DLword *)j); }

#define FPTOVP_ENTRY (FPTOVP_OFFSET >> 8)

static void sort_fptovp(DLword *fptovp, size_t size)
{
  size_t oldsize, i;
  ptrdiff_t oldloc, newloc;
  DLword *fptr;

  for (fptr = fptovp, i = 0; GETWORD(fptr) != FPTOVP_ENTRY && i < size; fptr++, i++)
    ;

  if (GETWORD(fptr) != FPTOVP_ENTRY) {
    DBPRINT(("Couldn't find FPTOVP_ENTRY; not munging\n"));
    return;
  }
  oldloc = fptr - fptovp;

  /* Found old fptovp table location, now sort the table */
  qsort(fptovp, size, sizeof(DLword), twowords);

ONE_MORE_TIME: /* Tacky, but why repeat code? */

  /* Look up FPTOVP_ENTRY again; if it's moved, need to shuffle stuff */
  for (fptr = fptovp, i = 0; GETWORD(fptr) != FPTOVP_ENTRY && i < size; fptr++, i++)
    ;

  if (GETWORD(fptr) != FPTOVP_ENTRY) error("Couldn't find FPTOVP_ENTRY second time!\n");
  newloc = fptr - fptovp;

  /* Supposedly all we have to do is adjust the fptovpstart and nactivepages
     the ifpage */
  InterfacePage->fptovpstart += (newloc - oldloc);
  oldsize = size;
  for (fptr = fptovp + (size - 1); GETWORD(fptr) == 0xffff;
       fptr--, InterfacePage->nactivepages--, size--)
    ;

  if (size != oldsize) {
    DBPRINT(("Found %d holes in fptovp table\n", oldsize - size));
  }

  /* Sanity check; it's just possible there are duplicate entries... */
  {
    int dupcount = 0;
    for (fptr = fptovp, i = 1; i < size; i++, fptr++)
      if (GETWORD(fptr) == GETWORD(fptr + 1)) {
        dupcount++;
        GETWORD(fptr) = 0xffff;
      }

    /* if duplicates were found, resort to squeeze them out, then mung the
       size and fptovpstart again (spaghetti-code, HO!) */
    if (dupcount) {
      qsort(fptovp, size, sizeof(DLword), twowords);
      oldloc = newloc;
      DBPRINT(("%d duplicates found\n", dupcount));
      goto ONE_MORE_TIME;
    }
  }
}
#endif
#endif
/************************************************************************/
/*									*/
/*				v m e m _ s a v e			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*
 * Argument:	char	*sysout_file_name
 *			 The file name in UNIX format specifying a file to which
 *			 the current Lisp image should be flushed.
 *
 * Value:	If succeed, returns Lisp NIL, otherwise one of the following Lisp integer
 *		indicating the reason of failure.
 *
 *			1	BADFILENAME
 *			2	NOFILESPACE
 *			3	FILECANNOTOPEN
 *			4	FILECANNOTSEEK
 *			5	FILECANNOTWRITE
 *			6	FILETIMEOUT
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Flush out the current Lisp image to the specified file.
 */

/* diagnostic flag value to limit the size of write() s */
extern unsigned maxpages;
unsigned maxpages = 65536;

LispPTR vmem_save(char *sysout_file_name)
{
  int sysout; /* SysoutFile descriptor */
#ifdef BIGVM
  unsigned int *fptovp;
#else
  DLword *fptovp; /* FPTOVP */
#endif          /* BIGVM */
  int vmemsize; /* VMEMSIZE */
  int i;
  char tempname[MAXPATHLEN];
  ssize_t rsize;
  off_t roff;
  int rval;
#ifndef DOS
  extern int ScreenLocked;
  extern DLword *EmCursorX68K;
  extern DLword *EmCursorY68K;
  extern DLword NullCursor[];
  extern DLword *EmCursorBitMap68K;
#endif /* DOS */

/* remove cursor image from screen */

#ifdef   DOS
  /*  For DOS, must also take the mouse cursor away (it's  */
  /*  written into the display-region bitmap).	     */
  currentdsp->device.locked++;
  (currentdsp->mouse_invisible)(currentdsp, IOPage);
#endif /* SUNDISPLAY || DOS */

  /* set FPTOVP */
  fptovp = FPtoVP + 1;

  /* set VMEMSIZE */
  vmemsize = InterfacePage->nactivepages;

  /*	[HH:6-Jan-89]
          Sequence of save image
          (1) Sysout image is saved to a temporary file, tempname.
          (2) if a specified file, sysout_file_name, is exist, the file is removed.
          (3) the temporary file is renamed to the specified file.
  */

  SETJMP(FILETIMEOUT);
#ifdef DOS
  /* Bloddy 8 char filenames in dos ... /jarl */
  make_old_version(tempname, sysout_file_name);
#else  /* DOS */
  sprintf(tempname, "%s-temp", sysout_file_name);
#endif /* DOS */

  /* Confirm protection of specified file by open/close */

  TIMEOUT(sysout = open(sysout_file_name, O_WRONLY, 0666));
  if (sysout == -1) {
    /* No file error skip return. */
    if (errno != ENOENT) return (FILECANNOTOPEN); /* No such file error.*/
  } else
    TIMEOUT(rval = close(sysout));

  /* open temp file */
  TIMEOUT(sysout = open(tempname, O_WRONLY | O_CREAT | O_TRUNC, 0666));
  if (sysout == -1) {
    err_mess("open", errno);
    return (FILECANNOTOPEN);
  }

  InterfacePage->machinetype = MACHINETYPE_MAIKO;

  /* Restore storagefull state */
  if (((*STORAGEFULLSTATE_word) & 0xffff) == SFS_NOTSWITCHABLE) {
    /* This sysout uses only 8 Mbyte lisp space.
       It may be able to use this SYSOUT which has more than
       8 Mbyte lisp space.
       To enable to expand lisp space, \\STORAGEFULLSTATE
       should be NIL.
    */
    *STORAGEFULLSTATE_word = NIL;
    InterfacePage->storagefullstate = NIL;
  } else {
    /*  Otherwise, just restore storagefullstate in IFPAGE */
    InterfacePage->storagefullstate = (*STORAGEFULLSTATE_word) & 0xffff;
  }

/* First, sort fptovp table, trying to get pages contiguous */
#ifndef BIGVM
#ifndef BYTESWAP
  /* Byte-swapped machines don't sort the table right. */
  sort_fptovp(fptovp, vmemsize);
#endif
#endif

  /* store vmem to sysoutfile */

  for (i = 0; i < vmemsize; i++) {
    if (GETPAGEOK(fptovp, i) != 0177777) {
      unsigned int oldfptovp = GETFPTOVP(fptovp, i);
      unsigned int saveoldfptovp = oldfptovp;
      unsigned int contig_pages = 0;
      DLword *base_addr;

      TIMEOUT(roff = lseek(sysout, i * BYTESPER_PAGE, SEEK_SET));
      if (roff == -1) {
        err_mess("lseek", errno);
        return (FILECANNOTSEEK);
      }
      base_addr = Lisp_world + (GETFPTOVP(fptovp, i) * DLWORDSPER_PAGE);

      /* Now, let's see how many pages we can dump */
      while (GETFPTOVP(fptovp, i) == oldfptovp && i < vmemsize) {
        contig_pages++; oldfptovp++; i++;
      }
      i--; /* Previous loop always overbumps i */
      DBPRINT(("%4d: writing %d pages from %tx (%d)\n", i, contig_pages, (char *)base_addr - (char *)Lisp_world, saveoldfptovp));

#ifdef BYTESWAP
      word_swap_page(base_addr, contig_pages * CELLSPER_PAGE);
#endif /* BYTESWAP */

      if (contig_pages > maxpages) {
        DLword *ba = base_addr;
        unsigned int pc = contig_pages;
        while (pc > maxpages) {
          TIMEOUT(rsize = write(sysout, ba, (size_t)maxpages * BYTESPER_PAGE));
          if (rsize == -1) {
            err_mess("write", errno);
            return ((errno == ENOSPC) || (errno == EDQUOT)) ? NOFILESPACE : FILECANNOTWRITE;
          }
          ba += maxpages * DLWORDSPER_PAGE;
          pc -= maxpages;
        }
        if (pc > 0) TIMEOUT(rsize = write(sysout, ba, pc * BYTESPER_PAGE));
      } else {
        unsigned int oldTT = TIMEOUT_TIME;
        /* As we can spend longer than TIMEOUT_TIME doing a big
           write, we adjust the timeout temporarily here */
        TIMEOUT_TIME += contig_pages >> 3;
        TIMEOUT(rsize = write(sysout, base_addr, contig_pages * BYTESPER_PAGE));
        TIMEOUT_TIME = oldTT;
      }
#ifdef BYTESWAP
      word_swap_page(base_addr, contig_pages * CELLSPER_PAGE);
#endif /* BYTESWAP */

      if (rsize == -1) {
        err_mess("write", errno);
	return ((errno == ENOSPC) || (errno == EDQUOT)) ? NOFILESPACE : FILECANNOTWRITE;
      }
    }
  }

  /* seek to IFPAGE */
  TIMEOUT(roff = lseek(sysout, (long)FP_IFPAGE, SEEK_SET));
  if (roff == -1) {
    err_mess("lseek", errno);
    return (FILECANNOTSEEK);
  }
#ifdef BYTESWAP
  word_swap_page(InterfacePage, CELLSPER_PAGE);
#endif /* BYTESWAP */

  TIMEOUT(rsize = write(sysout, (char *)InterfacePage, BYTESPER_PAGE));
#ifdef BYTESWAP
  word_swap_page(InterfacePage, CELLSPER_PAGE);
#endif /* BYTESWAP */

  if (rsize == -1) {
    err_mess("write", errno);
	return ((errno == ENOSPC) || (errno == EDQUOT)) ? NOFILESPACE : FILECANNOTWRITE;
  }

#ifdef OS5
  /* Seems to write all pages at close, so timeout
     is WAY to short, no matter how big.  JDS 960925 */
  rval = close(sysout);
#else
  TIMEOUT(rval = close(sysout));
#endif /* OS5 */
  if (rval == -1) { return (FILECANNOTWRITE); }

  TIMEOUT(rval = unlink(sysout_file_name));
  if (rval == -1) {
    /* No file error skip return. */
    if (errno != ENOENT) /* No such file error.*/
      return (FILECANNOTOPEN);
  }

  TIMEOUT(rval = rename(tempname, sysout_file_name));
  if (rval == -1) {
    (void)fprintf(stderr, "sysout is saved to temp file, %s.", tempname);
    return (FILECANNOTWRITE);
  }

/* restore cursor image to screen */
#ifdef DOS
  /* Must also put the mouse back. */
  (currentdsp->mouse_visible)(IOPage->dlmousex, IOPage->dlmousey);
  currentdsp->device.locked--;

#endif /* SUNDISPLAY */

  /*printf("vmem is saved completely.\n");*/
  return (COMPLETESYSOUT);
}

/************************************************************************/
/*									*/
/*			l i s p _ f i n i s h				*/
/*									*/
/*	Kill all forked sub-processes before exiting.			*/
/*									*/
/************************************************************************/

/* Make sure that we kill off any Unix subprocesses before we go away */

void lisp_finish(void) {
  char d[4];

  DBPRINT(("finish lisp_finish\n"));

  if (please_fork) { /* if lde runs with -NF(No fork), */
                     /* following 5 lines don't work well. */
    d[0] = 'E';
    d[3] = 1;
    /* These only happen if the fork really succeeded: */
    /* if (UnixPipeOut >= 0) write(UnixPipeOut, d, 4); */
    /* if (UnixPipeIn >= 0 read(UnixPipeIn, d, 4);*/ /* Make sure it's finished */
    /* if (UnixPID >= 0) kill(UnixPID, SIGKILL);*/   /* Then kill fork_Unix itself */
  }
  device_before_exit();
#ifdef DOS
  exit_host_filesystem();
#endif /* DOS */
  exit(0);
}
