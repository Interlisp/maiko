/* $Id: setsout.c,v 1.3 1999/05/31 23:35:41 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	setsout.c
 */
/************************************************************************/
/*									*/
/*			    S E T S Y S O U T				*/
/*									*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <unistd.h>
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

void set_sysout(int version, char *sysout_file_name);

void set_sysout(int version, char *sysout_file_name) {
  int sysout;    /* SysoutFile descriptor */
  IFPAGE ifpage; /* IFPAGE */
  char errmsg[255];

  /*
   * first read the IFPAGE(InterfacePage)
   */

  /* open SysoutFile */
  sysout = open(sysout_file_name, O_RDWR, NULL);
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

  ifpage.lversion = version;
  ifpage.minbversion = version;

  /* seek to IFPAGE */
  if (lseek(sysout, IFPAGE_ADDRESS, 0) == -1) {
    perror("sysout_loader: can't seek to IFPAGE");
    exit(-1);
  }
  /* reads IFPAGE into scratch_page */
  if (write(sysout, &ifpage, sizeof(IFPAGE)) == -1) {
    perror("Can't write IFPAGE");
    exit(-1);
  }

  close(sysout);
  printf("%d", ifpage.minbversion);
}

int main(int argc, char **argv) {
  int version, res;
  if (argc < 3) {
    printf("setsysout version sysout-name\n");
    return (-1);
  }
  if ((version = atoi(argv[1])) == 0) {
    printf("version must be an integer > 0.\n");
    return (-1);
  }
  set_sysout(version, argv[2]);
  exit(0);
}
