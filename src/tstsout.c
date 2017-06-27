/* $Id: tstsout.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: tstsout.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ Copyright (C) Venue";
/*
 *	tstsout.c
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

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

#define IFPAGE_ADDRESS 512
#define MBYTE 0x100000 /* 1 Mbyte */
extern int errno;

/* JDS protoize char *valloc(size_t); */

void check_sysout(char *sysout_file_name) {
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
  word_swap_page(&ifpage, (3 + sizeof(IFPAGE)) / 4);
#endif
  close(sysout);
  printf("%d", ifpage.minbversion);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("You forgot to supply a file name.");
    return (-1);
  }
  check_sysout(argv[1]);
  exit(0);
}
