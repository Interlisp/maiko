/* $Id: gcrcell.c,v 1.3 1999/05/31 23:35:32 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*************************************************************************/
/*                                                                       */
/*                       File Name : gcrcell.c                    	 */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                      Creation Date : July-7-1987                      */
/*                      Written by Tomoru Teruuchi                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions :                                                 */
/*                       gcreccell(cell);				 */
/*                       freelistcell(cell);                             */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/*  The functions "gcreccell" and "freelistcell" are the translated 	 */
/*  functions from the Lisp functions "\GCRECLAIMCELL" that is the UFN   */
/*  function of the opcode "RECLAIMCELL", and "\FREELISTCELL".           */
/*  These functions may have the following characteristics :             */
/*                                                                       */
/*  	gcreccell(cell)	LispPTR cell					 */
/* 		This function may always return NIL(= 0), as the Lisp    */
/*           	macro .RECLAIMCELLLP. in more upper level may use this   */
/*              return value as the further Garbage's pointer.(The Opcode*/
/*              "RECLAIMCELL"'s function is specified as this, but its   */
/*              UFN function is not. The gcreccell function's		 */
/*              behavior is same as the UFN function for speed and 	 */
/*		simplicity,this is, this function is closed in this level*/
/*		)							 */
/*		This function may reclaim the data of all types that is  */
/*		Garbage.Especially, the data whose types are ARRAYBLOCK  */
/*		(= 0), STACKP(= 8),VMEMPAGEP(= 10) and CODEBLOCK(= 54,55,*/
/*		56,57,58,59,60,61,62,63) may be reclaimed by each special*/
/*		processes that are specified and invoked by this function*/
/*		.The data whose type is LISTP is the main data type	 */
/*		processed in this function actually and only then the 	 */
/*		function "freelistcell" may be called for making linkage */
/*		of free list.						 */
/*									 */
/*	freelistcell(cell)	LispPTR cell				 */
/*		This function may make the linkage of free list of the	 */
/*		cons cell.The header of this linkage is DTD->NEXTPAGE of */
/*		LISTP and each cons page has its internal linkage of free*/
/*		cells.This return value is not considered as not used.	 */
/*									 */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include <stdio.h>        // for printf
#include "address.h"      // for POINTER_PAGE
#include "adr68k.h"       // for NativeAligned4FromLAddr, NativeAligned4FromLPage
#include "car-cdrdefs.h"  // for car, cdr
#include "cell.h"         // for conspage, freecons, FREECONS, CDR_INDIRECT
#include "commondefs.h"   // for error
#include "gccodedefs.h"   // for reclaimcodeblock
#include "gcdata.h"       // for DELREF, REC_GCLOOKUPV, ADDREF, REC_GCLOOKUP
#include "gchtfinddefs.h" // for htfind, rec_htfind
#include "gcfinaldefs.h"  // for reclaimarrayblock, reclaimstackp, releasing...
#include "gcrcelldefs.h"  // for freelistcell, gcreccell
#include "lispemul.h"     // for LispPTR, ConsCell, NIL, POINTERMASK, DLword
#include "lspglob.h"      // for ListpDTD
#include "lsptypes.h"     // for dtd, GetDTD, GetTypeNumber, TYPE_ARRAYBLOCK
#ifdef DTDDEBUG
#include "testtooldefs.h"
#endif

#ifdef NEWCDRCODING
#undef CONSPAGE_LAST
#define CONSPAGE_LAST 0x0ffffffff
#else
#undef CONSPAGE_LAST
#define CONSPAGE_LAST 0x0ffff
#endif /* NEWCDRCODING */

#define TODO_LIMIT 1000
#define ADD_TO_DO(ptr, offset)                               \
  do {                                                       \
    if (do_count < TODO_LIMIT) {                               \
      if ((ptr) & 0xF0000000) error("illegal ptr in addtodo"); \
      to_do[do_count] = (ptr);                                 \
      to_do_offset[do_count] = offset;                         \
      todo_uses++;                                             \
      /*REC_GCLOOKUP((ptr), ADDREF);*/                         \
      do_count++;                                              \
    } else { /* error("GC missing some to-do's"); */           \
      todo_misses++;                                           \
    }                                                          \
  } while (0)

static unsigned todo_uses = 0;
static unsigned todo_misses = 0;
static unsigned todo_reads = 0;

/************************************************************************/
/*									*/
/*				g c r e c c e l l			*/
/*									*/
/*	Reclaim a cell, doing necessary finalization &c.		*/
/*									*/
/************************************************************************/

