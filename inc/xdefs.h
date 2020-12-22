/* $Id: xdefs.h,v 1.4 2001/12/26 22:17:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-2001 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#ifndef XDEFS_H
#define XDEFS_H 1

#define DEF_WIN_X       20
#define DEF_WIN_Y       20
#define DEF_WIN_WIDTH  565
#define DEF_WIN_HEIGHT 430
#define WIN_MIN_WIDTH  150
#define WIN_MIN_HEIGHT 100
#define DEF_BDRWIDE      2
#define SCROLL_WIDTH	18
#define WINDOW_NAME    "Medley (C) Copyright 1980-2001 Venue"
#define ICON_NAME      "Medley"


#define WIN_MAX_WIDTH  2048
#define WIN_MAX_HEIGHT 2048
#define SCROLL_PITCH      30


#ifdef LOCK_X_UPDATES

#include <unistd.h>
#include <signal.h>
extern int XLocked;
extern int XNeedSignal;
/* this is !0 if we're locked; it should be 0 or larger always */

#define XLOCK do { XLocked++; /* printf("L"); fflush(stdout);*/} while (0)
#define XUNLOCK					\
  do { XLocked--;/* printf("U"); fflush(stdout);*/	\
    if (XNeedSignal)				\
      {						\
	XNeedSignal = 0;			\
	kill(getpid(), SIGPOLL);		\
      };					\
  } while (0)
#else
#define XLOCK
#define XUNLOCK
#endif	/* LOCK_X_UPDATES */

#endif /* XDEFS_H */
