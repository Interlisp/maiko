/* $Id: car-cdr.c,v 1.3 1999/05/31 23:35:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
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
                File Name :	car-cdr.c

                Desc	:	car-cdr management

                                        Date :		Apr 24, 1987
                                        Edited by :	Naoyuki Mitani

                Including :	car
                                cdr
                                rplaca
                                rplacd
                                OP_car
                                OP_cdr
                                OP_rplaca
                                OP_rplacd
*/
/**********************************************************************/

#include "address.h"       // for POINTER_PAGEBASE
#include "adr68k.h"        // for NativeAligned4FromLAddr, NativeAligned4FromLPage
#include "car-cdrdefs.h"   // for N_OP_car, N_OP_cdr, N_OP_rplaca, N_OP_rplacd
#include "cell.h"          // for freecons, conspage, FREECONS, CDR_INDIRECT
#include "commondefs.h"    // for error
#include "conspagedefs.h"  // for next_conspage
#include "emlglob.h"
#include "gcdata.h"        // for GCLOOKUP, ADDREF, DELREF
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "lispemul.h"      // for ConsCell, LispPTR, DLword, NIL_PTR, state
#include "lspglob.h"       // for ListpDTD
#include "lsptypes.h"      // for Listp, dtd

/************************************************************************/
/*									*/
/*				c a r					*/
/*									*/
/*	Returns CAR of its argument.  Meant to be called from C.	*/
/*									*/
/************************************************************************/

LispPTR car(LispPTR datum)
/* datum must be LISP pointer(word offset) */
{
  if (datum == NIL_PTR || datum == ATOM_T) {
    return datum;
  }
  if (Listp(datum)) {
    ConsCell *datum68k = (ConsCell *)(NativeAligned4FromLAddr(datum));
    if (datum68k->cdr_code == CDR_INDIRECT) {
      ConsCell *temp = (ConsCell *)NativeAligned4FromLAddr(datum68k->car_field);
      return (LispPTR)temp->car_field;
    }
    return (LispPTR)datum68k->car_field;
  }
  /** We assume CAR/CDRERR is CDR ***/
  if ((datum & SEGMASK) == 0) { /* LITATOM */
    return NIL_PTR;
  }
  error("car : ARG not list");
  return NIL_PTR; /* NOT REACHED */
} /* end of car */

/************************************************************************/
/*									*/
/*				c d r					*/
/*									*/
/*	Returns CDR of its argument.  Meant to be called from C.	*/
/*									*/
/************************************************************************/

LispPTR cdr(LispPTR datum)
/* datum must be LISP pointer(word offset) */
{
  ConsCell *datum68k;
  DLword cdr_code;
  ConsCell *temp;

  if (datum == NIL_PTR) {
    return NIL_PTR;
  }
  if (!Listp(datum)) {
    error("cdr : ARG not list");
    return NIL_PTR;
  }

  datum68k = (ConsCell *)NativeAligned4FromLAddr(datum);
  cdr_code = datum68k->cdr_code;

  if (cdr_code == CDR_NIL) {
    return NIL_PTR; /* cdr is nil */
  }
  if ((cdr_code & CDR_ONPAGE) != 0) { /* cdr-samepage */
#ifdef NEWCDRCODING
    return datum + ((cdr_code & 7) << 1);
#else
    return POINTER_PAGEBASE(datum) + ((cdr_code & 127) << 1);
#endif /* NEWCDRCODING */
  }
  if (cdr_code == CDR_INDIRECT) { /* cdr_code > CDR_ONPAGE cdr-indirect */
    return cdr((LispPTR)(datum68k->car_field));
  }
  /* cdr isn't a CONS, but is stored on this page. */
#ifdef NEWCDRCODING
  temp = (ConsCell *)NativeAligned4FromLAddr(datum + (cdr_code << 1));
#else
  temp = (ConsCell *)NativeAligned4FromLAddr(POINTER_PAGEBASE(datum) + (cdr_code << 1));
#endif /* NEWCDRCODING */
  return (LispPTR)temp->car_field;
} /* end of cdr */

/**********************************************************************/
/*
                Func name :	rplaca

                                Called from C program.

                                        Date :		Apr 15, 1987
                                        Edited by :	Naoyuki Mitani
*/
/**********************************************************************/
/**
 * Replace car of x with y
 *
 * @param x [in,out] LispPTR to object in which car will be replaced.
 * @param y [in] LispPTR to object that will become new car of x.
 * @return x, modified, or NIL if x is not a list.
 */
