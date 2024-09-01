/* $Id: gvar2.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"      // for NativeAligned4FromLAddr
#include "cell.h"        // for xpointer
#include "commondefs.h"  // for error
#include "dbprint.h"     // for DEBUGGER
#include "emlglob.h"
#include "gcdata.h"      // for FRPLPTR
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "gvar2defs.h"   // for N_OP_gvar_, N_OP_rplptr
#include "lispemul.h"    // for LispPTR, DLword, NEWATOM_VALUE_OFFSET, NEWAT...
#include "lspglob.h"     // for AtomSpace
#include "lsptypes.h"

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

LispPTR N_OP_gvar_(LispPTR tos, unsigned int atom_index) {
  LispPTR *pslot; /* Native pointer to GVAR slot of atom */

#ifdef BIGATOMS
  if (0 != (atom_index & SEGMASK))
    pslot = (LispPTR *)NativeAligned4FromLAddr(atom_index + NEWATOM_VALUE_OFFSET);
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

LispPTR N_OP_rplptr(LispPTR tos_m_1, LispPTR tos, unsigned int alpha) {
  struct xpointer *pslot;

  pslot = (struct xpointer *)NativeAligned4FromLAddr(tos_m_1 + alpha);
  FRPLPTR(pslot->addr, tos);
  return (tos_m_1);
}
