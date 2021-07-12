/* $Id: xcursor.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995, 2000 Venue.				*/
/*	    All Rights Reserved.					*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xdefs.h"

#include "lispemul.h"
#include "iopage.h"
#include "display.h"
#include "dbprint.h"
#include "devif.h"

#include "xcursordefs.h"

extern IOPAGE *IOPage;

XColor cursor_fore_xcsd, cursor_back_xcsd, xced;
extern Colormap Colors;

extern DspInterface currentdsp;
/* a simple linked list to remember X cursors */
struct MXCURSOR {
  struct MXCURSOR *next;
  DLword bitmap[CURSORHEIGHT];
  Cursor Xid;
} *cursorlist = NULL;

/* Hotspot X and Y values for current cursor.  SUBTRACT these from */
/* mouse positions before reporting them upward to the sysout, and */
/* ADD these to the positions we get in SetMouseXY calls.  This    */
/* way, X and lisp agree where the mouse is                        */

int Current_Hot_X = 0, Current_Hot_Y = 0;


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
  set_Xcursor(currentdsp, (uint8_t *)newbm, 0, 0, &(cursorlist->Xid), 1);
  DefineCursor(currentdsp, currentdsp->DisplayWindow, &(cursorlist->Xid));
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

void Set_XCursor(int x, int y)
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
    set_Xcursor(currentdsp, (uint8_t *)newbm, x, 15 - y, &(clp->Xid), 1);
#else
    set_Xcursor(currentdsp, (uint8_t *)newbm, 0, 0, &(clp->Xid), 1);
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
  DefineCursor(currentdsp, currentdsp->DisplayWindow, &(clp->Xid));
  XUNLOCK(currentdsp); /* Signals OK now */

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

void init_Xcursor(DspInterface dsp)
{
  TPRINT(("TRACE: init_Xcursor()\n"));

  XLOCK; /* Take no X signals during this activity (ISC 386) */

  XAllocNamedColor(dsp->display_id, Colors, "black", &cursor_fore_xcsd, &xced);
  XAllocNamedColor(dsp->display_id, Colors, "white", &cursor_back_xcsd, &xced);

  XUNLOCK(dsp); /* OK to take signals again */

} /* end init_Xcursor */

/************************************************************************/
/*									*/
/*			s e t _ X c u r s o r				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void set_Xcursor(DspInterface dsp, const uint8_t *bitmap, int hotspot_x, int hotspot_y, Cursor *return_cursor, int from_lisp)
{
  extern const unsigned char reversedbits[];
  unsigned char image[32];
  int i;
  Pixmap Cursor_src, Cursor_msk;

#ifdef BYTESWAP
  if (from_lisp)
    for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i ^ 3]];
  else
    for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i]];
#else
  for (i = 0; i < 32; i++) image[i] = reversedbits[bitmap[i]];
#endif /* BYTESWAP */

  XLOCK;
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
  XUNLOCK(dsp);

} /* end set_Xcursor */
