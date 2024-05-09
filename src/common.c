/* $Id: common.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <fcntl.h>         // for fcntl, F_GETFL, O_RDONLY, O_RDWR
#include <setjmp.h>        // for setjmp, jmp_buf
#include <stdio.h>         // for fflush, fprintf, printf, getchar, stderr
#include <stdlib.h>        // for exit
#include <string.h>        // for memset
#include <sys/select.h>    // for fd_set
#include "commondefs.h"    // for error, stab, warn
#include "dbprint.h"       // for DBPRINT
#include "emlglob.h"
#include "kprintdefs.h"    // for print
#include "lispemul.h"      // for NIL, DLword, LispPTR
#include "lspglob.h"
#include "uraiddefs.h"     // for device_after_raid, device_before_raid, ura...
#include "uraidextdefs.h"  // for URMAXFXNUM, URaid_inputstring, URaid_FXarray

void stab(void) { DBPRINT(("Now in stab\n")); }

/***************************************************************
error
        common sub-routine.

        Printout error message.
        Enter URAID.
        And exit.(takeshi)

******************************************************************/

extern fd_set LispReadFds;
extern int LispKbdFd;
extern struct screen LispScreen;
extern int displaywidth, displayheight;
extern DLword *DisplayRegion68k;
extern int FrameBufferFd;
extern jmp_buf BT_jumpbuf;
extern jmp_buf SD_jumpbuf;
extern int BT_temp; /* holds the continue-character the user typed */

/* Currentry Don't care Ether re-initial */
/* Medley only */

/************************************************************************/
/*									*/
/*				e r r o r				*/
/*									*/
/*	Last-ditch error handling; enters URAID, low-level debug.	*/
/*									*/
/************************************************************************/

LispPTR Uraid_mess = NIL;

int error(const char *cp) {
  char *ptr;
  if (device_before_raid() < 0) {
    (void)fprintf(stderr, "Can't Enter URAID.\n");
    exit(-1);
  }
  /* comm read */
  URaid_errmess = cp;
  (void)fprintf(stderr, "\n*Error* %s\n", cp);
  fflush(stdin);
  (void)fprintf(stderr, "Enter the URaid\n");
  print(Uraid_mess);
  putchar('\n');
  /* XXX: make sure output is flushed so we can see where we are */
  fflush(stdout); fflush(stderr);
  URaid_currentFX = URMAXFXNUM + 1;
  memset(URaid_FXarray, 0, URMAXFXNUM * 4);
#ifndef DOS
  {
    int stat = fcntl(fileno(stdin), F_GETFL, 0);
    if (stat != O_RDONLY && stat != O_RDWR)
      if (freopen("/dev/tty", "r", stdin) == NULL) {
        perror("Reopen of stdin failed.");
        exit(0);
      }
  }
#endif /* DOS */
uraidloop:
  if (setjmp(BT_jumpbuf) == 1) goto uraidloop;
  if (setjmp(SD_jumpbuf) == 1) goto uraidloop;
  for (;;) { /* URAID LOOP */

    uraid_commclear();
    BT_temp = 0; /* So we get the "more" option on screen-full */
    printf("\n< ");
    for (ptr = URaid_inputstring; (*ptr = getchar()) != '\n'; ptr++) {}
    URaid_argnum = sscanf(URaid_inputstring, "%1s%s%s", URaid_comm, URaid_arg1, URaid_arg2);

    if (uraid_commands() == NIL) break;
    /* XXX: make sure output is flushed so we can see where we are */
    fflush(stdout); fflush(stderr);
  } /* for end */

  /**TopOfStack = NIL;if error is called from subr TOS will be set NIL**/
  if (device_after_raid() < 0) {
    printf("Can't return to Lisp. Return to UNIX?");
    {
      int c;
      c = getchar();
      if ((c == 'Y') || (c == 'y')) exit(-1);
    }
    fflush(stdin);
    goto uraidloop;
  }
  return (0);
}

/************************************************************************/
/*									*/
/*				w a r n					*/
/*									*/
/*	Print a warning message, but don't stop running.		*/
/*									*/
/************************************************************************/

void warn(const char *s)
{ printf("\nWARN: %s \n", s); }
