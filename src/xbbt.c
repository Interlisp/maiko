/* $Id: xbbt.c,v 1.2 1999/01/03 02:07:46 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "lispemul.h"
#include "xdefs.h"
#include "devif.h"
#include "xbbtdefs.h"

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
unsigned long clipping_Xbitblt(DspInterface dsp, int dummy, int x, int y, int w, int h)
{
  int temp_x, temp_y, LowerRightX, LowerRightY;

  LowerRightX = dsp->Visible.x + dsp->Visible.width;
  LowerRightY = dsp->Visible.y + dsp->Visible.height;

  temp_x = x + w - 1;
  temp_y = y + h - 1;

  if ((temp_x < dsp->Visible.x) || (x > LowerRightX) || (temp_y < dsp->Visible.y) ||
      (y > LowerRightY))
    return (0);

  if ((x >= dsp->Visible.x) && (temp_x <= LowerRightX) && (y >= dsp->Visible.y) &&
      (temp_y <= LowerRightY)) {
    XLOCK;
    XPutImage(dsp->display_id, dsp->DisplayWindow, dsp->Copy_GC, &dsp->ScreenBitmap, x, y,
              x - dsp->Visible.x, y - dsp->Visible.y, w, h);
    XFlush(dsp->display_id);
    XUNLOCK;
    return (0);
  }

  if (x < dsp->Visible.x) {
    w -= dsp->Visible.x - x;
    x = dsp->Visible.x;
  }

  if (temp_x > LowerRightX) w -= temp_x - LowerRightX;

  if (y < dsp->Visible.y) {
    h -= dsp->Visible.y - y;
    y = dsp->Visible.y;
  }

  if (temp_y > LowerRightY) h -= temp_y - LowerRightY;

  if ((w > 0) && (h > 0)) {
    XLOCK;
    XPutImage(dsp->display_id, dsp->DisplayWindow, dsp->Copy_GC, &dsp->ScreenBitmap, x, y,
              x - dsp->Visible.x, y - dsp->Visible.y, w, h);
    XFlush(dsp->display_id);
    XUNLOCK;
  }
  return (0);

} /* end clipping_Xbitblt */
