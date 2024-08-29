/* $Id: conspage.c,v 1.3 1999/05/31 23:35:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-94 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/**
 *  @file conspage.c
 */

#include <stddef.h>        // for NULL
#include "address.h"       // for POINTER_PAGE
#include "adr68k.h"        // for NativeAligned4FromLPage, LPageFromNative, ...
#include "allocmdsdefs.h"  // for alloc_mdspage
#include "car-cdrdefs.h"   // for find_close_prior_cell
#include "cell.h"          // for conspage, freecons, FREECONS, CDR_NIL, CON...
#include "commondefs.h"    // for error
#include "conspagedefs.h"  // for N_OP_cons, cons, next_conspage
#include "gcdata.h"        // for GCLOOKUP, ADDREF, DELREF
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "lispemul.h"      // for ConsCell, DLword, LispPTR, NIL_PTR, DLWORD...
#include "lspglob.h"
#include "lsptypes.h"      // for dtd, Listp, TYPE_LISTP

/**
 * Initializes a page of CONS cells, sets up the free count and
 * chains the cells together for ease of searching.
 *
 *	A fresh CONS page looks like this:
 *
 *	+--------+--------+----------------+
 *    0 | count  |nextcell|   (padding)    |	nextcell = 254.
 *	+--------+--------+----------------+
 *    2 |      next_page                   |
 *	+--------+-------------------------+
 *    4 |     0  |         N I L           |
 *	+--------+-------------------------+
 *    6 |     4  |         N I L           |
 *	+--------+-------------------------+
 *  ... |   ...  |         N I L           |
 *	+--------+-------------------------+
 *  254 |   252  |         N I L           |
 *	+--------+-------------------------+
 *
 *	The cells are chained together thru their high 8 bits,
 *	using the (16-bit) word offset within page as the chain.
 *	Cells are chained from the top of the page down.
 *
 *	Experimental version goes nextcell = 248
 *	count/nextcell in cell 4, next_page in cell 6
 *	Chain up 4 down 8 ( ^ 6 into word count)
 *
 * @param[in,out] base Native pointer to cons page
 * @param[in] link Lisp page number of next cons page
 */

static void init_conspage(struct conspage *base, unsigned int link)
{
  ConsCell *cell;
  int j; /* DL-> int */

#ifdef TRACE2
  printf("TRACE: init_conspage()\n");
#endif

#ifdef NEWCDRCODING
  j = 254;
  base->next_cell = 6 ^ j;
  while (j > 8) {
    cell = (ConsCell *)((DLword *)base + (6 ^ j));
    cell->car_field = NIL_PTR;
    j -= 2;
    ((freecons *)cell)->next_free = (6 ^ j);
  }
  cell = (ConsCell *)((DLword *)base + (6 ^ j));
  cell->car_field = NIL_PTR;
  ((freecons *)cell)->next_free = 0;
  base->count = 124;
#else
  base->next_cell = j = 254;
  while (j != 0) {
    cell = (ConsCell *)((DLword *)base + j);
    cell->car_field = NIL_PTR;
    j -= 2;
    ((freecons *)cell)->next_free = j;
  }
  cell = (ConsCell *)((DLword *)base + j);
  cell->car_field = NIL_PTR;
  ((freecons *)cell)->next_free = 0;
  base->count = 127;
#endif /* NEWCDRCODING */
  base->next_page = link;

} /* init_conspage end */

/**********************************************************************/
/*
                Func name :	next_conspage

                GET NEXT CONS PAGE .

                Date :		January 13, 1987
                Edited by :	Takeshi Shimizu
                Changed	:	January 20, 1987 (take)
                Changed :	Feb-12-87 take
                Changed :	Feb-13-87 take

*/
/**********************************************************************/

struct conspage *next_conspage(void) {
  struct conspage *page1; /* Allocated 1st MDS page */
  struct conspage *page2; /* Allocated 2nd MDS page */
  struct conspage *pg, *priorpg;
  LispPTR next, prior;

#ifdef NEWCDRCODING
  /* Allocate 2 conspages and get 1st page base */
  page1 = (struct conspage *)alloc_mdspage(TYPE_LISTP);
  /* Calculate next conspage's base address */
  page2 = (struct conspage *)((DLword *)page1 + DLWORDSPER_PAGE);

