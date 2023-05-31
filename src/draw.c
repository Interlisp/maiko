/* $Id: draw.c,v 1.2 1999/01/03 02:06:56 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*				D R A W . C				*/
/*									*/
/*	Line-drawing code, N_OP_drawline.				*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stddef.h>       // for ptrdiff_t
#include <stdlib.h>       // for abs
#include "adr68k.h"       // for NativeAligned2FromLAddr
#include "bitblt.h"       // for MOUSEXR, MOUSEYH
#include "display.h"      // for DISPLAYBUFFER, DisplayRegion68k, in_display...
#include "drawdefs.h"     // for N_OP_drawline
#include "emlglob.h"
#include "initdspdefs.h"  // for flush_display_region
#include "lispemul.h"     // for DLword, BITSPER_DLWORD, SEGMASK, state, ERR...
#include "lispmap.h"      // for S_NEGATIVE, S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"     // for GETWORD

#ifdef DISPLAYBUFFER
extern struct pixrect *ColorDisplayPixrect, *DisplayRegionPixrect;
#endif

extern int displaywidth, displayheight;
extern int DisplayRasterWidth;
extern DLword *DisplayRegion68k_end_addr;

extern int ScreenLocked;

#ifdef COLOR
extern int MonoOrColor;
#endif /* COLOR */

#ifdef DISPLAYBUFFER
LispPTR n_new_cursorin_CG6(int dx, int dy, int w, int h)
{
  if ((dx < MOUSEXR) && (dx + w > MOUSEXL) && (dy < MOUSEYH) && (dy + h > MOUSEYL))
    return (T);
  else
    return (NIL);
}
#endif

/************************************************************
  #      name        len-1    stk level effect   UFN table entry
  73     DRAWLINE     0       -8                 \DRAWLINE.UFN

takes 9 (!) args from top of stack, does line draw inner loop
   Entry format:
     TOS  --> Maximum number of Y steps allowed {uYCount}
     SP-0 --> Maximum number of X steps allowed {uXCount}
     SP-2 --> Initial Delta {Delta}
     SP-4 --> Function number: {L2}
                0: Set pixel
                1: Reset Pixel
                2: Invert Pixel
     SP-6 --> Relative Y coordinate of end of line (for the bucket) {uYT}
     SP-8 --> Width of bitmap (in words), signed to indicate Y direction {uYIncHi , uYIncLo}
     SP-10 -> Relative X coordinate of end of line (for the bucket) {uXT}
     SP-12 -> bit index [0..15] of bit to start at {CurBit}
     SP-14 -> Pointer to first word in bitmap {uPAdrHi , uPAdrLo}


***********************************************************/
/*  curbit = bit position for next bit  */
/*  delta = current value of delta  */
/*  op = draw operation:  0/paint, 1/erase, 2/invert  */
/*****
plot:
        if x = xmax or y = ymax then exit
        data = data op curbit;
        if (ymax > xmax) then {movex(-), if delta neg, then movey(+), goto plot}
        else {movey(-), if delta neg, then movex(+), goto plot}

        movex = {
          xmax--
          delta = delta +/- ysize
          curbit >>= 1
          if curbit = 0 then {curbit = 0x8000, incx}
        }
        movey = {
          ymax--
          delta = delta +/- xsize
          incy
        }
*****/

/*
        ptr	pointer to "first word of bitmap"
        curbit	bit index to start with, and tracks therefrom.
        xsize	Rel X coord
        width	width of bitmap, in words
        ysize	Rel Y coord of end of line
        op	operation
        delta	initial delta
        numx	step count for X steps
        numy	step count for Y steps
*/

#ifdef XWINDOW
#define DISPLAYBUFFER
#endif /* XWINDOW */

