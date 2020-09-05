/* $Id: ubf3.c,v 1.3 1999/05/31 23:35:45 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: ubf3.c,v 1.3 1999/05/31 23:35:45 sybalsky Exp $ Copyright (C) Venue";
/*	ubf3.c
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "medleyfp.h"

#include "ubf3defs.h"

/************************************************************
        N_OP_ubfloat3  -- op 062
062/0	POLY
***********************************************************/

LispPTR N_OP_ubfloat3(int arg3, LispPTR arg2, LispPTR arg1, int alpha) {
  REGISTER float val;
  REGISTER float ans;
  REGISTER float *fptr;
  register int degree;
  int ret;
  float flot;

  val = *(float *)&arg3; /* why? */
  if (alpha) ERROR_EXIT(arg1);
  FPCLEAR;
  if ((arg1 & SEGMASK) != S_POSITIVE) ERROR_EXIT(arg1);
  degree = 0xFFFF & arg1;
  fptr = (float *)Addr68k_from_LADDR(arg2);
  ans = *((float *)fptr);
  while (degree--) ans = (ans * val) + *((float *)(++fptr));
  if (FPTEST(ans)) ERROR_EXIT(arg1); /* relies on contagion of inf, nan? */
  flot = ans;
  ret = *(int *)&flot; /* why? */
  return (ret);
} /* end N_OP_ubfloat3() */

/* end module */
