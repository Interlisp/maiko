/* $Id: xdefs.h,v 1.4 2001/12/26 22:17:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-2001 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

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

#ifdef XWINDOW
#ifdef XV11R4
#undef XV11R1
#undef XV11R2
#undef XV11R3
#endif /* XV11R4 */

#ifdef XV11R3
#undef XV11R1
#undef XV11R2
#undef XV11R4
#endif /* XV11R3 */

#ifdef XV11R2
#undef XV11R1
#undef XV11R3
#undef XV11R4
#endif /* XV11R2 */

#ifdef XV11R1
#undef XV11R2
#undef XV11R3
#undef XV11R4
#endif /* XV11R1 */

#if ( !(defined( XV11R1 ))  \
   && !(defined( XV11R2 ))  \
   && !(defined( XV11R3 ))  \
   && !(defined( XV11R4 )) )
#define XV11R4			/* newest version */
#endif
#endif /* XWINDOW */
