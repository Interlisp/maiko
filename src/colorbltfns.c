/* $Id: colorbltfns.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ (C) Copyright Venue, All Rights
 * Reserved  */
static char *id = "$Id: colorbltfns.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	 C O L O R   B I T B L T / G R A P H I C S   S U P P O R T	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

#include <sys/types.h>
#include "lispemul.h"
#include "lispglobal.h"
#include "lispmap.h"
#include "lisptypes.h"
#include "address68k.h"
#include "address.h"
#include "stream.h"
#include "displaydata.h"
#include "pilotbbt.h"
#include "debug.h"
#include "arith.h"
#include "bitblt.h"
#include "lldisplay.h"

#define IMIN(x, y) (((x) > (y)) ? (y) : (x))
#define IMAX(x, y) (((x) > (y)) ? (x) : (y))

/*******************************/
/* A LISP Big-bitmap structure */
/*******************************/
typedef struct {
  LispPTR bigbmwidth;
  LispPTR bigbmheight;
  LispPTR bigbmlist;
} BIGBM;

#define GetNewFragment(list, frag, type)      \
  frag = (type)Addr68k_from_LADDR(car(list)); \
  list = cdr(list);

LispPTR SLOWBLTCHAR_index;
#define SLOWBLTCHAR_argnum 2
#define PUNT_TO_SLOWBLTCHAR                                                                \
  {                                                                                        \
    if (SLOWBLTCHAR_index == 0xffffffff)                                                   \
      SLOWBLTCHAR_index = get_package_atom("\\PUNT.SLOWBLTCHAR", 17, "INTERLISP", 9, NIL); \
    if (SLOWBLTCHAR_index == 0xffffffff) {                                                 \
      error("SLOWBLTCHAR install fail");                                                   \
      return;                                                                              \
    }                                                                                      \
    CurrentStackPTR += (SLOWBLTCHAR_argnum - 1) * DLWORDSPER_CELL;                         \
    ccfuncall(SLOWBLTCHAR_index, SLOWBLTCHAR_argnum, 3);                                   \
    return;                                                                                \
  }

/***********************************************************/
/*
                C_slowbltchar

                by Takeshi
                June 6, 1989
*/
/***********************************************************/

/* I'll merge this macro with  FGetNum later */
#define SFGetNum(ptr, place)                     \
  {                                              \
    if (((ptr)&0xff0000) == S_POSITIVE) {        \
      (place) = ((ptr)&0xffff);                  \
    } else if (((ptr)&0xff0000) == S_NEGATIVE) { \
      (place) = (int)((ptr) | 0xffff0000);       \
    } else {                                     \
      PUNT_TO_SLOWBLTCHAR;                       \
    }                                            \
  }

/* place:		native pointer
   val:		native value(should be smallp)
   puntcase:	punt descriptions */
#define FReplaceSmallp(place, val, puntcase)                       \
  {                                                                \
    if ((0 <= (val)) && ((val) <= MAX_SMALL))                      \
      (LispPTR)(place) = (LispPTR)(S_POSITIVE | (val));            \
    else if (MIN_SMALL <= val)                                     \
      (LispPTR)(place) = (LispPTR)(S_NEGATIVE | (0xffff & (val))); \
    else {                                                         \
      puntcase;                                                    \
    }                                                              \
  }

/* charcode should be pos. smallp */
#define charcode (args[0] & 0xffff)
#define displaystream args[1]
#define Char8Code(x) ((u_char)((x)&0xff))
#define CharSet(x) ((x) >> 8)

#define PSEUDO_BLACK 255
#define PSEUDO_WHITE 0

LispPTR *SCREENBITMAPS68k; /* Initialized in initsysout.c */
LispPTR *COLORSCREEN68k;   /* Initialized in initsysout.c */
LispPTR COLORSCREEN_index; /* if it's 0xffffffff, not yet initialized */

/************************************************************************/
/*									*/
/*			C _ s l o w b l t c h a r			*/
/*									*/
/*	 args[0]		charcode 				*/
/*	 args[1]		displaystream				*/
/*									*/
/*									*/
/************************************************************************/

