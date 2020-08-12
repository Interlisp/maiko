/* $Id: xscroll.c,v 1.2 1999/01/03 02:07:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: xscroll.c,v 1.2 1999/01/03 02:07:48 sybalsky Exp $ Copyright (C) Venue";

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

int ScrollPitch = SCROLL_PITCH;

/* Move the DisplayWindow and the ScrollButtons to a new */
/* position. newX, newY refers to the upper left corner */
/* of the LispDisplay */
void Scroll(DspInterface dsp, int newX, int newY)
{
  /* Limit the newX and newY values. */
  dsp->Visible.x = bound(0, newX, dsp->Display.width - dsp->Visible.width);
  dsp->Visible.y = bound(0, newY, dsp->Display.height - dsp->Visible.height);

  newX = (int)((dsp->Visible.x * dsp->Visible.width) / dsp->Display.width);
  newY = (int)((dsp->Visible.y * dsp->Visible.height) / dsp->Display.height);

  XMoveWindow(dsp->display_id, dsp->HorScrollButton, newX, -dsp->InternalBorderWidth);
  XMoveWindow(dsp->display_id, dsp->VerScrollButton, -dsp->InternalBorderWidth, newY);

  (dsp->bitblt_to_screen)(dsp, 0, dsp->Visible.x, dsp->Visible.y, dsp->Visible.width,
                          dsp->Visible.height);
} /* end Scroll */

void JumpScrollVer(DspInterface dsp, int y)
{ Scroll(dsp, dsp->Visible.x, (int)((dsp->Display.width * y) / dsp->Visible.height)); }

void JumpScrollHor(DspInterface dsp, int x)
{ Scroll(dsp, (int)((dsp->Display.width * x) / dsp->Visible.width), dsp->Visible.y); }

void ScrollLeft(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x - ScrollPitch, dsp->Visible.y); }

void ScrollRight(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x + ScrollPitch, dsp->Visible.y); }

void ScrollUp(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x, dsp->Visible.y - ScrollPitch); }

void ScrollDown(DspInterface dsp)
{ Scroll(dsp, dsp->Visible.x, dsp->Visible.y + ScrollPitch); }
