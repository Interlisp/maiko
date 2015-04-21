/* $Id: common.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: common.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
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
#include  <setjmp.h>
#include  <fcntl.h>
#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lspglob.h"
#include "emlglob.h"
#include "stack.h"
#include "dbprint.h"

#ifndef NOPIXRECT
#include <pixrect/pixrect_hs.h>
#endif

stab()
  {
    DBPRINT(("Now in stab\n"));
  }




/***************************************************************
error
	common sub-routine.

	Printout error message.
	Enter URAID.
	And exit.(takeshi)

******************************************************************/
#define URMAXFXNUM		100

extern unsigned int LispReadFds,LispWindowFd,LispKbdFd;
extern struct screen LispScreen;
extern int displaywidth,displayheight;
extern DLword *DisplayRegion68k;
extern int FrameBufferFd;
 extern char URaid_inputstring[];
 extern char URaid_comm;
 extern char URaid_arg1[256];
 extern char URaid_arg2[10];
 extern int URaid_argnum;
 extern char *URaid_errmess;
 extern  int URaid_currentFX;
 extern FX *URaid_FXarray[];
 extern jmp_buf BT_jumpbuf;
 extern jmp_buf SD_jumpbuf;
  extern int BT_temp;	/* holds the continue-character the user typed */

 LispPTR Uraid_mess=NIL;

/* Currentry Don't care Ether re-initial */
/* Medley only */



/************************************************************************/
/*									*/
/*				e r r o r				*/
/*									*/
/*	Last-ditch error handling; enters URAID, low-level debug.	*/
/*									*/
/************************************************************************/

#define URMAXCOMM	512
error(cp)
  char	*cp;
  {
    char *ptr;
    if(device_before_raid()<0)
      {
	printf("Can't Enter URAID.\n");
	exit(-1);
      }
     /* comm read */
    URaid_errmess=cp;
    printf("\n*Error* %s\n",cp);
    fflush(stdin);
    printf("Enter the URaid\n");
    print(Uraid_mess); putchar('\n');
    URaid_currentFX=URMAXFXNUM +1;
    memset(URaid_FXarray, 0, URMAXFXNUM *4);
#ifndef DOS
      {
	int stat = fcntl(fileno(stdin), F_GETFL, 0);
	if (stat != O_RDONLY && stat != O_RDWR)
	  if (freopen("/dev/tty", "r", stdin) == NULL)
	    {
	      perror("Reopen of stdin failed.");
	      exit(0);
	    }
      }
#endif /* DOS */
uraidloop:
    if(setjmp(BT_jumpbuf) == 1) goto uraidloop;
    if(setjmp(SD_jumpbuf) == 1) goto uraidloop;
    for(;;)
      { /* URAID LOOP */

	uraid_commclear();
	BT_temp = 0;	/* So we get the "more" option on screen-full */
	printf("\n< ");
	for( ptr=URaid_inputstring;( *ptr= getchar() ) != '\n' ; ptr++) {}
	URaid_argnum = sscanf(URaid_inputstring,
			      "%1s%s%s",&URaid_comm, URaid_arg1, URaid_arg2);

	if(uraid_commands()==NIL) break;
      } /* for end */

    /**TopOfStack = NIL;if error is called from subr TOS will be set NIL**/
    if(device_after_raid() <0)
      {
	printf("Can't return to Lisp. Return to UNIX?");
	{ int c;
	  c= getchar();
	  if((c=='Y')||(c=='y')) exit(-1);
	}
	fflush(stdin);
	goto uraidloop;
      }
    return(0);
  }



/************************************************************************/
/*									*/
/*				w a r n					*/
/*									*/
/*	Print a warning message, but don't stop running.		*/
/*									*/
/************************************************************************/

warn(s)
  char *s;
  { printf("\nWARN: %s \n",s); }




/*****************************************************************
stackcheck

	common sub-routine.

	Not Implemented.

	1.check Stack overflow.
		(check CurrentStackPTR)
	2.if overflow, return T (not 0).
	  Otherwise, return F (0).
******************************************************************/
stackcheck()
  {
#ifdef	TRACE2
    printf("TRACE:stackcheck()\n");
#endif
    return(0);
  }




/*****************************************************************
stackoverflow

	common sub-routine.

	Not Implemented.

	1.error handling of stack overflow.
******************************************************************/

stackoverflow()
  {
#ifdef	TRACE2
    printf("TRACE:stackoverflow()\n");
#endif
    printf("stackoverflow \n");
  }

