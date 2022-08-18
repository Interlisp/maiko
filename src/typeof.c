/* $Id: typeof.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/****************************************************************/
/* LISTP(3Q),NTYPEX(4Q),TYPEP(5Q),DTEST(6Q) ,INSTANCEP(303Q)    */
/****************************************************************/
/*
                                changed : Jan. 13 1987 take
                                changed : Feb. 05 1987 take
                                changed : Jul. 24 1987 take

*/

#include "lispemul.h"    // for LispPTR, ATOM_T, NIL_PTR
#include "lspglob.h"
#include "lsptypes.h"    // for dtd, GetDTD, GetTypeNumber
#include "typeofdefs.h"  // for N_OP_instancep

/************************************************************************/
/*									*/
/*			N _ O P _ i n s t a n c e p			*/
/*									*/
/*	Returns T if tos has type named by atom_index, else NIL.	*/
/*									*/
/************************************************************************/

LispPTR N_OP_instancep(LispPTR tos, int atom_index) {
  struct dtd *dtd68k;

  for (dtd68k = (struct dtd *)GetDTD(GetTypeNumber(tos));
#ifdef BIGVM
       atom_index != dtd68k->dtd_name;
#else
       atom_index != dtd68k->dtd_namelo + (dtd68k->dtd_namehi << 16);
#endif /* BIGVM */
       dtd68k = (struct dtd *)GetDTD(dtd68k->dtd_supertype)) {
    if (dtd68k->dtd_supertype == 0) { return (NIL_PTR); }
  }
  return (ATOM_T);

} /* N_OP_instancep END */
