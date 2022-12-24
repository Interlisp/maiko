#ifndef BITBLT_H
#define BITBLT_H 1
/* $Id: bitblt.h,v 1.2 1999/01/03 02:05:54 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
 *	Copyright (C) 1988 by Fuji Xerox Co., Ltd. All rights reserved.
 *
 *	File :  bitblt.h
 *
 *	Author :  Osamu Nakamura
 *
 */

/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for DLword */

#define	REPLACE	0
#define	PAINT	2
#define	ERASE	1
#define	INVERT	3
#define	ERROR	PIX_SRC

#define PixOperation( SRCTYPE, OPERATION )	\
        ( (SRCTYPE) == ERASE ?                                 \
		((OPERATION) == REPLACE ? PIX_NOT(PIX_SRC) : \
		((OPERATION) == PAINT   ? PIX_NOT(PIX_SRC) | PIX_DST : \
		((OPERATION) == ERASE   ? PIX_NOT(PIX_SRC) & PIX_DST : \
		((OPERATION) == INVERT  ? PIX_NOT(PIX_SRC) ^ PIX_DST : ERROR)))) : \
	/*  SRCTYPE == INPUT */ \
		((OPERATION) == REPLACE ? PIX_SRC : \
		((OPERATION) == PAINT   ? PIX_SRC | PIX_DST : \
		((OPERATION) == ERASE   ? PIX_SRC & PIX_DST : \
		((OPERATION) == INVERT  ? PIX_SRC ^ PIX_DST : ERROR)))))


extern DLword	*EmMouseX68K, *EmMouseY68K;
extern int DisplayRasterWidth;
#define	XDELTA	50
#define	YDELTA	50
#define	MOUSEXL	((int)*EmMouseX68K - XDELTA)
#define	MOUSEXR	((int)*EmMouseX68K + XDELTA)
#define	MOUSEYL	((int)*EmMouseY68K - YDELTA)
#define	MOUSEYH	((int)*EmMouseY68K + YDELTA)


#ifdef DOS
#define HideCursor { (currentdsp->mouse_invisible)(currentdsp, IOPage); }
#define ShowCursor { (currentdsp->mouse_visible)(IOPage->dlmousex, \
												  IOPage->dlmousey); }

#else
extern DLword *EmCursorX68K,*EmCursorY68K;
#define	HideCursor	{ taking_mouse_down();}
#define ShowCursor	{ taking_mouse_up(*EmCursorX68K,*EmCursorY68K);}
#endif

#define refresh_CG6 										\
	HideCursor;											\
	pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight,	\
			PIX_SRC,DisplayRegionPixrect, 0, 0);				\
	ShowCursor;

#define clear_CG6											\
	HideCursor;											\
	pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight,	\
			PIX_CLR, ColorDisplayPixrect, 0, 0);					\
	ShowCursor;


/* Macro for locking and unlocking screen to prevent multiple updates */

#ifdef DOS
#define LOCKSCREEN currentdsp->device.locked++
#define UNLOCKSCREEN currentdsp->device.locked--

#else

#define LOCKSCREEN ScreenLocked = T
#define UNLOCKSCREEN ScreenLocked = NIL

#endif /* DOS */
#endif /* BITBLT_H */
