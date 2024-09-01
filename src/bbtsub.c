/* $Id: bbtsub.c,v 1.3 2001/12/24 01:08:59 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*                                                                      */
/*                      File:   bbtsub.c                                */
/*                                                                      */
/*      Subroutines to support BITBLT, \BLTCHAR, and \TEDIT.BLTCHAR     */
/*      lisp functions, providing performance improvement.              */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef XWINDOW
#ifndef DOS
#include <sys/ioctl.h>
#endif /* DOS */
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xdefs.h"
#endif /* XWINDOW */


#include "lispemul.h"
#include "lspglob.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "adr68k.h"
#include "address.h"
#include "arith.h"
#include "stack.h"
#include "return.h"
#include "cell.h"

#include "bbtsubdefs.h"
#include "car-cdrdefs.h"
#include "commondefs.h"
#include "gcarraydefs.h"
#include "initdspdefs.h"
#include "kprintdefs.h"
#include "llstkdefs.h"
#include "returndefs.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"
#include "dspdata.h"
#include "display.h"
#include "dbprint.h"

#ifdef INIT
#include "initkbddefs.h"
extern int  kbd_for_makeinit;

#define init_kbd_startup   \
  do {			   \
    if (!kbd_for_makeinit) { \
    init_keyboard(0);      \
    kbd_for_makeinit = 1;  \
    }                      \
  } while (0)
#endif

#include "devif.h"
extern DspInterface currentdsp;

#ifdef COLOR
extern int MonoOrColor;
#endif /* COLOR */

/*******************************************/
/*  REALCURSOR is defined iff we need to   */
/*  take care of cursor movement & hiding  */
/*  (during bitblts to screen) ourselves.  */
/*******************************************/
#if defined(SUNDISPLAY) || defined(DOS)
#define REALCURSOR
#else
#undef REALCURSOR
#endif

/* same definition is in my.h */
#ifdef BIGVM
#define IsNumber(address) ((GETWORD(MDStypetbl + (((address)&0x0fffff00) >> 9))) & 0x1000)
#else
#define IsNumber(address) ((GETWORD(MDStypetbl + (((address)&0x0ffff00) >> 9))) & 0x1000)
#endif

#define BITSPERWORD (16) /* temp definition      */

#ifndef BYTESWAP
#ifdef BIGVM
typedef struct {
  unsigned nil1 : 4;
  unsigned pilotbbt : 28;
  unsigned nil2 : 4;
  unsigned displaydata : 28;
  unsigned nil3 : 16;
  unsigned char8code : 16;
  unsigned nil4 : 16;
  unsigned curx : 16;
  unsigned nil5 : 16;
  unsigned left : 16;
  unsigned nil6 : 16;
  unsigned right : 16;
} BLTC;
#else
typedef struct {
  unsigned nil1 : 8;
  unsigned pilotbbt : 24;
  unsigned nil2 : 8;
  unsigned displaydata : 24;
  unsigned nil3 : 16;
  unsigned char8code : 16;
  unsigned nil4 : 16;
  unsigned curx : 16;
  unsigned nil5 : 16;
  unsigned left : 16;
  unsigned nil6 : 16;
  unsigned right : 16;
} BLTC;
#endif /* BIGVM */
#else
#ifdef BIGVM
typedef struct {
  unsigned pilotbbt : 28;
  unsigned nil1 : 4;
  unsigned displaydata : 28;
  unsigned nil2 : 4;
  unsigned char8code : 16;
  unsigned nil3 : 16;
  unsigned curx : 16;
  unsigned nil4 : 16;
  unsigned left : 16;
  unsigned nil5 : 16;
  unsigned right : 16;
  unsigned nil6 : 16;
} BLTC;
#else
typedef struct {
  unsigned pilotbbt : 24;
  unsigned nil1 : 8;
  unsigned displaydata : 24;
  unsigned nil2 : 8;
  unsigned char8code : 16;
  unsigned nil3 : 16;
  unsigned curx : 16;
  unsigned nil4 : 16;
  unsigned left : 16;
  unsigned nil5 : 16;
  unsigned right : 16;
  unsigned nil6 : 16;
} BLTC;
#endif /* BIGVM */
#endif /* BYTESWAP */

/****************************************/
/*                                      */
/*      Arguments to NEWBLTCHAR                 */
/*                                      */
/****************************************/
#ifndef BYTESWAP
typedef struct {
  DLword nil;             /* Unused word */
  unsigned charset : 8;   /* High 8 bits of character code */
  unsigned char8code : 8; /* Low 8 bits of character code  */
  LispPTR displaystream;  /* The display stream to print on */
  LispPTR displaydata;    /* The image data (margins, etc)  */
} BLTARG;
#else
typedef struct {
  unsigned char8code : 8; /* Low 8 bits of character code  */
  unsigned charset : 8;   /* High 8 bits of character code */
  DLword nil;             /* Unused word */
  LispPTR displaystream;  /* The display stream to print on */
  LispPTR displaydata;    /* The image data (margins, etc)  */
} BLTARG;
#endif /* BYTESWAP */

#ifndef BYTESWAP
typedef struct tbta {
  DLword nil;
  unsigned charset : 8;
  unsigned char8code : 8;
  LispPTR displaystream;
  unsigned int nil2 : 16;
  unsigned int current_x : 16; /* this is always positive */
  LispPTR displaydata;
  LispPTR ddpilotbitblt;
  unsigned int nil3 : 16;
  unsigned int clipright : 16; /* this is always positive */
} TBLTARG;
#else
typedef struct tbta {
  unsigned char8code : 8;
  unsigned charset : 8;
  DLword nil;
  LispPTR displaystream;
  unsigned int current_x : 16; /* this is always positive */
  unsigned int nil2 : 16;
  LispPTR displaydata;
  LispPTR ddpilotbitblt;
  unsigned int clipright : 16; /* this is always positive */
  unsigned int nil3 : 16;
} TBLTARG;
#endif /* BYTESWAP */

extern int ScreenLocked; /* for mouse tracking */
/*****************************************************************
(PUTPROPS \SETPBTFUNCTION MACRO
        (OPENLAMBDA (BBT SourceType Operation)
                (PROGN (replace (PILOTBBT PBTOPERATION) of BBT with
                                (SELECTQ Operation
                                        (ERASE 1)
                                        (PAINT 2)
                                        (INVERT 3)
                                        0))
                       (replace (PILOTBBT PBTSOURCETYPE) of BBT with
                            (COND ((EQ (EQ SourceType (QUOTE INVERT))
                                       (EQ Operation (QUOTE ERASE))) 0)
                                  (T 1))))))
*****************************************************************/
#define PixOperationLisp(SRCTYPE, OPERATION)                                                       \
  ((SRCTYPE) == INVERT_atom                                                                        \
       ? ((OPERATION) == REPLACE_atom                                                              \
              ? PIX_NOT(PIX_SRC)                                                                   \
              : ((OPERATION) == PAINT_atom                                                         \
                     ? PIX_NOT(PIX_SRC) | PIX_DST                                                  \
                     : ((OPERATION) == ERASE_atom                                                  \
                            ? PIX_SRC & PIX_DST                                                    \
                            : ((OPERATION) == INVERT_atom ? PIX_NOT(PIX_SRC) ^ PIX_DST : ERROR)))) \
       : /*  SRCTYPE == INPUT, TEXTURE */                                                          \
       ((OPERATION) == REPLACE_atom                                                                \
            ? PIX_SRC                                                                              \
            : ((OPERATION) == PAINT_atom                                                           \
                   ? PIX_SRC | PIX_DST                                                             \
                   : ((OPERATION) == ERASE_atom                                                    \
                          ? PIX_NOT(PIX_SRC) & PIX_DST                                             \
                          : ((OPERATION) == INVERT_atom ? PIX_SRC ^ PIX_DST : ERROR)))))

#define bbop(SRCTYPE, OPERATION)                \
  ((OPERATION) == PAINT_atom                    \
       ? op_fn_or                               \
       : ((OPERATION) == ERASE_atom ? op_fn_and \
                                    : ((OPERATION) == INVERT_atom ? op_fn_xor : op_repl_src)))

/********************************************************/
/*                                                      */
/*                  b b s r c _ t y p e                         */
/*                                                      */
/*      Returns 1 if the source bits must be inverted   */
/*      as part of the BITBLT.  This is true if the     */
/*      OPERATION argument to BITBLT is 'ERASE, or      */
/*      if the SOURCETYPE argument is 'INVERT.          */
/*                                                      */
/********************************************************/

