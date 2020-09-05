/* $Id: mkcell.c,v 1.3 1999/05/31 23:35:39 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: mkcell.c,v 1.3 1999/05/31 23:35:39 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***********************************************************************/
/*
                File Name :	mkcell.c

                Desc	:

                                Date :		Jun. 4, 1987
                                Edited by :	Takeshi Shimizu
                                Changed :	9 Jun 1987 take
                                                26 Oct. 1987 take(add mask)

                Including :	OP_createcell


*/
/**********************************************************************/

#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "cell.h"
#include "gc.h"

#include "mkcelldefs.h"
#include "allocmdsdefs.h"
#include "commondefs.h"
#include "gchtfinddefs.h"

static LispPTR oldoldfree;
static LispPTR oldfree;

LispPTR *alloc_mdspage(register short int type);
LispPTR initmdspage(register LispPTR *base, register DLword size, register LispPTR prev);

LispPTR N_OP_createcell(register LispPTR tos) {
  register struct dtd *dtd68k;
  register DLword *ptr, *lastptr;
  register LispPTR newcell;
  register unsigned int type;

  if ((tos & SEGMASK) != S_POSITIVE) ERROR_EXIT(tos);
  type = tos & 0xffff;

#ifdef DTDDEBUG
  if (type == TYPE_LISTP) error("N_OP_createcell : Can't create Listp cell with CREATECELL");
  check_dtd_chain(type);
#endif

  dtd68k = (struct dtd *)GetDTD(type);

  oldoldfree = oldfree;
  oldfree = dtd68k->dtd_free;

  if (dtd68k->dtd_size == 0) ERROR_EXIT(tos);
/* error("OP_createcell : Attempt to create a cell not declared yet"); */

retry:
  if ((tos = newcell = ((dtd68k->dtd_free) & POINTERMASK)) != NIL) {
    ptr = (DLword *)Addr68k_from_LADDR(newcell);
    if (917505 == *(LispPTR *)ptr) error("N_OP_createcell E0001 error");
    /* replace dtd_free with newcell's top DLword (it may keep next chain)*/
    dtd68k->dtd_free = (*((LispPTR *)ptr)) & POINTERMASK;
    if (dtd68k->dtd_free & 0x8000001) error("bad entry on free chain.");

    dtd68k->dtd_oldcnt++;

    /* clear 0  */
    for (lastptr = ptr + dtd68k->dtd_size; ptr != lastptr; ptr++) { GETWORD(ptr) = 0; }

    /*	 IncAllocCnt(1); */
    GCLOOKUP(tos, DELREF);
    return (tos);
  } else {
    dtd68k->dtd_free = initmdspage(alloc_mdspage(dtd68k->dtd_typeentry), dtd68k->dtd_size, NIL);
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");
    goto retry;
  }

} /* N_OP_createcell end */

DLword *createcell68k(unsigned int type) {
  register struct dtd *dtd68k;
  register DLword *ptr, *lastptr;
  register LispPTR newcell;
#ifdef DTDDEBUG
  if (type == TYPE_LISTP) error("createcell : Can't create Listp cell with CREATECELL");
  if (type == TYPE_STREAM) stab();

  check_dtd_chain(type);

#endif

  dtd68k = (struct dtd *)GetDTD(type);

  if (dtd68k->dtd_size == 0) error("createcell : Attempt to create a cell not declared yet");

retry:
  if ((newcell = (dtd68k->dtd_free & POINTERMASK)) != NIL) {
#ifdef DTDDEBUG
    if (type != GetTypeNumber(newcell)) error("createcell : BAD cell in dtdfree");
    if (newcell > POINTERMASK) error("createcell : BAD Lisp address");
#endif

    ptr = Addr68k_from_LADDR(newcell);

    if (917505 == *(LispPTR *)ptr) error("N_OP_createcell E0001 error");

    /* replace dtd_free with newcell's top DLword (it may keep next chain)*/
    dtd68k->dtd_free = (*((LispPTR *)ptr)) & POINTERMASK;
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");

#ifdef DTDDEBUG
    if ((dtd68k->dtd_free != 0) && (type != GetTypeNumber(dtd68k->dtd_free)))
      error("createcell : BAD cell in next dtdfree");
    check_dtd_chain(type);

#endif

    dtd68k->dtd_oldcnt++;

    /* clear 0  */
    for (lastptr = ptr + dtd68k->dtd_size; ptr != lastptr; ptr++) { GETWORD(ptr) = 0; }

    /*	IncAllocCnt(1); */
    GCLOOKUP(newcell, DELREF);

#ifdef DTDDEBUG
    check_dtd_chain(type);
#endif

    return (Addr68k_from_LADDR(newcell));

  } else {
    dtd68k->dtd_free = initmdspage(alloc_mdspage(dtd68k->dtd_typeentry), dtd68k->dtd_size, NIL);
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");

#ifdef DTDDEBUG
    check_dtd_chain(type);
#endif

    goto retry;
  }

} /* createcell68k end */

/**********************************************************/
/*  Create a Cell of Specified Type & Set to given Value  */
/*  Works with 32 bit typed values only.                  */
/*  (Initially used only by native code)                  */
/**********************************************************/

LispPTR Create_n_Set_Cell(unsigned int type, LispPTR value) {
  register struct dtd *dtd68k;
  register DLword *ptr, *lastptr;
  register LispPTR newcell;

  dtd68k = (struct dtd *)GetDTD(type);

  if (dtd68k->dtd_size == 0) error("createcell : Attempt to create a cell not declared yet");

retry:
  if ((newcell = (dtd68k->dtd_free & POINTERMASK)) != NIL) {
    ptr = Addr68k_from_LADDR(newcell);

    /* replace dtd_free with newcell's top DLword (it may keep next chain)*/

    dtd68k->dtd_free = (*((LispPTR *)ptr)) & POINTERMASK;
    dtd68k->dtd_oldcnt++;
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");

    /* clear 0  */
    for (lastptr = ptr + dtd68k->dtd_size; ptr != lastptr; ptr++) { GETWORD(ptr) = 0; }

    /*	IncAllocCnt(1); */
    GCLOOKUP(newcell, DELREF);
    (*((LispPTR *)Addr68k_from_LADDR(newcell))) = value;
    return (newcell);

  } else {
    dtd68k->dtd_free = initmdspage(alloc_mdspage(dtd68k->dtd_typeentry), dtd68k->dtd_size, NIL);
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");

    goto retry;
  }

} /* createcell68k end */
