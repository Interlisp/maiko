/* $Id: ubf3.c,v 1.3 1999/05/31 23:35:45 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	ubf3.c
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"    // for NativeAligned4FromLAddr
#include "lispemul.h"  // for state, ERROR_EXIT, LispPTR, SEGMASK
#include "lispmap.h"   // for S_POSITIVE
#include "lspglob.h"
#include "medleyfp.h"  // for FPCLEAR, FPTEST
#include "ubf3defs.h"  // for N_OP_ubfloat3

/************************************************************
        N_OP_ubfloat3  -- op 062
062/0	POLY
***********************************************************/

LispPTR N_OP_ubfloat3(int arg3, LispPTR arg2, LispPTR arg1, int alpha) {
  float val;
  float ans;
  float *fptr;
  int degree;
  int ret;
  float flot;

  val = *(float *)&arg3; /* why? */
  if (alpha) ERROR_EXIT(arg1);
  FPCLEAR;
  if ((arg1 & SEGMASK) != S_POSITIVE) ERROR_EXIT(arg1);
  degree = 0xFFFF & arg1;
  fptr = (float *)NativeAligned4FromLAddr(arg2);
  ans = *((float *)fptr);
  while (degree--) ans = (ans * val) + *((float *)(++fptr));
  if (FPTEST(ans)) ERROR_EXIT(arg1); /* relies on contagion of inf, nan? */
  flot = ans;
  ret = *(int *)&flot; /* why? */
  return (ret);
} /* end N_OP_ubfloat3() */

/* end module */
