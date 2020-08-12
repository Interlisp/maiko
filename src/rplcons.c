/* $Id: rplcons.c,v 1.3 1999/05/31 23:35:41 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: rplcons.c,v 1.3 1999/05/31 23:35:41 sybalsky Exp $ Copyright (C) Venue";

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

#include "lispemul.h"
#include "emlglob.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "gc.h"
#include "cell.h"
#include "conspage.h"
#include "car-cdr.h"

/***************************************************/

LispPTR N_OP_rplcons(register LispPTR list, register LispPTR item) {
  register struct conspage *conspage;
  register ConsCell *new_cell, *list68k;

  LispPTR register page;

  if (!Listp(list)) ERROR_EXIT(item);

  page = POINTER_PAGE(list);
  list68k = (ConsCell *)Addr68k_from_LADDR(list);

/* There are some rest Cell and "list" must be ONPAGE cdr_coded */
#ifndef NEWCDRCODING
  if ((GetCONSCount(page) != 0) && (list68k->cdr_code > CDR_MAXINDIRECT)) {
    GCLOOKUP(item, ADDREF);
    GCLOOKUP(cdr(list), DELREF);

    conspage = (struct conspage *)Addr68k_from_LPAGE(page);
    new_cell = (ConsCell *)GetNewCell_68k(conspage);

    conspage->count--;
    conspage->next_cell = ((freecons *)new_cell)->next_free;

    new_cell->car_field = item;
    new_cell->cdr_code = CDR_NIL;

    ListpDTD->dtd_cnt0++;

    list68k->cdr_code = CDR_ONPAGE | ((LADDR_from_68k(new_cell) & 0xff) >> 1);

    return (LADDR_from_68k(new_cell));

  } else
#endif /* ndef NEWCDRCODING */
  {
    N_OP_rplacd(list, item = cons(item, NIL_PTR));
    return (item);
  }
}