#define bbsrc_type(SRCTYPE, OPERATION)                                                             \
  ((SRCTYPE) == INVERT_atom ? ((OPERATION) == ERASE_atom ? 0 : 1) /*  SRCTYPE == INPUT, TEXTURE */ \
                          : ((OPERATION) == ERASE_atom ? 1 : 0))

extern struct pixrect *SrcePixRect, *DestPixRect, *TexturePixRect;
extern struct pixrect *BlackTexturePixRect, *WhiteTexturePixRect;

/************************************************************************/
/*                                                                      */
/*                         b i t b l t s u b                            */
/*                                                                      */
/*      Implements the lisp function \BITBLTSUB, which is where                 */
/*      all BITBLT & BLTSHADE calls bottom out.  This is distinct       */
/*      from the PILOTBITBLT opcode, which is implemented in bitblt.c.  */
/*                                                                      */
/*                                                                      */
/*      args[0] :       PILOTBBT                                        */
/*      args[1] :       SOURCEBITMAP                                    */
/*      args[2] :       SLX (SourceLeftX)               sx              */
/*      args[3] :       STY (SourceTopY)                sty             */
/*      args[4] :       DESTINATIONBITMAP                               */
/*      args[5] :       DLX (DestinationLeftX)          dx              */
/*      args[6] :       DTY (DestinationTopY)           dty             */
/*      args[7] :       HEIGHT                                          */
/*      args[8] :       SourceType                                      */
/*      args[9] :       Operation                                       */
/*      args[10] :      Texture                                                 */
/*      args[11] :      WindowXOffset                                   */
/*      args[12] :      WindowYOffset                                   */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void bitbltsub(LispPTR *argv) {
  int sourcetype, operation;
  int sty, dty, texture, wxoffset, wyoffset;
  int h, w;
#ifdef REALCURSOR
  int displayflg = 0;
#endif
  int backwardflg = 0, sx, dx, srcbpl=2147483647, dstbpl, src_comp, op;
  DLword *srcbase, *dstbase;
  int gray = 0, num_gray = 0, curr_gray_line = 0;
  DLword grayword[4] = {0, 0, 0, 0};

  { /* Initialization code, in a block so it optimizes independently */
    LispPTR *args = argv;
    PILOTBBT *pbt;
    BITMAP *srcebm, *destbm;
    BITMAP *texture68k;
    DLword *base;

#ifdef INIT
    init_kbd_startup;
#endif

    pbt = (PILOTBBT *)NativeAligned4FromLAddr(args[0]);
    srcebm = (BITMAP *)NativeAligned4FromLAddr(args[1]);
    sx = (args[2] & 0xFFFF);
    sty = (args[3] & 0xFFFF);
    destbm = (BITMAP *)NativeAligned4FromLAddr(args[4]);
    dx = (args[5] & 0xFFFF);
    dty = (args[6] & 0xFFFF);
    sourcetype = (args[8] == NIL_PTR ? INPUT_atom : args[8]);
    operation = (args[9] == NIL_PTR ? REPLACE_atom : args[9]);

    w = pbt->pbtwidth;
    h = pbt->pbtheight;
    if ((h <= 0) || (w <= 0)) return;
    src_comp = bbsrc_type(sourcetype, operation);
    op = bbop(sourcetype, operation);

    dstbpl = destbm->bmrasterwidth << 4;

    if (sourcetype == TEXTURE_atom) {
      texture = args[10];
      wxoffset = (args[11] == NIL_PTR ? 0 : args[11] & 0xFFFF);
      wyoffset = (args[12] == NIL_PTR ? 0 : args[12] & 0xFFFF);
      sx = ((wxoffset) ? (dx - wxoffset) : dx) % BITSPERWORD;
      dstbase = (DLword *)NativeAligned2FromLAddr(ADDBASE(destbm->bmbase, destbm->bmrasterwidth * dty));
      gray = 1;
      if (texture == NIL_PTR) { /* White Shade */
        grayword[0] = 0;
        srcbase = &grayword[0];
        num_gray = 1;
        curr_gray_line = 0;
        goto do_it_now;
      } else if (IsNumber(texture)) {
        if ((texture &= 0xffff) == 0) { /* White Shade */
          grayword[0] = 0;
          srcbase = &grayword[0];
          num_gray = 1;
          curr_gray_line = 0;
          goto do_it_now;
        } else if (texture == 0xffff) { /* Black Shade */
          grayword[0] = 0xFFFF;
          srcbase = &grayword[0];
          num_gray = 1;
          curr_gray_line = 0;
          goto do_it_now;
        } else { /* 4x4 */
          srcbase = base = (DLword *)(&grayword[0]);
          GETWORD(base++) = Expand4Bit(((texture >> 12) & 0xf));
          GETWORD(base++) = Expand4Bit(((texture >> 8) & 0xf));
          GETWORD(base++) = Expand4Bit(((texture >> 4) & 0xf));
          GETWORD(base++) = Expand4Bit((texture & 0xf));
          num_gray = 4;
          curr_gray_line = (dty + wyoffset) & 3;
          srcbase += curr_gray_line;
          goto do_it_now;
        }
      } else { /* A bitmap that is 16 bits wide. */
        texture68k = (BITMAP *)NativeAligned4FromLAddr(texture);
        srcbase = (DLword *)NativeAligned2FromLAddr(texture68k->bmbase);
        num_gray = min(texture68k->bmheight, 16);
        curr_gray_line = (dty + wyoffset) % num_gray;
        srcbase += curr_gray_line;
        goto do_it_now;
      }
    }
    /* ; INPUT or INVERT      */
    srcbpl = srcebm->bmrasterwidth << 4;

    /* compute flags */
    /* out for now
    if(srcebm == destbm)
    if(sty <= dty)
    if(dty <= (sty + h))
    if((sty != dty) || ((sx < dx) && (dx < (sx + w))))
                      backwardflg = T;
    out for now */
    /* compute flags */
    if (srcebm != destbm)
      ;
    else if (sty > dty)
      ;
    else if (dty > (sty + h))
      ;
    else if ((sty != dty) || ((sx < dx) && (dx < (sx + w))))
      backwardflg = T;

    if (backwardflg) {
      srcbase = (DLword *)NativeAligned2FromLAddr(
          ADDBASE(srcebm->bmbase, srcebm->bmrasterwidth * (sty + h - 1)));
      dstbase = (DLword *)NativeAligned2FromLAddr(
          ADDBASE(destbm->bmbase, destbm->bmrasterwidth * (dty + h - 1)));
      srcbpl = 0 - srcbpl;
      dstbpl = 0 - dstbpl;
    } else {
      srcbase = (DLword *)NativeAligned2FromLAddr(ADDBASE(srcebm->bmbase, srcebm->bmrasterwidth * sty));
      dstbase = (DLword *)NativeAligned2FromLAddr(ADDBASE(destbm->bmbase, destbm->bmrasterwidth * dty));
    }
#ifdef REALCURSOR
    displayflg = n_new_cursorin(srcbase, sx, sty, w, h);
#endif /* REALCURSOR */
  }

do_it_now:
#ifdef DOS
  currentdsp->device.locked++;
#else
  ScreenLocked = T;
#endif /* DOS */

#ifdef REALCURSOR
  displayflg |= n_new_cursorin(dstbase, dx, dty, w, h);
  if (displayflg) HideCursor;
#endif /* REALCURSOR */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, op, gray, num_gray,
         curr_gray_line);
#else
  new_bitblt_code;
#endif

#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

    /* Copy the changed section of display bank to the frame buffer */
    if (in_display_segment(dstbase)) {
      /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",dx, dty, w,h)); */
      flush_display_region(dx, dty, w, h);
    }
#endif

#ifdef XWINDOW
  if (in_display_segment(dstbase)) flush_display_region(dx, dty, w, h);
#endif /* XWINDOW */

#ifdef SDL
  if (in_display_segment(dstbase)) flush_display_region(dx, dty, w, h);
#endif /* XWINDOW */

#ifdef DOS
  /* Copy the changed section of display bank to the frame buffer */
  if (in_display_segment(dstbase)) {
    /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",dx, dty, w,h)); */
    flush_display_region(dx, dty, w, h);
  }
#endif /* DOS */

#ifdef REALCURSOR
  if (displayflg) ShowCursor;
#endif /* REALCURSOR */

#ifdef DOS
  currentdsp->device.locked--;
#else
  ScreenLocked = NIL;
#endif /* DOS */

} /* end of bitbltsub */

