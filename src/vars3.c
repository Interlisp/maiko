/* $Id: vars3.c,v 1.4 2001/12/24 01:09:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"       // for NativeAligned4FromLAddr
#include "car-cdrdefs.h"  // for car, cdr
#include "cell.h"         // for cadr_cell, CDR_NIL, CDR_INDIRECT, S_N_CHECK...
#include "emlglob.h"
#include "lispemul.h"     // for state, ConsCell, LispPTR, NIL_PTR, DLword
#include "lispmap.h"      // for S_POSITIVE
#include "lspglob.h"      // for Stackspace
#include "lsptypes.h"     // for Listp
#include "stack.h"        // for frameex1
#include "vars3defs.h"    // for N_OP_arg0, N_OP_assoc, cadr

/*******************************************
cadr

        common routine.
        used by OP_assoc, OP_fmemb


*******************************************/
struct cadr_cell cadr(LispPTR cell_adr)
/* address of cell */
/* Lisp address (word addressing) */
{
  ConsCell *pcons;
  struct cadr_cell cadr1; /* return value */
  short offset;

  if (Listp(cell_adr) == NIL) {
    if (cell_adr == NIL) {
      cadr1.car_cell = 0;
      cadr1.cdr_cell = 0;
      return (cadr1);
    } else {
      cadr1.car_cell = car(cell_adr);
      cadr1.cdr_cell = cdr(cell_adr);
      return (cadr1);
    }
  }
  pcons = (ConsCell *)NativeAligned4FromLAddr(cell_adr);
  while (pcons->cdr_code == CDR_INDIRECT) {
    /* CDR indirect */
    cell_adr = pcons->car_field;
    pcons = (ConsCell *)NativeAligned4FromLAddr(pcons->car_field);
  } /* skip CDR_INDIRECT cell */

  cadr1.car_cell = pcons->car_field;

  if (pcons->cdr_code == CDR_NIL) {
    /* CDR nil */
    cadr1.cdr_cell = NIL_PTR;
    return (cadr1);
  }
#ifdef NEWCDRCODING
  offset = (0x7 & pcons->cdr_code) << 1;
  if (pcons->cdr_code > CDR_NIL) {
    /* CDR on page */
    cadr1.cdr_cell = cell_adr + offset;
  } else {
    /* CDR different page */
    pcons = (ConsCell *)NativeAligned4FromLAddr((cell_adr) + offset);
    cadr1.cdr_cell = pcons->car_field;
  }
#else
  offset = (0x7F & pcons->cdr_code) << 1;
  if (pcons->cdr_code > CDR_NIL) {
    /* CDR on page */
    cadr1.cdr_cell = (mPAGEMASK & cell_adr) | offset;
  } else {
    /* CDR different page */
    pcons = (ConsCell *)NativeAligned4FromLAddr(((mPAGEMASK & cell_adr) | offset));
    cadr1.cdr_cell = pcons->car_field;
  }
#endif /* NEWCDRCODING */

  return (cadr1);
}

/***********************************************************
N_OP_arg0

        Entry:	ARG0	opcode[0141]

        <Entry>
        TopOfStack : slot number of IVAR area
        <Exit>
        return : the contents of the slot.

        No effect to CurrentStack.


************************************************************/

LispPTR N_OP_arg0(LispPTR tos) {
  int num;
  DLword *bf; /* index of Basic frame */
  int nargs;

  if ((SEGMASK & tos) != S_POSITIVE) {
    /* error("OP_arg0: Bad TopOfStack\n"); */
    ERROR_EXIT(tos);
  } else
    num = 0xFFFF & tos;
  if (CURRENTFX->alink & 0x1) {
    /* slow */
    bf = Stackspace + CURRENTFX->blink;
  } else {
    /* fast */
    bf = ((DLword *)CURRENTFX) - BFSIZE;
    /* bf : pointer to 1st word of BasicFramePointer */
  }
  nargs = ((UNSIGNED)bf - (UNSIGNED)IVar) >> 2;
  /* nargs : number of IVAR slots */
  if ((num == 0) || (num > nargs)) {
    /* error("OP_arg0: Bad argument number\n"); */
    ERROR_EXIT(tos);
  }
  return (*((int *)IVar + num - 1));
}

/*******************************************
N_OP_assoc

        Entry:	ASSOC	opcode[026]

        TopOfStack -- A-list (cons cell of Lisp address)
        *(int *)(CurrentStackPTR) -- Key (cons cell of Lisp address)



*******************************************/

LispPTR N_OP_assoc(LispPTR key, LispPTR list) {
  struct cadr_cell cadr1;
  LispPTR cdr; /* address of (cdr A-list); Lisp address */

  if (list == NIL_PTR) { return (NIL_PTR); }

  if (!Listp(list)) { return (NIL_PTR); }

  S_N_CHECKANDCADR(list, cadr1, list);

  do {
    cdr = cadr1.cdr_cell; /* the rest of A-list */
    if (Listp(cadr1.car_cell) && key == car(cadr1.car_cell)) {
      /* cons data found */
      return (cadr1.car_cell);
    }
    /* search the rest of A-list */
    if (Listp(cdr))
      cadr1 = cadr(cdr);
    else
      cdr = NIL;
    /* check for interrupts and punt to handle one safely */
    if (!Irq_Stk_End) {
      TopOfStack = cdr; /* for next execution */
      TIMER_EXIT(cdr);
    }
  } while (cdr != NIL_PTR);

  return (NIL_PTR);
}
