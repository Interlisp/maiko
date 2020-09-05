/* $Id: dspsubrs.c,v 1.3 2001/12/26 22:17:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: dspsubrs.c,v 1.3 2001/12/26 22:17:02 sybalsky Exp $ Copyright (C) Venue";
/*** ADOPTED NEW VERSION ***/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-2000 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#ifndef NOPIXRECT
#include <sunwindow/window_hs.h>
#include <pixrect/memvar.h>
#endif /* NOPIXRECT */

#include "lispemul.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "display.h"
#include "arith.h"

#include "dspsubrsdefs.h"
#include "commondefs.h"
#ifdef XWINDOW
#include "xcursordefs.h"
#include "xlspwindefs.h"
#endif

extern int DebugDSP;
extern int displaywidth, displayheight;

#ifdef XWINDOW
extern int Mouse_Included;
#endif /* XWINDOW */

/****************************************************
 *
 *	DSP_dspbout() entry of SUBRCALL 9 1
 *			called from (DSPBOUT X)
 *
 ****************************************************/

void DSP_dspbout(LispPTR *args) /* args[0] :	charcode	*/
{ putc((args[0] & 0xFFFF) & 0x7f, BCPLDISPLAY); }

/****************************************************
 *
 *	DSP_showdisplay() entry of SUBRCALL 19 2
 *			called from (SHOWDISPLAY BASE RASTERWIDTH)
 *
 ****************************************************/

extern int DisplayInitialized;

void DSP_showdisplay(LispPTR *args)
{ DisplayInitialized = 1; }

/****************************************************
 *
 *	DSP_VideoColor() entry of SUBRCALL 66 1
 *			called from (VIDEOCLOR BLACKFLG)
 *
 ****************************************************/

LispPTR DSP_VideoColor(LispPTR *args) /* args[0] :	black flag	*/
{
  int invert;
#ifdef SUNDISPLAY
  return NIL;
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  invert = args[0] & 0xFFFF;
  lisp_Xvideocolor(invert);
  if (invert)
    return ATOM_T;
  else
    return NIL;
#endif /* XWINDOW */
}

extern struct cursor CurrentCursor;
extern int LispWindowFd;

extern int errno;

/****************************************************
 *
 *	DSP_Cursor() entry of SUBRCALL 64 2
 *			called from \HARDCURSORUP etc.
 *
 ****************************************************/
void DSP_Cursor(LispPTR *args, int argnum)
/* args[0] :	hot spot X
 * args[1] :	hot spot Y
 */
{
  extern int ScreenLocked;
  extern DLword *EmCursorX68K, *EmCursorY68K;
  extern int LastCursorX, LastCursorY;
  static int Init = T;

#ifdef SUNDISPLAY
  if (argnum == 2) {
    CurrentCursor.cur_xhot = args[0] & 0xffff;
    CurrentCursor.cur_yhot = args[1] & 0xffff;
  };

#ifdef OLD_CURSOR
  win_setcursor(LispWindowFd, &CurrentCursor);
#else
#ifndef INIT
  ScreenLocked = T;
  if (!Init) {
    taking_mouse_down();
    taking_mouse_up(*EmCursorX68K, *EmCursorY68K);
  } else {
    Init = NIL;
    cursor_hidden_bitmap(0, 0);
    taking_mouse_up(0, 0);
    *EmCursorX68K = LastCursorX = 0;
    *EmCursorY68K = LastCursorY = 0;
  }

  ScreenLocked = NIL;
#else
  /* Init specific lde only */
  ScreenLocked = T;
  if (!Init) {
    taking_mouse_down();
    taking_mouse_up(0, 0);
  } else {
    Init = NIL;
    cursor_hidden_bitmap(0, 0);
    taking_mouse_up(0, 0);
  }

  ScreenLocked = NIL;
#endif /* INIT */

#endif
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  /* For X-Windows, set the cursor the the given location. */
  Set_XCursor((int)(args[0] & 0xFFFF), (int)(args[1] & 0xFFFF));
#endif /* XWINDOW */
}