/************************************************************************/
/*                                                                      */
/*                      n _ n e w _ c u r s o r i n                     */
/*                                                                      */
/*      Is the cursor over the spot we're about to change on the        */
/*      screen?                                                                 */
/*                                                                      */
/*      This version takes a native address for the bitmap base,        */
/*      plus x, y, width, and height for the intended change area.      */
/*      Returns T if the cursor overlaps the intended change, NIL       */
/*      otherwise.                                                      */
/*                                                                      */
/************************************************************************/

#ifndef COLOR

/********************************************************/
/*                                                      */
/*              Monochrome-only version                         */
/*                                                      */
/********************************************************/

LispPTR n_new_cursorin(DLword *baseaddr, int dx, int dy, int w, int h) {
  extern DLword *DisplayRegion68k;

#ifdef INIT
  init_kbd_startup; /* MUST START KBD FOR INIT BEFORE FIRST BITBLT */
#endif

  if (in_display_segment(baseaddr)) {
    if ((dx < MOUSEXR) && (dx + w > MOUSEXL) && (dy < MOUSEYH) && (dy + h > MOUSEYL))
      return (T);
    else
      return (NIL);
  } else
    return (NIL);
}
#else
/********************************************************/
/*                                                      */
/*                 Mono / color version                         */
/*                                                      */
/********************************************************/
extern DLword *DisplayRegion68k, *ColorDisplayRegion68k;
extern int MonoOrColor;

LispPTR n_new_cursorin(DLword *baseaddr, int dx, int dy, int w, int h) {
#ifdef INIT
  init_kbd_startup; /* MUST START KBD FOR INIT BEFORE FIRST BITBLT */
#endif

  if (MonoOrColor == MONO_SCREEN) { /* in MONO screen */
    if (in_display_segment(baseaddr)) {
      if ((dx < MOUSEXR) && (dx + w > MOUSEXL) && (dy < MOUSEYH) && (dy + h > MOUSEYL)) {
        return (T);
      } else {
        return (NIL);
      }
    } else
      return (NIL);
  }      /* if for MONO end */
  else { /* in COLOR screen */
    if ((ColorDisplayRegion68k <= baseaddr) && (baseaddr <= COLOR_MAX_Address)) {
      dx = dx >> 3;
      /*printf("new_c in COLOR mx=%d my=%d x=%d y=%d\n"
      ,*EmMouseX68K,*EmMouseY68K,dx,dy);*/
      if ((dx < MOUSEXR) && (dx + w > MOUSEXL) && (dy < MOUSEYH) &&
          (dy + h > MOUSEYL)) { /*  printf("new_c T\n");*/
        return (T);
      } else {
        return (NIL);
      }
    } else
      return (NIL);
  }
}
#endif /* COLOR */

#define BITBLTBITMAP_argnum 14
#define PUNT_TO_BITBLTBITMAP                                                                  \
  do {                                                                                           \
    if (BITBLTBITMAP_index == 0xffffffff) {                                                   \
      BITBLTBITMAP_index = get_package_atom("\\PUNT.BITBLT.BITMAP", 19, "INTERLISP", 9, NIL); \
      if (BITBLTBITMAP_index == 0xffffffff) {                                                 \
        error("BITBLTBITMAP install failed");                                                 \
        return (NIL);                                                                         \
      }                                                                                       \
    }                                                                                         \
    CurrentStackPTR += (BITBLTBITMAP_argnum - 1) * DLWORDSPER_CELL;                           \
    ccfuncall(BITBLTBITMAP_index, BITBLTBITMAP_argnum, 3);                                    \
    return (ATOM_T);                                                                               \
  } while (0)

LispPTR BITBLTBITMAP_index;
/************************************************************************/
/*                                                                      */
/*                      b i t b l t _ b i t m a p                       */
/*                                                                      */
/*      C implementation of the Lisp function \BITBLT.BITMAP,           */
/*      which does bitmap-to-bitmap cases of BITBLT, after BITBLT       */
/*      does some setup and massaging first.                            */
/*                                                                      */
/*      args[0]:  SourceBitmap                                          */
/*      args[1]:  SourceLeft            Must be a SMALLPOSP             */
/*      args[2]:  SourceBottom          Must be a SMALLPOSP             */
/*      args[3]:  DestBitmap                                            */
/*      args[4]:  DestLeft              Must be a SMALLPOSP             */
/*      args[5]:  DestBottom            Must be a SMALLPOSP             */
/*      args[6]:  Width                 Must be a SMALLPOSP             */
/*      args[7]:  Height                Must be a SMALLPOSP             */
/*      args[8]:  SourceType            [May be NIL]                    */
/*      args[9]:  Operation             [May be NIL]                    */
/*      args[10]: Texture               [May be NIL]                    */
/*      args[11]: ClippingRegion        [May be NIL]                    */
/*      args[12]: ClippedSrcLeft        Must be a SMALLPOSP             */
/*      args[13]: ClippedSrcBottom      Must be a SMALLPOSP             */
/*                                                                      */
/*                                                                      */
/*      SourceType must not be MERGE, which should be handled by        */
/*      the lisp code in \BITBLT.BITMAP.                                */
/*      This function can't handle COLOR & sourcetype == MERGE case.    */
/*      It causes punting to \\PUNT.BITBLT.BITMAP.                      */
/*      Therefore SYSOUT must contain \\PUNT.BITBLT.BITMAP.             */
/*                                                                      */
/************************************************************************/

LispPTR bitblt_bitmap(LispPTR *args) {
  BITMAP *SourceBitmap, *DestBitmap;
  int sleft, sbottom, dleft, dbottom, width, height, clipleft, clipbottom;
  LispPTR clipreg;
  int stodx, stody, right, top, destbits, sourcebits, left, bottom;
  LispPTR sourcetype, operation, texture;
  DLword *srcbase, *dstbase;
  int dlx, dty, slx, sty, srcbpl, dstbpl, op, src_comp, backwardflg = 0, displayflg = 0;

#ifdef INIT
  init_kbd_startup;
#endif

  /* Get arguments  and check the possibilities of PUNT */
  SourceBitmap = (BITMAP *)NativeAligned4FromLAddr(args[0]);
  DestBitmap = (BITMAP *)NativeAligned4FromLAddr(args[3]);
  /* It does not handle COLOR ..... maybe later */
  destbits = DestBitmap->bmbitperpixel;
  sourcebits = SourceBitmap->bmbitperpixel;
  if ((destbits != 1) || (sourcebits != 1)) {
    PUNT_TO_BITBLTBITMAP;
  }
  sourcetype = args[8];
  /* sourcetype == MERGE_atom case must be handled in Lisp function \\PUNT.BITBLT.BITMAP */
  if (sourcetype == MERGE_atom) { PUNT_TO_BITBLTBITMAP; }

  N_GETNUMBER(args[1], sleft, bad_arg);
  N_GETNUMBER(args[2], sbottom, bad_arg);
  N_GETNUMBER(args[4], dleft, bad_arg);
  N_GETNUMBER(args[5], dbottom, bad_arg);
  N_GETNUMBER(args[6], width, bad_arg);
  N_GETNUMBER(args[7], height, bad_arg);
  operation = args[9];
  texture = args[10];
  clipreg = args[11];
  N_GETNUMBER(args[12], clipleft, bad_arg);
  N_GETNUMBER(args[13], clipbottom, bad_arg);

  left = bottom = 0;
  top = DestBitmap->bmheight;
  right = DestBitmap->bmwidth;

  if (clipreg != NIL_PTR) {
    /* clip the BITBLT using the clipping region supplied */
    LispPTR clipvalue;
    int temp, cr_left, cr_bot;

    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, cr_left, bad_arg);
    left = max(left, cr_left);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, cr_bot, bad_arg);
    bottom = max(bottom, cr_bot);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, temp, bad_arg);
    right = min(right, cr_left + temp);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, temp, bad_arg);
    top = min(top, cr_bot + temp);
  }

  left = max(left, dleft);
  right = min(right, dleft + width);
  bottom = max(bottom, dbottom);
  top = min(top, dbottom + height);
  stody = dbottom - sbottom;
  stodx = dleft - sleft;

  {
    int temp;
    left = max(clipleft, max(0, left - stodx));
    bottom = max(clipbottom, max(0, bottom - stody));
    temp = SourceBitmap->bmwidth;
    right = min(temp, min(right - stodx, clipleft + width));
    temp = SourceBitmap->bmheight;
    top = min(temp, min(top - stody, clipbottom + height));
  }

  if ((right <= left) || (top <= bottom)) return (NIL);

  /*** PUT SOURCETYPE MERGE special code HERE ***/
  /**** See above, earlier in this code check sourcetype and punting. *****/
  /**** sourcebits CANNOT be unequal to destbits from earlier check */

  if (sourcebits != destbits) {
    /* DBPRINT(("BITBLT between bitmaps of different sizes, unimplemented.")); */
      return NIL;
  }
  /* if not 1-bit-per-pixel adjust limits by pixel size */
  switch (sourcebits) {
  case 1:
      break;
  case 4:
      left = left * 4;
      right = right * 4;
      stodx = stodx * 4;
      /* Put color texture merge case here */
      break;

  case 8:
      left = left * 8;
      right = right * 8;
      stodx = stodx * 8;
      /* Put color texture merge case here */
      break;

  case 24:
      left = left * 24;
      right = right * 24;
      stodx = stodx * 24;
      /* Put color texture merge case here */
      break;
  }

  height = top - bottom;
  width = right - left;
  dty = DestBitmap->bmheight - (top + stody);
  dlx = left + stodx;
  sty = SourceBitmap->bmheight - top;
  slx = left;

  /*** Stolen from bitbltsub, to avoid the call overhead: ***/
  src_comp = bbsrc_type(sourcetype, operation);
  op = bbop(sourcetype, operation);

  dstbpl = DestBitmap->bmrasterwidth << 4;

  /* Sourcetype guaranteed not to be TEXTURE by BITBLT fn */
  srcbpl = SourceBitmap->bmrasterwidth << 4;

  /* compute flags */
  if (SourceBitmap != DestBitmap)
    ;
  else if (sty > dty)
    ;
  else if (dty > (sty + height))
    ;
  else if ((sty != dty) || ((slx < dlx) && (dlx < (slx + width))))
    backwardflg = T;

  if (backwardflg) {
    srcbase = (DLword *)NativeAligned2FromLAddr(
        ADDBASE(SourceBitmap->bmbase, SourceBitmap->bmrasterwidth * (sty + height - 1)));
    dstbase = (DLword *)NativeAligned2FromLAddr(
        ADDBASE(DestBitmap->bmbase, DestBitmap->bmrasterwidth * (dty + height - 1)));
    srcbpl = 0 - srcbpl;
    dstbpl = 0 - dstbpl;
  } else {
    srcbase = (DLword *)NativeAligned2FromLAddr(
        ADDBASE(SourceBitmap->bmbase, SourceBitmap->bmrasterwidth * sty));
    dstbase =
        (DLword *)NativeAligned2FromLAddr(ADDBASE(DestBitmap->bmbase, DestBitmap->bmrasterwidth * dty));
  }

  displayflg = n_new_cursorin(srcbase, slx, sty, width, height);