  init_conspage(page2, 0); /* No next page */
  init_conspage(page1, LPageFromNative(page2));

  priorpg = NULL;
  prior = 0;
  for (pg = (struct conspage *)NativeAligned4FromLPage(next = ListpDTD->dtd_nextpage);
       next && (next != CONSPAGE_LAST);
       pg = (struct conspage *)NativeAligned4FromLPage(next = pg->next_page)) {
    priorpg = pg;
    prior = next;
  }

  if (prior)
    priorpg->next_page = LPageFromNative(page1);
  else
    ListpDTD->dtd_nextpage = LPageFromNative(page1);

  if (page2->next_page) error("page2 has a next page??");
  if (page2 == priorpg) error("loop in conspage next_pages");
#else
  for (next = (int)ListpDTD->dtd_nextpage; /* /!\ Note no exit condition /!\ */ /* get next free conspage */
       ; ListpDTD->dtd_nextpage = next = page1->next_page, page1->next_page = 0xffff) {
    if (next == 0) {
      /* Allocate 2 conspages and get 1st page base */
      page1 = (struct conspage *)alloc_mdspage(TYPE_LISTP);

      /* Calculate next conspage's base address */
      page2 = (struct conspage *)((DLword *)page1 + DLWORDSPER_PAGE);

      /* XXX: why is the link for page2's next here?
       * when it was previously commented as "Doesn't exist next page"
       */
      init_conspage(page2, ListpDTD->dtd_nextpage);
      init_conspage(page1, LPageFromNative(page2));

      ListpDTD->dtd_nextpage = LPageFromNative(page1);
      goto ex; /* replaced break */
    } else {
      page1 = (struct conspage *)NativeAligned4FromLPage(next); /*Jan-21*/
    }

    if (page1->count > 1) break;

  } /* for loop end */
#endif /* NEWCDRCODING */
ex:
  return (page1);
} /* next_conspage end */

/************************************************************************/
/*									*/
/*			f i n d _ c d r c o d a b l e _ p a i r		*/
/*									*/
/*	Find a pair of CONS cells that are close enough (within 7)	*/
/*	that the second can be cdr-coded as the cdr of the first.	*/
/*	Set up the cdr code in the first cell, and return it.		*/
/*									*/
/*	First searches the CONS page given, then the free-page chain	*/
/*	finally, calls conspage to get a fresh (and guaranteed useful) page.*/
/*									*/
/************************************************************************/

static ConsCell *find_pair_in_page(struct conspage *pg, LispPTR cdrval) {
  ConsCell *carcell, *cdrcell;
  unsigned int offset, prior, priorprior, noffset, nprior, poffset;

  if (pg->count < 2) return ((ConsCell *)0);

  priorprior = prior = nprior = 0;

  for (offset = pg->next_cell; offset; offset = FREECONS(pg, offset)->next_free) {
    if (prior) {
      /* if ((6^prior) <= (6^offset)) error("free list in CONS page corrupt."); */
      if ((prior > offset) && (prior <= offset + 14)) {
        poffset = offset;
        noffset = FREECONS(pg, offset)->next_free;
        while ((noffset > offset) && (noffset < prior)) {
          nprior = offset;
          poffset = prior;
          offset = noffset;
          noffset = FREECONS(pg, offset)->next_free;
        }
        carcell = (ConsCell *)((DLword *)pg + offset);
        cdrcell = (ConsCell *)((DLword *)pg + prior);
        if (priorprior)
          FREECONS(pg, priorprior)->next_free = FREECONS(pg, poffset)->next_free;
        else
          pg->next_cell = FREECONS(pg, poffset)->next_free;
        if (nprior) FREECONS(pg, nprior)->next_free = FREECONS(pg, offset)->next_free;
        carcell->cdr_code = cdrcell - carcell;
#ifdef NEWCDRCODING
        if ((cdrcell - carcell) > 7) error("in find_pair_in_page, cdr code too big.");
        if (254 < (offset + (carcell->cdr_code << 1))) error("in fpip, page overflow.");
#endif /* NEWCDRCODING */
        pg->count -= 2;
        *((LispPTR *)cdrcell) = cdrval;
        return (carcell);
      } else if ((offset > prior) && (offset <= prior + 14)) {
        carcell = (ConsCell *)((DLword *)pg + prior);
        cdrcell = (ConsCell *)((DLword *)pg + offset);
        if (priorprior)
          FREECONS(pg, priorprior)->next_free = ((freecons *)cdrcell)->next_free;
        else
          pg->next_cell = ((freecons *)cdrcell)->next_free;
        carcell->cdr_code = cdrcell - carcell;
#ifdef NEWCDRCODING
        if ((cdrcell - carcell) > 7) error("in find_pair_in_page, cdr code too big.");
        if (254 < (prior + (carcell->cdr_code << 1))) error("in fpip, page overflow.");
#endif /* NEWCDRCODING */
        pg->count -= 2;
        *((LispPTR *)cdrcell) = cdrval;
        return (carcell);
      }
    }
    priorprior = prior;
    prior = offset;
  }
  return ((ConsCell *)0);
}

