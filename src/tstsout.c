/* $Id: tstsout.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
/*
 *	tstsout.c
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "adr68k.h"
#include "lispemul.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "lspglob.h"
#include "ifpage.h"
#include "dbprint.h"
#include "byteswapdefs.h"

#define IFPAGE_ADDRESS 512
#define MBYTE 0x100000 /* 1 Mbyte */
extern int errno;

void check_sysout(char *sysout_file_name, int verbose);
void usage(char *prog);

void check_sysout(char *sysout_file_name, int verbose) {
  int sysout; /* SysoutFile descriptor */

  IFPAGE ifpage; /* IFPAGE */

  char errmsg[255];

  /*
   * first read the IFPAGE(InterfacePage)
   */

  /* open SysoutFile */
  sysout = open(sysout_file_name, O_RDONLY, NULL);
  if (sysout == -1) {
    sprintf(errmsg, "sysout_loader: can't open sysout file: %s", sysout_file_name);
    perror(errmsg);
    exit(-1);
  }
  /* seek to IFPAGE */
  if (lseek(sysout, IFPAGE_ADDRESS, 0) == -1) {
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
  close(sysout);
  if (verbose) {
      printf("ifpage.minbversion %d\n", ifpage.minbversion);
      printf("ifpage.process_size %d\n", ifpage.process_size);
#ifdef NEW_STORAGE
      printf("ifpage.storagefullstate %d\n", ifpage.storagefullstate);
#endif
      printf("ifpage.nactivepages %d\n", ifpage.nactivepages);
  } else {
      printf("%d", ifpage.minbversion);
  }
}

void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-v] sysout-filename\n", prog);
    exit(-1);
}

int main(int argc, char **argv) {
    switch (argc) {
    case 2:
        check_sysout(argv[1], 0);
        break;
    case 3:
        if (0 == strncmp(argv[1], "-v", 2)) {
            check_sysout(argv[2], 1);
        } else {
            usage(argv[0]);
        }
        break;
    default:
        usage(argv[0]);
    }
  exit(0);
}
