/* $Id: xlspwin.c,v 1.4 2001/12/26 22:17:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 		*/
/*                	1994, 1995, 1999, 2000, 2001 Venue.		*/
/*	    All Rights Reserved.					*/
/*									*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <X11/X.h>        // for Cursor, CWOverrideRedirect, GCBackground
#include <X11/Xlib.h>     // for XCreateSimpleWindow, XMapWindow, XChangeWin...
#include <X11/Xutil.h>    // for XSizeHints, XStringListToTextProperty, XWMH...
#include <stdio.h>        // for NULL
#include "commondefs.h"   // for error
#include "dbprint.h"      // for TPRINT
#include "devif.h"        // for (anonymous), MRegion, OUTER_SB_WIDTH, Defin...
#include "keyboard.h"     // for RING, KBEVENT, KB_ALLUP, KEYEVENTSIZE, MAXK...
#include "lispemul.h"     // for DLword, ATOM_T, LispPTR, NIL, T
#include "version.h"
#include "xbitmaps.h"     // for LISP_CURSOR, default_cursor, horizscroll_cu...
#include "xcursordefs.h"  // for set_Xcursor, init_Xcursor
#include "xdefs.h"        // for XLOCK, XUNLOCK
#include "xlspwindefs.h"  // for Create_LispWindow, DoRing, lisp_Xvideocolor
#include "xmkicondefs.h"  // for make_Xicon

extern DLword *EmKbdAd068K, *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K,
    *EmRealUtilin68K;
extern DLword *CTopKeyevent;
extern LispPTR *KEYBUFFERING68k;
extern int URaid_req;

extern DLword *DisplayRegion68k;

extern int Current_Hot_X, Current_Hot_Y; /* X Cursor hotspots */

extern char Window_Title[255];
extern char Icon_Title[255];

extern int save_argc;
extern char **save_argv;

extern DspInterface currentdsp;

XGCValues gcv;
XEvent report;

Cursor WaitCursor, DefaultCursor, VertScrollCursor, VertThumbCursor, ScrollUpCursor,
    ScrollDownCursor, HorizScrollCursor, HorizThumbCursor, ScrollLeftCursor, ScrollRightCursor;

