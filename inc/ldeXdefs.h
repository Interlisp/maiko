/* $Id: ldeXdefs.h,v 1.2 1999/01/03 02:06:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* * * * * X defs for all files in Medley * * * * */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/


#ifndef __LDEXDEF__

#define __LDEXDEF__ 1
#include <signal.h>
#ifdef LOCK_X_UPDATES
#define XLOCK { XLocked++; /* printf("L"); fflush(stdout);*/}
#define XUNLOCK					\
  { XLocked--;/* printf("U"); fflush(stdout);*/	\
    if (XNeedSignal)				\
      {						\
	XNeedSignal = 0;			\
	kill(getpid(), SIGPOLL);		\
      };						\
  }
#else
#define XLOCK
#define XUNLOCK
#endif	/* LOCK_X_UPDATES */

extern int XLocked;
extern int XNeedSignal;
/* this is !0 if we're locked; it should be 0 or larger always */
#endif

