/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"      // for NativeAligned4FromLAddr
#include "arith.h"       // for N_IGETNUMBER, N_ARITH_SWITCH, N_GETNUMBER
#include "arithopsdefs.h"  // for N_OP_difference, N_OP_greaterp, N_OP_idiffer...
#include "fpdefs.h"      // for N_OP_fdifference, N_OP_fgreaterp, N_OP_fplus2
#include "lispemul.h"    // for state, ERROR_EXIT, LispPTR, ATOM_T, NIL_PTR
#include "lispmap.h"     // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"

/************************************************************
N_OP_plus2
        entry		PLUS2		OPCODE[0324]
        entry		IPLUS2		OPCODE[0330]
        return(tos + b)
************************************************************/

LispPTR N_OP_plus2(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_iplus2(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_difference(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_idifference(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_logxor(LispPTR tosm1, LispPTR tos) {
    int arg1, arg2;

    N_IGETNUMBER(tosm1, arg1, do_ufn);
    N_IGETNUMBER(tos, arg2, do_ufn);

    arg1 = arg1 ^ arg2;

    N_ARITH_SWITCH(arg1);

  do_ufn:
    ERROR_EXIT(tos);
}

/************************************************************
N_OP_logand
        entry		LOGAND2		OPCODE[0345]
        return(tosm1 & tos)
************************************************************/
LispPTR N_OP_logand(LispPTR tosm1, LispPTR tos) {
    int arg1, arg2;

    N_IGETNUMBER(tosm1, arg1, do_ufn);
    N_IGETNUMBER(tos, arg2, do_ufn);

    arg1 = arg1 & arg2;

    N_ARITH_SWITCH(arg1);

  do_ufn:
    ERROR_EXIT(tos);
}

/************************************************************
N_OP_logor
        entry		LOGOR2		OPCODE[0344]
        return(tosm1 | tos)
************************************************************/
LispPTR N_OP_logor(LispPTR tosm1, LispPTR tos) {
      int arg1, arg2;

    N_IGETNUMBER(tosm1, arg1, do_ufn);
    N_IGETNUMBER(tos, arg2, do_ufn);

    arg1 = arg1 | arg2;

    N_ARITH_SWITCH(arg1);

  do_ufn:
    ERROR_EXIT(tos);
}

/************************************************************
N_OP_greaterp
        entry		GREATERP		OPCODE[0363]
        entry		IGREATERP		OPCODE[0361]
        return(tosm1 > tos)

************************************************************/
LispPTR N_OP_greaterp(LispPTR tosm1, LispPTR tos) {
  int arg1, arg2;

  N_GETNUMBER(tosm1, arg1, do_ufn);
  N_GETNUMBER(tos, arg2, do_ufn);

  if (arg1 > arg2)
    return (ATOM_T);
  else
    return (NIL_PTR);

do_ufn:
  return (N_OP_fgreaterp(tosm1, tos));
}

LispPTR N_OP_igreaterp(LispPTR tosm1, LispPTR tos) {
  int arg1, arg2;

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
LispPTR N_OP_iplusn(LispPTR tos, int n) {
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
LispPTR N_OP_idifferencen(LispPTR tos, int n) {
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

/************************************************************************/
/*									*/
/*			N _ O P _ m a k e n u m b e r			*/
/*									*/
/*	Given the 2 halves of a FIXP as SMALLP's, create a number	*/
/*	box for the number, and fill it in.				*/
/*									*/
/************************************************************************/

LispPTR N_OP_makenumber(LispPTR tosm1, LispPTR tos) {
  int result;

  if (((tosm1 & 0xFFFF0000) != S_POSITIVE) || ((tos & 0xFFFF0000) != S_POSITIVE)) ERROR_EXIT(tos);
  /* UB: left shift of 49152 by 16 places cannot be represented in type 'int' */
  result = (int)(((tosm1 & 0xffff) << 16) | (tos & 0xffff));
  N_ARITH_SWITCH(result);
} /* end OP_makenumber */

/************************************************************************/
/*									*/
/*			N _ O P _ b o x i p l u s			*/
/*									*/
/*	Given a FIXP box and a number to add to it, add the number,	*/
/*	leaving the result in the box given.  Used to avoid garbaging.	*/
/*									*/
/************************************************************************/

LispPTR N_OP_boxiplus(LispPTR a, LispPTR tos) {
  int arg2;

  if (GetTypeNumber(a) == TYPE_FIXP) {
    N_GETNUMBER(tos, arg2, bad);
    *((int *)NativeAligned4FromLAddr(a)) += arg2;
    return (a);
  }
bad:
  ERROR_EXIT(tos);

} /* OP_boxiplus */

/************************************************************************/
/*									*/
/*			O P _ b o x i d i f f				*/
/*									*/
/*	Given a FIXP box and a number to subtract from it, do the	*/
/*	subtraction, and leave the result in the box given.		*/
/*	Used to avoid allocating storage in low-level routines.		*/
/*									*/
/************************************************************************/

LispPTR N_OP_boxidiff(LispPTR a, LispPTR tos) {
  int arg2;

  if (GetTypeNumber(a) == TYPE_FIXP) {
    N_GETNUMBER(tos, arg2, bad);
    *((int *)NativeAligned4FromLAddr(a)) -= arg2;
    return (a);
  }
bad:
  ERROR_EXIT(tos);

} /* end OP_boxidiff */

/**********************************************************************/
/*

                Func name :	N_OP_times2

*/
/**********************************************************************/
LispPTR N_OP_times2(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_itimes2(LispPTR tosm1, LispPTR tos) {
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
LispPTR N_OP_quot(LispPTR tosm1, LispPTR tos) {
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

LispPTR N_OP_iquot(LispPTR tosm1, LispPTR tos) {
  int arg1, arg2;
  int result;

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

LispPTR N_OP_iremainder(LispPTR tosm1, LispPTR tos) {
  int arg1, arg2;
  int result;

  N_IGETNUMBER(tosm1, arg1, doufn);
  N_IGETNUMBER(tos, arg2, doufn);
  if (arg2 == 0) goto doufn;

  result = arg1 % arg2;
  N_ARITH_SWITCH(result);

doufn:
  ERROR_EXIT(tos);

} /* end N_OP_iremainder */
