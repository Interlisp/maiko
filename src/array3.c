/* This is G-file @(#) array3.c Version 2.9 (10/12/88). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*			     A R R A Y 3 . C				*/
/*									*/
/*	Contains:	N_OP_aref1					*/
/*									*/
/************************************************************************/

#include "version.h"
#include "adr68k.h"      // for Addr68k_from_LADDR
#include "array3defs.h"  // for N_OP_aref1
#include "emlglob.h"
#include "lispemul.h"    // for state, LispPTR, ERROR_EXIT
#include "lspglob.h"
#include "lsptypes.h"    // for OneDArray, GetTypeNumber, TYPE_ONED_ARRAY
#include "my.h"          // for aref_switch, N_GetPos

/***	N_OP_aref1   -- op 266   (array index)   ***/
LispPTR N_OP_aref1(register LispPTR arrayarg, register LispPTR inx) {
  register LispPTR baseL;
  register int index;
  register OneDArray *arrayblk;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_ONED_ARRAY) ERROR_EXIT(inx);
  arrayblk = (OneDArray *)Addr68k_from_LADDR(arrayarg);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);
  if (index >= arrayblk->totalsize) ERROR_EXIT(inx);
  index += arrayblk->offset;

  /*  setup base  */
  baseL = arrayblk->base;

  /*  disp on type  */
  return (aref_switch(arrayblk->typenumber, inx, baseL, index));
} /*  end N_OP_aref1()  */
