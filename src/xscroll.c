/* $Id: xscroll.c,v 1.2 1999/01/03 02:07:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: xscroll.c,v 1.2 1999/01/03 02:07:48 sybalsky Exp $ Copyright (C) Venue";



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

int ScrollPitch = SCROLL_PITCH;


/* Move the DisplayWindow and the ScrollButtons to a new */
/* position. newX, newY refers to the uppre left corner */
/* of the LispDisplay */
void Scroll( dsp, newX, newY)
     DspInterface dsp;
     int newX, newY;
{
  /* Limit the newX and newY values. */
  dsp->Vissible.x = bound(0, newX, dsp->Display.width - dsp->Vissible.width);
  dsp->Vissible.y = bound(0, newY, dsp->Display.height - dsp->Vissible.height);

  newX = (int)((dsp->Vissible.x * dsp->Vissible.width) / dsp->Display.width);
  newY = (int)((dsp->Vissible.y * dsp->Vissible.height) / dsp->Display.height);

  XMoveWindow( dsp->display_id, dsp->HorScrollButton, newX, -dsp->InternalBorderWidth);
  XMoveWindow( dsp->display_id, dsp->VerScrollButton, -dsp->InternalBorderWidth, newY );

  (dsp->bitblt_to_screen)( dsp, 0, dsp->Vissible.x, dsp->Vissible.y,
			  dsp->Vissible.width, dsp->Vissible.height);
}/* end Scroll */

void JumpScrollVer( dsp, y )
     DspInterface dsp;
{
  Scroll( dsp, dsp->Vissible.x, (int)((dsp->Display.width *y) / dsp->Vissible.height));
}

void JumpScrollHor( dsp, x )
     DspInterface dsp;
{
  Scroll( dsp, (int)((dsp->Display.width *x) / dsp->Vissible.width), dsp->Vissible.y);
}

void ScrollLeft( dsp )
     DspInterface dsp;
{
  Scroll( dsp, dsp->Vissible.x - ScrollPitch, dsp->Vissible.y);
}

void ScrollRight( dsp )
     DspInterface dsp;
{
  Scroll( dsp, dsp->Vissible.x + ScrollPitch, dsp->Vissible.y);
}

void ScrollUp( dsp )
     DspInterface dsp;
{
  Scroll( dsp, dsp->Vissible.x, dsp->Vissible.y - ScrollPitch);
}

void ScrollDown( dsp )
     DspInterface dsp;
{
  Scroll( dsp, dsp->Vissible.x, dsp->Vissible.y + ScrollPitch);
}
