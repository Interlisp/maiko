/* $Id: mkcell.c,v 1.3 1999/05/31 23:35:39 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

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

#include "adr68k.h"        // for NativeAligned2FromLAddr
#include "allocmdsdefs.h"  // for alloc_mdspage, initmdspage
#include "commondefs.h"    // for error
#include "emlglob.h"
#include "gcdata.h"        // for DELREF, GCLOOKUP
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "lispemul.h"      // for LispPTR, DLword, NIL, POINTERMASK, state
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"      // for dtd, GETWORD, GetDTD
#include "mkcelldefs.h"    // for N_OP_createcell, createcell68k
#ifdef DTDDEBUG
#include "testtooldefs.h"
#endif

static LispPTR oldoldfree;
static LispPTR oldfree;

LispPTR N_OP_createcell(LispPTR tos) {
  struct dtd *dtd68k;
  DLword *ptr, *lastptr;
  LispPTR newcell;
  unsigned int type;

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
    ptr = (DLword *)NativeAligned2FromLAddr(newcell);
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

void *createcell68k(unsigned int type) {
  struct dtd *dtd68k;
  DLword *ptr, *lastptr;
  LispPTR newcell;
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

    ptr = NativeAligned2FromLAddr(newcell);

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

    return (NativeAligned2FromLAddr(newcell)); /* XXX: is it really only aligned(2)? */

  } else {
    dtd68k->dtd_free = initmdspage(alloc_mdspage(dtd68k->dtd_typeentry), dtd68k->dtd_size, NIL);
    if (dtd68k->dtd_free & 0x8000000) error("bad entry on free chain.");

#ifdef DTDDEBUG
    check_dtd_chain(type);
#endif

    goto retry;
  }

} /* createcell68k end */
