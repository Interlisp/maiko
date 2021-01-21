/* $Id: arith2.c,v 1.4 2001/12/24 01:08:58 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "medleyfp.h"
#include "arith.h"

#include "arith2defs.h"
#include "fpdefs.h"
#include "mkcelldefs.h"

/************************************************************
N_OP_plus2
        entry		PLUS2		OPCODE[0324]
        entry		IPLUS2		OPCODE[0330]
        return(tos + b)
************************************************************/

LispPTR N_OP_plus2(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_GETNUMBER(tos, arg1, doufn);
  N_GETNUMBER(tosm1, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_sadd_overflow(arg1, arg2, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else
  /* UB: signed integer overflow: 2147483647 + 2147483647 cannot be represented in type 'int' */
  result = arg1 + arg2;
  if (((arg1 >= 0) == (arg2 >= 0)) && ((result >= 0) != (arg1 >= 0))) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif

doufn:
  return (N_OP_fplus2(tosm1, tos));
}

/************************************************************************/
/*									*/
/*			N _ O P _ i p l u s 2				*/
/*									*/
/*	Implements the IPLUS2 opcode--add the two arguments, which	*/
/*	must be SMALLP or FIXP						*/
/*									*/
/************************************************************************/

LispPTR N_OP_iplus2(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_IGETNUMBER(tos, arg1, doufn);
  N_IGETNUMBER(tosm1, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_sadd_overflow(arg1, arg2, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else

  /* UB: signed integer overflow: 2147483647 + 2147483647 cannot be represented in type 'int' */
  result = arg1 + arg2;
  if (((arg1 >= 0) == (arg2 >= 0)) && ((result >= 0) != (arg1 >= 0))) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif

doufn:
  ERROR_EXIT(tos);
}

/************************************************************
N_OP_difference
        entry		DIFFERENCE		OPCODE[0325]
        entry		IDIFFERENCE		OPCODE[0331]
        return(a - tos)
************************************************************/

LispPTR N_OP_difference(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_GETNUMBER(tosm1, arg1, doufn);
  N_GETNUMBER(tos, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_ssub_overflow(arg1, arg2, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else

  /* UB: signed integer overflow: -2147483647 - 320 cannot be represented in type 'int' */
  result = arg1 - arg2;
  if (((arg1 >= 0) == (arg2 < 0)) && ((result >= 0) != (arg1 >= 0))) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif

doufn:
  return (N_OP_fdifference(tosm1, tos));
}

LispPTR N_OP_idifference(int tosm1, int tos) {
  int arg1, arg2;
  int result;

  N_IGETNUMBER(tosm1, arg1, doufn);
  N_IGETNUMBER(tos, arg2, doufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_ssub_overflow(arg1, arg2, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else
  /* UB: signed integer overflow: -2147483647 - 100 cannot be represented in type 'int' */
  result = arg1 - arg2;
  if (((arg1 >= 0) == (arg2 < 0)) && ((result >= 0) != (arg1 >= 0))) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif
doufn:
  ERROR_EXIT(tos);
}

/************************************************************
N_OP_logxor
        entry		LOGXOR2		OPCODE[0346]
        return(tosm1 ^ tos)
************************************************************/

LispPTR N_OP_logxor(int tosm1, int tos) { N_IARITH_BODY_2(tosm1, tos, ^); }

/************************************************************
N_OP_logand
        entry		LOGAND2		OPCODE[0345]
        return(tosm1 & tos)
************************************************************/
LispPTR N_OP_logand(int tosm1, int tos) { N_IARITH_BODY_2(tosm1, tos, &); }

/************************************************************
N_OP_logor
        entry		LOGOR2		OPCODE[0344]
        return(tosm1 | tos)
************************************************************/
LispPTR N_OP_logor(int tosm1, int tos) { N_IARITH_BODY_2(tosm1, tos, |); }

/************************************************************
N_OP_greaterp
        entry		GREATERP		OPCODE[0363]
        entry		IGREATERP		OPCODE[0361]
        return(tosm1 > tos)

************************************************************/
LispPTR N_OP_greaterp(int tosm1, int tos) {
  register int arg1, arg2;

  N_GETNUMBER(tosm1, arg1, do_ufn);
  N_GETNUMBER(tos, arg2, do_ufn);

  if (arg1 > arg2)
    return (ATOM_T);
  else
    return (NIL_PTR);

do_ufn:
  return (N_OP_fgreaterp(tosm1, tos));
}

LispPTR N_OP_igreaterp(int tosm1, int tos) {
  register int arg1, arg2;

  N_IGETNUMBER(tosm1, arg1, do_ufn);
  N_IGETNUMBER(tos, arg2, do_ufn);

  if (arg1 > arg2)
    return (ATOM_T);
  else
    return (NIL_PTR);

do_ufn:
  ERROR_EXIT(tos);
}

/************************************************************
N_OP_iplusn
        entry		IPLUS.N		OPCODE[0335]
        return(tos + n)
************************************************************/
LispPTR N_OP_iplusn(int tos, int n) {
  int arg1;
  int result;

  N_IGETNUMBER(tos, arg1, do_ufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_sadd_overflow(arg1, n, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else

  result = arg1 + n;
  if ((result < 0) && (arg1 >= 0)) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif

do_ufn:
  ERROR_EXIT(tos);
}

/************************************************************
N_OP_idifferencen
        entry		IDIFFERENCE.N		OPCODE[0336]
        return(tos - n)
************************************************************/
LispPTR N_OP_idifferencen(int tos, int n) {
  int arg1;
  int result;

  N_IGETNUMBER(tos, arg1, do_ufn);

#ifdef USE_OVERFLOW_BUILTINS

  if (__builtin_ssub_overflow(arg1, n, &result)) {
    ERROR_EXIT(tos);
  }
  N_ARITH_SWITCH(result);

#else

  result = arg1 - n;
  if ((result >= 0) && (arg1 < 0)) { ERROR_EXIT(tos); }
  N_ARITH_SWITCH(result);

#endif

do_ufn:
  ERROR_EXIT(tos);
}
