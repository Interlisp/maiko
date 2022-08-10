/* $Id: arith4.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/***********************************************************************/
/*
                File Name :	arith4.c

                Including :	OP_times2 326Q(OP_itimes2 332Q)

                                OP_quot 327Q(OP_iquot 333Q)
                                OP_reminder 334Q

*/
/**********************************************************************/
#include "version.h"
#include "arith.h"       // for N_IGETNUMBER, N_ARITH_SWITCH, N_GETNUMBER
#include "arith4defs.h"  // for N_OP_iquot, N_OP_iremainder, N_OP_itimes2
#include "fpdefs.h"      // for N_OP_fquotient, N_OP_ftimes2
#include "lispemul.h"    // for state, ERROR_EXIT, LispPTR
#include "lspglob.h"
#include "lsptypes.h"

/**********************************************************************/
/*

                Func name :	N_OP_times2(itimes2)

*/
/**********************************************************************/
LispPTR N_OP_times2(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_GETNUMBER(tosm1, arg1, doufn);
  N_GETNUMBER(tos, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_smul_overflow(arg1, arg2, &result)) {
    goto doufn2;
  }
  N_ARITH_SWITCH(result);

#else

  result = arg1 * arg2;
  if ((arg2 != 0) && ((result / arg2) != arg1)) goto doufn2;
  N_ARITH_SWITCH(result);

#endif

doufn2:
  ERROR_EXIT(tos);
doufn:
  return (N_OP_ftimes2(tosm1, tos));

} /* end N_OP_times2 */

LispPTR N_OP_itimes2(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_IGETNUMBER(tosm1, arg1, doufn);
  N_IGETNUMBER(tos, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_smul_overflow(arg1, arg2, &result)) {
    goto doufn;
  }
  N_ARITH_SWITCH(result);

#else

  /* UB: signed integer overflow: 1073741824 * 32768 cannot be represented in type 'int' */
  result = arg1 * arg2;
  if ((arg2 != 0) && ((result / arg2) != arg1)) { goto doufn; }
  N_ARITH_SWITCH(result);

#endif

doufn:
  ERROR_EXIT(tos);

} /* end N_OP_itimes2 */

/**********************************************************************/
/*

                Func name :	N_OP_quot(iquot)

*/
/**********************************************************************/
LispPTR N_OP_quot(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_GETNUMBER(tosm1, arg1, doufn);
  N_GETNUMBER(tos, arg2, doufn);
  if (arg2 == 0) goto doufn2;

  result = arg1 / arg2; /* lmm: note: no error case!! */
  N_ARITH_SWITCH(result);

doufn2:
  ERROR_EXIT(tos);
doufn:
  return (N_OP_fquotient(tosm1, tos));

} /* end N_OP_quot */

LispPTR N_OP_iquot(int tosm1, int tos) {
  register int arg1, arg2;
  register int result;

  N_IGETNUMBER(tosm1, arg1, doufn);
  N_IGETNUMBER(tos, arg2, doufn);
  if (arg2 == 0) goto doufn;

  result = arg1 / arg2;
  N_ARITH_SWITCH(result);

doufn:
  ERROR_EXIT(tos);

} /* end N_OP_quot */

/**********************************************************************/
/*

                Func name :	N_OP_iremainder

*/
/**********************************************************************/

LispPTR N_OP_iremainder(int tosm1, int tos) {
  register int arg1, arg2;
  register int result;

  N_IGETNUMBER(tosm1, arg1, doufn);
  N_IGETNUMBER(tos, arg2, doufn);
  if (arg2 == 0) goto doufn;

  result = arg1 % arg2;
  N_ARITH_SWITCH(result);

doufn:
  ERROR_EXIT(tos);

} /* end N_OP_iremainder */
