/* This is G-file @(#) array4.c Version 2.7 (10/12/88). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*				A R R A Y 4 . C				*/
/*									*/
/*	Contains:	N_OP_aset1					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "gcdata.h"
#include "my.h"

#include "array4defs.h"
#include "gchtfinddefs.h"

/***	N_OP_aset1   -- op 267   (new-value array index)   ***/

/************************************************************************/
/*									*/
/*			N _ O P _ a s e t 1				*/
/*									*/
/*	1-dimensional array setter.					*/
/*									*/
/************************************************************************/

LispPTR N_OP_aset1(register LispPTR data, LispPTR arrayarg, register int inx) {
  register OneDArray *arrayblk;
  register LispPTR base;
  register int new;
  register int index;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_ONED_ARRAY) ERROR_EXIT(inx);
  arrayblk = (OneDArray *)Addr68k_from_LADDR(arrayarg);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);
  if (index >= arrayblk->totalsize) ERROR_EXIT(inx);
  index += arrayblk->offset;

  /*  setup base  */
  base = arrayblk->base;

  /*  disp on type  */
  aset_switch(arrayblk->typenumber, inx);

doufn:
  ERROR_EXIT(inx);

} /*  end N_OP_aset1()  */
