/* $Id: xbbt.c,v 1.2 1999/01/03 02:07:46 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: xbbt.c,v 1.2 1999/01/03 02:07:46 sybalsky Exp $ Copyright (C) Venue";



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



#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "lispemul.h"
#include "xdefs.h"
#include "devif.h"

extern DspInterface currentdsp;



/************************************************************************/
/*									*/
/*		    c l i p p i n g _ X b i t b l t			*/
/*									*/
/*	BITBLT from the display region to the X server's display,	*/
/*	clipping to fit the window we're in on the server.		*/
/*									*/
/*	dummy is the placeholder for the bitmap to be blitted		*/
/*									*/
/************************************************************************/
unsigned long
clipping_Xbitblt(dsp, dummy, x, y, w, h)
     DspInterface dsp;
     int dummy, x, y, w, h;
{ 
  int temp_x, temp_y, LowerRightX, LowerRightY;

  LowerRightX = dsp->Vissible.x + dsp->Vissible.width;
  LowerRightY = dsp->Vissible.y + dsp->Vissible.height;

  temp_x = x + w - 1; 
  temp_y = y + h - 1; 

  if ( (temp_x < dsp->Vissible.x)
      || (x > LowerRightX) 
      || (temp_y < dsp->Vissible.y) 
      || (y > LowerRightY)) return(0); 
	
  if ( ( x >= dsp->Vissible.x ) 
      && ( temp_x <= LowerRightX ) 
      && ( y >= dsp->Vissible.y ) 
      && ( temp_y <= LowerRightY ) )
    { 
      XLOCK;
      XPutImage( dsp->display_id,
		dsp->DisplayWindow,
		dsp->Copy_GC,
		&dsp->ScreenBitmap,
		x , y, x - dsp->Vissible.x,
		y - dsp->Vissible.y, w, h );
      XFlush(dsp->display_id);
      XUNLOCK;
      return(0); 
    }
 
  if ( x < dsp->Vissible.x )
    { 
      w -= dsp->Vissible.x - x; 
      x = dsp->Vissible.x; 
    }
 
  if ( temp_x > LowerRightX ) w -= temp_x - LowerRightX; 
 
  if ( y < dsp->Vissible.y )
    { 
      h -= dsp->Vissible.y - y; 
      y = dsp->Vissible.y; 
    }
 
  if ( temp_y > LowerRightY ) h -= temp_y - LowerRightY; 
 
  if ((w>0) && (h>0))
    { 
      XLOCK;
      XPutImage( dsp->display_id,
		dsp->DisplayWindow,
		dsp->Copy_GC,
		&dsp->ScreenBitmap,
		x , y, x - dsp->Vissible.x,
		y - dsp->Vissible.y, w, h );
      XFlush(dsp->display_id);
      XUNLOCK;
      return(0);
    }

}				/* end clipping_Xbitblt */