LispPTR rplaca(LispPTR x, LispPTR y) {
  ConsCell *x_68k;
  ConsCell *temp;

#ifdef TRACE2
  printf("TRACE: rplaca()\n");
#endif

  /* fast cases before checking type */
  if (x == NIL_PTR) {
    if (y == NIL_PTR) {
      return NIL_PTR;
    }
    error("Attempt to RPLACA NIL");
    return (NIL_PTR);
  }
  if (!Listp(x)) { /* arg isn't a CONS cell */
    error("ARG not List");
    return (NIL_PTR); /* NOT REACHED */
  }

  x_68k = (ConsCell *)NativeAligned4FromLAddr(x);

  GCLOOKUP(car(x), DELREF); /* set up reference count */
  GCLOOKUP(y, ADDREF);

  if (x_68k->cdr_code == CDR_INDIRECT) {
    temp = (ConsCell *)NativeAligned4FromLAddr((LispPTR)x_68k->car_field);
    temp->car_field = y;
  } else {
    x_68k->car_field = y;
  }
  return x;
} /* end of rplaca */

/**********************************************************************/
/*
                Func name :	rplacd

                                Called from C program.

                                        Date :		Apr 16, 1987
                                        Edited by :	Naoyuki Mitani
*/
/**********************************************************************/
#ifdef NEWCDRCODING
static ConsCell *find_cdrable_pair(LispPTR carpart, LispPTR cdrpart); /* below... */
static ConsCell *find_close_cell(struct conspage *page, LispPTR oldcell);
#endif
/**
 * Replace cdr of x with y
 *
 * @param x [in,out] LispPTR to object in which cdr will be replaced.
 * @param y [in] LispPTR to object that will become new cdr of x.
 * @return x, modified, or errors if x is not a list.
 */