C_slowbltchar(LispPTR *args)
{
  Stream *n_dstream;
  DISPLAYDATA *n_dd;
  FONTDESC *n_fontd;
  CHARSETINFO *n_csinfo;
  BITMAP *n_destbitmap;
  int dest_bpp;
  LispPTR csinfo;
  int curx, cury, newx, rmargin, lmargin, xoff, yoff;
  DLword cl_left, cl_right, cl_bottom, cl_top;
  DLword src_w, src_h, src_x, src_y, dst_x, dst_y, w, h;

  u_char forecolor, backcolor;
  register int displayflg;

  extern LispPTR *TOPWDS68k;

  n_dstream = (Stream *)Addr68k_from_LADDR(displaystream);
  n_dd = (DISPLAYDATA *)Addr68k_from_LADDR(n_dstream->IMAGEDATA);
  n_fontd = (FONTDESC *)Addr68k_from_LADDR(n_dd->ddfont);
  n_destbitmap = (BITMAP *)Addr68k_from_LADDR(n_dd->dddestination);

  if ((n_fontd->ROTATION & 0xffff) == 0) {
    if ((csinfo = *(((LispPTR *)Addr68k_from_LADDR(n_fontd->FONTCHARSETVECTOR)) +
                    CharSet(charcode))) == NIL)
      PUNT_TO_SLOWBLTCHAR; /* CSINFO is not cached */

    n_csinfo = (CHARSETINFO *)Addr68k_from_LADDR(csinfo);

    SFGetNum(n_dd->ddxposition, curx);
    SFGetNum(n_dd->ddyposition, cury);
    SFGetNum(n_dd->ddrightmargin, rmargin);
    SFGetNum(n_dd->ddleftmargin, lmargin);
    SFGetNum(n_dd->ddxoffset, xoff);
    SFGetNum(n_dd->ddyoffset, yoff);
    cl_left = n_dd->ddclippingleft;
    cl_right = n_dd->ddclippingright;
    cl_bottom = n_dd->ddclippingbottom;
    cl_top = n_dd->ddclippingtop;

    newx = curx + *(DLword *)Addr68k_from_LADDR(n_dd->ddwidthscache + Char8Code(charcode));

    if (newx > rmargin) PUNT_TO_SLOWBLTCHAR; /* do \DSPPRINTCR/LF */

    /* If we care about TOPW then it's too slow to create Menu etc.
       But,if we don't,it causes some error  **/
    {
      WINDOW *window;
      SCREEN *ColorScreenData;
      if (COLORSCREEN_index == 0xffffffff) { /* Make sure COLOR lives? */
        COLORSCREEN_index = MAKEATOM("\\COLORSCREEN");
        COLORSCREEN68k = (LispPTR *)Addr68k_from_LADDR(VALS_OFFSET + (COLORSCREEN_index << 1));
      }
      ColorScreenData = (SCREEN *)Addr68k_from_LADDR(*COLORSCREEN68k);
      window = (WINDOW *)Addr68k_from_LADDR(ColorScreenData->SCTOPW);
      if ((displaystream != ColorScreenData->SCTOPW) && (displaystream != window->DSP) &&
          (displaystream != *TOPWDS68k) && ((fmemb(n_dd->dddestination, *SCREENBITMAPS68k)) != NIL))
        PUNT_TO_SLOWBLTCHAR;
    }

    FReplaceSmallp(n_dd->ddxposition, newx, PUNT_TO_SLOWBLTCHAR);

    /* make curx abs coord */
    curx += xoff;
    cury += yoff;

    {
      register PILOTBBT *pbt;
      register BITMAP *n_destBM, *n_srcBM;
      register BIGBM *n_destBIGBM;
      register int destYOffset;
      register int width, sourceBitOffset;
      extern int ScreenLocked;
      extern int displayheight;

      n_srcBM = (BITMAP *)Addr68k_from_LADDR(n_csinfo->CHARSETBITMAP);
      src_h = n_srcBM->bmheight;
      src_w = n_srcBM->bmwidth;

      src_x = *((DLword *)Addr68k_from_LADDR(n_dd->ddoffsetscache + Char8Code(charcode)));
      src_y = 0;
      w = *(DLword *)Addr68k_from_LADDR(n_dd->ddcharimagewidths + Char8Code(charcode));
      h = src_h;

      (short)dst_x = (short)curx;
      (short)dst_y = (short)cury - (short)n_csinfo->CHARSETDESCENT;

      { /* clipping */
        short left, right, bottom, top;
        short stodx, stody;

        left = (short)IMAX((short)dst_x, (short)cl_left);
        right = (short)IMIN((short)dst_x + w, (short)cl_right);
        bottom = (short)IMAX((short)dst_y, (short)cl_bottom);
        top = (short)IMIN((short)dst_y + h, (short)cl_top);
        stodx = (short)dst_x - (short)src_x;
        stody = (short)dst_y - (short)src_y;
        left = IMAX((short)src_x, IMAX(left - stodx, 0));
        bottom = IMAX((short)src_y, IMAX(bottom - stody, 0));
        right = IMIN((short)src_w, IMIN((short)(src_x + w), right - stodx));
        top = IMIN((short)src_h, IMIN((short)(src_y + h), top - stody));
        if ((right <= left) || (top <= bottom)) return;
        w = (DLword)(right - left);
        h = (DLword)(top - bottom);
        dst_x = (DLword)(left + stodx);
        dst_y = (DLword)(bottom + stody);
        src_x = (DLword)left;
        src_y = (DLword)bottom;
      }

      /* forground and bacground color */

      if (n_dd->ddcolor == NIL_PTR) {
        forecolor = PSEUDO_BLACK;
        backcolor = PSEUDO_WHITE;
      } else {
        backcolor = 0xff & cdr(n_dd->ddcolor);
        forecolor = 0xff & car(n_dd->ddcolor);
      }

      if (GetTypeNumber(n_dd->dddestination) == TYPE_BITMAP) { /* Bitap */
        n_destBM = (BITMAP *)Addr68k_from_LADDR(n_dd->dddestination);
        ScreenLocked = T;
        /*  xposition is shifted 3 Kludge for cursorin
               in color(8bpp) ** x's meaning  is different from
              bitbltsub's. For now,I use this func with Kludge */
        displayflg = n_new_cursorin(Addr68k_from_LADDR(n_destBM->bmbase), dst_x << 3,
                                    /* Kludge:YCoordination upside down*/
                                    displayheight - cury, w, h);
        if (displayflg) HideCursor;

        ColorizeFont8(n_srcBM, src_x, src_y, n_destBM, dst_x, dst_y, w, h, backcolor, forecolor,
                      n_dd->ddsourcetype, n_dd->ddoperation);

        if (displayflg) ShowCursor;
        ScreenLocked = NIL;

      } else { /* BIGBM */
        ScreenLocked = T;
        n_destBIGBM = (BIGBM *)n_destbitmap;
        ColorizeFont8_BIGBM(n_srcBM, src_x, src_y, n_destBIGBM, dst_x, dst_y, w, h, backcolor,
                            forecolor, n_dd->ddsourcetype, n_dd->ddoperation);
        ScreenLocked = NIL;
      } /* end if( TYPE_BITMAP ) */
    }
  } else {
    /* ROTATE case ,do-PUNT */
    PUNT_TO_SLOWBLTCHAR;
  }

} /* end C_slowbltchar */

