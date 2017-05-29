/* $Id: mkvdate.c,v 1.5 2001/12/26 22:17:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: mkvdate.c,v 1.5 2001/12/26 22:17:03 sybalsky Exp $ Copyright (C) Venue";

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

/************************************************************************/
/*									*/
/*			m a k e v d a t e . c 				*/
/*									*/
/*	Make the version number to place in the emulator.  That number	*/
/*	will be smashed into the running sysout at the field		*/
/*	InterfacePage->rversion.  The version number is the number	*/
/*	of whole days since 12:00 13-Oct-87 (Takeshi's birthday).	*/
/*									*/
/*	This program prints a single line:				*/
/*		long MDate = <the version number>			*/
/*									*/
/*	That output is redirected to create the file vdate.c, which	*/
/*	is then compiled as part of the emulator MAKE.			*/
/*									*/
/************************************************************************/

#ifdef OSF1
#include "time.h"
#endif

#include <stdio.h>
#ifdef USETIMEFN
#include <time.h>
#elif DOS
#include <time.h>
#else
#include <sys/time.h>
#endif /* USETIMEFN */

#ifdef USETIMEFN
/* RISCOS doesn't have the BSD time functions */
int main(void) {
  long dtime;
  time(&dtime);
  fprintf(stderr, "Mdate :%d\n", dtime);
  printf("long MDate= %d;\n", dtime);
  return (0);
}
#else
/* Version for every other Unix */
int main(void) {
  struct timeval time;

  gettimeofday(&time, NULL);
  fprintf(stderr, "Mdate :%d\n", time.tv_sec);
  fprintf(stderr, "Version: %s\n", ctime(&time.tv_sec));

  printf("long MDate= %d;\n", time.tv_sec);
  return (0);
}

#endif /* SYSVONLY */
