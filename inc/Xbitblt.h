#ifndef XBITBLT_H
#define XBITBLT_H 1
/* $Id: Xbitblt.h,v 1.2 1999/01/03 02:05:49 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
*
*
* Copyright (C) 1988 by Fuji Xerox co.,Ltd. All rights reserved.
*
*		Author: Mitsunori Matsuda
*		Date  : August 11, 1988
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


#define Xbitblt( display, x,y,w,h) { \
		XPutImage( display, Display_Window.win \
			, *(Display_Window.gc) \
			, &XScreenBitmap \
			, x \
			, y \
			, x - ScreenRegion.left_x \
			, y - ScreenRegion.top_y \
			, w \
			, h ); \
	/*	XFlush( display ); */	 \
} 

#define clipping_Xbitblt(x,y,w,h) { \
		int temp_x \
		  , temp_y; \
		temp_x = x + w - 1; \
		temp_y = y + h - 1; \
		if( ( temp_x < ScreenRegion.left_x ) \
		 || ( x > ScreenRegion.right_x ) \
		 || ( temp_y < ScreenRegion.top_y ) \
		 || ( y > ScreenRegion.bottom_y ) ) { \
			return; \
		} \
		if( ( x >= ScreenRegion.left_x ) \
		 && ( temp_x <= ScreenRegion.right_x ) \
		 && ( y >= ScreenRegion.top_y ) \
		 && ( temp_y <= ScreenRegion.bottom_y ) ) { \
			Xbitblt( x, y, w, h ); \
			return; \
		} \
		if( x < ScreenRegion.left_x ) { \
			w -= ScreenRegion.left_x - x; \
			x = ScreenRegion.left_x; \
		} \
		if( temp_x > ScreenRegion.right_x ) { \
			w -= temp_x - ScreenRegion.right_x; \
		} \
		if( y < ScreenRegion.top_y ) { \
			h -= ScreenRegion.top_y - y; \
			y = ScreenRegion.top_y; \
		} \
		if( temp_y > ScreenRegion.bottom_y ) { \
			h -= temp_y - ScreenRegion.bottom_y; \
		} \
		if( ( w>0 ) && ( h>0 ) ) { \
			Xbitblt( x, y, w, h ); \
		} \
}
#endif /* XBITBLT_H */