/****************************************************
 *
 *	DSP_SetMousePos() entry of SUBRCALL 65 2
 *			called from macro \SETMOUSEXY etc.
 *
 ****************************************************/
 /* args[0] :	X pos
  * args[1] :	Y pos
  */
void DSP_SetMousePos(register LispPTR *args)
{
#ifdef SUNDISPLAY
#ifdef OLD_CURSOR
  register int x, y;
  x = GetSmalldata(args[0]);
  y = GetSmalldata(args[1]); /* debug */
  win_setmouseposition(LispWindowFd, GetSmalldata(args[0]), GetSmalldata(args[1]));
#else
  extern int ScreenLocked;
  extern DLword *EmCursorX68K, *EmCursorY68K, *EmMouseX68K, *EmMouseY68K;
  register int x, y;
  ScreenLocked = T;
  x = GetSmalldata(args[0]);
  y = GetSmalldata(args[1]);
  /* for Suntool's invisible cursor */
  win_setmouseposition(LispWindowFd, x, y);
  /* for REAL cursor image */
  taking_mouse_down();
  taking_mouse_up(x, y);

#ifndef INIT
  *EmMouseX68K = x;
  *EmMouseY68K = y;
#endif
  ScreenLocked = NIL;
#endif
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  if (Mouse_Included)
    set_Xmouseposition((int)(GetSmalldata(args[0])), (int)(GetSmalldata(args[1])));
#endif /* XWINDOW */
}

/****************************************************
 *
 *	DSP_ScreenWidth() entry of SUBRCALL 67 0
 *			called from  \Katana.DisplayWidth.
 *
 ****************************************************/
LispPTR DSP_ScreenWidth(LispPTR *args)
{ return (S_POSITIVE | (0xFFFF & displaywidth)); }

/****************************************************
 *
 *	DSP_ScreenHight() entry of SUBRCALL 68 0
 *			called from  \Katana.DisplayHeight.
 *
 ****************************************************/
LispPTR DSP_ScreenHight(LispPTR *args)
{ return (S_POSITIVE | (0xFFFF & displayheight)); }

/****************************************************
 *
 *	flip_cursor()
 *
 ****************************************************/

extern DLword *EmCursorBitMap68K;
extern int for_makeinit;

#ifdef XWINDOW
extern int Current_Hot_X, Current_Hot_Y;
#endif /* XWINDOW */

void flip_cursor() {
  register DLword *word;
  register int cnt;
  extern int ScreenLocked;
  extern DLword *EmCursorX68K, *EmCursorY68K;

  word = EmCursorBitMap68K;

#ifdef INIT

  /* since this is called frequently, and you don't want to have
     to build a different LDE to run the 2 parts of a Loadup, there is
     an ifdef AND a test.  This way we don't generate
     extra code for anybody elses building an LDE
     except those who want to try building loadups.  */

  if (!for_makeinit) {
    for (cnt = CURSORHEIGHT; (cnt--);) { GETWORD(word++) ^= 0xFFFF; };
  };

#else

  for (cnt = CURSORHEIGHT; (cnt--);) { GETWORD(word++) ^= 0xFFFF; };

#endif

#ifdef SUNDISPLAY
#ifdef OLD_CURSOR

  win_setcursor(LispWindowFd, &CurrentCursor);
#else
  ScreenLocked = T;
  taking_mouse_down();
#ifndef INIT
  taking_mouse_up(*EmCursorX68K, *EmCursorY68K);
#else
  if (!for_makeinit)
    taking_mouse_up(*EmCursorX68K, *EmCursorY68K);
  else
    taking_mouse_up(0, 0);
#endif /* INIT */

  ScreenLocked = NIL;
#endif
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  /* JDS 011213: 15- cur y, as function does same! */
  Set_XCursor(Current_Hot_X, 15 - Current_Hot_Y);
#endif /* XWINDOW */
}