extern int LispWindowRequestedX, LispWindowRequestedY;
extern unsigned LispWindowRequestedWidth, LispWindowRequestedHeight;
extern int noscroll;

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void Create_LispWindow(DspInterface dsp)
{
  XSizeHints szhint = {0};
  XWMHints Lisp_WMhints = {0};
  XClassHint xclasshint = {0};
  XTextProperty IconText = {0}, WindowNameText = {0};
  XSetWindowAttributes Lisp_SetWinAttributes = {0};

  Screen *screen;

  int Col2, Row2, Col3, Row3, GravSize;
  char *WT, *IT;

  WT = Window_Title;
  IT = Icon_Title;

  GravSize = (int)(dsp->ScrollBarWidth / 2) - (dsp->InternalBorderWidth);
  Col2 = dsp->Visible.width;
  Row2 = dsp->Visible.height;
  Col3 = dsp->Visible.width + (int)(OUTER_SB_WIDTH(dsp) / 2);
  Row3 = dsp->Visible.height + (int)(OUTER_SB_WIDTH(dsp) / 2);

  screen = ScreenOfDisplay(dsp->display_id, DefaultScreen(dsp->display_id));
  dsp->LispWindow = XCreateSimpleWindow(
      dsp->display_id, RootWindowOfScreen(screen), LispWindowRequestedX, /* Default upper left */
      LispWindowRequestedY,                                              /* Default upper left */
      dsp->Visible.width + OUTER_SB_WIDTH(dsp),                         /* Default width */
      dsp->Visible.height + OUTER_SB_WIDTH(dsp),                        /* Default height */
      0,                                                                 /* Default border */
      BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));

  Lisp_SetWinAttributes.bit_gravity = dsp->BitGravity;
  Lisp_SetWinAttributes.override_redirect = False;
  Lisp_SetWinAttributes.backing_store = DoesBackingStore(screen);

  XChangeWindowAttributes(dsp->display_id, dsp->LispWindow, CWBitGravity | CWOverrideRedirect,
                          &Lisp_SetWinAttributes);

  dsp->DisableEventMask = NoEventMask;
  dsp->EnableEventMask = ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask |
                         StructureNotifyMask | PointerMotionMask | ExposureMask | KeyPressMask |
                         KeyReleaseMask;

  /* Have to make the GC before we make the icon. */
  gcv.function = GXcopy;
  gcv.foreground =
      BlackPixelOfScreen(ScreenOfDisplay(dsp->display_id, DefaultScreen(dsp->display_id)));
  gcv.background =
      WhitePixelOfScreen(ScreenOfDisplay(dsp->display_id, DefaultScreen(dsp->display_id)));
  dsp->Copy_GC =
      XCreateGC(dsp->display_id, dsp->LispWindow, GCForeground | GCBackground | GCFunction, &gcv);

  szhint.max_width = dsp->Display.width + OUTER_SB_WIDTH(dsp);
  szhint.max_height = dsp->Display.height + OUTER_SB_WIDTH(dsp);
  szhint.min_width = OUTER_SB_WIDTH(dsp);
  szhint.min_height = OUTER_SB_WIDTH(dsp);
  szhint.win_gravity = dsp->BitGravity;
  szhint.flags = PMaxSize | PWinGravity | PSize;

  Lisp_WMhints.icon_pixmap = make_Xicon(dsp);
  Lisp_WMhints.input = True;
  Lisp_WMhints.flags = IconPixmapHint | InputHint;

  xclasshint.res_name = *save_argv;
  xclasshint.res_class = *save_argv;

  XStringListToTextProperty(&WT, 1, &WindowNameText);
  XStringListToTextProperty(&IT, 1, &IconText);

  XSetWMProperties(dsp->display_id, dsp->LispWindow, &WindowNameText, &IconText, save_argv,
                   save_argc, &szhint, &Lisp_WMhints, &xclasshint);

  XSelectInput(dsp->display_id, dsp->LispWindow, dsp->EnableEventMask);
  init_Xcursor(dsp);

  dsp->DisplayWindow = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, 0, 0,
                                           dsp->Visible.width, dsp->Visible.height, 0,
                                           BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));
  XChangeWindowAttributes(dsp->display_id, dsp->DisplayWindow,
                          CWBitGravity | CWOverrideRedirect | CWBackingStore,
                          &Lisp_SetWinAttributes);
  XSelectInput(dsp->display_id, dsp->DisplayWindow, dsp->EnableEventMask);
  XMapWindow(dsp->display_id, dsp->DisplayWindow);

  /*********************************************************************/
  /* Create all the vanilla pixmaps and cursors for the display window */
  /*********************************************************************/

  dsp->ScrollBarPixmap = XCreatePixmapFromBitmapData(
      dsp->display_id, dsp->LispWindow, (char *)check_bits, check_width, check_height,
      BlackPixelOfScreen(screen), WhitePixelOfScreen(screen), DefaultDepthOfScreen(screen));
  dsp->GravityOnPixmap = XCreatePixmapFromBitmapData(
      dsp->display_id, dsp->LispWindow, (char *)check_bits, 16, 16, BlackPixelOfScreen(screen),
      WhitePixelOfScreen(screen), DefaultDepthOfScreen(screen));

  dsp->GravityOffPixmap = XCreatePixmapFromBitmapData(
      dsp->display_id, dsp->LispWindow, (char *)plain_bits, 16, 16, BlackPixelOfScreen(screen),
      WhitePixelOfScreen(screen), DefaultDepthOfScreen(screen));

  set_Xcursor(dsp, default_cursor.cuimage, (int)default_cursor.cuhotspotx,
              (int)(15 - default_cursor.cuhotspoty), &DefaultCursor, 0);
  set_Xcursor(dsp, wait_cursor.cuimage, (int)wait_cursor.cuhotspotx,
              (int)(15 - wait_cursor.cuhotspoty), &WaitCursor, 0);
  set_Xcursor(dsp, scrolldown_cursor.cuimage, (int)scrolldown_cursor.cuhotspotx,
              (int)(15 - scrolldown_cursor.cuhotspoty), &ScrollDownCursor, 0);
  set_Xcursor(dsp, scrollleft_cursor.cuimage, (int)scrollleft_cursor.cuhotspotx,
              (int)(15 - scrollleft_cursor.cuhotspoty), &ScrollLeftCursor, 0);
  set_Xcursor(dsp, vertscroll_cursor.cuimage, (int)vertscroll_cursor.cuhotspotx,
              (int)(15 - vertscroll_cursor.cuhotspoty), &VertScrollCursor, 0);
  set_Xcursor(dsp, vertthumb_cursor.cuimage, (int)vertthumb_cursor.cuhotspotx,
              (int)(15 - vertthumb_cursor.cuhotspoty), &VertThumbCursor, 0);
  set_Xcursor(dsp, horizscroll_cursor.cuimage, (int)horizscroll_cursor.cuhotspotx,
              (int)(15 - horizscroll_cursor.cuhotspoty), &HorizScrollCursor, 0);
  set_Xcursor(dsp, horizthumb_cursor.cuimage, (int)horizthumb_cursor.cuhotspotx,
              (int)(15 - horizthumb_cursor.cuhotspoty), &HorizThumbCursor, 0);
  set_Xcursor(dsp, scrollright_cursor.cuimage, (int)scrollright_cursor.cuhotspotx,
              (int)(15 - scrollright_cursor.cuhotspoty), &ScrollRightCursor, 0);
  set_Xcursor(dsp, scrollup_cursor.cuimage, (int)scrollup_cursor.cuhotspotx,
              (int)(15 - scrollup_cursor.cuhotspoty), &ScrollUpCursor, 0);

  if (noscroll == 0) {
    /********************************/
    /* Make all the toolkit windows */
    /********************************/
    dsp->VerScrollBar = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, Col2,
                                            0 - dsp->InternalBorderWidth, /* y */
                                            dsp->ScrollBarWidth,          /* width */
                                            dsp->Visible.height, dsp->InternalBorderWidth,
                                            BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));
    DefineCursor(dsp, dsp->VerScrollBar, &VertScrollCursor);
    XMapWindow(dsp->display_id, dsp->VerScrollBar);

    dsp->HorScrollBar = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow,
                                            0 - dsp->InternalBorderWidth, Row2, /* y */
                                            dsp->Visible.width,                /* width */
                                            dsp->ScrollBarWidth, dsp->InternalBorderWidth,
                                            BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));
    DefineCursor(dsp, dsp->HorScrollBar, &HorizScrollCursor);
    XChangeWindowAttributes(dsp->display_id, dsp->HorScrollBar, CWOverrideRedirect,
                            &Lisp_SetWinAttributes);
    XMapWindow(dsp->display_id, dsp->HorScrollBar);

    dsp->VerScrollButton = XCreateSimpleWindow(
                                               dsp->display_id, dsp->VerScrollBar, 0 - dsp->InternalBorderWidth,      /* x */
                                               (int)((dsp->Visible.y * dsp->Visible.height) / dsp->Display.height), /* y */
                                               dsp->ScrollBarWidth,                                                   /* width */
                                               (int)((dsp->Visible.height * dsp->Visible.height) / dsp->Display.height) + 1,
                                               dsp->InternalBorderWidth, BlackPixelOfScreen(screen), WhitePixelOfScreen(screen));
    XChangeWindowAttributes(dsp->display_id, dsp->VerScrollButton, CWOverrideRedirect,
                            &Lisp_SetWinAttributes);
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->VerScrollButton, dsp->ScrollBarPixmap);
    XClearWindow(dsp->display_id, dsp->VerScrollButton);
    XMapWindow(dsp->display_id, dsp->VerScrollButton);

    dsp->HorScrollButton = XCreateSimpleWindow(
                                               dsp->display_id, dsp->HorScrollBar,
                                               (int)((dsp->Visible.x * dsp->Visible.width) / dsp->Display.width),
                                               0 - dsp->InternalBorderWidth, /* y */
                                               (int)((dsp->Visible.width * dsp->Visible.width) / dsp->Display.width) + 1,
                                               dsp->ScrollBarWidth, dsp->InternalBorderWidth, BlackPixelOfScreen(screen),
                                               WhitePixelOfScreen(screen));
    XChangeWindowAttributes(dsp->display_id, dsp->HorScrollButton, CWOverrideRedirect,
                            &Lisp_SetWinAttributes);
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->HorScrollButton, dsp->ScrollBarPixmap);
    XClearWindow(dsp->display_id, dsp->HorScrollButton);
    XMapWindow(dsp->display_id, dsp->HorScrollButton);

    dsp->NWGrav = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, Col2, Row2, GravSize,
                                      GravSize, dsp->InternalBorderWidth, BlackPixelOfScreen(screen),
                                      WhitePixelOfScreen(screen));
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->NWGrav, dsp->GravityOnPixmap);
    DefineCursor(dsp, dsp->NWGrav, &DefaultCursor);
    XChangeWindowAttributes(dsp->display_id, dsp->NWGrav, CWOverrideRedirect, &Lisp_SetWinAttributes);
    XClearWindow(dsp->display_id, dsp->NWGrav);
    XMapWindow(dsp->display_id, dsp->NWGrav);

    dsp->SEGrav = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, Col3, Row3, GravSize,
                                      GravSize, dsp->InternalBorderWidth, BlackPixelOfScreen(screen),
                                      WhitePixelOfScreen(screen));
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->SEGrav, dsp->GravityOffPixmap);
    DefineCursor(dsp, dsp->SEGrav, &DefaultCursor);
    XChangeWindowAttributes(dsp->display_id, dsp->SEGrav, CWOverrideRedirect, &Lisp_SetWinAttributes);
    XClearWindow(dsp->display_id, dsp->NWGrav);
    XMapWindow(dsp->display_id, dsp->SEGrav);

    dsp->SWGrav = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, Col2, Row3, GravSize,
                                      GravSize, dsp->InternalBorderWidth, BlackPixelOfScreen(screen),
                                      WhitePixelOfScreen(screen));
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->SWGrav, dsp->GravityOffPixmap);
    DefineCursor(dsp, dsp->SWGrav, &DefaultCursor);
    XClearWindow(dsp->display_id, dsp->NWGrav);
    XMapWindow(dsp->display_id, dsp->SWGrav);

    dsp->NEGrav = XCreateSimpleWindow(dsp->display_id, dsp->LispWindow, Col3, Row2, GravSize,
                                      GravSize, dsp->InternalBorderWidth, BlackPixelOfScreen(screen),
                                      WhitePixelOfScreen(screen));
    XSetWindowBackgroundPixmap(dsp->display_id, dsp->NEGrav, dsp->GravityOffPixmap);
    DefineCursor(dsp, dsp->NEGrav, &DefaultCursor);
    XClearWindow(dsp->display_id, dsp->NWGrav);
    XMapWindow(dsp->display_id, dsp->NEGrav);
  }
  /* DefineCursor( dsp, dsp->DisplayWindow, &WaitCursor ); */

  XLOCK;
  XMapWindow(dsp->display_id, dsp->LispWindow);
  XFlush(dsp->display_id);
  XUNLOCK(dsp);
}