/************************************************************************/
/*									*/
/*			C o l o r i z e d F o n t 8			*/
/*									*/
/*	Expand a 1bpp font to 8bpp colorized font bitmap, using		*/
/*	col1 and col0 as the colors for 1 bits & 0 bits, respectively.	*/
/*									*/
/*	dbm must be the output bitmap (not a bigbm), whose bpp is 8.	*/
/*									*/
/************************************************************************/

#define MAXFONTHEIGHT 48
#define MAXFONTWIDTH 48
#define BITSPERNIBBLE 4
#define BITSPERDLWORD 16
#define BPP 8

u_int ColorizedFont8CACHE[MAXFONTHEIGHT / BITSPERNIBBLE * MAXFONTWIDTH / BITSPERNIBBLE];

ColorizeFont8(BITMAP *sBM, DLword sXOffset, DLword sYOffset, BITMAP *dBM, DLword dXOffset, DLword dYOffset, DLword width, DLword height, u_char col0, u_char col1,
              LispPTR sourcetype, LispPTR operation)
{
  register DLword *nbase;
  register u_char *dbase;
  register int i;

  sYOffset = sBM->bmheight - (sYOffset + height);
  dYOffset = dBM->bmheight - (dYOffset + height);

  nbase = (DLword *)Addr68k_from_LADDR(sBM->bmbase) + (sBM->bmrasterwidth * sYOffset);
  (DLword *)dbase = (DLword *)Addr68k_from_LADDR(dBM->bmbase) + (dBM->bmrasterwidth * dYOffset);
  for (i = 0, dbase += dXOffset; /* 8bpp */
       i < height; i++, nbase += sBM->bmrasterwidth, ((DLword *)dbase) += dBM->bmrasterwidth) {
    lineBlt8(nbase, (int)sXOffset, dbase, (int)width, col0, col1, sourcetype, operation);
  } /* for end */

} /* ColorizeFont8 end */

