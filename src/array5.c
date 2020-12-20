/* This is G-file @(#) array5.c Version 2.7 (10/12/88). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*			    A R R A Y 5 . C				*/
/*									*/
/*	Contains:	N_OP_aref2  2-d AREF opcode			*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"

#include "array5defs.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "my.h"

/************************************************************************/
/*									*/
/*			N _ O P _ a r e f 2				*/
/*									*/
/*	2-d AREF op 356   (array index0 index1)				*/
/*									*/
/************************************************************************/

LispPTR N_OP_aref2(LispPTR arrayarg, LispPTR inx0, LispPTR inx1) {
#define REG
  LispPTR baseL;
  int type;
  int arindex, temp;
  LispArray *arrayblk;
  int j;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_TWOD_ARRAY) ERROR_EXIT(inx1);
  arrayblk = (LispArray *)Addr68k_from_LADDR(arrayarg);
  baseL = arrayblk->base;

  /*  test and setup index  */
  N_GetPos(inx1, temp, inx1);
  if (temp >= (j = arrayblk->Dim1)) ERROR_EXIT(inx1);
  N_GetPos(inx0, arindex, inx1);
  if (arindex >= arrayblk->Dim0) ERROR_EXIT(inx1);
  arindex *= j;
  arindex += temp;

  /*  setup typenumber  */
  type = 0xFF & arrayblk->typenumber;

/*  disp on type  */
#ifdef OS4
  aref_switch(type, inx1, baseL, arindex);
#else
  return (aref_switch(type, inx1, baseL, arindex));
#endif

} /*  end N_OP_aref2()  */
