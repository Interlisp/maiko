/* $Id: misc7.c,v 1.2 1999/01/03 02:07:22 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */
static char *id = "$Id: misc7.c,v 1.2 1999/01/03 02:07:22 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*	misc7.c
 */
#include <stdio.h>
#ifndef DOS
#include <sys/ioctl.h>
#endif /* DOS */
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "gc.h"
#include "arith.h"
#include "my.h"
#include "bitblt.h"
#include "bbtsub.h"
#include "dbprint.h"
/* osamu '90/02/08
 * add display.h, because in_display_secment() is changed as
 * macro. difinition is in display.h
 */
#include "display.h"

#ifndef NOPIXRECT
#include <sunwindow/window_hs.h>
#include <sunwindow/win_ioctl.h>

#include <suntool/sunview.h>
#include <signal.h>
#include <sunwindow/cms_mono.h>
#include <suntool/canvas.h>
#endif

/*************************************************/
/*  Possible operation fields for FBITMAPBIT     */
/*************************************************/

#define OP_INVERT 0 /* Invert the bit at the given location */
#define OP_ERASE 1  /* Turn the given bit off. */
#define OP_READ 2   /* Just read the bit that's there. */
#define OP_PAINT 3  /* Turn the bit on. */

extern int LispWindowFd;
extern int ScreenLocked;

/***	N_OP_misc7  -- pseudocolor or fbitmapbit   ***/
LispPTR N_OP_misc7(LispPTR arg1, LispPTR arg2, LispPTR arg3, LispPTR arg4, LispPTR arg5, LispPTR arg6, LispPTR arg7, int alpha)
{
  DLword *base;
  int x, y, operation, heightminus1, rasterwidth, oldbit;
  int displayflg;

  DBPRINT(("MISC7 op with alpha byte %d.\n", alpha));

  if (alpha != 1) ERROR_EXIT(arg7);

  base = Addr68k_from_LADDR(arg1);
  N_GETNUMBER(arg2, x, doufn);
  N_GETNUMBER(arg3, y, doufn);
  N_GETNUMBER(arg4, operation, doufn);
  N_GETNUMBER(arg5, heightminus1, doufn);
  N_GETNUMBER(arg6, rasterwidth, doufn);

  DBPRINT(("MISC7 args OK.\n"));

  displayflg = n_new_cursorin(base, x, (heightminus1 - y), 1, 1);

  base = base + (rasterwidth * (heightminus1 - y)) + (0xFFFF & (x >> 4));
  x = 32768 >> (x & 0xF);

  oldbit = x & *base;

  ScreenLocked = T;

#ifdef SUNDISPLAY
  if (displayflg) HideCursor;
#endif /* SUNDISPLAY */

  switch (operation) {
    case OP_INVERT: *base ^= x; break;
    case OP_ERASE: *base &= ~x; break;
    case OP_READ: break;
    default: *base |= x;
  };

#ifdef SUNDISPLAY
#ifdef DISPLAYBUFFER
  if (in_display_segment(base)) flush_display_ptrregion(base, 0, 16, 1);
#endif
  if (displayflg) ShowCursor;
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  if (in_display_segment(base)) flush_display_ptrregion(base, 0, 16, 1);
#endif /* XWINDOW */

  ScreenLocked = NIL;
  DBPRINT(("FBITMAPBIT old bit = 0x%x.\n", oldbit));
  return (S_POSITIVE | (oldbit ? 1 : 0));

doufn:
  ERROR_EXIT(arg7);

} /*  end N_OP_misc7()  */