static ConsCell *find_cdrcodable_pair(LispPTR cdrval) {
  ConsCell *cell;
  struct conspage *pg;
  unsigned pgno = ListpDTD->dtd_nextpage;

  for (pg = (struct conspage *)NativeAligned4FromLPage(pgno); pgno;
       pg = (struct conspage *)NativeAligned4FromLPage(pgno = pg->next_page))
    if ((cell = find_pair_in_page(pg, cdrval))) return (cell);

  pg = next_conspage();
  cell = find_pair_in_page(pg, cdrval);
  return (cell);
} /* end of find_cdrcodable_pair */

static ConsCell *find_free_cons_cell(void) {
  ConsCell *cell;
  struct conspage *pg;
  unsigned pgno = ListpDTD->dtd_nextpage;

  for (pg = (struct conspage *)NativeAligned4FromLPage(pgno); pgno;
       pg = (struct conspage *)NativeAligned4FromLPage(pgno))
    if (pg->count) {
      pg->count--;
      cell = (ConsCell *)(((DLword *)pg) + (pg->next_cell));
      pg->next_cell = ((freecons *)cell)->next_free;
      return (cell);
    } else { /* remove the empty page from the free chain */
      pgno = ListpDTD->dtd_nextpage = pg->next_page;
      pg->next_page = CONSPAGE_LAST;
    }

  return ((ConsCell *)0);
} /* end of find_free_cons_cell */

/**********************************************************************/
/*
        Func name :N_OP_cons
                Execute CONS OPCODE

                        Date :		March 29 1988
                        Edited by :	Bob Krivacic

*/
/**********************************************************************/