/************************************************************************/
/*									*/
/*		C o l o r i z e d F o n t 8 _ B I G B M			*/
/*									*/
/*	Expand a 1bpp font to 8bpp colorized font bitmap, using		*/
/*	col1 and col0 as the colors for 1 bits & 0 bits, respectively.	*/
/*									*/
/*	dbm must be the output BIGBM (not a bitmap), whose bpp is 8.	*/
/*									*/
/************************************************************************/

ColorizeFont8_BIGBM(BITMAP *sBM, DLword sXOffset, DLword sYOffset, BIGBM *dBM, DLword dXOffset, DLword dYOffset, DLword width, DLword height, u_char col0, u_char col1,
              LispPTR sourcetype, LispPTR operation)
{
  register DLword *nbase;
  register u_char *dbase;
  register int i;
  int dest_bottom, dest_bigbmheight, dest_fragtop, dest_fragbottom, dest_yoffset, dest_h;

  LispPTR dest_bmlist;
  BITMAP *dest_frag;

  SFGetNum(dBM->bigbmheight, dest_bigbmheight);

  sYOffset = sBM->bmheight - (sYOffset + height);
  dYOffset = dest_bigbmheight - (dYOffset + height);

  dest_bottom = dYOffset + height;
  dest_bmlist = (LispPTR)dBM->bigbmlist;
  GetNewFragment(dest_bmlist, dest_frag, BITMAP *);
  dest_fragtop = 0;
  dest_fragbottom = dest_frag->bmheight;

  /* search fragment of bitmaps including the desitnation top. */
  while (dest_fragbottom <= dYOffset) {
    GetNewFragment(dest_bmlist, dest_frag, BITMAP *);
    if (dest_frag == (BITMAP *)Addr68k_from_LADDR(NIL_PTR)) return;
    dest_fragtop = dest_fragbottom;
    dest_fragbottom += dest_frag->bmheight;
  } /* end while */

  /* y offset form bitmap top. */
  dest_yoffset = dYOffset - dest_fragtop;

loop:
  /* height of lineBlt8 */
  if (dest_fragbottom > dest_bottom) {
    /* this fragment inludes dest bottom. */
    dest_h = dest_bottom - (dest_fragtop + dest_yoffset);
  } else {
    /* remaining fragments include dest bottom. */
    dest_h = dest_fragbottom - (dest_fragtop + dest_yoffset);
  } /* end if */

  dbase =
      (DLword *)Addr68k_from_LADDR(dest_frag->bmbase) + (dest_frag->bmrasterwidth * dest_yoffset);
  nbase = (DLword *)Addr68k_from_LADDR(sBM->bmbase) + (sBM->bmrasterwidth * sYOffset);

  sYOffset += (DLword)dest_h; /* next src yoffset */

  for (i = 0, dbase += dXOffset; i < dest_h;
       i++, nbase += sBM->bmrasterwidth, ((DLword *)dbase) += dest_frag->bmrasterwidth) {
    lineBlt8(nbase, (int)sXOffset, dbase, (int)width, col0, col1, sourcetype, operation);
  }

  /* remaining height */
  height -= dest_h;
  if (height > 0) {
    GetNewFragment(dest_bmlist, dest_frag, BITMAP *);
    if (dest_frag != (BITMAP *)Addr68k_from_LADDR(NIL_PTR)) {
      dest_fragtop = dest_fragbottom;
      dest_fragbottom = dest_fragtop + dest_frag->bmheight;
      dest_yoffset = 0; /* y offset must be zero. */
      nbase += sBM->bmrasterwidth;
      goto loop;
    } /* end if(dest_frag) */
  }   /* end if(height) */

} /* end ColorizeFont8_BIGBM() */

/************************************************************************/
/*									*/
/*			n e w C o l o r i z e F o n t 8			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

newColorizeFont8(PILOTBBT *pbt, u_char backcolor, u_char forecolor, LispPTR srctype, LispPTR ope)
{
  register DLword *nbase;
  register u_char *dbase;
  register int i;

  nbase = (DLword *)Addr68k_from_LADDR((pbt->pbtsourcehi << 16) | (pbt->pbtsourcelo));
  (DLword *)dbase = (DLword *)Addr68k_from_LADDR((pbt->pbtdesthi << 16) | (pbt->pbtdestlo));
  dbase += pbt->pbtdestbit;
  for (i = 0; i < pbt->pbtheight;
       i++, nbase += pbt->pbtsourcebpl / 16, dbase += pbt->pbtdestbpl / 8) {
    lineBlt8(nbase, pbt->pbtsourcebit, dbase, pbt->pbtwidth, backcolor, forecolor, srctype, ope);
  } /* for end */
}

