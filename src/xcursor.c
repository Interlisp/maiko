/* $Id: xcursor.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: xcursor.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995, 2000 Venue.				*/
/*	    All Rights Reserved.					*/
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
#if defined(MACOSX) || defined(FREEBSD)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xdefs.h"

#include "lispemul.h"
#include "iopage.h"
#include "display.h"
#include "dbprint.h"
#include "devif.h"

extern IOPAGE *IOPage;

extern XGCValues gcv;
extern int Lisp_Xinitialized, Bitmap_Pad, Default_Depth;

XImage CursorImage;
Pixmap CursorPixmap_source, CursorPixmap_mask;
GC cursor_source_gc, cursor_mask_gc;
XColor cursor_fore_xcsd, cursor_back_xcsd, xced;
extern Colormap Colors;

extern DspInterface currentdsp;
/* a simple linked list to remember X cursors */
struct MXCURSOR {
  struct MXCURSOR *next;
  DLword bitmap[CURSORHEIGHT];
  Cursor Xid;
} *cursorlist = NULL;

/*
Cursor LispCursor[2];
int    cursor_sw;
*/

/* Hotspot X and Y values for current cursor.  SUBTRACT these from */
/* mouse positions before reporting them upward to the sysout, and */
/* ADD these to the positions we get in SetMouseXY calls.  This    */
/* way, X and lisp agree where the mouse is                        */

int Current_Hot_X = 0, Current_Hot_Y = 0;

void set_Xcursor(DspInterface, unsigned char *, int, int, Cursor *, int);

/************************************************************************/
/*									*/
/*			I n i t _ X C u r s o r				*/
/*									*/
/*	Initial setup for X cursor handling--create an initial		*/
/*	cursor, and get it displayed.					*/
/*									*/
/************************************************************************/

void Init_XCursor() {
  int i;
  DLword *newbm = (DLword *)(IOPage->dlcursorbitmap);

  TPRINT(("TRACE: Init_DisplayCursor()\n"));
  /* this is guaranteed to be our first cursor, isn't it? */
  cursorlist = (struct MXCURSOR *)malloc(sizeof(struct MXCURSOR));
  cursorlist->next = NULL;
  for (i = 0; i < CURSORHEIGHT; i++) cursorlist->bitmap[i] = newbm[i];
  set_Xcursor(currentdsp, (unsigned char *)newbm, 0, 0, &(cursorlist->Xid), 1);
  DefineCursor(currentdsp->display_id, currentdsp->DisplayWindow, &(cursorlist->Xid));
} /* end Init_XCursor */

/************************************************************************/
/*									*/
/*			S e t _ X C u r s o r				*/
/*									*/
/*	Set the X cursor from the Lisp bitmap, with hot spot at x,y	*/
/*									*/
/*  WARNING:  If you call this function inside the C code to change	*/
/*            the cursor or flip a cursor bar, DON'T just use		*/
/*            Current_Hot_Y as the 2nd arg.  Instead use		*/
/*            "15-Current_Hot_Y", because this function SETS C_H_Y	*/
/*            to 15-its 2nd arg.  This has led to cursor-alignment	*/
/*            bugs in the past.						*/
/*									*/
/************************************************************************/

void Set_XCursor(x, y) int x, y;
{
  /* compare cursor in IOPage memory with cursors we've seen before */
  register struct MXCURSOR *clp, *clbp;
  register DLword *newbm = ((DLword *)(IOPage->dlcursorbitmap));
  register int i;

  XLOCK; /* No signals while setting the cursor */
  for (clp = cursorlist; clp != NULL; clbp = clp, clp = clp->next) {
    for (i = 0; i < CURSORHEIGHT; i++)
      if (clp->bitmap[i] != newbm[i]) break;
    if (i == CURSORHEIGHT) break;
  }

  if (clp == NULL) { /* it isn't there, push on a new one */
    clp = (struct MXCURSOR *)malloc(sizeof(struct MXCURSOR));
    /* and fill it up with the current new cursor */
    for (i = 0; i < CURSORHEIGHT; i++) clp->bitmap[i] = newbm[i];
#ifdef NEWXCURSOR
    /* JDS 000521 Added "15-" to fix cursor troubles at window edge */
    set_Xcursor(currentdsp, (unsigned char *)newbm, x, 15 - y, &(clp->Xid), 1);
#else
    set_Xcursor(currentdsp, (unsigned char *)newbm, 0, 0, &(clp->Xid), 1);
#endif /* NEWXCURSOR */
    clp->next = cursorlist;
    cursorlist = clp;
  } else
      /* found it, move it to the front of the list
         (this should reduce search time on the average by keeping
         the popular cursors near the front of the list)
         */
      if (clp != cursorlist) { /* don't move if it's already there */
    clbp->next = clp->next;
    clp->next = cursorlist;
    cursorlist = clp;
  }
  DefineCursor(currentdsp->display_id, currentdsp->DisplayWindow, &(clp->Xid));
  XUNLOCK; /* Signals OK now */

#ifdef NEWXCURSOR
  /* Save the hotspot for later position reporting/setting */

  Current_Hot_X = x;
  Current_Hot_Y = 15 - y; /* Added 15- to fix window-edge trouble */
#endif                    /* NEWXCURSOR */

} /* end Set_XCursor */