LispPTR gcreccell(LispPTR cell) {
  ConsCell *ptr;
  struct dtd *typdtd;
  DLword typ;
  LispPTR tmpptr, donext, tmpcell, val;
  LispPTR ptrfield, carfield;
  int index, code;
  LispPTR *field;

#ifdef NEWCDRCODING
  LispPTR to_do[TODO_LIMIT];      /* table of pointers to follow, since Cdr coding lost */
  short to_do_offset[TODO_LIMIT]; /* offset in datatype */
  unsigned do_count = 0;          /* counter of entries in to_do table */
#endif                            /* NEWCDRCODING */

  val = NIL;
  tmpptr = cell;
  index = -1;
  donext = NIL;
lp:
  ptr = (ConsCell *)NativeAligned4FromLAddr(tmpptr & -2);
/* # ifdef CHECK
  if (refcnt(tmpptr) != 1) error("reclaiming cell w/refcnt not 1");
 # endif
*/
#ifdef DEBUG
  if (tmpptr & 1) error("Reclaiming cell pointer with low bit 1.");
#else
  tmpptr &= -2; /* turn off low bit of pointer, so we never reclaim odd'ns */
#endif

  if ((tmpptr & 0x0FFF0000) == 0x60000) error("freeing an old atom??");

  typ = GetTypeNumber(tmpptr);
#ifdef DEBUG
  if (typ == 6) printf("Reclaiming array ptr 0x%x.\n", tmpptr);
#endif
  switch (typ) {
    case TYPE_LISTP: {
      if ((code = ptr->cdr_code) == CDR_INDIRECT) /* indirect */
      {
        tmpcell = ptr->car_field; /* Monitor */
        freelistcell(tmpptr);
        ptr = (ConsCell *)NativeAligned4FromLAddr(tmpcell);
        tmpptr = tmpcell;
        code = ptr->cdr_code;
      }
      if (index != -1) /* car part */
        index = -1;
      else {
        REC_GCLOOKUPV(car(tmpptr), DELREF, val);
        if (val != NIL) {
          ptr->car_field = donext;
          ptr->cdr_code = code;
          donext = tmpptr;
          goto doval;
        }
      }
      REC_GCLOOKUPV(cdr(tmpptr), DELREF, val);
      if (code <= CDR_MAXINDIRECT) {
#ifdef NEWCDRCODING
        tmpcell = tmpptr + ((code - CDR_INDIRECT) << 1);
#else
        tmpcell = POINTER_PAGEBASE(tmpptr) + ((code - CDR_INDIRECT) << 1);
#endif /* NEWCDRCODING */
        freelistcell(tmpcell);
      }
      freelistcell(tmpptr);
      goto doval;
    }
    case TYPE_ARRAYBLOCK:
      if ((index == -1) && reclaimarrayblock(tmpptr))
        goto trynext;
      else
        break;
    case TYPE_STACKP:
      if ((index == -1) && reclaimstackp(tmpptr)) goto trynext;
      break;
    case TYPE_VMEMPAGEP:
      if ((index == -1) && releasingvmempage(tmpptr)) {
        goto trynext;
      } else
        break;
    case TYPE_CODEHUNK1:
    case TYPE_CODEHUNK2:
    case TYPE_CODEHUNK3:
    case TYPE_CODEHUNK4:
    case TYPE_CODEHUNK5:
    case TYPE_CODEHUNK6:
    case TYPE_CODEHUNK7:
    case TYPE_CODEHUNK8:
    case TYPE_CODEHUNK9:
    case TYPE_CODEHUNK10:
      if ((index == -1) && reclaimcodeblock(tmpptr))
        goto trynext;
      else
        break;
    default:;
  }
normal:
  typdtd = (struct dtd *)GetDTD(typ);
  ptrfield = typdtd->dtd_ptrs;
  if (index != -1) {
    index = (index << 1);
    ptrfield = cdr(ptrfield);
    while ((car(ptrfield) & 0x0ffff) != index) ptrfield = cdr(ptrfield);
    index = -1;
  }
  while (ptrfield != NIL) {
    carfield = car(ptrfield);
    ptrfield = cdr(ptrfield);
    carfield &= 0x0ffff;
    REC_GCLOOKUPV((POINTERMASK & *(LispPTR *)NativeAligned4FromLAddr(tmpptr + carfield)), DELREF, val);
#ifndef NEWCDRCODING
    if (val != NIL) {
      if (ptrfield != NIL) {
        ptr = (ConsCell *)NativeAligned4FromLAddr(tmpptr);
        ptr->car_field = donext;
        ptr->cdr_code = ((car(ptrfield) & 0x0ffff) >> 1);
        donext = tmpptr;
        goto doval;
      } else
        goto addtofreelist;
    }
#else
    if (val != NIL) {
      if (ptrfield != NIL) {
        if ((carfield = car(ptrfield) & 0x0ffff) >> 1 < 15) {
          ptr = (ConsCell *)NativeAligned4FromLAddr(tmpptr);
          ptr->car_field = donext;
          ptr->cdr_code = ((car(ptrfield) & 0x0ffff) >> 1);
          donext = tmpptr;
          goto doval;
        } else {
          ADD_TO_DO(tmpptr, (car(ptrfield) & 0xffff) >> 1);
          goto doval;
        }
      } else
        goto addtofreelist;
    }
#endif /* NEWCDRCODING */
  }
addtofreelist:
  field = (LispPTR *)NativeAligned4FromLAddr(tmpptr);
  *field = typdtd->dtd_free;
  typdtd->dtd_free = tmpptr & POINTERMASK;
#ifdef DTDDEBUG
  check_dtd_chain(GetTypeNumber(tmpptr & POINTERMASK));
#endif

/******************************/
/*								*/
/*  Freeing one cell made another cell's refcnt = 0. */
/*  ADDREF the second cell (to remove it from the GC table) */
/*  and reclaim it.				*/
/************************************************************/
doval:
  if (val != NIL) {
    tmpptr = val;
    REC_GCLOOKUP(tmpptr, ADDREF);
    /*	GCLOOKUP(0x8000, ADDREF,tmpptr); */
    val = NIL;
    goto lp;
  }

/***************************************************************/
/*									*/
/*  Finished freeing the main cell, but we may have saved other */
/*  cells whose refcnt's went to 0 along the way.  This is      */
/*  where we work down the list of saved items to free.         */
/*									*/
/****************************************************************/
trynext:
  if (donext != NIL) {
    tmpptr = donext;
    ptr = (ConsCell *)NativeAligned4FromLAddr(tmpptr);
    donext = (LispPTR)ptr->car_field;
    index = ptr->cdr_code;
    goto lp;
  }
#ifdef NEWCDRCODING
  if (do_count) /* If there are other cells to collect */
  {
    do_count--;
    tmpptr = to_do[do_count];
    index = to_do_offset[do_count];
    todo_reads++;
    /*REC_GCLOOKUP(tmpptr, ADDREF); */
    goto lp;
  }
#endif /*NEWCDRCODING */
  return (NIL);
}

