/* $Id: lsthandl.c,v 1.4 1999/05/31 23:35:38 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*
                Including :	OP_fmemb
                                OP_listget

*/
/**********************************************************************/

#include "version.h"

#include "car-cdrdefs.h"   // for car, cdr
#include "cell.h"          // for cadr_cell
#include "emlglob.h"
#include "lispemul.h"      // for state, LispPTR, ERROR_EXIT, NIL_PTR, Scrat...
#include "lspglob.h"
#include "lsptypes.h"      // for Listp, GetTypeNumber, TYPE_LISTP
#include "lsthandldefs.h"  // for N_OP_fmemb, N_OP_listget, fmemb
#include "vars3defs.h"     // for cadr

/***********************************************************************/
/*	 N_OP_fmemb							*/
/**********************************************************************/

LispPTR N_OP_fmemb(LispPTR item, LispPTR tos) { /* OP 34Q */

  while (Listp(tos)) {
    if (item == car(tos)) return tos;
    tos = cdr(tos);
    /* if we get an interrupt, punt so we can handle it safely */
    if (!Irq_Stk_End) { TIMER_EXIT(tos); }
  }
  if (tos) ERROR_EXIT(tos);
  return tos;

} /* N_OP_fmemb end */

/***********************************************************************/
/*
        Func Name :	fmemb(item,list)
        >>For User programming<<
        NOTE: You should not handle long list, because it doesn't care
                about interrupt.

*/
/**********************************************************************/

LispPTR fmemb(LispPTR item, LispPTR list) {
  while (Listp(list)) {
    if (item == car(list)) return (list);
    list = cdr(list);
  }

  if (list) return (list);
  return (list);

} /* fmemb end */

/***********************************************************************/
/*
                Func Name :	N_OP_listget
                Opcode	:	47Q
 */
/**********************************************************************/

#define SAVE_ERROR_EXIT2(topcstk, tos) \
  do {                                  \
    Scratch_CSTK = topcstk;            \
    ERROR_EXIT(tos);                   \
  } while (0)

#define S_N_CHECKANDCADR2(sour, dest, tos, tcstk) \
  do {                                            \
    LispPTR parm = sour;                 \
    if (GetTypeNumber(parm) != TYPE_LISTP) {      \
      SAVE_ERROR_EXIT2(tcstk, tos);               \
    } else                                        \
      (dest) = cadr(parm);                        \
  } while (0)

LispPTR N_OP_listget(LispPTR plist, LispPTR tos) {
  struct cadr_cell cadrobj;

  while (plist != NIL_PTR) {
    S_N_CHECKANDCADR2(plist, cadrobj, tos, plist);

    if (cadrobj.car_cell == tos) {
      if (cadrobj.cdr_cell == NIL_PTR) return NIL_PTR;

      if (Listp(cadrobj.cdr_cell))
        return (car(cadrobj.cdr_cell));
      else /* must punt in case car/cdrerr */
        SAVE_ERROR_EXIT2(plist, tos);
    }

    if (!Listp(cadrobj.cdr_cell)) { /* this list ended before we found prop */
      return (NIL_PTR);
    }

    S_N_CHECKANDCADR2(cadrobj.cdr_cell, cadrobj, tos, plist);
    plist = cadrobj.cdr_cell;

    if (!Irq_Stk_End) {
      /* for continuation, it becomes plist on next time */
      Scratch_CSTK = plist;
      TIMER_EXIT(tos);
    }
  }

  return (NIL_PTR);

} /* N_OP_listget end */
