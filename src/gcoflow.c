/* $Id: gcoflow.c,v 1.3 1999/05/31 23:35:32 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*************************************************************************/
/*                                                                       */
/*                       File Name : gcpunt.c                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                      Creation Date : July-8-1987                      */
/*                      Written by Tomoru Teruuchi                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions : gc_handleoverflow(arg);                         */
/*                       gcmaptable(arg);                                */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include "version.h"

#include "arith.h"        // for GetSmalldata
#include "gcdata.h"       // for htoverflow, REC_GCLOOKUP
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "gcoflowdefs.h"  // for gc_handleoverflow, gcmaptable
#include "gcrdefs.h"      // for doreclaim
#include "lispemul.h"     // for NIL, DLword, LispPTR
#include "lspglob.h"      // for Reclaim_cnt_word, HToverflow, MaxTypeNumber_word
#include "lsptypes.h"     // for dtd, GetDTD, TYPE_LISTP

#define Increment_Allocation_Count(n) \
  do {                                \
    if (*Reclaim_cnt_word != NIL) {     \
      if (*Reclaim_cnt_word > (n))      \
        (*Reclaim_cnt_word) -= (n);     \
      else {                            \
        *Reclaim_cnt_word = NIL;        \
        doreclaim();                    \
      }                                 \
    }                                   \
  } while (0)

DLword gc_handleoverflow(DLword arg) {
  struct htoverflow *cell;
  struct dtd *ptr;
  LispPTR cellcnt;
  LispPTR addr;
  cell = (struct htoverflow *)HToverflow;
  /* This proc. protected from interrupt */
  while ((addr = cell->ptr) != NIL) {
    REC_GCLOOKUP(addr, cell->pcase);
    cell->ptr = 0;
    cell->pcase = 0;
    ++cell; /* (\ADDBASE CELL WORDSPERCELL) */
  }
  ptr = (struct dtd *)GetDTD(TYPE_LISTP);
  /* same as "extern struct dtd *ListpDTD" */
  if ((cellcnt = ptr->dtd_cnt0) > 1024) {
    Increment_Allocation_Count(cellcnt);
    ptr->dtd_oldcnt += cellcnt;
    ptr->dtd_cnt0 = 0;
  }
  return (arg);
}

DLword gcmaptable(DLword arg) {
  struct htoverflow *cell;
  struct dtd *ptr;
  LispPTR cellcnt;
  int typnum;
  LispPTR addr;
  int maxtypenumber = GetSmalldata(*MaxTypeNumber_word);

  cell = (struct htoverflow *)HToverflow;
  /* This proc. protected from interrupt */
  while ((addr = cell->ptr) != NIL) {
    REC_GCLOOKUP(addr, cell->pcase);
    cell->ptr = 0;
    cell->pcase = 0;
    ++cell; /* (\ADDBASE CELL WORDSPERCELL) */
  }
  for (typnum = 1; typnum <= maxtypenumber; ++typnum)
  /* applied alltype */
  {
    ptr = (struct dtd *)GetDTD(typnum);
    if ((cellcnt = ptr->dtd_cnt0) != 0) {
      ptr->dtd_oldcnt += cellcnt;
      ptr->dtd_cnt0 = 0;
      Increment_Allocation_Count(cellcnt);
    }
  }
  return (arg);
}