do_it_now:
  LOCKSCREEN;

#ifdef REALCURSOR
  displayflg |= n_new_cursorin(dstbase, dlx, dty, width, height);
  if (displayflg) HideCursor;
#endif /*  REALCURSOR */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, slx, dlx, width, height, srcbpl, dstbpl, backwardflg, src_comp, op, 0, 0,
         0);
#else
#define gray 0
#define dx dlx
#define sx slx
#define w width
#define h height
#define curr_gray_line dx
#define num_gray 0
  new_bitblt_code;
#undef gray
#undef dx
#undef sx
#undef w
#undef h
#undef curr_gray_line
#undef num_gray
#endif

#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

    /* Copy the changed section of display bank to the frame buffer */
    if (in_display_segment(dstbase)) {
      /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",dlx, dty, width,height));*/
      flush_display_region(dlx, dty, width, height);
    }
#endif

#ifdef XWINDOW
  if (in_display_segment(dstbase)) flush_display_region(dlx, dty, width, height);
#endif /* XWINDOW */

#ifdef SDL
  if (in_display_segment(dstbase)) flush_display_region(dlx, dty, width, height);
#endif /* SDL */

#ifdef DOS
  /* Copy the changed section of display bank to the frame buffer */
  if (in_display_segment(dstbase)) {
    /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",dx, dty, w,h)); */
    flush_display_region(dlx, dty, width, height);
  }
#endif /* DOS */

#ifdef REALCURSOR
  if (displayflg) ShowCursor;
#endif /* REALCURSOR */

  UNLOCKSCREEN;

  return (ATOM_T);

bad_arg:
  return (NIL);
} /* end of bitblt_bitmap */

#define BLTSHADEBITMAP_argnum 8
#define PUNT_TO_BLTSHADEBITMAP                                                                    \
  do {                                                                                               \
    if (BLTSHADEBITMAP_index == 0xffffffff) {                                                     \
      BLTSHADEBITMAP_index = get_package_atom("\\PUNT.BLTSHADE.BITMAP", 21, "INTERLISP", 9, NIL); \
      if (BLTSHADEBITMAP_index == 0xffffffff) {                                                   \
        error("BLTSHADEBITMAP install failed");                                                   \
        return (NIL);                                                                             \
      }                                                                                           \
    }                                                                                             \
    CurrentStackPTR += (BLTSHADEBITMAP_argnum - 1) * DLWORDSPER_CELL;                             \
    ccfuncall(BLTSHADEBITMAP_index, BLTSHADEBITMAP_argnum, 3);                                    \
    return (ATOM_T);                                                                                   \
  } while (0)

LispPTR BLTSHADEBITMAP_index;

/************************************************************************/
/*                                                                      */
/*                  b i t s h a d e _ b i t m a p                       */
/*                                                                      */
/*      C implementation of the Lisp function \BITSHADE.BITMAP,                 */
/*      which does bitmap-to-bitmap cases of BITSHADE, after BITSHADE   */
/*      does some setup and massaging first.                            */
/*      This func. can't handle TEXTURE == LISTP or LITATOM case.       */
/*      It causes punting to \\PUNT.BLTSHADE.BITMAP.                    */
/*      Therefore SYSOUT must contain \\PUNT.BLTSHADE.BITMAP.           */
/*                                                                      */
/*      args[0]: Texture                                                */
/*      args[1]: DestBitmap                                             */
/*      args[2]: DestLeft               Must be a SMALLPOSP             */
/*      args[3]: DestBottom             Must be a SMALLPOSP             */
/*      args[4]: Width                  Must be a SMALLPOSP             */
/*      args[5]: height                         Must be a SMALLPOSP             */
/*      args[6]: Operation                                              */
/*      args[7]: ClippingRegion                                                 */
/*                                                                      */
/*      The numeric arguments are assumed to be SMALLPOSPs;             */
/*      the result with anything else isn't guaranteed correct.                 */
/*       This Func should punt if TEXTURE==LITATOM(NOT NIL) or LISTP    */
/*              or DestBitmap's BPP !=1                                         */
/*                                                                      */
/*                                                                      */
/************************************************************************/

