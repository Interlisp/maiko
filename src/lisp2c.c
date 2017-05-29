/* $Id: lisp2c.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: lisp2c.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ Copyright (C) Venue";
/* File containing the conversion functions between lisp and C */
/* -jarl */

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

#include <stdio.h> /* for sprintf */

#include "lispemul.h"
#include "lspglob.h"
#include "emlglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "medleyfp.h"
#include "arith.h"

int LispStringP(LispPTR object) {
  int type;

  type = ((OneDArray *)Addr68k_from_LADDR(object))->typenumber;
  return ((type == THIN_CHAR_TYPENUMBER) || (type == FAT_CHAR_TYPENUMBER));
}

int LispStringLength(LispPTR lispstring) {
  OneDArray *arrayp;

  arrayp = (OneDArray *)(Addr68k_from_LADDR(lispstring));
  return (arrayp->fillpointer);
}

void LispStringToCStr(LispPTR lispstring, char *cstring) {
  OneDArray *arrayp;
  char *base;
  short *sbase;
  int i, Len;

  arrayp = (OneDArray *)(Addr68k_from_LADDR((UNSIGNED)lispstring));
  Len = arrayp->fillpointer;

  switch (arrayp->typenumber) {
    case THIN_CHAR_TYPENUMBER:
      base = ((char *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset));
      for (i = 0; i < Len; i++) cstring[i] = base[i];
      cstring[Len] = '\0';
      break;

    case FAT_CHAR_TYPENUMBER:
      sbase = ((short *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset));
      base = (char *)sbase;
      for (i = 0; i < Len * 2; i++) cstring[i] = base[i];
      cstring[Len * 2] = '\0';
      break;

    default: error("Arg not Lisp string.\n");
  }
}

int LispIntToCInt(LispPTR lispint) {
  switch ((0xFFFF0000 & lispint)) {
    case S_POSITIVE: return (lispint & 0xFFFF); break;
    case S_NEGATIVE: return (lispint | 0xFFFF0000); break;
    default:
      if (GetTypeNumber(lispint) == TYPE_FIXP) {
        return (*((int *)Addr68k_from_LADDR(lispint)));
      } else {
        char msg[200];
        sprintf(msg, "Arg 0x%x isn't a lisp integer.", lispint);
        error(msg);
      }
      break;
  }
}

LispPTR CIntToLispInt(int cint) {
  if (abs(cint) > 0xFFFF) { /* its a fixp! */
    register LispPTR *wordp;
    wordp = (LispPTR *)createcell68k(TYPE_FIXP);
    *((int *)wordp) = cint;
    return (LADDR_from_68k(wordp));
  } else if (cint >= 0) { /* its a positive smallp! */
    return (S_POSITIVE | cint);
  } else { /* its a negative smallp! */
    return (S_NEGATIVE | (0xFFFF & cint));
  }
}

DLword CIntToSmallp(int cint) {
  if (abs(cint) > 0xFFFF) { /* its a fixp! */
    error("Arg not a Smallp.\n");
  } else if (cint >= 0) { /* its a positive smallp! */
    return (S_POSITIVE | cint);
  } else { /* its a negative smallp! */
    return (S_NEGATIVE | (0xFFFF & cint));
  }
}
