/* $Id: typeof.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: typeof.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ Copyright (C) Venue";

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

/****************************************************************/
/* LISTP(3Q),NTYPEX(4Q),TYPEP(5Q),DTEST(6Q) ,INSTANCEP(303Q)    */
/****************************************************************/
/*
                                changed : Jan. 13 1987 take
                                changed : Feb. 05 1987 take
                                changed : Jul. 24 1987 take

*/

#include "lispemul.h"
#include "lsptypes.h"
#include "cell.h"
#include "lispmap.h"
#include "lspglob.h"

/************************************************************************/
/*									*/
/*			N _ O P _ d t e s t				*/
/*									*/
/*	Check for type conformity, else error.				*/
/*									*/
/************************************************************************/

LispPTR N_OP_dtest(register LispPTR tos, register int atom_index) {
  register struct dtd *dtd68k;

  for (dtd68k = (struct dtd *)GetDTD(GetTypeNumber(tos));
#ifdef BIGVM
       atom_index != dtd68k->dtd_name;
#else
       atom_index != dtd68k->dtd_namelo + (dtd68k->dtd_namehi << 16);
#endif /* BIGVM */
       dtd68k = (struct dtd *)GetDTD(dtd68k->dtd_supertype)) {
    if (dtd68k->dtd_supertype == 0) ERROR_EXIT(tos);
  }
  return (tos);
} /* OP_DTEST END */

/************************************************************************/
/*									*/
/*			N _ O P _ i n s t a n c e p			*/
/*									*/
/*	Returns T if tos has type named by atom_index, else NIL.	*/
/*									*/
/************************************************************************/

LispPTR N_OP_instancep(register LispPTR tos, register int atom_index) {
  register unsigned int type;
  register struct dtd *dtd68k;

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