LispPTR bitshade_bitmap(LispPTR *args) {
  BITMAP *DestBitmap, *texture68k;
  int dleft, dbottom, width, height;
  LispPTR clipreg;
  int right, top, destbits, left, bottom;
  LispPTR operation, texture;
  DLword *srcbase = NULL, *dstbase = NULL, *base = NULL;
  int dty, slx, dstbpl, op, src_comp;
#ifdef REALCURSOR
  int displayflg = 0;
#endif
  int rasterwidth;
  int num_gray = 0, curr_gray_line = 0;
  DLword grayword[4] = {0, 0, 0, 0};

  texture = args[0];
  {
    int temp;
    temp = GetTypeNumber(texture);
    if (((temp == TYPE_LITATOM) && (texture != NIL_PTR)) || (temp == TYPE_LISTP)) {
      PUNT_TO_BLTSHADEBITMAP;
    }
  }

  DestBitmap = (BITMAP *)NativeAligned4FromLAddr(args[1]);
  if ((destbits = DestBitmap->bmbitperpixel) != 1) { PUNT_TO_BLTSHADEBITMAP; }

  N_GETNUMBER(args[2], dleft, bad_arg);
  N_GETNUMBER(args[3], dbottom, bad_arg);
  operation = args[6];
  clipreg = args[7];

  rasterwidth = DestBitmap->bmrasterwidth;
  top = DestBitmap->bmheight;
  left = bottom = 0;
  right = DestBitmap->bmwidth;
  if (clipreg != NIL_PTR) { /* clip the BITBLT using the clipping region supplied */
    LispPTR clipvalue;
    int temp, cr_left, cr_bot;

    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, cr_left, bad_arg);
    left = max(left, cr_left);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, cr_bot, bad_arg);
    bottom = max(bottom, cr_bot);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, temp, bad_arg);
    right = min(right, cr_left + temp);

    clipreg = cdr(clipreg);
    clipvalue = car(clipreg);
    N_GETNUMBER(clipvalue, temp, bad_arg);
    top = min(top, cr_bot + temp);
  }

  left = max(left, dleft);
  bottom = max(bottom, dbottom);
  if (args[4] != NIL_PTR) {
    N_GETNUMBER(args[4], width, bad_arg);
    right = min(right, dleft + width);
  }
  if (args[5] != NIL_PTR) {
    N_GETNUMBER(args[5], height, bad_arg);
    top = min(top, dbottom + height);
  }

  if ((right <= left) || (top <= bottom)) return (NIL);

  height = top - bottom;
  width = right - left;
  dty = DestBitmap->bmheight - top;

  if ((dty < 0) || ((dty + height) > DestBitmap->bmheight)) error("dty bad.");
  if ((bottom < 0)) error("bottom bad.");
  if ((bottom > 2048)) error("bottom suspicious");

  /*** Stolen from bitbltsub, to avoid the call overhead: ***/
  src_comp = bbsrc_type(0, operation);
  op = bbop(0, operation);

  dstbpl = rasterwidth << 4;

  if ((left < 0) || (right > dstbpl)) error("left/right bad.");

  slx = left % BITSPERWORD;

  dstbase = (DLword *)NativeAligned2FromLAddr(ADDBASE(DestBitmap->bmbase, (rasterwidth * dty)));

  if (GetTypeNumber(texture) == TYPE_LITATOM) {
    if (texture == NIL_PTR) { /* White Shade */
      grayword[0] = 0;
      srcbase = &grayword[0];
      num_gray = 1;
      curr_gray_line = 0;
      goto do_it_now;
    }
    /* temp DEBUFG stuff */
    else {
      print(texture);
      error("Should not!");
    }
  } else if (IsNumber(texture)) {
    if ((texture &= 0xffff) == 0) { /* White Shade */
      grayword[0] = 0;
      srcbase = &grayword[0];
      num_gray = 1;
      curr_gray_line = 0;
    } else if (texture == 0xffff) { /* Black Shade */
      grayword[0] = 0xFFFF;
      srcbase = &grayword[0];
      num_gray = 1;
      curr_gray_line = 0;
    } else { /* 4x4 */
      srcbase = base = (DLword *)(&grayword[0]);
      GETWORD(base++) = Expand4Bit(((texture >> 12) & 0xf));
      GETWORD(base++) = Expand4Bit(((texture >> 8) & 0xf));
      GETWORD(base++) = Expand4Bit(((texture >> 4) & 0xf));
      GETWORD(base++) = Expand4Bit((texture & 0xf));
      num_gray = 4;
      curr_gray_line = (dty)&3;
      srcbase += curr_gray_line;
    }
  } else /**** Need to handle texture = listp case, too ***/
  /* Listp case alway punt to LISP */
  { /* A bitmap that is 16 bits wide. */
    texture68k = (BITMAP *)NativeAligned4FromLAddr(texture);
    srcbase = (DLword *)NativeAligned2FromLAddr(texture68k->bmbase);
    num_gray = min(texture68k->bmheight, 16);
    curr_gray_line = (dty) % num_gray;
    srcbase += curr_gray_line;
  }

do_it_now:
  LOCKSCREEN;

#ifdef REALCURSOR
  displayflg |= n_new_cursorin(dstbase, left, dty, width, height);
  if (displayflg) HideCursor;
#endif /* REALCURSOR */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, slx, left, width, height, 0, dstbpl, 0, src_comp, op, 1, num_gray,
         curr_gray_line);
#else
#define gray 1
#define backwardflg 0
#define srcbpl 0
#define w width
#define h height
#define dx left
#define sx slx
  new_bitblt_code;
#undef gray
#undef backwardflg
#undef srcbpl
#undef w
#undef h
#undef dx
#undef sx
#endif

#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

    /* Copy the changed section of display bank to the frame buffer */
    if (in_display_segment(dstbase)) {
      /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",left, dty, width,height));*/
      flush_display_region(left, dty, width, height);
    }
#endif

#ifdef XWINDOW
  if (in_display_segment(dstbase)) flush_display_region(left, dty, width, height);
#endif /* XWINDOW */

#ifdef SDL
  if (in_display_segment(dstbase)) flush_display_region(left, dty, width, height);
#endif /* SDL */

#ifdef DOS
  /* Copy the changed section of display bank to the frame buffer */
  if (in_display_segment(dstbase)) {
    /*      DBPRINT(("bltsub: x %d, y %d, w %d, h %d.\n",dx, dty, w,h)); */
    flush_display_region(left, dty, width, height);
  }
#endif /* DOS */

#ifdef REALCURSOR
  if (displayflg) ShowCursor;
#endif /*  REALCURSOR */

  UNLOCKSCREEN;

  return (ATOM_T);

bad_arg:
  return (NIL);
} /* end of bitshade_bitmap */

/*
 *
 *
 ********       BLTCHAR         BLTCHAR         BLTCHAR         BLTCHAR         *********
 *
 */

/** \BLTCHAR ****
(freplace (PILOTBBT PBTDESTBIT) of LOCAL1 with LEFT)
(freplace (PILOTBBT PBTWIDTH) of LOCAL1 with (IDIFFERENCE RIGHT LEFT))
(freplace (PILOTBBT PBTSOURCEBIT) of LOCAL1 with
    (IDIFFERENCE
        (IPLUS (\GETBASE
                    (ffetch (\DISPLAYDATA DDOFFSETSCACHE) of DISPLAYDATA)
                    CHAR8CODE)
                LEFT)
        CURX))
(\PILOTBITBLT LOCAL1 0)
*********/


/************************************************************************/
/*                                                                      */
/*                              b l t c h a r                           */
/*                                                                      */
/*      BITBLT character images onto a display stream.                  */
/*                                                                      */
/*                                                                      */
/************************************************************************/
/********************************************************/
/*             Non-PIXRECT version of the code          */
/********************************************************/

void bltchar(LispPTR *args)
/*      args[0] :       PILOTBBT
 *      args[1] :       DISPLAYDATA
 *      args[2] :       CHAR8CODE
 *      args[3] :       CURX
 *      args[4] :       LEFT
 *      args[5] :       RIGHT
 */
{
  PILOTBBT *pbt;
  DISPLAYDATA *dspdata;
  int base;
#ifdef REALCURSOR
  int displayflg;
#endif
  int w, h;
  int backwardflg = 0, sx, dx, srcbpl, dstbpl, src_comp, op;
  DLword *srcbase, *dstbase;
  int gray = 0;
#ifdef NEWBITBLT
  int num_gray = 0, curr_gray_line = 0;
#endif

  pbt = (PILOTBBT *)NativeAligned4FromLAddr(((BLTC *)args)->pilotbbt);
  dspdata = (DISPLAYDATA *)NativeAligned4FromLAddr(((BLTC *)args)->displaydata);

  srcbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtsourcehi, pbt->pbtsourcelo));

  dstbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtdesthi, pbt->pbtdestlo));

  srcbpl = abs(pbt->pbtsourcebpl);
  dstbpl = abs(pbt->pbtdestbpl);
  h = pbt->pbtheight;
  w = ((BLTC *)args)->right - ((BLTC *)args)->left;
  if ((h <= 0) || (w <= 0)) return;

  base = GETWORD(NativeAligned2FromLAddr(dspdata->ddoffsetscache + ((BLTC *)args)->char8code));
  sx = base + ((BLTC *)args)->left - ((BLTC *)args)->curx;
  dx = ((BLTC *)args)->left;

#ifdef REALCURSOR
  /* if displayflg != 0 then source or destination is DisplayBitMap
   * Now we consider about only destination
   */
  displayflg = cursorin(pbt->pbtdesthi, (pbt->pbtdestlo + (((BLTC *)args)->left >> 4)),
                        (((BLTC *)args)->right - ((BLTC *)args)->left), pbt->pbtheight, pbt->pbtbackward);
#endif /* REALCURSOR */

  op = pbt->pbtoperation;
  src_comp = pbt->pbtsourcetype;

  LOCKSCREEN;

#ifdef REALCURSOR
  if (displayflg) HideCursor;
#endif /* REALCURSOR */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, op, gray, num_gray,
         curr_gray_line);
#else
  new_char_bitblt_code;
#endif

#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

    if (in_display_segment(dstbase)) { flush_display_lineregion(dx, dstbase, w, h); }
#endif

#ifdef XWINDOW
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* XWINDOW */

#ifdef SDL
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* SDL */

