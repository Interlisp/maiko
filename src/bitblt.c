/* $Id: bitblt.c,v 1.2 1999/01/03 02:06:47 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>


#ifdef XWINDOW
#define DISPLAYBUFFER
#endif /* XWINDOW */

#include "lispemul.h"
#include "lspglob.h"
#include "lispmap.h"
#include "adr68k.h"
#include "address.h"

#include "pilotbbt.h"
#include "display.h"
#include "bitblt.h"
#include "bb.h"

#include "bitbltdefs.h"
#include "initdspdefs.h"

#if defined(INIT)
#include "initkbddefs.h"
extern int kbd_for_makeinit;
#endif

#ifdef DOS
#include "devif.h"
extern DspInterface currentdsp;
#endif

extern int ScreenLocked;

#ifdef COLOR
extern int MonoOrColor;
#endif /* COLOR */

/*****************************************************************************/
/**                                                                         **/
/**                             N_OP_pilotbitblt                            **/
/**                                                                         **/
/**       The Native-code compatible version of the opcode for bitblt.      **/
/**                                                                         **/
/**                                                                         **/
/*****************************************************************************/

LispPTR N_OP_pilotbitblt(LispPTR pilot_bt_tbl, LispPTR tos)
{
  PILOTBBT *pbt;
  DLword *srcbase, *dstbase;
#if defined(SUNDISPLAY) || defined(DOS)
  int displayflg;
#endif
  int sx, dx, w, h, srcbpl, dstbpl, backwardflg;
  int src_comp, op, gray, num_gray, curr_gray_line;

#ifdef INIT

  /* for init, we have to initialize the pointers at the
     first call to pilotbitblt or we die.  If we do it
     earlier we die also.  We set a new flag so we don't
     do it more than once which is a lose also.

     I put this in an ifdef so there won't be any extra
     code when making a regular LDE.  */

  if (!kbd_for_makeinit) {
    init_keyboard(0);
    kbd_for_makeinit = 1;
  }

#endif

  pbt = (PILOTBBT *)NativeAligned4FromLAddr(pilot_bt_tbl);

  w = pbt->pbtwidth;
  h = pbt->pbtheight;
  if ((h <= 0) || (w <= 0)) return (pilot_bt_tbl);
  dx = pbt->pbtdestbit;
  sx = pbt->pbtsourcebit;
  backwardflg = pbt->pbtbackward;
/* if displayflg != 0 then source or destination is DisplayBitMap */
#ifdef DOS
  currentdsp->device.locked++;
#else
  ScreenLocked = T;
#endif /* DOS */

#if defined(SUNDISPLAY) || defined(DOS)
  displayflg = cursorin(pbt->pbtdesthi, (pbt->pbtdestlo + (dx >> 4)), w, h, backwardflg) ||
               cursorin(pbt->pbtsourcehi, (pbt->pbtsourcelo + (sx >> 4)), w, h, backwardflg);
#endif /* SUNDISPLAY */

  srcbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtsourcehi, pbt->pbtsourcelo));
  dstbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtdesthi, pbt->pbtdestlo));

  srcbpl = pbt->pbtsourcebpl;
  dstbpl = pbt->pbtdestbpl;
  src_comp = pbt->pbtsourcetype;
  op = pbt->pbtoperation;
  gray = pbt->pbtusegray;
  num_gray = ((TEXTUREBBT *)pbt)->pbtgrayheightlessone + 1;
  curr_gray_line = ((TEXTUREBBT *)pbt)->pbtgrayoffset;

#ifdef DOS
  if (displayflg) (currentdsp->mouse_invisible)(currentdsp, IOPage);
  ;
#endif /* SUNDISPLAY / DOS */

  new_bitblt_code;

#ifdef DOS
      flush_display_lineregion(dx, dstbase, w, h);
  if (displayflg) (currentdsp->mouse_visible)(IOPage->dlmousex, IOPage->dlmousey);
#endif /* SUNDISPLAY / DOS */

#ifdef XWINDOW
  flush_display_lineregion(dx, dstbase, w, h);
#endif /* XWINDOW */

#ifdef DOS
  currentdsp->device.locked--;
#else
  ScreenLocked = NIL;
#endif /* DOS */

  return (pilot_bt_tbl);

} /* end of N_OP_pilotbitblt */

/************************************************************************/
/*                                                                      */
/*                              c u r s o r i n                                 */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifndef COLOR
/* for MONO only */
int cursorin(DLword addrhi, DLword addrlo, int w, int h, int backward)
{
  int x, y;
  if (addrhi == DISPLAY_HI) {
    y = addrlo / DisplayRasterWidth;
    x = (addrlo - y * DisplayRasterWidth) << 4;
  } else if (addrhi == DISPLAY_HI + 1) {
    y = (addrlo + DLWORDSPER_SEGMENT) / DisplayRasterWidth;
    x = ((addrlo + DLWORDSPER_SEGMENT) - y * DisplayRasterWidth) << 4;
  } else
    return (NIL);

  if (backward) y -= h;

  if ((x < MOUSEXR) && (x + w > MOUSEXL) && (y < MOUSEYH) && (y + h > MOUSEYL))
    return (T);
  else
    return (NIL);
}
#else

/* for COLOR & MONO */
int cursorin(DLword addrhi, DLword addrlo, int w, int h, int backward)
{
  int x, y;
  DLword *base68k;
  extern int MonoOrColor;
  extern int displaywidth;
  extern DLword *ColorDisplayRegion68k;

  if (MonoOrColor == MONO_SCREEN) { /* On MONO screen */
    if (addrhi == DISPLAY_HI) {
      y = addrlo / DisplayRasterWidth;
      x = (addrlo - y * DisplayRasterWidth) << 4;
    } else if (addrhi == DISPLAY_HI + 1) {
      y = (addrlo + DLWORDSPER_SEGMENT) / DisplayRasterWidth;
      x = ((addrlo + DLWORDSPER_SEGMENT) - y * DisplayRasterWidth) << 4;
    } else
      return (NIL);

    if (backward) y -= h;

    if ((x < MOUSEXR) && (x + w > MOUSEXL) && (y < MOUSEYH) && (y + h > MOUSEYL))
      return (T);
    else
      return (NIL);
  } else {
    base68k = (DLword *)NativeAligned2FromLAddr(addrhi << 16 | addrlo);
    if ((ColorDisplayRegion68k <= base68k) && (base68k <= COLOR_MAX_Address)) {
      y = (base68k - ColorDisplayRegion68k) / displaywidth;
      x = (UNSIGNED)(base68k - ColorDisplayRegion68k) - (y * displaywidth);
      /* printf("cursorin: IN COLOR mx=%d my=%d x=%d y%d w=%d h=%d\n"
      ,*EmMouseX68K,*EmMouseY68K,x,y,w,h); */
    } else
      return (NIL);

    if (backward) y -= h;

    if ((x < MOUSEXR) && ((x + (w >> 3)) > MOUSEXL) && (y < MOUSEYH) &&
        (y + h > MOUSEYL)) { /* printf("cursorin T\n"); */
      return (T);
    } else
      return (NIL);

  } /* on COLOR screen */
}
#endif /* COLOR */