/************************************************************************/
/*									*/
/*			U n c o l o r i z e _ B i t m a p		*/
/*									*/
/*	Uncolorize bitmap from 8bpp to 1bpp.				*/
/*									*/
/************************************************************************/

Uncolorize_Bitmap(LispPTR args[])
{
  BITMAP *s_bitmap, *d_bitmap;
  register DLword *OnOff;

  register u_char *s_base;
  register DLword *d_base;
  register int y;
  int s_height, s_width, s_bitsperpixel, s_rasterwidth, d_rasterwidth;

  s_bitmap = (BITMAP *)Addr68k_from_LADDR(args[0]);
  d_bitmap = (BITMAP *)Addr68k_from_LADDR(args[1]);
  OnOff = (DLword *)Addr68k_from_LADDR(args[2]);

  s_height = s_bitmap->bmheight;
  s_width = s_bitmap->bmwidth;
  s_bitsperpixel = s_bitmap->bmbitperpixel;

  if (s_bitsperpixel != 8) return;

  s_base = (u_char *)Addr68k_from_LADDR(s_bitmap->bmbase);
  d_base = (DLword *)Addr68k_from_LADDR(d_bitmap->bmbase);
  s_rasterwidth = s_bitmap->bmrasterwidth;
  d_rasterwidth = d_bitmap->bmrasterwidth;

  for (y = 0; y < s_height; y++) {
    register int x;
    register DLword word;
    word = 0;
    for (x = 0; x < s_width; x++) {
      if (OnOff[*(s_base + x)]) {
        switch (x & 0xF) {
          case 0: word |= 0x8000; break;
          case 1: word |= 0x4000; break;
          case 2: word |= 0x2000; break;
          case 3: word |= 0x1000; break;
          case 4: word |= 0x800; break;
          case 5: word |= 0x400; break;
          case 6: word |= 0x200; break;
          case 7: word |= 0x100; break;
          case 8: word |= 0x80; break;
          case 9: word |= 0x40; break;
          case 10: word |= 0x20; break;
          case 11: word |= 0x10; break;
          case 12: word |= 0x8; break;
          case 13: word |= 0x4; break;
          case 14: word |= 0x2; break;
          case 15:
            word |= 0x1;
            break;
          defualts:
            break;
        } /* end switch( x ) */
      }   /* end if( ) */
      if ((x & 0xF) == 0xF) {
        *(d_base++) = word;
        word = 0;
      } /* end if( x ) */
    }   /* end for( x ) */

    if ((x & 0xF) != 0) *(d_base++) = word;

    if (y != (s_height - 1)) { (DLword *)s_base += s_rasterwidth; } /* end if( y ) */

  } /* end for( y ) */

} /* end Uncolorize_Bitmap() */

/************************************************************************/
/*									*/
/*			C o l o r i z e _ B i t m a p			*/
/*									*/
/*	Colorize bitmap from 1bpp to 8bpp.				*/
/*									*/
/************************************************************************/

extern DLword INPUT_atom, REPLACE_atom;

Colorize_Bitmap(LispPTR args[])
{
  BITMAP *s_bitmap, *d_bitmap;
  int s_left, s_bottom, d_left, d_bottom, width, height, color0, color1, d_nbits;
  register DLword *s_base;
  register u_char *d_base;
  register int i;

  N_GETNUMBER(args[10], d_nbits, bad_arg);
  if (d_nbits != 8) return (NIL); /* do nothing. */

  s_bitmap = (BITMAP *)Addr68k_from_LADDR(args[0]);
  N_GETNUMBER(args[1], s_left, bad_arg);
  N_GETNUMBER(args[2], s_bottom, bad_arg);
  d_bitmap = (BITMAP *)Addr68k_from_LADDR(args[3]);
  N_GETNUMBER(args[4], d_left, bad_arg);
  N_GETNUMBER(args[5], d_bottom, bad_arg);
  N_GETNUMBER(args[6], width, bad_arg);
  N_GETNUMBER(args[7], height, bad_arg);
  N_GETNUMBER(args[8], color0, bad_arg);
  N_GETNUMBER(args[9], color1, bad_arg);

  s_base = (DLword *)Addr68k_from_LADDR(s_bitmap->bmbase) +
           s_bitmap->bmrasterwidth * (s_bitmap->bmheight - (s_bottom + height));
  (DLword *)d_base = (DLword *)Addr68k_from_LADDR(d_bitmap->bmbase) +
                     d_bitmap->bmrasterwidth * (d_bitmap->bmheight - (d_bottom + height));

  for (i = 0, d_base += d_left; i < height;
       i++, s_base += s_bitmap->bmrasterwidth, (DLword *)d_base += d_bitmap->bmrasterwidth) {
    lineBlt8(s_base, s_left, d_base, width, (u_char)color0, (u_char)color1, INPUT_atom,
             REPLACE_atom);

  } /* end for(i) */

bad_arg:
  return (NIL);

} /* end Colorize_Bitmap() */

