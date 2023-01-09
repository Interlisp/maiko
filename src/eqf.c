/* $Id: eqf.c,v 1.3 1999/05/31 23:35:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "arith.h"     // for FIXP_VALUE, FLOATP_VALUE
#include "eqfdefs.h"   // for N_OP_clequal, N_OP_eqlop, N_OP_eqq, N_OP_equal
#include "lispemul.h"  // for NIL, state, ATOM_T, ERROR_EXIT, SEGMASK, LispPTR
#include "lispmap.h"   // for ATOM_OFFSET, S_CHARACTER, S_NEGATIVE, S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"  // for TYPE_FLOATP, GetTypeNumber, TYPE_FIXP, TYPE_SM...
#include "my.h"        // for N_MakeFloat

/************************************************************
op 072   N_OP_eqlop	EQL
op 0314  N_OP_clequal   CL:EQUAL
op 0360  (inline)	EQ
op 0364  N_OP_equal	IL:EQUAL
op 0377  N_OP_eqq	CL:=
***********************************************************/
/* differences between these operations:

EQ is a strict pointer comparison, equivalent to C's ==

EQL (common lisp) does no conversions before comparison, but will
        compare equal FIXPs or equal FLOATPs.

CL:=  will do a numeric comparison
      and will compare floats. If given integers, it will convert
        to floating point first.

IL:EQUAL  is a recursive comparison which will compare 1 = 1.0
          it work like code with CL:= for the most part

CL:EQUAL is a recursive comparison which uses EQL at the leaves

Interlisp operations IEQP, FEQP have no opcodes, although there
is an unboxed FEQP.

number types include:

SMALLP	(immediate with S_POSITIVE or S_NEGATIVE)
FIXP	(32 bit boxed value, handled in C. Usually canonical, i.e.,
         will be SMALLP. (IPLUS x 0) will always canonicallize.)
FLOATP  (32 bit boxed value, handled in C, usually)
RATIO	(a/b. Always canonical, i.e., b doesn't divide a evenly)
COMPLEX (a+bi. Not handled in C)
BIGNUM  (integer that can't be represented bigger than 32 bits)

*/

#define IF_IMMEDIATE(arg, doit, doitsmall)                                 \
  do {                                                                     \
    switch (SEGMASK & (arg)) {                                             \
      case ATOM_OFFSET: doit;       /* NOLINT(bugprone-macro-parentheses) */ \
      case S_CHARACTER: doit;       /* NOLINT(bugprone-macro-parentheses) */ \
      case S_POSITIVE: doitsmall;   /* NOLINT(bugprone-macro-parentheses) */ \
      case S_NEGATIVE: doitsmall;   /* NOLINT(bugprone-macro-parentheses) */ \
    }                                                                      \
  } while (0)

/************************************************************************/
/*									*/
/*			N _ O P _ c l e q u a l				*/
/*									*/
/*	Common Lisp EQUAL, opcode 0314.					*/
/*									*/
/************************************************************************/

LispPTR N_OP_clequal(LispPTR arg1, LispPTR arg2) {
  int type;

  if (arg2 == arg1) return (ATOM_T);
  IF_IMMEDIATE(arg1, return (NIL), return (NIL));
  IF_IMMEDIATE(arg2, return (NIL), return (NIL));

  /* CL:EQUAL is true for two strings that have different Interlisp
    type numbers; cannot currently handle it here. */

  /* can return NIL if one is a number and the other isn't */

  if (Numberp(arg1)) {
    if (!Numberp(arg2)) return (NIL);
  } else {
    if (Numberp(arg2)) {
      return (NIL);
    } else
      ERROR_EXIT(arg2);
  }

  /* now we know both are numbers */

  if ((type = GetTypeNumber(arg1)) != (GetTypeNumber(arg2))) return (NIL);

  /* now we know both are the same type. Shouldn't see any SMALLPs */

  switch (type) {
    case TYPE_FIXP:
      if (FIXP_VALUE(arg1) == FIXP_VALUE(arg2)) { return (ATOM_T); }
      return (NIL);

    case TYPE_FLOATP:
      if (FLOATP_VALUE(arg1) == FLOATP_VALUE(arg2)) { return (ATOM_T); }
      return (NIL);

    default: ERROR_EXIT(arg2);
  }

} /* end N_OP_clequal */

/************************************************************************/
/*									*/
/*			N _ O P _ e q l o p				*/
/*									*/
/*	Common Lisp EQL.						*/
/*									*/
/************************************************************************/

