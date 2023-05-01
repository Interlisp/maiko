/* $Id: z2.c,v 1.3 1999/05/31 23:35:47 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
 *	Author :  don charnley
 */
/***********************************************************************/
/*
                File Name :	z2.c

                Including :	N_OP_clfmemb   -- op 035
                                N_OP_classoc   -- op 033
                                N_OP_restlist  -- op 043

*/
/**********************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "car-cdrdefs.h"   // for car, cdr
#include "cell.h"          // for cadr_cell, S_N_CHECKANDCADR
#include "conspagedefs.h"  // for cons
#include "emlglob.h"
#include "lispemul.h"      // for state, LispPTR, NIL_PTR, ERROR_EXIT, Irq_S...
#include "lispmap.h"       // for ATOM_OFFSET, S_CHARACTER, S_NEGATIVE, S_PO...
#include "lspglob.h"
#include "lsptypes.h"      // for Listp, GetTypeNumber, TYPE_LISTP
#include "vars3defs.h"     // for cadr
#include "z2defs.h"        // for N_OP_classoc, N_OP_clfmemb, N_OP_restlist

/*   N_OP_classoc()  OP 33Q  */
LispPTR N_OP_classoc(LispPTR key, LispPTR list) {
  struct cadr_cell cadr1;
  LispPTR cdrcell; /* address of (cdr A-list); Lisp address */

  switch (key & SEGMASK) {
    case S_POSITIVE: break;
    case S_NEGATIVE: break;
    case S_CHARACTER: break;
    case ATOM_OFFSET: break;
    default: ERROR_EXIT(list);
  }
  /* JRB - Don Charnley claims the code below should be identical to the
        code in IL:ASSOC, so I copied my new and exciting version over */

  if (list == NIL_PTR) { return (NIL_PTR); }

  if (GetTypeNumber(list) != TYPE_LISTP) { return (NIL_PTR); }

  S_N_CHECKANDCADR(list, cadr1, list);

  do {
    cdrcell = cadr1.cdr_cell; /* the rest of A-list */
    if (Listp(cadr1.car_cell) && key == car(cadr1.car_cell)) {
      /* cons data found */
      return (cadr1.car_cell);
    }
    /* search the rest of A-list */
    if (Listp(cdrcell))
      cadr1 = cadr(cdrcell);
    else
      cdrcell = NIL;
    /* check for interrupts and punt to handle one safely */
    if (!Irq_Stk_End) {
      TopOfStack = cdrcell; /* for continuation */
      TIMER_EXIT(cdrcell);
    }
  } while (cdrcell != NIL_PTR);

  return (NIL_PTR);
} /* end N_OP_classoc() */

/*   (CL:FMEMB item list)  OP 35Q  */
LispPTR N_OP_clfmemb(LispPTR item, LispPTR list) { /* OP 35Q */

  switch (item & SEGMASK) {
    case S_POSITIVE: break;
    case S_NEGATIVE: break;
    case S_CHARACTER: break;
    case ATOM_OFFSET: break;
    default: ERROR_EXIT(list);
  }
  /* JRB - Don Charnley claims the code below should be identical to IL:FMEMB,
        so I copied it */

  while (Listp(list)) {
    if (item == car(list)) return list;
    list = cdr(list);
    /* if we get an interrupt, punt so we can handle it safely */
    if (!Irq_Stk_End) {
      TopOfStack = list; /* for continuation */
      TIMER_EXIT(list);
    }
  }
  if (list) ERROR_EXIT(list);
  return list;

} /* end N_OP_clfmemb() */

/************************************************************
         43      RESTLIST

        alpha = skip  --  number of args to skip
        tos = last  --  last arg#
        tos-1 = tail

        IF tail = NIL THEN
                page _ NEXTCONSPAGE
                GOTO make
        ELSE
                AddRef tail
                page _ CONSPAGE[tail]
                GOTO make
        make:
                get [cnt,,next] from page
        make1:
                tail _ CONSCELL (CAR = IVar(last), CDR = tail)
                AddRef IVar(last)
                IF skip = last THEN GOTO fin
                last _ last - 1
                GOTO make1
        noroomonconspage:
        fin:
                store updated [cnt,,next]
                update ListpDTD:COUNTER
                DelRef tail
                IF noroomonconspage THEN UFN
                ELSEIF ListpDTD:COUNTER overflow then GCPUNT
                ELSEIF overflow entries then GCHANDLEOVERFLOW
                ELSE NEXTOPCODE

        alpha = skip  --  number of args to skip
        tos = last  --  last arg#
        tos-1 = tail

        AddRef tail
make1:	tail , cons(IVar(last), tail)
        AddRef IVar(last)
        IF skip = last THEN GOTO fin
        last _ last - 1
        GOTO make1

fin:	DelRef tail

***********************************************************/

LispPTR N_OP_restlist(LispPTR tail, int last, int skip) {
  last &= 0xFFFF;
  while (skip <= last) { tail = cons(GetLongWord(IVar + (--last << 1)), tail); }
  return (tail);
} /* end N_OP_restlist() */

/* end module */