#ifdef DOS
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* DOS */

#ifdef REALCURSOR
  if (displayflg) ShowCursor;
#endif /* REALCURSOR */

  UNLOCKSCREEN;
}


/************************************************************************/
/*                                                                      */
/*                          n e w b l t c h a r                         */
/*                                                                      */
/*      BITBLT character images onto a display stream.  This version    */
/*      handles clipping in the C code, and checks for changes of       */
/*      character set and hitting the right margin.  In either of       */
/*      those cases, it punts to the old LISP code for BLTCHAR. It      */
/*      also punts if the display stream we're writing to isn't for     */
/*      the top window on the screen's window stack.                    */
/*                                                                      */
/************************************************************************/

#define BLTCHAR_argnum 3
#ifndef INIT
#define PUNT_TO_BLTCHAR                                                                 \
  do {                                                                                     \
    if ((BLTCHAR_index == 0)) {                                                         \
      BLTCHAR_index = get_package_atom("\\MAIKO.PUNTBLTCHAR", 18, "INTERLISP", 9, NIL); \
      if (BLTCHAR_index == 0xffffffff) {                                                \
        error("\\MAIKO.PUNTBLTCHAR install failed");                                    \
        return;                                                                         \
      }                                                                                 \
    }                                                                                   \
    CurrentStackPTR += (BLTCHAR_argnum - 1) * DLWORDSPER_CELL;                          \
    ccfuncall(BLTCHAR_index, BLTCHAR_argnum, 3);                                        \
    return;                                                                             \
  } while (0)
#else
#define PUNT_TO_BLTCHAR                                                                 \
  do { /* Version that is silent instead of erroring for init */                           \
    if ((BLTCHAR_index == 0)) {                                                         \
      BLTCHAR_index = get_package_atom("\\MAIKO.PUNTBLTCHAR", 18, "INTERLISP", 9, NIL); \
      if (BLTCHAR_index == 0xffffffff) {                                                \
        /*   error("\\MAIKO.PUNTBLTCHAR install failed");      */                       \
        return;                                                                         \
      }                                                                                 \
    }                                                                                   \
    CurrentStackPTR += (BLTCHAR_argnum - 1) * DLWORDSPER_CELL;                          \
    ccfuncall(BLTCHAR_index, BLTCHAR_argnum, 3);                                        \
    return;                                                                             \
  } while (0)
#endif /* INIT */

#define TEDIT_BLTCHAR_argnum 6
#define PUNT_TO_TEDIT_BLTCHAR                                                             \
  do {                                                                                       \
    if (TEDIT_BLTCHAR_index == 0xffffffff) {                                              \
      TEDIT_BLTCHAR_index = get_package_atom("\\TEDIT.BLTCHAR", 14, "INTERLISP", 9, NIL); \
      if (TEDIT_BLTCHAR_index == 0xffffffff) {                                            \
        error("TEDIT install failed");                                                    \
        return;                                                                           \
      }                                                                                   \
    }                                                                                     \
    CurrentStackPTR += (TEDIT_BLTCHAR_argnum - 1) * DLWORDSPER_CELL;                      \
    ccfuncall(TEDIT_BLTCHAR_index, TEDIT_BLTCHAR_argnum, 3);                              \
    return;                                                                               \
  } while (0)

#define FGetNum(ptr, place)                     \
  do {                                          \
    if (((ptr)&SEGMASK) == S_POSITIVE) {        \
      (place) = ((ptr)&0xffff);                 \
    } else if (((ptr)&SEGMASK) == S_NEGATIVE) { \
      (place) = (int)((ptr) | 0xffff0000);      \
    } else {                                    \
      PUNT_TO_BLTCHAR;                          \
    }                                           \
  } while (0)
#if 0
/* see changecharset_display and sfffixy */
#define FGetNum2(ptr, place)                    \
  do {                                          \
    if (((ptr)&SEGMASK) == S_POSITIVE) {        \
      (place) = ((ptr)&0xffff);                 \
    } else if (((ptr)&SEGMASK) == S_NEGATIVE) { \
      (place) = (int)((ptr) | 0xffff0000);      \
    } else {                                    \
      return (-1);                              \
    }                                           \
  } while (0)
#endif

LispPTR *TOPWDS68k;          /* Top of window stack's DS */
LispPTR BLTCHAR_index;       /* Atom # for \PUNTBLTCHAR punt fn */
LispPTR TEDIT_BLTCHAR_index; /* if NIL ,TEDIT is not yet loaded */



/************************************************************************/
/*                                                                      */
/*                          n e w b l t c h a r                                 */
/*                         (non-PIXRECT version)                        */
/*                                                                      */
/************************************************************************/

void newbltchar(LispPTR *args) {
  DISPLAYDATA *displaydata68k;
  int right, left, curx;
  PILOTBBT *pbt;
  int lmargin, rmargin, xoff;
  int base;
  int h, w;
#ifdef REALCURSOR
  int displayflg;
#endif
  int backwardflg = 0, sx, dx, srcbpl, dstbpl, src_comp, op;
  DLword *srcbase, *dstbase;
  int gray = 0;
#ifdef NEWBITBLT
  int num_gray = 0, curr_gray_line = 0;
#endif

  displaydata68k = (DISPLAYDATA *)NativeAligned4FromLAddr(((BLTARG *)args)->displaydata);

  if ((displaydata68k->ddcharset & 0xFFFF) != ((BLTARG *)args)->charset) {
    /*if(changecharset_display(displaydata68k, ((BLTARG *)args)->charset) ==-1)*/
    PUNT_TO_BLTCHAR;
  }

  if (displaydata68k->ddslowprintingcase) { PUNT_TO_BLTCHAR; /** \SLOWBLTCHAR--return;**/ }

  FGetNum(displaydata68k->ddxposition, curx);
  FGetNum(displaydata68k->ddrightmargin, rmargin);
  FGetNum(displaydata68k->ddleftmargin, lmargin);
  FGetNum(displaydata68k->ddxoffset, xoff);

  right =
      curx +
      GETWORD((DLword *)NativeAligned2FromLAddr(displaydata68k->ddcharimagewidths + ((BLTARG *)args)->char8code));

  if ((right > rmargin) && (curx > lmargin)) PUNT_TO_BLTCHAR;
  if (((BLTARG *)args)->displaystream != *TOPWDS68k) PUNT_TO_BLTCHAR;

  {
    int newpos;
    newpos = curx +
             GETWORD((DLword *)NativeAligned2FromLAddr(displaydata68k->ddwidthscache + ((BLTARG *)args)->char8code));

    if ((0 <= newpos) && (newpos < 65536))
      (displaydata68k->ddxposition) = (LispPTR)(S_POSITIVE | newpos);
    else if (-65537 < newpos)
      (displaydata68k->ddxposition) = (LispPTR)(S_NEGATIVE | (0xffff & newpos));
    else {
      PUNT_TO_BLTCHAR;
    }
  }

  curx += xoff;
  right += xoff;
  if (right > (int)(displaydata68k->ddclippingright)) right = displaydata68k->ddclippingright;

  if (curx > (int)(displaydata68k->ddclippingleft))
    left = curx;
  else
    left = displaydata68k->ddclippingleft;

  pbt = (PILOTBBT *)NativeAligned4FromLAddr(displaydata68k->ddpilotbbt);
  h = pbt->pbtheight;
  w = right - left;
  if ((h <= 0) || (w <= 0)) return;

  srcbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtsourcehi, pbt->pbtsourcelo));

  dstbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtdesthi, pbt->pbtdestlo));

  op = pbt->pbtoperation;
  src_comp = pbt->pbtsourcetype;

  srcbpl = abs(pbt->pbtsourcebpl);
  dstbpl = abs(pbt->pbtdestbpl);
  base = GETWORD(NativeAligned2FromLAddr(displaydata68k->ddoffsetscache + ((BLTARG *)args)->char8code));
  sx = base + left - curx;
  dx = left;

  LOCKSCREEN;

#ifdef REALCURSOR
  displayflg = (cursorin(pbt->pbtdesthi, (pbt->pbtdestlo + (left >> 4)), (right - left),
                         pbt->pbtheight, pbt->pbtbackward));
  if (displayflg) HideCursor;
#endif /* REALCURSOR */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, op, gray, num_gray,
         curr_gray_line);
#else
  new_char_bitblt_code;
#endif

#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

    if (in_display_segment(dstbase)) {
      /*      DBPRINT(("newbltchar:  x %d, y 0x%x, w %d, h %d.\n", dx, dstbase, w, h));*/
      flush_display_lineregion(dx, dstbase, w, h);
    }
