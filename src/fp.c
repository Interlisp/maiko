/* $Id: fp.c,v 1.3 1999/05/31 23:35:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: fp.c,v 1.3 1999/05/31 23:35:29 sybalsky Exp $ Copyright (C) Venue";

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
#include <stdio.h>
#include <math.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "my.h"
#include "medleyfp.h"

#include "fpdefs.h"

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
  REGISTER float arg1;
  REGISTER float arg2;
  REGISTER float result;
  register DLword *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
#ifdef I386
  I386Round;
#endif
  result = arg1 + arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = createcell68k(TYPE_FLOATP);
  *((float *)wordp) = result;
  return (LADDR_from_68k(wordp));
} /* end N_OP_fplus2()  */

/************************************************************************/
/*									*/
/*		      N _ O P _ f d i f f e r e n c e			*/
/*									*/
/*	2-arugment floating-point subtraction.				*/
/*									*/
/************************************************************************/

LispPTR N_OP_fdifference(LispPTR parg1, LispPTR parg2) {
  REGISTER float arg1, arg2;
  REGISTER float result;
  register DLword *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
#ifdef I386
  I386Round;
#endif
  result = arg1 - arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = createcell68k(TYPE_FLOATP);
  *((float *)wordp) = result;
  return (LADDR_from_68k(wordp));
} /* end N_OP_fdifference()  */

/************************************************************************/
/*									*/
/*			    N _ O P _ f t i m e s 2			*/
/*									*/
/*	Floating-point multiplication					*/
/*									*/
/************************************************************************/

LispPTR N_OP_ftimes2(LispPTR parg1, LispPTR parg2) {
  REGISTER float arg1, arg2;
  REGISTER float result;
  register DLword *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
#ifdef I386
  I386Round;
#endif
  result = arg1 * arg2;
  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = createcell68k(TYPE_FLOATP);
  *((float *)wordp) = result;
  return (LADDR_from_68k(wordp));
} /* end N_OP_ftimes2()  */

/************************************************************************/
/*									*/
/*			N _ O P _ f q u o t i e n t			*/
/*									*/
/*	floating-point division						*/
/*									*/
/************************************************************************/

LispPTR N_OP_fquotient(LispPTR parg1, LispPTR parg2) {
  REGISTER float arg1, arg2;
  REGISTER float result;
  register DLword *wordp;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  FPCLEAR;
#ifdef I386
  I386Round;
#endif
  result = arg1 / arg2;

  if (FPTEST(result)) ERROR_EXIT(parg2);
  wordp = createcell68k(TYPE_FLOATP);
  *((float *)wordp) = result;
  return (LADDR_from_68k(wordp));
} /* end N_OP_fquotient()  */

/************************************************************************/
/*									*/
/*			N _ O P _ f g r e a t e r p			*/
/*									*/
/*	Floating-point >						*/
/*									*/
/************************************************************************/

LispPTR N_OP_fgreaterp(LispPTR parg1, LispPTR parg2) {
  REGISTER float arg1, arg2;
  register DLword *wordp;
  register LispPTR lptr;

  N_MakeFloat(parg1, arg1, parg2);
  N_MakeFloat(parg2, arg2, parg2);
  if (arg1 > arg2)
    return (ATOM_T);
  else
    return (NIL_PTR);
} /* end N_OP_fgreaterp()  */
