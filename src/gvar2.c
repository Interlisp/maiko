/* $Id: gvar2.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */
static char *id = "$Id: gvar2.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "adr68k.h"
#include "gc.h"
#include "emlglob.h"
#include "cell.h"
#include "dbprint.h"

#include "gvar2defs.h"
#include "gchtfinddefs.h"

/************************************************************************/
/*									*/
/*			    N _ O P _ g v a r _				*/
/*									*/
/*	GVAR_ opcode (027).  Assign a value to a global variable.	*/
/*									*/
/*	atom_index is the "atom number," either the lo half of the	*/
/*	old litatom, or the new-atom itself.				*/
/*									*/
/*	* call gclookup with DELREF and address of GVAR slot.		*/
/*	* call gclookup with ADDREF and TopOFStack.			*/
/*	* replace GVAR slot with tos.					*/
/*	* If Hash Table is overflow, call fn1ext.			*/
/*									*/
/************************************************************************/

LispPTR N_OP_gvar_(register LispPTR tos, unsigned int atom_index) {
  register LispPTR *pslot; /* pointer to argued GVAR slot */

#ifdef BIGATOMS
  if (0 != (atom_index & SEGMASK))
    pslot = (LispPTR *)Addr68k_from_LADDR(atom_index + NEWATOM_VALUE_OFFSET);
  else
#endif /* BIGATOMS */

#ifdef BIGVM
    pslot = ((LispPTR *)AtomSpace) + (5 * atom_index) + NEWATOM_VALUE_PTROFF;
#else
    pslot = (LispPTR *)Valspace + atom_index;
#endif /* BIGVM */
  DEBUGGER(if (tos & 0xF0000000) error("Setting GVAR with high bits on"));
  FRPLPTR(((struct xpointer *)pslot)->addr, tos);
  return (tos);
}

/************************************************************************/
/*									*/
/*			    N _ O P _ r p l p t r			*/
/*									*/
/*	RPLPTR opcode (024).  Replace a pointer field somewhere,	*/
/*	updating the reference counts for the old value and the new	*/
/*	value (DELREF and ADDREF, respectively).			*/
/*									*/
/*	tos_m_1 is the base, and alpha is a word-offset for finding	*/
/*	the cell to replace contents of.				*/
/*	tos is the new value.						*/
/*									*/
/************************************************************************/

LispPTR N_OP_rplptr(register LispPTR tos_m_1, register LispPTR tos, unsigned int alpha) {
  register DLword *pslot; /* pointer to argued slot (68 address) */

  pslot = Addr68k_from_LADDR(tos_m_1 + alpha);
  FRPLPTR(((struct xpointer *)pslot)->addr, tos);
  return (tos_m_1);
}