LispPTR N_OP_eqlop(LispPTR arg1, LispPTR arg2) {
  int type;

  if (arg2 == arg1) return (ATOM_T);
  IF_IMMEDIATE(arg1, return (NIL), return (NIL));
  IF_IMMEDIATE(arg2, return (NIL), return (NIL));

  /* EQL is true if EQ or both are numbers, the same type, and EQUAL */

  /* can return NIL if one is a number and the other isn't */

  if ((type = GetTypeNumber(arg1)) != (GetTypeNumber(arg2))) return (NIL);

  /* now we know both are the same type. Shouldn't see any SMALLPs */

  switch (type) {
    case TYPE_FIXP:
      if (FIXP_VALUE(arg1) == FIXP_VALUE(arg2)) { return (ATOM_T); }
      return (NIL);

    case TYPE_FLOATP:
      if (FLOATP_VALUE(arg1) == FLOATP_VALUE(arg2)) { return (ATOM_T); }
      return (NIL);

    default:
      if (Numberp(arg1)) {
        ERROR_EXIT(arg2);
      } else
        return (NIL);
  }

} /* end N_OP_eqlop */

/************************************************************************/
/*									*/
/*			N _ O P _ e q u a l				*/
/*									*/
/*	IL:EQUAL, opcode 0364.						*/
/*									*/
/************************************************************************/

LispPTR N_OP_equal(LispPTR arg1, LispPTR arg2) {
  int type, type2;

  if (arg2 == arg1) return (ATOM_T);

  IF_IMMEDIATE(arg1, return (NIL), goto arg1_small);
  IF_IMMEDIATE(arg2, return (NIL), goto arg2_small);
  goto arg2_small;

arg1_small:
  IF_IMMEDIATE(arg2, return (NIL), return (NIL)); /* arg2 atom or both small */

arg2_small:

  if (Numberp(arg1)) {
    if (!Numberp(arg2)) return (NIL);
  } else {
    if (Numberp(arg2)) {
      return (NIL);
    } else
      ERROR_EXIT(arg2);
  }

  /* now we know both are numbers */

  type = GetTypeNumber(arg1);
  type2 = GetTypeNumber(arg2);

  if (type == type2) {
    switch (GetTypeNumber(arg1)) {
      case TYPE_SMALLP: return (NIL);
      case TYPE_FIXP:
        if (FIXP_VALUE(arg1) == FIXP_VALUE(arg2)) { return (ATOM_T); }
        return (NIL);
      case TYPE_FLOATP:
        if (FLOATP_VALUE(arg1) == FLOATP_VALUE(arg2)) { return (ATOM_T); }
        return (NIL);
      default: ERROR_EXIT(arg2);
    }
  }

  if ((type == TYPE_FLOATP) || (type2 == TYPE_FLOATP)) {
    float f1, f2;
    N_MakeFloat(arg1, f1, arg2);
    N_MakeFloat(arg2, f2, arg2);
    if ((f1 + 0.0f) == (f2 + 0.0f))
      return (ATOM_T);
    else
      return (NIL);
  } else
    return (NIL); /* neither is float, types are different */

} /* end N_OP_equal */

/************************************************************************/
/*									*/
/*			N _ O P _ e q q					*/
/*									*/
/*	Common Lisp =, opcode 0377.  Numeric compare, will convert	*/
/*				     among representations as needed.	*/
/*									*/
/************************************************************************/

LispPTR N_OP_eqq(LispPTR arg1, LispPTR arg2) /* CL:=    opcode 0377 */

{
  int type1, type2;
  float f1, f2;

  if (!((type1 = GetTypeEntry(arg1)) & TT_NUMBERP)) ERROR_EXIT(arg2);
  if (arg2 == arg1) return (ATOM_T);
  if (!((type2 = GetTypeEntry(arg2)) & TT_NUMBERP)) ERROR_EXIT(arg2);
  type1 &= 0x7ff;
  type2 &= 0x7ff;

  switch (type1) {
    case TYPE_SMALLP:
      switch (type2) {
        case TYPE_SMALLP: return (NIL);
        case TYPE_FIXP: return (NIL);
        case TYPE_FLOATP: goto checkfloats;
        default: ERROR_EXIT(arg2);
      }

    case TYPE_FIXP:
      switch (type2) {
        case TYPE_SMALLP: return (NIL);
        case TYPE_FIXP:
          if (FIXP_VALUE(arg1) == FIXP_VALUE(arg2))
            return (ATOM_T);
          else
            return (NIL);
        case TYPE_FLOATP: goto checkfloats;
        default: ERROR_EXIT(arg2);
      }

    case TYPE_FLOATP:
      switch (type2) {
        case TYPE_SMALLP: goto checkfloats;
        case TYPE_FIXP: goto checkfloats;
        case TYPE_FLOATP: goto checkfloats;
        default: ERROR_EXIT(arg2);
      }

    default: ERROR_EXIT(arg2);
  }

checkfloats:

  N_MakeFloat(arg1, f1, arg2);
  N_MakeFloat(arg2, f2, arg2);
  if (f1 == f2) return (ATOM_T);
  if ((f1 == -0.0f) && (f2 == 0.0f)) return (ATOM_T);
  if ((f1 == 0.0f) && (f2 == -0.0f)) return (ATOM_T);
  return (NIL);
} /* end N_OP_eqq() */
