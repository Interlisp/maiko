/* $Id: rplcons.c,v 1.3 1999/05/31 23:35:41 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
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
                File Name :	rplcons.c

                Desc	:	rplcons

                Including :	rplcons
                                OP_rplcons

*/
/**********************************************************************/

#include "car-cdrdefs.h"   // for N_OP_rplacd
#include "conspagedefs.h"  // for cons
#include "emlglob.h"
#include "lispemul.h"      // for LispPTR, state, ERROR_EXIT, NIL_PTR
#include "lspglob.h"
#include "lsptypes.h"      // for Listp
#include "rplconsdefs.h"   // for N_OP_rplcons
#ifndef NEWCDRCODING
#include "gcdata.h"
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "address.h"
#endif

/***************************************************/

LispPTR N_OP_rplcons(LispPTR list, LispPTR item) {
#ifndef NEWCDRCODING
  struct conspage *conspage;
  ConsCell *new_cell;
  ConsCell *list68k;
  LispPTR page;
#endif

  if (!Listp(list)) ERROR_EXIT(item);

/* There are some rest Cell and "list" must be ONPAGE cdr_coded */
#ifndef NEWCDRCODING
  page = POINTER_PAGE(list);
  list68k = (ConsCell *)NativeAligned4FromLAddr(list);

  if ((GetCONSCount(page) != 0) && (list68k->cdr_code > CDR_MAXINDIRECT)) {
    GCLOOKUP(item, ADDREF);
    GCLOOKUP(cdr(list), DELREF);

    conspage = (struct conspage *)NativeAligned4FromLPage(page);
    new_cell = (ConsCell *)GetNewCell_68k(conspage);

    conspage->count--;
    conspage->next_cell = ((freecons *)new_cell)->next_free;

    new_cell->car_field = item;
    new_cell->cdr_code = CDR_NIL;

    ListpDTD->dtd_cnt0++;

    list68k->cdr_code = CDR_ONPAGE | ((LAddrFromNative(new_cell) & 0xff) >> 1);

    return (LAddrFromNative(new_cell));

  } else
#endif /* ndef NEWCDRCODING */
  {
    N_OP_rplacd(list, item = cons(item, NIL_PTR));
    return (item);
  }
}