LispPTR rplacd(LispPTR x, LispPTR y) {
  ConsCell *x_68k;
  ConsCell *temp68k;
  ConsCell *cdr_cell68k;
  LispPTR cdr_cell;
  LispPTR rp_page;
  DLword cdr_code;
  struct conspage *cons68k;

  /* fast cases before checking type */
  if (x == NIL_PTR) {
    if (y == NIL_PTR) {
      return NIL_PTR;
    }
    error("Attempt to RPLACD NIL");
    return NIL_PTR;
  }
  if (!Listp(x)) {
    error("ARG not List");
    return NIL_PTR;
  }

  x_68k = (ConsCell *)NativeAligned4FromLAddr(x);

  GCLOOKUP(cdr(x), DELREF); /* set up reference count */
  GCLOOKUP(y, ADDREF);

  cdr_code = x_68k->cdr_code;

  if (cdr_code == CDR_INDIRECT) {
    /* cdr-indirect */

    rp_page = (LispPTR)x_68k->car_field;
    temp68k = (ConsCell *)NativeAligned4FromLAddr(rp_page);
#ifdef NEWCDRCODING
    cdr_cell = (rp_page) + (temp68k->cdr_code << 1);
#else
    cdr_cell = POINTER_PAGEBASE(rp_page) + (temp68k->cdr_code << 1);
#endif /* NEWCDRCODING */

    cdr_cell68k = (ConsCell *)NativeAligned4FromLAddr(cdr_cell);
    *(LispPTR *)cdr_cell68k = y & POINTERMASK; /* cdr_code is set to 0 */
  } else if (cdr_code <= CDR_MAXINDIRECT) {
    /* cdr-differentpage */
#ifdef NEWCDRCODING
    cdr_cell = x + (cdr_code << 1);
#else
    cdr_cell = POINTER_PAGEBASE(x) + (cdr_code << 1);
#endif /* NEWCDRCODING */
    cdr_cell68k = (ConsCell *)NativeAligned4FromLAddr(cdr_cell);
    *(LispPTR *)cdr_cell68k = y & POINTERMASK; /* cdr_code is set to 0 */

  } else if (y == NIL_PTR) {
    /* cdr-samepage & y is nil */
    x_68k->cdr_code = CDR_NIL;
#ifdef NEWCDRCODING
  } else if (((rp_page = POINTER_PAGEBASE(x)) == POINTER_PAGEBASE(y)) && (y > x) &&
             (y <= (x + 14))) {
    /* cdr-samepage & x and y are on same page */
    x_68k->cdr_code = CDR_ONPAGE + ((y - x) >> 1);
#else
  } else if ((rp_page = POINTER_PAGEBASE(x)) == POINTER_PAGEBASE(y)) {
        /* cdr-samepage & x and y are on same page */
        x_68k->cdr_code = CDR_ONPAGE + ((y & 0xff) >> 1);
#endif /* NEWCDRCODING */
  } else {
    /* cdr-samepage & x and y are on different page */

    cons68k = (struct conspage *)(NativeAligned4FromLAddr(rp_page));
#ifdef NEWCDRCODING
    if ((cons68k->count > 0) && (cdr_cell68k = find_close_cell(cons68k, x))) {
      /* at least one free-cell on x's conspage */
      /* AND it's within CDR-code range of x. */

      *(LispPTR *)cdr_cell68k = y & POINTERMASK; /* cdr_code set to 0 */

      x_68k->cdr_code = (LAddrFromNative(cdr_cell68k) - x) >> 1;
    }
#else
    if (cons68k->count > 0) {
      /* at least one free-cell on x's conspage */
      cdr_cell68k = GetNewCell_68k(cons68k);
      cons68k->count--;
      cons68k->next_cell = ((freecons *)cdr_cell68k)->next_free;

      *(LispPTR *)cdr_cell68k = y & POINTERMASK; /* cdr_code set to 0 */

      x_68k->cdr_code = (LAddrFromNative(cdr_cell68k) - rp_page) >> 1;
    }
#endif /* NEWCDRCODING */
    else {
      /* no more free-cell on x's conspage */
#ifdef NEWCDRCODING
      temp68k = (ConsCell *)find_cdrable_pair(x, y);
      temp68k->car_field = x_68k->car_field;
      x_68k->car_field = LAddrFromNative(temp68k);
      x_68k->cdr_code = CDR_INDIRECT;
#else
      cons68k = next_conspage();

      cdr_cell68k = GetNewCell_68k(cons68k);
      cons68k->next_cell = ((freecons *)cdr_cell68k)->next_free;
      temp68k = GetNewCell_68k(cons68k);
      cons68k->next_cell = ((freecons *)temp68k)->next_free;

      cons68k->count -= 2;

      *(LispPTR *)cdr_cell68k = y & POINTERMASK; /* cdr_code set to 0 */

      temp68k->car_field = x_68k->car_field;
      x_68k->car_field = LAddrFromNative(temp68k);

      temp68k->cdr_code = (LAddrFromNative(cdr_cell68k) & 0xff) >> 1;

      x_68k->cdr_code = CDR_INDIRECT;
#endif /* NEWCDRCODING */
    }
  }
  return x;
} /* end of rplacd */

/**********************************************************************/
/*
                Func name :	N_OP_car

                                car management

                                        Date :		March 21, 1988
                                        Edited by :	Robert Krivacic

*/
/**********************************************************************/

LispPTR N_OP_car(LispPTR tos) {
  ConsCell *datum68k;

  if (tos == NIL_PTR || tos == ATOM_T) {
    return tos;
  }
  if (!Listp(tos)) {
    ERROR_EXIT(tos);
    return NIL_PTR;
  }
  datum68k = (ConsCell *)NativeAligned4FromLAddr(tos);
  if (datum68k->cdr_code == CDR_INDIRECT) {
    ConsCell *temp = (ConsCell *)NativeAligned4FromLAddr(datum68k->car_field);
    return (LispPTR)temp->car_field;
  }
  return (LispPTR)datum68k->car_field;
} /* end of N_OP_car */

/**********************************************************************/
/*
                Func name :	N_OP_cdr

                                cdr management

                                        Date :		March 21, 1988
                                        Edited by :	Robert Krivacic
*/
/**********************************************************************/