void lisp_Xvideocolor(int flag)
{
  Screen *screen;
  XEvent event;
  unsigned long newForeground;

  XLOCK;
  screen = ScreenOfDisplay(currentdsp->display_id, DefaultScreen(currentdsp->display_id));
  newForeground = flag ? WhitePixelOfScreen(screen) : BlackPixelOfScreen(screen);

  /* window -- are we making a change? */
  XGetGCValues(currentdsp->display_id, currentdsp->Copy_GC, GCForeground | GCBackground, &gcv);
  if (newForeground != gcv.foreground) {
      /* swap foreground and background in the graphics context*/
      gcv.background = gcv.foreground;
      gcv.foreground = newForeground;
      XChangeGC(currentdsp->display_id, currentdsp->Copy_GC, GCForeground | GCBackground, &gcv);
      /* notify the display code to refresh the visible screen with new fg/bg colors */
      event.type = Expose;
      event.xexpose.window = currentdsp->DisplayWindow;
      event.xexpose.x = 0;
      event.xexpose.y = 0;
      event.xexpose.width = currentdsp->Visible.width;
      event.xexpose.height = currentdsp->Visible.height;
      XSendEvent(currentdsp->display_id, currentdsp->DisplayWindow, True, 0, &event);
  }

  XFlush(currentdsp->display_id);
  XUNLOCK(currentdsp);

} /* end lisp_Xvideocolor */