/************************************************************************/
/*									*/
/*		    D r a w _ 8 B p p C o l o r L i n e			*/
/*									*/
/*	Draw a line in the  8-bpp display bitmap.			*/
/*									*/
/************************************************************************/

#define op_replace
#define op_erase &
#define op_invert ^
#define op_paint |

#define draw8bpplinex(op)                    \
  while ((x0 <= xlimit) && (y0 <= ylimit)) { \
    *(base++)op = color;                     \
    ++x0;                                    \
    cdl += dy;                               \
    if (dx <= cdl) {                         \
      cdl -= dx;                             \
      base += yinc;                          \
      ++y0;                                  \
    }                                        \
  }

#define draw8bppliney(op)                    \
  while ((x0 <= xlimit) && (y0 <= ylimit)) { \
    *base op = color;                        \
    base += yinc;                            \
    ++y0;                                    \
    cdl += dx;                               \
    if (dy <= cdl) {                         \
      cdl -= dy;                             \
      ++base;                                \
      ++x0;                                  \
    }                                        \
  }

Draw_8BppColorLine(LispPTR *args)
{
  extern DLword REPLACE_atom, INVERT_atom, PAINT_atom, ERASE_atom;

  register u_char color;
  register u_char *base;
  register short x0, y0, xlimit, ylimit, dx, dy, cdl, yinc, raster_width;
  int mode, displayflg;
  u_char *n_bmbase;

  x0 = (short)(args[0] & 0xffff);
  y0 = (short)(args[1] & 0xffff);
  xlimit = (short)(args[2] & 0xffff);
  ylimit = (short)(args[3] & 0xffff);
  dx = (short)(args[4] & 0xffff);
  dy = (short)(args[5] & 0xffff);
  cdl = (short)(args[6] & 0xffff);
  yinc = (short)(args[7] & 0xffff);
  yinc *= 2; /* for byte addressing */

  if (args[8] == PAINT_atom)
    mode = 3;
  else if (args[8] == INVERT_atom)
    mode = 2;
  else if (args[8] == ERASE_atom)
    mode = 1;
  else
    mode = 0; /* REPLACE_atom */

  n_bmbase = (u_char *)Addr68k_from_LADDR(args[9]);
  raster_width = (short)(args[10] & 0xffff);
  color = (u_char)(args[11] & 0xff);

  if (yinc >= 0) {
    displayflg = n_new_cursorin((DLword *)n_bmbase, (int)(x0 << 3), (int)y0, (int)(xlimit << 3),
                                (int)ylimit);
  } else {
    displayflg = n_new_cursorin((DLword *)n_bmbase, (int)(x0 << 3), (int)(y0 - ylimit),
                                (int)(xlimit << 3), (int)ylimit);
  }

  base = n_bmbase + y0 * (raster_width << 1) + x0;
  x0 = y0 = 0;

  if (displayflg) {
    ScreenLocked = T;
    HideCursor;
  }

  if (dx >= dy) { /* .draw8bpplinex */
    switch (mode) {
      case 0: draw8bpplinex(op_replace); break;
      case 1: draw8bpplinex(op_erase); break;
      case 2: draw8bpplinex(op_invert); break;
      case 3: draw8bpplinex(op_paint); break;
    } /* end switch */

  } else { /* .draw8bppliney */
    switch (mode) {
      case 0: draw8bppliney(op_replace); break;
      case 1: draw8bppliney(op_erase); break;
      case 2: draw8bppliney(op_invert); break;
      case 3: draw8bppliney(op_paint); break;
    } /* end switch */

  } /* end if( dx >= dy ) */

  if (displayflg) {
    ShowCursor;
    ScreenLocked = NIL;
  }

} /* end Draw_8BppColorLine */