int N_OP_drawline(LispPTR ptr, int curbit, int xsize, int width, int ysize, int op, int delta, int numx, int numy)
{
  DLword *dataptr;
  ScreenLocked = T;

#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */


  delta &= 0xFFFF;
  op &= 3;

  if ((SEGMASK & width) == S_POSITIVE)
    width &= 0xFFFF;
  else if ((SEGMASK & width) == S_NEGATIVE)
    width |= 0xFFFF0000;
  else
    ERROR_EXIT(numy);

  curbit = 0x8000 >> (curbit & 0xFFFF);
  dataptr = NativeAligned2FromLAddr(ptr);
  numy &= 0xFFFF;
  numx &= 0xFFFF;
  ysize &= 0xFFFF;
  xsize &= 0xFFFF;
  if (xsize > ysize) {
    delta = xsize - delta - 1;
    switch (op) {
      case 0:
        while (numx && numy) {
          if (!curbit) {
            curbit = 0x8000;
            dataptr++;
          } /* end if curbit */
          GETWORD(dataptr) |= curbit;
          numx--;
          delta -= ysize;
          curbit >>= 1;
          if (delta < 0) {
            numy--;
            delta += xsize;
            dataptr += width;
          } /* end if delta */
        }   /* end while */
        break;
      case 1:
        while (numx && numy) {
          if (!curbit) {
            curbit = 0x8000;
            dataptr++;
          } /* end if curbit */
          GETWORD(dataptr) &= ~curbit;
          numx--;
          delta -= ysize;
          curbit >>= 1;
          if (delta < 0) {
            numy--;
            delta += xsize;
            dataptr += width;
          } /* end if delta */
        }   /* end while */
        break;
      case 2:
        while (numx && numy) {
          if (!curbit) {
            curbit = 0x8000;
            dataptr++;
          }
          GETWORD(dataptr) ^= curbit;
          numx--;
          delta -= ysize;
          curbit >>= 1;
          if (delta < 0) {
            numy--;
            delta += xsize;
            dataptr += width;
          } /* end if delta */
        }   /* end while */
        break;
    }    /* end switch */
  }      /* end if */
  else { /* yfirst */
    delta = ysize - delta - 1;
    switch (op) {
      case 0:
        while (numx && numy) {
          GETWORD(dataptr) |= curbit;
          numy--;
          delta -= xsize;
          dataptr += width;
          if (delta < 0) {
            numx--;
            delta += ysize;
            curbit >>= 1;
            if (!curbit) {
              curbit = 0x8000;
              dataptr++;
            }
          } /* end if delta */
        }   /* end while */
        break;
      case 1:
        while (numx && numy) {
          GETWORD(dataptr) &= ~curbit;
          numy--;
          delta -= xsize;
          dataptr += width;
          if (delta < 0) {
            numx--;
            delta += ysize;
            curbit >>= 1;
            if (!curbit) {
              curbit = 0x8000;
              dataptr++;
            }
          } /* end if delta */
        }   /* end while */
        break;
      case 2:
        while (numx && numy) {
          GETWORD(dataptr) ^= curbit;
          numy--;
          delta -= xsize;
          dataptr += width;
          if (delta < 0) {
            numx--;
            delta += ysize;
            curbit >>= 1;
            if (!curbit) {
              curbit = 0x8000;
              dataptr++;
            }
          } /* end if delta */
        }   /* end while */
        break;
    } /* end switch */
  }   /* end else */
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */


#ifdef DISPLAYBUFFER
#ifdef COLOR
  if (MonoOrColor == MONO_SCREEN)
#endif /* COLOR */

  {
    DLword *start_addr;
    start_addr = (DLword *)NativeAligned2FromLAddr(ptr);

    if (in_display_segment(start_addr) && in_display_segment(dataptr)) {
      int start_x, start_y, end_x, end_y, w, h;
      ptrdiff_t temp_s, temp_e;

      temp_s = start_addr - DisplayRegion68k;
      temp_e = dataptr - DisplayRegion68k;

      start_y = temp_s / DisplayRasterWidth;
      start_x = (temp_s % DisplayRasterWidth) * BITSPER_DLWORD;

      end_y = temp_e / DisplayRasterWidth;
      end_x = (temp_e % DisplayRasterWidth) * BITSPER_DLWORD + (BITSPER_DLWORD - 1);

      w = abs(start_x - end_x) + 1;
      h = abs(start_y - end_y) + 1;

      if (start_x > end_x) start_x = end_x;
      if (start_y > end_y) start_y = end_y;

#if defined(XWINDOW) || defined(BYTESWAP)
      flush_display_region(start_x, start_y, w, h);
#endif /* XWINDOW */
    }
  }
#endif /* DISPLAYBUFFER */

  ScreenLocked = NIL;

  return (0); /* return a value for the error test to check. */

} /* end N_OP_drawline()  */

#ifdef XWINDOW
#undef DISPLAYBUFFER
#endif /* XWINDOW */

/* end module */