#endif

#ifdef XWINDOW
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* XWINDOW */
#ifdef SDL
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* SDL */
#ifdef DOS
  if (in_display_segment(dstbase)) flush_display_lineregion(dx, dstbase, w, h);
#endif /* DOS */

#ifdef REALCURSOR
  if (displayflg) ShowCursor;
#endif /* REALCURSOR */

  UNLOCKSCREEN;

} /* end of newbltchar */


/******************************************************************/
#ifndef BYTESWAP
#ifdef BIGVM
typedef struct {
  LispPTR FONTDEVICE;
  LispPTR FONTFAMILY;
  LispPTR FONTSIZE;
  LispPTR FONTFACE;
  DLword SFAscent;
  DLword SFDescent;
  DLword SFHeight;
  DLword ROTATION;
  short FBBOX;
  short FBBOY;
  short FBBDX;
  short FBBDY;
  LispPTR SFLKerns;
  LispPTR SFRWidths;
  LispPTR FONTDEVICESPEC;
  LispPTR OTHERDEVICEFONTPROPS;
  LispPTR FONTSCALE;
  unsigned SFFACECODE : 8;
  unsigned nil : 8;
  DLword FONTAVGCHARWIDTH;
  LispPTR FONTIMAGEWIDTHS;
  LispPTR FONTCHARSETVECTOR;
  LispPTR FONTEXTRAFIELD2;
} FONTDESC;
#else
typedef struct {
  LispPTR FONTDEVICE;
  LispPTR SFObsolete1;
  LispPTR FONTFAMILY;
  LispPTR FONTSIZE;
  LispPTR FONTFACE;
  LispPTR SFObsolete2;
  LispPTR SFObsolete3;
  LispPTR SFObsolete4;
  DLword SFObsolete5;
  DLword SFObsolete6;
  DLword SFAscent;
  DLword SFDescent;
  DLword SFHeight;
  DLword ROTATION;
  short FBBOX;
  short FBBOY;
  short FBBDX;
  short FBBDY;
  unsigned SFFACECODE : 8;
  unsigned SFLKerns : 24;
  LispPTR SFRWidths;
  LispPTR FONTDEVICESPEC;
  LispPTR OTHERDEVICEFONTPROPS;
  LispPTR FONTSCALE;
  DLword FONTAVGCHARWIDTH;
  DLword dum;
  LispPTR FONTIMAGEWIDTHS;
  LispPTR FONTCHARSETVECTOR;
  LispPTR FONTEXTRAFIELD2;
} FONTDESC;
#endif /* BIGVM */

typedef struct {
  LispPTR WIDTHS;
  LispPTR OFFSETS;
  LispPTR IMAGEWIDTHS;
  LispPTR CHARSETBITMAP;
  LispPTR YWIDTHS;
  DLword CHARSETASCENT;
  DLword CHARSETDESCENT;
  LispPTR LEFTKERN;
} CHARSETINFO;
#else
typedef struct {
  LispPTR FONTDEVICE;
  LispPTR SFObsolete1;
  LispPTR FONTFAMILY;
  LispPTR FONTSIZE;
  LispPTR FONTFACE;
  LispPTR SFObsolete2;
  LispPTR SFObsolete3;
  LispPTR SFObsolete4;
  DLword SFObsolete6;
  DLword SFObsolete5;
  DLword SFDescent;
  DLword SFAscent;
  DLword ROTATION;
  DLword SFHeight;
  short FBBOY;
  short FBBOX;
  short FBBDY;
  short FBBDX;
  unsigned SFLKerns : 24;
  unsigned SFFACECODE : 8;
  LispPTR SFRWidths;
  LispPTR FONTDEVICESPEC;
  LispPTR OTHERDEVICEFONTPROPS;
  LispPTR FONTSCALE;
  DLword dum;
  DLword FONTAVGCHARWIDTH;
  LispPTR FONTIMAGEWIDTHS;
  LispPTR FONTCHARSETVECTOR;
  LispPTR FONTEXTRAFIELD2;
} FONTDESC;

typedef struct {
  LispPTR WIDTHS;
  LispPTR OFFSETS;
  LispPTR IMAGEWIDTHS;
  LispPTR CHARSETBITMAP;
  LispPTR YWIDTHS;
  DLword CHARSETDESCENT;
  DLword CHARSETASCENT;
  LispPTR LEFTKERN;
} CHARSETINFO;
#endif /* BYTESWAP */

#define IMIN(x, y) (((x) > (y)) ? (y) : (x))
#define IMAX(x, y) (((x) > (y)) ? (x) : (y))

/** changecharset_display,sfffixy are not tested *****I don't use TAKE **/
#if 0
static LispPTR sfffixy(DISPLAYDATA *displaydata68k, CHARSETINFO *csinfo68k, PILOTBBT *pbt68k)

{
  int y;
  int chartop, top;
  BITMAP *bm68k;
  LispPTR base, ypos, yoff;

  FGetNum2(displaydata68k->ddyoffset, yoff);
  FGetNum2(displaydata68k->ddyposition, ypos);

  y = ypos + yoff;

  displaydata68k->ddcharsetascent = csinfo68k->CHARSETASCENT;
  chartop = y + displaydata68k->ddcharsetascent;

  bm68k = (BITMAP *)NativeAligned4FromLAddr(displaydata68k->dddestination);
  base = bm68k->bmbase;
  top = IMAX(IMIN(displaydata68k->ddclippingtop, chartop), 0);
  base = base + (bm68k->bmrasterwidth * (bm68k->bmheight - top));
  pbt68k->pbtdesthi = base >> 16;
  pbt68k->pbtdestlo = base;

  bm68k = (BITMAP *)NativeAligned4FromLAddr(csinfo68k->CHARSETBITMAP);
  base = bm68k->bmbase;
  displaydata68k->ddcharheightdelta = IMIN(IMAX(chartop - top, 0), 65535); /* always positive */
  base = base + bm68k->bmrasterwidth * displaydata68k->ddcharheightdelta;
  pbt68k->pbtsourcehi = base >> 16;
  pbt68k->pbtsourcelo = base;

  displaydata68k->ddcharsetdescent = csinfo68k->CHARSETDESCENT;

  pbt68k->pbtheight =
      IMAX(0, top - (IMAX(y - displaydata68k->ddcharsetdescent, displaydata68k->ddclippingbottom)));
  return (T);

} /* sfffixy */

static LispPTR changecharset_display(DISPLAYDATA *displaydata68k, DLword charset) {
  PILOTBBT *pbt68k;
  FONTDESC *fontd68k;
  LispPTR csinfo;
  CHARSETINFO *csinfo68k;
  BITMAP *bm68k;
  LispPTR *base68k;

  pbt68k = (PILOTBBT *)NativeAligned4FromLAddr(displaydata68k->ddpilotbbt);
  fontd68k = (FONTDESC *)NativeAligned4FromLAddr(displaydata68k->ddfont);
  base68k = (LispPTR *)NativeAligned4FromLAddr(fontd68k->FONTCHARSETVECTOR);

  if ((csinfo = *(base68k + charset)) == NIL) { return (-1); /* punt case */ }
  csinfo68k = (CHARSETINFO *)NativeAligned4FromLAddr(csinfo);
  /* REF CNT */

  FRPLPTR(displaydata68k->ddwidthscache, csinfo68k->WIDTHS);
  FRPLPTR(displaydata68k->ddoffsetscache, csinfo68k->OFFSETS);
  FRPLPTR(displaydata68k->ddcharimagewidths, csinfo68k->IMAGEWIDTHS);
  displaydata68k->ddcharset = charset;

  bm68k = (BITMAP *)NativeAligned4FromLAddr(csinfo68k->CHARSETBITMAP);

  pbt68k->pbtsourcebpl = (bm68k->bmrasterwidth) << 4;

  if ((displaydata68k->ddcharsetascent != csinfo68k->CHARSETASCENT) ||
      (displaydata68k->ddcharsetdescent != csinfo68k->CHARSETDESCENT)) {
    printf("CCD1\n");
    return (sfffixy(displaydata68k, csinfo68k, pbt68k));
  } else {
    LispPTR addr;
    int num;
    FGetNum2(displaydata68k->ddcharheightdelta, num); /* if not number, return -1 */
    addr = bm68k->bmbase + (bm68k->bmrasterwidth * num);
    printf("CCD2 num=%d\n", num);
    pbt68k->pbtsourcehi = addr >> 16;
    pbt68k->pbtsourcelo = addr; /* don't care REFCNT */
    return (T);
  }
} /* changecharset_display */
#endif
/******************************************************************/