/************************************************************************/
/*									*/
/*			f r e e l i s t c e l l				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void freelistcell(LispPTR cell) {
  struct conspage *pbase;
  ConsCell *cell68k;
  unsigned int offset, prior, celloffset;

  cell68k = (ConsCell *)NativeAligned4FromLAddr(cell);
  pbase = (struct conspage *)NativeAligned4FromLPage(POINTER_PAGE(cell));
  celloffset = (LispPTR)cell & 0xFF;
#ifdef NEWCDRCODING
  if (celloffset < 8) error("freeing CONS cell that's really freelist ptr");
#endif /* NEWCDRCODING */

  if (pbase->count) /* There are free cells on the page already */
  {
    prior = 0;

    for (offset = pbase->next_cell; offset; offset = FREECONS(pbase, offset)->next_free) {
#ifdef NEWCDRCODING
      if ((6 ^ offset) < (6 ^ celloffset))
#else
      if (offset < celloffset)
#endif /* NEWCDRCODING */
      {
        break;
      }
      prior = offset;
    }

    if (prior)
      FREECONS(pbase, prior)->next_free = celloffset;
    else
      pbase->next_cell = celloffset;
    ((freecons *)cell68k)->next_free = offset;
  } else /* NO FREE CELLS.  Just replace next_free. */
  {
    pbase->next_cell = celloffset;
    FREECONS(pbase, celloffset)->next_free = 0; /* And this is end of the chain */
  }

  if ((++pbase->count > 32) && (pbase->next_page == CONSPAGE_LAST)) {
    pbase->next_page = ListpDTD->dtd_nextpage;
    ListpDTD->dtd_nextpage = POINTER_PAGE(cell);
  }
}
