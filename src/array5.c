/* This is G-file @(#) array5.c Version 2.7 (10/12/88). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*			    A R R A Y 5 . C				*/
/*									*/
/*	Contains:	N_OP_aref2  2-d AREF opcode			*/
/*									*/
/************************************************************************/

#include "version.h"
#include "adr68k.h"      // for Addr68k_from_LADDR
#include "array5defs.h"  // for N_OP_aref2
#include "emlglob.h"
#include "lispemul.h"    // for state, LispPTR, ERROR_EXIT
#include "lspglob.h"
#include "lsptypes.h"    // for LispArray, GetTypeNumber, TYPE_TWOD_ARRAY
#include "my.h"          // for aref_switch, N_GetPos

/************************************************************************/
/*									*/
/*			N _ O P _ a r e f 2				*/
/*									*/
/*	2-d AREF op 356   (array index0 index1)				*/
/*									*/
/************************************************************************/

LispPTR N_OP_aref2(LispPTR arrayarg, LispPTR inx0, LispPTR inx1) {
  LispPTR baseL;
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

  /*  disp on type  */
  return (aref_switch(arrayblk->typenumber, inx1, baseL, arindex));
} /*  end N_OP_aref2()  */