void ccfuncall(unsigned int atom_index, int argnum, int bytenum)
 /* Atomindex for Function you want to invoke */
 /* Number of ARGS on TOS and STK */
 /* Number of bytes of Caller's OPCODE(including multi-byte) */
{
  struct definition_cell *defcell68k; /* Definition Cell PTR */
  short pv_num;                       /* scratch for pv */
  struct fnhead *tmp_fn;
  int rest; /* use for alignments */

  /* Get Next Block offset from argnum */
  CURRENTFX->nextblock = (LAddrFromNative(CurrentStackPTR) & 0x0ffff) - (argnum << 1) + 4 /* +3  */;

  /* Setup IVar */ /* XXX: is it really only 2-byte aligned? */
  IVar = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

  /* Set PC to the Next Instruction and save into FX */
  CURRENTFX->pc = ((UNSIGNED)PC - (UNSIGNED)FuncObj) + bytenum;

  PushCStack; /* save TOS */

  /* Get DEFCELL 68k address */
  defcell68k = (struct definition_cell *)GetDEFCELL68k(atom_index);

  tmp_fn = (struct fnhead *)NativeAligned4FromLAddr(defcell68k->defpointer);

  if ((UNSIGNED)(CurrentStackPTR + tmp_fn->stkmin + STK_SAFE) >= (UNSIGNED)EndSTKP) {
    LispPTR test;
    test = *((LispPTR *)CurrentStackPTR);
    DOSTACKOVERFLOW(argnum, bytenum - 1);
    S_CHECK(test == *((LispPTR *)CurrentStackPTR), "stack overflow in ccfuncall");
  }
  FuncObj = tmp_fn;

  if (FuncObj->na >= 0) {
    /* This Function is Spread Type */
    /* Arguments on Stack Adjustment  */
    rest = argnum - FuncObj->na;

    while (rest < 0) {
      PushStack(NIL_PTR);
      rest++;
    }
    CurrentStackPTR -= (rest << 1);
  } /* if end */

  /* Set up BF */
  CurrentStackPTR += 2;
  GETWORD(CurrentStackPTR) = BF_MARK;
  GETWORD(CurrentStackPTR + 1) = CURRENTFX->nextblock;
  CurrentStackPTR += 2;

  /* Set up FX */
  GETWORD(CurrentStackPTR) = FX_MARK;

  /* Now SET new FX */
  ((struct frameex1 *)CurrentStackPTR)->alink = LAddrFromNative(PVar);
  PVar = (DLword *)CurrentStackPTR + FRAMESIZE;
#ifdef BIGVM
  ((struct frameex1 *)CurrentStackPTR)->fnheader = (defcell68k->defpointer);
#else
  ((struct frameex1 *)CurrentStackPTR)->lofnheader = (defcell68k->defpointer) & 0x0ffff;
  ((struct frameex1 *)CurrentStackPTR)->hi2fnheader = ((defcell68k->defpointer) & SEGMASK) >> 16;
#endif /* BIGVM */
  CurrentStackPTR = PVar;

  /* Set up PVar area */
  pv_num = FuncObj->pv + 1; /* Changed Apr.27 */

  while (pv_num > 0) {
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    pv_num--;
  }

  /* Set PC points New Function's first OPCODE */
  PC = (ByteCode *)FuncObj + FuncObj->startpc;
  /* It assume that ccfuncall is called  for PUNTing from SUBRCALL */
  PC -= 3; /* Ajust PC */

} /* end ccfuncall */

/****************************************************************/
/*                                                              */
/*                        tedit_bltchar                         */
/*                                                              */
/*     C-coded version of TEdit's character-painting function   */
/*              \\TEDIT.BLTCHAR                                         */
/*                                                              */
/****************************************************************/

/***************************/
/*   Non-PIXRECT version   */
/***************************/
void tedit_bltchar(LispPTR *args)
{
#define backwardflg 0
  DISPLAYDATA *displaydata68k;
  int right;
  PILOTBBT *pbt;
  int imagewidth, newx;
  /* for new_char_bitblt_code */
  int h, w;
  int sx, dx, srcbpl, dstbpl, src_comp, op;
  DLword *srcbase, *dstbase;
  int gray = 0;
#ifdef NEWBITBLT
  int num_gray = 0, curr_gray_line = 0;
#endif

  displaydata68k = (DISPLAYDATA *)NativeAligned4FromLAddr(((TBLTARG *)args)->displaydata);
  if (displaydata68k->ddcharset != ((TBLTARG *)args)->charset) {
    /**if(changecharset_display(displaydata68k, ((TBLTARG *)args)->charset)== -1)**/
    { PUNT_TO_TEDIT_BLTCHAR; }
  }
  imagewidth = *((DLword *)NativeAligned2FromLAddr(displaydata68k->ddcharimagewidths + ((TBLTARG *)args)->char8code));
  newx = ((TBLTARG *)args)->current_x + imagewidth;
  dx = ((TBLTARG *)args)->current_x;
  right = IMIN(newx, ((TBLTARG *)args)->clipright);

  if (dx < right) {
    pbt = (PILOTBBT *)NativeAligned4FromLAddr(displaydata68k->ddpilotbbt);
    h = pbt->pbtheight;
    srcbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtsourcehi, pbt->pbtsourcelo));

    dstbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtdesthi, pbt->pbtdestlo));
    srcbpl = abs(pbt->pbtsourcebpl);
    dstbpl = abs(pbt->pbtdestbpl);

    op = pbt->pbtoperation;
    src_comp = pbt->pbtsourcetype;

    /*dx=left;  I'll optimize  later*/
    sx = GETBASE(NativeAligned2FromLAddr(displaydata68k->ddoffsetscache), ((TBLTARG *)args)->char8code);
    w = IMIN(imagewidth, (right - dx));
#ifdef NEWBITBLT
    bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, op, gray,
           num_gray, curr_gray_line);
#else

    new_char_bitblt_code;
#endif
  }
#undef backwardflg

} /* end tedit_bltchar */

#if defined(REALCURSOR)
#ifndef COLOR
/* Lisp addr hi-word, lo-word, ... */
static int old_cursorin(DLword addrhi, DLword addrlo, int x, int w, int h, int y, int backward)
{
#ifdef INIT
  init_kbd_startup;
#endif
  if (addrhi == DISPLAY_HI)
    y = addrlo / DisplayRasterWidth;
  else if (addrhi == DISPLAY_HI + 1)
    y = (addrlo + DLWORDSPER_SEGMENT) / DisplayRasterWidth;
  else
    return (NIL);

  if (backward) y -= h;

  if ((x < MOUSEXR) && (x + w > MOUSEXL) && (y < MOUSEYH) && (y + h > MOUSEYL))
    return (T);
  else
    return (NIL);
}

#else
/* For MONO and COLOR */
/* Lisp addr hi-word, lo-word, ... */
static int old_cursorin(DLword addrhi, DLword addrlo, int x, int w, int h, int y, int backward)
{
  DLword *base68k;
  extern int MonoOrColor;
  extern int displaywidth;
#ifdef INIT
  init_kbd_startup;
#endif

  if (MonoOrColor == MONO_SCREEN) {
    if (addrhi == DISPLAY_HI)
      y = addrlo / DisplayRasterWidth;
    else if (addrhi == DISPLAY_HI + 1)
      y = (addrlo + DLWORDSPER_SEGMENT) / DisplayRasterWidth;
    else
      return (NIL);

    if (backward) y -= h;

    if ((x < MOUSEXR) && (x + w > MOUSEXL) && (y < MOUSEYH) && (y + h > MOUSEYL))
      return (T);
    else
      return (NIL);
  } /* MONO case end */
  else {
    base68k = (DLword *)NativeAligned2FromLAddr(addrhi << 16 | addrlo);
    if ((ColorDisplayRegion68k <= base68k) && (base68k <= COLOR_MAX_Address)) {
      y = (base68k - ColorDisplayRegion68k) / displaywidth;
    } else
      return (NIL);

    if (backward) y -= h;
    /*  printf("old_c:x=%d,y=%d,w=%d,h=%d\n",x,y,w,h);*/
    if ((x < MOUSEXR) && ((x + (w >> 3)) > MOUSEXL) && (y < MOUSEYH) &&
        (y + h > MOUSEYL)) { /* printf("old_c T\n");*/
      return (T);
    } else
      return (NIL);
  } /* COLOR case end */
}
#endif /* COLOR */
#endif /* defined(REALCURSOR) */
