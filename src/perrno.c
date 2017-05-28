/* $Id: perrno.c,v 1.4 2001/12/26 22:17:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: perrno.c,v 1.4 2001/12/26 22:17:04 sybalsky Exp $ Copyright (C) Venue";







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


	
#include	<stdio.h>
#include	<errno.h>
#include	"osmsg.h"



/************************************************************************/
/*									*/
/*			p e r r o r n					*/
/*									*/
/*	Print the error message to go with a given error number.	*/
/*									*/
/************************************************************************/

extern int errno;
#if defined(MACOSX) || defined(FREEBSD)
extern const char * const sys_errlist[];
extern const int sys_nerr;
#else
int sys_nerr;
#ifndef LINUX
extern char *sys_errlist[];
#endif /* LINUX */
#endif

void perrorn(char *s, int n)
{
  if ( s != NULL && *s != '\0' ) {
    fprintf(stderr, "%s: ",s);
  }
  if (n > 0 && n < sys_nerr) {
    fprintf(stderr, "%s\n", sys_errlist[n]);
  } else {
    fprintf(stderr, "???\n");
  }
}



/************************************************************************/
/*									*/
/*				e r r _ m e s s				*/
/*									*/
/*	Print an error message and call 'perror' to get the		*/
/*	canonical error explanation.  Called by emulator I/O code.	*/
/*									*/
/************************************************************************/

void err_mess(char *from, int no)
{
    int	save_errno=errno;	/* Save errno around OSMESSAGE_PRINT */


    OSMESSAGE_PRINT({fprintf(stderr,
			     "System call error: %s errno=%d ",
			     from, no); perror("");});

    errno=save_errno;

  }
