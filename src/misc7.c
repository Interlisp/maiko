/* $Id: misc7.c,v 1.2 1999/01/03 02:07:22 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*	misc7.c
 */
#include "adr68k.h"       // for NativeAligned2FromLAddr
#include "arith.h"        // for N_GETNUMBER
#include "bbtsubdefs.h"   // for n_new_cursorin
#include "dbprint.h"      // for DBPRINT
#include "display.h"      // for in_display_segment
#include "initdspdefs.h"  // for flush_display_ptrregion
#include "lispemul.h"     // for LispPTR, DLword, state, BITSPER_DLWORD, ERR...
#include "lispmap.h"      // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"     // for GETWORDBASEWORD
#include "misc7defs.h"    // for N_OP_misc7


/*************************************************/
/*  Possible operation fields for FBITMAPBIT     */
/*************************************************/

#define OP_INVERT 0 /* Invert the bit at the given location */
#define OP_ERASE 1  /* Turn the given bit off. */
#define OP_READ 2   /* Just read the bit that's there. */
#define OP_PAINT 3  /* Turn the bit on. */

extern int ScreenLocked;

/***	N_OP_misc7  -- pseudocolor or fbitmapbit   ***/
LispPTR N_OP_misc7(LispPTR arg1, LispPTR arg2, LispPTR arg3, LispPTR arg4, LispPTR arg5, LispPTR arg6, LispPTR arg7, int alpha)
{
  DLword *base;
  int x, y, operation, heightminus1, rasterwidth, oldbit;
  int offset;
  DLword bmdata;
  DLword bmmask;
  int displayflg;

  DBPRINT(("MISC7 op with alpha byte %d.\n", alpha));

  if (alpha != 1) ERROR_EXIT(arg7);

  base = NativeAligned2FromLAddr(arg1);
  N_GETNUMBER(arg2, x, doufn);
  N_GETNUMBER(arg3, y, doufn);
  N_GETNUMBER(arg4, operation, doufn);
  N_GETNUMBER(arg5, heightminus1, doufn);
  N_GETNUMBER(arg6, rasterwidth, doufn);

  DBPRINT(("MISC7 args OK.\n"));

  displayflg = n_new_cursorin(base, x, (heightminus1 - y), 1, 1);

/* Bitmaps use a positive integer coordinate system with the lower left
   corner pixel at coordinate (0, 0). Storage is allocated in 16-bit words
   from the upper left corner (0, h-1), with rasterwidth 16-bit words per row.
*/
  offset = (rasterwidth * (heightminus1 - y)) + (x / BITSPER_DLWORD);
  bmmask = (1 << (BITSPER_DLWORD - 1)) >> (x & (BITSPER_DLWORD - 1));
  bmdata = GETWORDBASEWORD(base, offset);
  oldbit = bmdata & bmmask;

  ScreenLocked = T;

  switch (operation) {
  case OP_INVERT: GETWORDBASEWORD(base, offset) = bmdata ^ bmmask; break;
  case OP_ERASE: GETWORDBASEWORD(base, offset) = bmdata & ~bmmask; break;
  case OP_READ: break;
  default: GETWORDBASEWORD(base, offset) = bmdata | bmmask;
  }


#ifdef XWINDOW
  if (in_display_segment(base)) {
    /* NB: base + offset doesn't need WORDPTR() wrapper */
    flush_display_ptrregion(base + offset, 0, 16, 1);
  }
#endif /* XWINDOW */

  ScreenLocked = NIL;
  DBPRINT(("FBITMAPBIT old bit = 0x%x.\n", oldbit));
  return (S_POSITIVE | (oldbit ? 1 : 0));

doufn:
  ERROR_EXIT(arg7);

} /*  end N_OP_misc7()  */
