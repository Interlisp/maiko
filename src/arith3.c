/* $Id: arith3.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: arith3.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ Copyright (C) Venue";

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

#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "cell.h"
#include "arith.h"

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