/************************************************************************/
/*									*/
/*			i n i t _ X c u r s o r				*/
/*									*/
/*	Initialization code for X-windows cursors.			*/
/*									*/
/************************************************************************/

void init_Xcursor(display, window) Display *display;
{
  TPRINT(("TRACE: init_Xcursor()\n"));

  XLOCK; /* Take no X signals during this activity (ISC 386) */

  CursorImage.width = CURSORWIDTH;
  CursorImage.height = CURSORHEIGHT;
  CursorImage.xoffset = 0;
  CursorImage.format = XYBitmap;
#if (defined(XV11R1) || defined(BYTESWAP))
  CursorImage.byte_order = LSBFirst;
#else  /* XV11R1 | BYTESWAP */
  CursorImage.byte_order = MSBFirst;
#endif /* XV11R1 | BYTESWAP */

  CursorImage.bitmap_unit = BITSPER_DLWORD;
#ifdef AIX
  CursorImage.bitmap_pad = 32;
#else
  CursorImage.bitmap_pad = Bitmap_Pad;
#endif /* AIX */
  CursorImage.depth = 1;
  CursorImage.bytes_per_line = BITSPER_DLWORD / 8;
  CursorImage.bitmap_bit_order = MSBFirst;

  CursorPixmap_source = XCreatePixmap(display, window, CURSORWIDTH, CURSORHEIGHT, 1);
  CursorPixmap_mask = XCreatePixmap(display, window, CURSORWIDTH, CURSORHEIGHT, 1);

  gcv.function = GXcopy;
  gcv.foreground = BlackPixelOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));
  gcv.background = WhitePixelOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));
#ifdef AIX
  gcv.plane_mask = 1;
#endif /* AIX */

  cursor_source_gc = XCreateGC(display, window,
                               GCForeground | GCBackground | GCFunction
#ifdef AIX
                                   | GCPlaneMask
#endif /* AIX */
                               ,
                               &gcv);
  cursor_mask_gc = XCreateGC(display, window,
                             GCForeground | GCBackground | GCFunction
#ifdef AIX
                                 | GCPlaneMask
#endif /* AIX */
                             ,
                             &gcv);

  XAllocNamedColor(display, Colors, "black", &cursor_fore_xcsd, &xced);
  XAllocNamedColor(display, Colors, "white", &cursor_back_xcsd, &xced);

  XUNLOCK; /* OK to take signals again */

} /* end init_Xcursor */

/************************************************************************/
/*									*/
/*			s e t _ X c u r s o r				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void set_Xcursor(dsp, bitmap, hotspot_x, hotspot_y, return_cursor, from_lisp) DspInterface dsp;
unsigned char *bitmap;
int hotspot_x, hotspot_y, from_lisp;
Cursor *return_cursor;
{
  extern unsigned char reversedbits[];
  unsigned char image[32];
  int i;
  Pixmap Cursor_src, Cursor_msk;
  Screen *screen;

#ifdef BYTESWAP
  if (from_lisp)
    for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i ^ 3]];
  else
    for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i]];
#else
  for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i]];
#endif /* BYTESWAP */

  XLOCK;
  screen = ScreenOfDisplay(dsp->display_id, DefaultScreen(dsp->display_id));
  Cursor_src = XCreatePixmapFromBitmapData(dsp->display_id, dsp->DisplayWindow, (char *)image,
					   16, 16, 1, 0, 1); /* Has to have a depth of 1! */
  Cursor_msk = XCreatePixmapFromBitmapData(dsp->display_id, dsp->DisplayWindow, (char *)image,
					   16, 16, 1, 0, 1); /* Has to have a depth of 1! */
  *return_cursor = XCreatePixmapCursor(dsp->display_id, Cursor_src, Cursor_msk, &cursor_fore_xcsd,
                                       &cursor_back_xcsd, hotspot_x, hotspot_y);

  /* Should free these now (doc says server may not copy them) */
  XFreePixmap(dsp->display_id, Cursor_src);
  XFreePixmap(dsp->display_id, Cursor_msk);

  XFlush(dsp->display_id);
  XUNLOCK;

} /* end set_Xcursor */
