#ifndef LDEXDEFS_H
#define LDEXDEFS_H 1

/* $Id: ldeXdefs.h,v 1.2 1999/01/03 02:06:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* * * * * X defs for all files in Medley * * * * */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#ifdef LOCK_X_UPDATES
#define XLOCK do { XLocked++; } while (0)
#define XUNLOCK(dsp)				\
  do { 						\
    if (XLocked == 1 && XNeedSignal)		\
      {						\
	XNeedSignal = 0;			\
	getXsignaldata(dsp);			\
      };					\
    XLocked--;					\
  } while (0)
#else
#define XLOCK
#define XUNLOCK
#endif	/* LOCK_X_UPDATES */

#include <signal.h>
extern volatile sig_atomic_t XLocked;
extern volatile sig_atomic_t XNeedSignal;

#endif

