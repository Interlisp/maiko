/* $Id: fp.c,v 1.3 1999/05/31 23:35:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*				  F P . C  				*/
/*									*/
/*	Floating-point arithmetic code.					*/
/*									*/
/************************************************************************/

#include "adr68k.h"      // for LAddrFromNative
#include "emlglob.h"
#include "fpdefs.h"      // for N_OP_fdifference, N_OP_fgreaterp, N_OP_fplus2
#include "lispemul.h"    // for state, LispPTR, DLword, ERROR_EXIT, ATOM_T
#include "lspglob.h"
#include "lsptypes.h"    // for TYPE_FLOATP
#include "medleyfp.h"    // for FPCLEAR, FPTEST
#include "mkcelldefs.h"  // for createcell68k
#include "my.h"          // for N_MakeFloat

/************************************************************
        N_OP_fplus2       -- op 350
        N_OP_fdifference  -- op 351
        N_OP_ftimes2      -- op 352
        N_OP_fquotient    -- op 353
        N_OP_fgreaterp    -- op 362
***********************************************************/

/************************************************************************/
/*									*/
/*			N _ O P _ f p l u s 2				*/
/*									*/
/*	2-argument floating point addition opcode			*/
/*									*/
/************************************************************************/

LispPTR N_OP_fplus2(LispPTR parg1, LispPTR parg2) {
  float arg1;
  float arg2;
  float result;
  float *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
  result = arg1 + arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = (float *)createcell68k(TYPE_FLOATP);
  *wordp = result;
  return (LAddrFromNative(wordp));
} /* end N_OP_fplus2()  */

/************************************************************************/
/*									*/
/*		      N _ O P _ f d i f f e r e n c e			*/
/*									*/
/*	2-argument floating-point subtraction.				*/
/*									*/
/************************************************************************/

LispPTR N_OP_fdifference(LispPTR parg1, LispPTR parg2) {
  float arg1, arg2;
  float result;
  float *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
  result = arg1 - arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = (float *)createcell68k(TYPE_FLOATP);
  *wordp = result;
  return (LAddrFromNative(wordp));
} /* end N_OP_fdifference()  */

/************************************************************************/
/*									*/
/*			    N _ O P _ f t i m e s 2			*/
/*									*/
/*	Floating-point multiplication					*/
/*									*/
/************************************************************************/

LispPTR N_OP_ftimes2(LispPTR parg1, LispPTR parg2) {
  float arg1, arg2;
  float result;
  float *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
  result = arg1 * arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = (float *)createcell68k(TYPE_FLOATP);
  *wordp = result;
  return (LAddrFromNative(wordp));
} /* end N_OP_ftimes2()  */

/************************************************************************/
/*									*/
/*			N _ O P _ f q u o t i e n t			*/
/*									*/
/*	floating-point division						*/
/*									*/
/************************************************************************/

LispPTR N_OP_fquotient(LispPTR parg1, LispPTR parg2) {
  float arg1, arg2;
  float result;
  float *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
  result = arg1 / arg2;

  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = (float *)createcell68k(TYPE_FLOATP);
  *wordp = result;
  return (LAddrFromNative(wordp));
} /* end N_OP_fquotient()  */

/************************************************************************/
/*									*/
/*			N _ O P _ f g r e a t e r p			*/
/*									*/
/*	Floating-point >						*/
/*									*/
/************************************************************************/

LispPTR N_OP_fgreaterp(LispPTR parg1, LispPTR parg2) {
  float arg1, arg2;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  if (arg1 > arg2)
    return (ATOM_T);
  else
    return (NIL_PTR);
} /* end N_OP_fgreaterp()  */
