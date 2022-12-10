/* $Id: xscroll.c,v 1.2 1999/01/03 02:07:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <X11/Xlib.h>     // for XMoveWindow
#include "devif.h"        // for (anonymous), MRegion, DspInterface
#include "xdefs.h"        // for SCROLL_PITCH
#include "xscrolldefs.h"  // for JumpScrollHor, JumpScrollVer, Scroll, Scrol...

int ScrollPitch = SCROLL_PITCH;

/* sbound: return (signed) value if it is between lower and upper otherwise lower or upper */
static inline int sbound(int lower, int value, int upper)
{
  if (value <= lower)
    return (lower);
  else if (value >= upper)
    return (upper);
  else
    return (value);
}

/* Move the DisplayWindow and the ScrollButtons to a new */
/* position. newX, newY refers to the upper left corner */
/* of the LispDisplay */
void Scroll(DspInterface dsp, int newX, int newY)
{
  /* Limit the newX and newY values. */
  dsp->Visible.x = sbound(0, newX, dsp->Display.width - dsp->Visible.width);
  dsp->Visible.y = sbound(0, newY, dsp->Display.height - dsp->Visible.height);

  newX = (dsp->Visible.x * (int)dsp->Visible.width) / (int)dsp->Display.width;
  newY = (dsp->Visible.y * (int)dsp->Visible.height) / (int)dsp->Display.height;

  XMoveWindow(dsp->display_id, dsp->HorScrollButton, newX, -(int)dsp->InternalBorderWidth);
  XMoveWindow(dsp->display_id, dsp->VerScrollButton, -(int)dsp->InternalBorderWidth, newY);

  (dsp->bitblt_to_screen)(dsp, 0, dsp->Visible.x, dsp->Visible.y, dsp->Visible.width,
                          dsp->Visible.height);
} /* end Scroll */

void JumpScrollVer(DspInterface dsp, int y)
{ Scroll(dsp, dsp->Visible.x, ((int)dsp->Display.width * y) / (int)dsp->Visible.height); }

void JumpScrollHor(DspInterface dsp, int x)
{ Scroll(dsp, (((int)dsp->Display.width * x) / (int)dsp->Visible.width), dsp->Visible.y); }

void ScrollLeft(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x - ScrollPitch, dsp->Visible.y); }

void ScrollRight(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x + ScrollPitch, dsp->Visible.y); }

void ScrollUp(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x, dsp->Visible.y - ScrollPitch); }

void ScrollDown(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x, dsp->Visible.y + ScrollPitch); }