LispPTR N_OP_cons(LispPTR cons_car, LispPTR cons_cdr) {
  struct conspage *new_conspage;
  ConsCell *new_cell;
#ifndef NEWCDRCODING
  ConsCell *temp_cell;
#endif
  LispPTR new_page; /* hold the return  val of nextconspage */

  GCLOOKUP(cons_cdr &= POINTERMASK, ADDREF);
  GCLOOKUP(cons_car, ADDREF);

  if (cons_cdr == NIL_PTR) {
#ifdef NEWCDRCODING
    if ((new_cell = find_free_cons_cell())) { /* next page has 1 or more free cells */
#else
    if ((ListpDTD->dtd_nextpage != 0) &&
        (GetCONSCount(ListpDTD->dtd_nextpage) > 0)) { /* next page has 1 or more free cells */
      new_page = ListpDTD->dtd_nextpage;
      new_conspage = (struct conspage *)NativeAligned4FromLPage(new_page);
      if (new_conspage->next_cell == 0) error("count ne 0, but nothing on free chain.");
      new_cell = GetNewCell_68k(new_conspage); /* get new cell */

      new_conspage->count--;                                       /* decrement free cnt. */
      new_conspage->next_cell = ((freecons *)new_cell)->next_free; /* update free cell chain */
#endif /* NEWCDRCODING */

      /* filling new cell with the data */
      new_cell->car_field = cons_car;
      new_cell->cdr_code = CDR_NIL;

      ListpDTD->dtd_cnt0++;

    }      /* if (ListpDTD.. end */
    else { /* Need to get a new CONS page */
      new_conspage = next_conspage();

      new_cell = GetNewCell_68k(new_conspage);

      new_conspage->count--;                                       /* decrement free cnt. */
      new_conspage->next_cell = ((freecons *)new_cell)->next_free; /* update free cell chain */

      /* filling new cell with the data */
      new_cell->car_field = cons_car;
      new_cell->cdr_code = CDR_NIL;

      ListpDTD->dtd_oldcnt++;

    }  /* else 1 end */
  }    /* if(cons_cdr.. end */
  else /* cons_cdr != NIL */
  {
    new_page = POINTER_PAGE(cons_cdr); /* Y's page num */
    new_conspage = (struct conspage *)NativeAligned4FromLPage(new_page);
#ifdef NEWCDRCODING
    if (Listp(cons_cdr) && (new_conspage->count > 0) &&
        (new_cell = find_close_prior_cell(new_conspage, cons_cdr)))
#else
    if (Listp(cons_cdr) && (new_conspage->count > 0))
#endif /* NEWCDRCODING */
    {  /* The cdr is itself a CONS cell, and can be */
       /* represented using CDR_ONPAGE representation */

#ifndef NEWCDRCODING
      new_cell = GetNewCell_68k(new_conspage);
#ifdef DEBUG
      if (new_cell->car_field != NIL) {
        printf("CELL 0x%x has non-NIL car = 0x%x \n", LAddrFromNative(new_cell),
               new_cell->car_field);
        error("QUIT from N_OP_cons");
      }
#endif

      new_conspage->count--;                                       /* decrement free cnt. */
      new_conspage->next_cell = ((freecons *)new_cell)->next_free; /* update free cell chain */
#endif                                                             /*NEWCDRCODING */

      new_cell->car_field = cons_car;
/* cdr_onpage + cell offset in this conspage */
#ifdef NEWCDRCODING
#else
      new_cell->cdr_code = CDR_ONPAGE | ((cons_cdr & 0xff) >> 1);
#endif /* NEWCDRCODING */
      ListpDTD->dtd_cnt0++;

    } /* if (listp.. end */
    else {
/* UFN case : CDR_INDIRECT */
#ifdef NEWCDRCODING
      new_cell = find_cdrcodable_pair(cons_cdr);
#else
      new_conspage = next_conspage();

      /* get 2 cells from conspage */
      temp_cell = GetNewCell_68k(new_conspage);
#ifdef DEBUG
      if (temp_cell->car_field != NIL) {
        printf("CDR indirect CELL 0x%x has non-NIL car 0x%x \n", LAddrFromNative(new_cell),
               temp_cell->car_field);
        error("QUIT from N_OP_cons");
      }
#endif

      new_conspage->next_cell = ((freecons *)temp_cell)->next_free; /* update free cell chain */
      new_cell = GetNewCell_68k(new_conspage);
#ifdef DEBUG
      if (new_cell->car_field != NIL) {
        printf("CDR ind-2 CELL 0x%x has non-NIL car = 0x%x \n", LAddrFromNative(new_cell),
               new_cell->car_field);
        error("QUIT from N_OP_cons");
      }
#endif

      new_conspage->next_cell = ((freecons *)new_cell)->next_free; /* update free cell chain */
      new_conspage->count -= 2;                                    /* decrement free cnt. */

      /* filling cell */
      *((LispPTR *)temp_cell) = cons_cdr; /* Indirect CDR ptr */
#endif /* NEWCDRCODING */
      new_cell->car_field = cons_car;

#ifndef NEWCDRCODING
      /* culc. cdr code */
      new_cell->cdr_code = (((LispPTR)LAddrFromNative(temp_cell)) & 0xff) >> 1;
#endif /* NEWCDRCODING */

      ListpDTD->dtd_oldcnt++; /* added feb-12 take */

    } /* else end */

  } /* else (cons_cdr==NIL end) */

  new_page = LAddrFromNative(new_cell);
  GCLOOKUP(new_page, DELREF);
#ifdef NEWCDRCODING
  if (254 < ((new_page & 0xff) + ((new_cell->cdr_code & 7) << 1)))
    error("in CONS, cdr code too big.");
#endif /* NEWCDRCODING */
  return (new_page);

} /* N_OP_cons() end */

/**********************************************************************/
/* function cons same as N_OP_cons				      */
/**********************************************************************/

LispPTR cons(LispPTR cons_car, LispPTR cons_cdr) { return (N_OP_cons(cons_car, cons_cdr)); }
