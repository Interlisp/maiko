/* $Id: arith3.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*			    A R I T H 3 . C				*/
/*									*/
/*	Including :	OP_makenumber					*/
/*			OP_boxiplus					*/
/*			OP_boxidiff					*/
/*									*/
/*									*/
/************************************************************************/

#include "version.h"
#include "adr68k.h"      // for Addr68k_from_LADDR
#include "arith.h"       // for N_GETNUMBER, N_ARITH_SWITCH
#include "arith3defs.h"  // for N_OP_boxidiff, N_OP_boxiplus, N_OP_makenumber
#include "lispemul.h"    // for state, LispPTR, ERROR_EXIT
#include "lispmap.h"     // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"    // for GetTypeNumber, TYPE_FIXP

/************************************************************************/
/*									*/
/*			N _ O P _ m a k e n u m b e r			*/
/*									*/
/*	Given the 2 halves of a FIXP as SMALLP's, create a number	*/
/*	box for the number, and fill it in.				*/
/*									*/
/************************************************************************/

LispPTR N_OP_makenumber(int tosm1, int tos) {
  register int result;

  if (((tosm1 & 0xFFFF0000) != S_POSITIVE) || ((tos & 0xFFFF0000) != S_POSITIVE)) ERROR_EXIT(tos);
  /* UB: left shift of 49152 by 16 places cannot be represented in type 'int' */
  result = ((tosm1 & 0xffff) << 16) | (tos & 0xffff);
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

LispPTR N_OP_boxiplus(register int a, int tos) {
  register int arg2;

  if (GetTypeNumber(a) == TYPE_FIXP) {
    N_GETNUMBER(tos, arg2, bad);
    *((LispPTR *)Addr68k_from_LADDR(a)) += arg2;
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

LispPTR N_OP_boxidiff(register int a, int tos) {
  register int arg2;

  if (GetTypeNumber(a) == TYPE_FIXP) {
    N_GETNUMBER(tos, arg2, bad);
    *((LispPTR *)Addr68k_from_LADDR(a)) -= arg2;
    return (a);
  }
bad:
  ERROR_EXIT(tos);

} /* end OP_boxidiff */
