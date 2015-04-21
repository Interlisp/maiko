/* $Id: Xcursdef.h,v 1.2 1999/01/03 02:05:49 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
*
*
* Copyright (C) 1988 by Fuji Xerox co.,Ltd. All rights reserved.
*
*		Author: Mitsunori Matsuda
*		Date  : August 29, 1988
*/


/************************************************************************/
/*									*/
/*	Structure used to describe cursors in the X version of Medley.	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


typedef struct {
	short cubitsprepixel;	/* bits per pixel in the cursor, mostly 1 */
	char *cuimage;		/* the image bitmap */
	char *cumask;		/* the mask bitmap */
	short cuhotspotx;	/* hot-spot X coordinate */
	short cuhotspoty;	/* hot-spot Y coordinate */
	short cudata;		/* ?? */
} LISP_CURSOR;
