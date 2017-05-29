/* $Id: atom.c,v 1.3 1999/05/31 23:35:23 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: atom.c,v 1.3 1999/05/31 23:35:23 sybalsky Exp $ Copyright (C) Venue";

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

/**********************************************************************/
/*
                File Name :	atom.c

                Desc	:	implement opcode ATOMCELL.N

                                        Date :		Apr 13, 1987
                                        Edited by :	Naoyuki Mitani

                Including :	OP_atomcellN

*/
/**********************************************************************/

#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"

#ifndef BIGATOMS
N_OP_atomcellN(tos, n) register int tos;
int n;
{
  if ((tos & 0xffff0000) != 0) ERROR_EXIT(tos);

  tos = (tos << 1);

  switch (n) {
    case D_DEFSHI: return (DEFS_OFFSET + tos);
    case D_VALSHI: return (VALS_OFFSET + tos);
    case D_PLISHI: return (PLIS_OFFSET + tos);
    case D_PNHI: return (PNP_OFFSET + tos);
    default: ERROR_EXIT(tos);
  }
}

#else
N_OP_atomcellN(register int tos, int n) {
  if ((tos & 0xffff0000) == 0) { /* XeroxLisp traditional symbol */
    tos = (tos << 1);
    switch (n) {
      case D_DEFSHI: return (DEFS_OFFSET + tos);
      case D_VALSHI: return (VALS_OFFSET + tos);
      case D_PLISHI: return (PLIS_OFFSET + tos);
      case D_PNHI: return (PNP_OFFSET + tos);
      default: ERROR_EXIT(tos);
    }
  } else { /* New Symbol */
    switch (n) {
      case D_DEFSHI: return (NEWATOM_DEFN_OFFSET + tos);
      case D_VALSHI: return (NEWATOM_VALUE_OFFSET + tos);
      case D_PLISHI: return (NEWATOM_PLIST_OFFSET + tos);
      case D_PNHI: return (NEWATOM_PNAME_OFFSET + tos);
      default: ERROR_EXIT(tos);
    }
  }
}

#endif