LispPTR N_OP_cdr(LispPTR tos) {
  ConsCell *datum68k;
  DLword cdr_code;

  if (tos == NIL_PTR) {
    return (tos);
  }
  if (!Listp(tos)) {
    ERROR_EXIT(tos);
  }

  datum68k = (ConsCell *)NativeAligned4FromLAddr(tos);
  cdr_code = datum68k->cdr_code;

  if (cdr_code == CDR_NIL) {
    return NIL_PTR; /* cdr-nil */
  }
  if (cdr_code > CDR_ONPAGE) { /* cdr-samepage */
#ifdef NEWCDRCODING
    return tos + ((cdr_code & 7) << 1);
#else
    return POINTER_PAGEBASE(tos) + ((cdr_code & 127) << 1);
#endif /*NEWCDRCODING */
  }
  if (cdr_code == CDR_INDIRECT) { /* cdr-indirect */
    return cdr((LispPTR)(datum68k->car_field));
  }
  /* cdr-differentpage */
#ifdef NEWCDRCODING
  return (LispPTR)((ConsCell *)(NativeAligned4FromLAddr(tos + (cdr_code << 1))))->car_field;
#else
  return (LispPTR)((ConsCell *)(NativeAligned4FromLAddr(POINTER_PAGEBASE(tos) + (cdr_code << 1))))
      ->car_field;
#endif /*NEWCDRCODING */
} /* end of N_OP_cdr */

/**********************************************************************/
/*

                Func name :	N_OP_rplaca

                                rplaca management

                                        Date :		March 21, 1988
                                        Edited by :	Robert Krivacic
*/
/**********************************************************************/

LispPTR N_OP_rplaca(LispPTR tosm1, LispPTR tos) {
  ConsCell *x_68k;
  ConsCell *temp;

  if (tosm1 == NIL_PTR) {
    if (tos == NIL_PTR) {
      return NIL_PTR;
    }
    ERROR_EXIT(tos);
  }
  if (!Listp(tosm1)) {
    ERROR_EXIT(tos);
  }

  x_68k = (ConsCell *)NativeAligned4FromLAddr(tosm1);

  GCLOOKUP(car(tosm1), DELREF); /* set up reference count */
  GCLOOKUP(tos, ADDREF);

  if (x_68k->cdr_code == CDR_INDIRECT) {
    temp = (ConsCell *)NativeAligned4FromLAddr((LispPTR)x_68k->car_field);
    temp->car_field = tos;
  } else {
    x_68k->car_field = tos;
  }
  return (tosm1);
} /* end of N_OP_rplaca */

/**********************************************************************/
/*
                Func name :	N_OP_rplacd

                                rplacd management

                                        Date :		March 21, 1988
                                        Edited by :	Robert Krivacic


*/
/**********************************************************************/

LispPTR N_OP_rplacd(LispPTR tosm1, LispPTR tos) {
  if (tosm1 == NIL_PTR) {
    if (tos == NIL_PTR) {
      return NIL_PTR;
    }
    ERROR_EXIT(tos);
  }
  if (!Listp(tosm1)) {
    ERROR_EXIT(tos);
  }
  rplacd(tosm1, tos);
  return tosm1;
} /* end of N_OP_rplacd */

/************************************************************************/
/*									*/
/*	        f i n d _ c l o s e _ p r i o r _ c e l l		*/
/*									*/
/*	Given the real address of a CONS page and an existing cell	*/
/*	on that page, return another cell that is close enough to	*/
/*	that the existing cell can be its CDR (i.e. up to 7 cells	*/
/*	earlier.  If no such cell exists, return 0.			*/
/*									*/
/*	If a cell is found, it is taken off the free chain before	*/
/*	being returned.							*/
/*									*/
/************************************************************************/

ConsCell *find_close_prior_cell(struct conspage *page, LispPTR oldcell) {
  unsigned oldoffset = oldcell & 0xFF;
  unsigned offset = page->next_cell;
  unsigned prior = 0;
  unsigned noffset;
  ConsCell *cell;

  while (offset) {
    if ((offset < oldoffset) && (offset >= (oldoffset - 14))) {
      noffset = FREECONS(page, offset)->next_free;
      while ((noffset > offset) && (noffset < oldoffset)) {
        prior = offset;
        offset = noffset;
        noffset = FREECONS(page, offset)->next_free;
      }
      cell = (ConsCell *)((DLword *)page + offset);
      if (prior) {
        FREECONS(page, prior)->next_free = FREECONS(page, offset)->next_free;
      } else {
        page->next_cell = FREECONS(page, offset)->next_free;
      }
      page->count -= 1;
      cell->cdr_code = CDR_ONPAGE | ((oldoffset - offset) >> 1);
      if (254 < (offset + ((cell->cdr_code & 7) << 1))) {
        error("in fcpc, page overflow.");
      }
      return (cell);
    }

    prior = offset;
    offset = ((freecons *)((DLword *)page + offset))->next_free;
  }
  return (ConsCell *)0; /* No cell close enough */
}

