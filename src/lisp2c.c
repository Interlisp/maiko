/* $Id: lisp2c.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
/* File containing the conversion functions between lisp and C */
/* -jarl */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>       // for sprintf
#include <stdlib.h>      // for abs
#include "adr68k.h"      // for NativeAligned4FromLAddr, LAddrFromNative
#include "commondefs.h"  // for error
#include "emlglob.h"
#include "lisp2cdefs.h"  // for CIntToLispInt, LispIntToCInt, LispStringSimpleLength
#include "lispemul.h"    // for LispPTR
#include "lispmap.h"     // for S_NEGATIVE, S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"    // for OneDArray, FAT_CHAR_TYPENUMBER, THIN_CHAR_TY...
#include "mkcelldefs.h"  // for createcell68k

int LispStringP(LispPTR object) {
  int type;

  type = ((OneDArray *)NativeAligned4FromLAddr(object))->typenumber;
  return ((type == THIN_CHAR_TYPENUMBER) || (type == FAT_CHAR_TYPENUMBER));
}

int LispStringSimpleLength(LispPTR lispstring) {
  OneDArray *arrayp;

  arrayp = (OneDArray *)(NativeAligned4FromLAddr(lispstring));
  return (arrayp->fillpointer);
}

void LispStringToCStr(LispPTR lispstring, char *cstring) {
  OneDArray *arrayp;
  char *base;
  short *sbase;
  int i, Len;

  arrayp = (OneDArray *)(NativeAligned4FromLAddr(lispstring));
  Len = arrayp->fillpointer;

  switch (arrayp->typenumber) {
    case THIN_CHAR_TYPENUMBER:
      base = ((char *)(NativeAligned2FromLAddr(arrayp->base))) + ((int)(arrayp->offset));
      for (i = 0; i < Len; i++) cstring[i] = base[i];
      cstring[Len] = '\0';
      break;

    case FAT_CHAR_TYPENUMBER:
      sbase = ((short *)(NativeAligned2FromLAddr(arrayp->base))) + ((int)(arrayp->offset));
      base = (char *)sbase;
      for (i = 0; i < Len * 2; i++) cstring[i] = base[i];
      cstring[Len * 2] = '\0';
      break;

    default: error("Arg not Lisp string.\n");
  }
}

int LispIntToCInt(LispPTR lispint) {
  switch ((0xFFFF0000 & lispint)) {
    case S_POSITIVE: return (lispint & 0xFFFF);
    case S_NEGATIVE: return (lispint | 0xFFFF0000);
    default:
      if (GetTypeNumber(lispint) == TYPE_FIXP) {
        return (*((int *)NativeAligned4FromLAddr(lispint)));
      } else {
        char msg[200];
        sprintf(msg, "Arg 0x%x isn't a lisp integer.", lispint);
        error(msg);
        /* NOTREACHED */
        return(0);
      }
  }
}

LispPTR CIntToLispInt(int cint) {
  if (abs(cint) > 0xFFFF) { /* its a fixp! */
    LispPTR *wordp;
    wordp = (LispPTR *)createcell68k(TYPE_FIXP);
    *((int *)wordp) = cint;
    return (LAddrFromNative(wordp));
  } else if (cint >= 0) { /* its a positive smallp! */
    return (S_POSITIVE | cint);
  } else { /* its a negative smallp! */
    return (S_NEGATIVE | (0xFFFF & cint));
  }
}