void set_Xmouseposition(int x, int y)
{
  int dest_x, dest_y;

  TPRINT(("set_Xmouseposition(%d,%d)\n", x, y));

  dest_x = (x & 0xFFFF) + Current_Hot_X - currentdsp->Visible.x;
  dest_y = (y & 0xFFFF) + Current_Hot_Y - currentdsp->Visible.y;

  if ((dest_x >= 0) && (dest_x <= currentdsp->Visible.width) && (dest_y >= 0) &&
      (dest_y <= currentdsp->Visible.height)) {
    XLOCK;
    XWarpPointer(currentdsp->display_id, (Window)NULL, currentdsp->DisplayWindow, 0, 0, 0, 0,
                 dest_x, dest_y);
    XFlush(currentdsp->display_id);
    XUNLOCK(currentdsp);
  }
} /* end set_Xmouseposition */

/************************************************************************/
/*									*/
/*				D o R i n g				*/
/*									*/
/*	Take keyboard events and turn them into Lisp event info		*/
/*	(when running under X)						*/
/*									*/
/************************************************************************/

void DoRing() {
  DLword w, r;
  KBEVENT *kbevent;

  TPRINT(("TRACE: DoRing()\n"));
do_ring:
  /* DEL is not generally present on a Mac X keyboard, Ctrl-shift-ESC would be 18496 */
  if (((*EmKbdAd268K) & 2113) == 0) { /*Ctrl-shift-NEXT*/
    error("******  EMERGENCY Interrupt ******");
    *EmKbdAd268K = KB_ALLUP;          /*reset*/
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  } else if (((*EmKbdAd268K) & 2114) == 0) { /* Ctrl-Shift-DEL */
    *EmKbdAd268K = KB_ALLUP;                 /*reset*/
    URaid_req = T;
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  }

#ifdef OS4_TYPE4BUG
  else if (((*EmKbdAd268K) & 2120) == 0) { /* Ctrl-Shift-Return */
    *EmKbdAd268K = KB_ALLUP;               /*reset*/
    URaid_req = T;
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  }
#endif

  r = RING_READ(CTopKeyevent);
  w = RING_WRITE(CTopKeyevent);

  if (r == w) /* event queue FULL */
    goto KBnext;

  kbevent = (KBEVENT *)(CTopKeyevent + w);
  /*    RCLK(kbevent->time); */
  kbevent->W0 = *EmKbdAd068K;
  kbevent->W1 = *EmKbdAd168K;
  kbevent->W2 = *EmKbdAd268K;
  kbevent->W3 = *EmKbdAd368K;
  kbevent->W4 = *EmKbdAd468K;
  kbevent->W5 = *EmKbdAd568K;
  kbevent->WU = *EmRealUtilin68K;

  if (r == 0) /* Queue was empty */
    ((RING *)CTopKeyevent)->read = w;
  if (w >= MAXKEYEVENT)
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  else
    ((RING *)CTopKeyevent)->write = w + KEYEVENTSIZE;

KBnext:
  if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;
}