#ifdef NEWCDRCODING
/************************************************************************/
/*									*/
/*			f i n d _ c l o s e _ c e l l			*/
/*									*/
/*	Given the real address of a CONS page and an existing cell	*/
/*	on that page, return another cell that is close enough to	*/
/*	be used as the CDR of the existing cell (i.e., within 7		*/
/*	cells.  If no such cell exists, return 0.			*/
/*									*/
/*	If a cell is found, it is taken off the free chain before	*/
/*	being returned.							*/
/*									*/
/************************************************************************/

static ConsCell *find_close_cell(struct conspage *page, LispPTR oldcell) {
  unsigned oldoffset = oldcell & 0xFF;
  unsigned offset = page->next_cell;
  unsigned prior = 0;

  while (offset) {
    if ((offset > oldoffset) && (offset <= (oldoffset + 14))) {
      if (prior) {
        ((freecons *)((DLword *)page + prior))->next_free =
            ((freecons *)((DLword *)page + offset))->next_free;
      } else {
        page->next_cell = ((freecons *)((DLword *)page + offset))->next_free;
      }
      page->count -= 1;
      return (ConsCell *)((DLword *)page + offset);
    }
    prior = offset;
    offset = ((freecons *)((DLword *)page + offset))->next_free;
  }
  return (ConsCell *)0; /* No cell close enough */
}

/************************************************************************/
/*									*/
/*		f i n d _ c d r p a i r _ i n _ p a g e			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

static ConsCell *find_cdrpair_in_page(struct conspage *pg, LispPTR carpart, LispPTR cdrpart) {
  unsigned offset, prior, priorprior, nprior, poffset, noffset;

  prior = priorprior = nprior = 0;

  if (pg->count < 2) {
    return (ConsCell *)0;
  }

  offset = pg->next_cell;

  while (offset) {
    if (prior && (offset < prior) && (prior <= offset + 14)) {
      ConsCell *carcell, *cdrcell;

      poffset = offset;
      noffset = FREECONS(pg, offset)->next_free;
      while ((noffset > offset) && (noffset < prior)) {
        nprior = offset;
        poffset = prior;
        offset = noffset;
        noffset = FREECONS(pg, offset)->next_free;
      }

      carcell = (ConsCell *)(((DLword *)pg) + offset);
      cdrcell = (ConsCell *)(((DLword *)pg) + prior);
      if (priorprior) {
        FREECONS(pg, priorprior)->next_free = FREECONS(pg, poffset)->next_free;
      } else {
        pg->next_cell = FREECONS(pg, poffset)->next_free;
      }
      if (nprior) {
        FREECONS(pg, nprior)->next_free = FREECONS(pg, offset)->next_free;
      }
      pg->count -= 2;

      *(LispPTR *)carcell = carpart;
      *(LispPTR *)cdrcell = cdrpart;

      carcell->cdr_code = cdrcell - carcell;
      return carcell;
    }
    if (prior && (offset > prior) && (offset <= prior + 14)) {
      ConsCell *carcell, *cdrcell;

      carcell = (ConsCell *)(((DLword *)pg) + prior);
      cdrcell = (ConsCell *)(((DLword *)pg) + offset);
      if (priorprior) {
        FREECONS(pg, priorprior)->next_free = ((freecons *)cdrcell)->next_free;
      } else {
        pg->next_cell = ((freecons *)cdrcell)->next_free;
      }
      pg->count -= 2;

      *(LispPTR *)carcell = carpart;
      *(LispPTR *)cdrcell = cdrpart;

      carcell->cdr_code = (cdrcell - carcell);
      return carcell;
    }
    priorprior = prior;
    prior = offset;
    offset = FREECONS(pg, offset)->next_free;
  }

  return (ConsCell *)0; /* found no entries in this page, so return failure code */
}

/************************************************************************/
/*									*/
/*		f i n d _ c d r a b l e _ p a i r			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

static ConsCell *find_cdrable_pair(LispPTR carpart, LispPTR cdrpart) {
  unsigned pgno;
  struct conspage *pg;
  ConsCell *cell;

  for (pg = (struct conspage *)NativeAligned4FromLPage(pgno = ListpDTD->dtd_nextpage); pgno;
       pg = (struct conspage *)NativeAligned4FromLPage(pgno = pg->next_page)) {
    cell = find_cdrpair_in_page(pg, carpart, cdrpart);
    if (cell) {
      return (cell);
    }
  }
  return (find_cdrpair_in_page(next_conspage(), carpart, cdrpart));
}
#endif
