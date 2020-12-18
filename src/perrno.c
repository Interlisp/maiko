/* $Id: perrno.c,v 1.4 2001/12/26 22:17:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lispemul.h"
#include "osmsg.h"

#include "perrnodefs.h"
#include "osmsgdefs.h"

/************************************************************************/
/*									*/
/*			p e r r o r n					*/
/*									*/
/*	Print the error message to go with a given error number.	*/
/*									*/
/************************************************************************/

void perrorn(char *s, int n) {
  if (s != NULL && *s != '\0') { fprintf(stderr, "%s: ", s); }
  fprintf(stderr, "%s\n", strerror(n));
}

/************************************************************************/
/*									*/
/*				e r r _ m e s s				*/
/*									*/
/*	Print an error message and call 'perror' to get the		*/
/*	canonical error explanation.  Called by emulator I/O code.	*/
/*									*/
/************************************************************************/

void err_mess(char *from, int no) {
  int save_errno = errno; /* Save errno around OSMESSAGE_PRINT */

  OSMESSAGE_PRINT({
    fprintf(stderr, "System call error: %s errno=%d ", from, no);
    perror("");
  });

  errno = save_errno;
}
