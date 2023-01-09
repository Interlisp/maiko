/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"
#include "adr68k.h"       // for NativeAligned4FromLAddr
#include "arrayopsdefs.h" // for N_OP_misc3, N_OP_misc4, N_OP_aref1, N_OP_aset1, N_OP_aref2, N_OP_aset2
#include "emlglob.h"
#include "lispemul.h"     // for state, LispPTR, ERROR_EXIT
#include "lspglob.h"
#include "lsptypes.h"
#include "my.h"           // for aref_switch, N_GetPos


/************************************************************
 OP_claref  -- op 372/9 (base typenumber index)

type    size    typenumber
  0       0             0      unsigned  : 1 bit
  0       3             3      unsigned  : 8 bits
  0       4             4      unsigned  : 16 bits
  1       4            20      signed    : 16 bits
  1       6            22      signed    : 32 bits
  2       6            38      pointer   : 32 bits
  3       6            54      float     : 32 bits
  4       3            67      character : 8 bits
  4       4            68      character : 16 bits
  5       6            86      Xpointer  : 32 bits


***********************************************************/

/***	N_OP_misc3  -- op 372/9 (base typenumber index)   ***/
LispPTR N_OP_misc3(LispPTR baseL, LispPTR typenumber, LispPTR inx, int alpha) {
  int index, type;

  if (alpha != 9) ERROR_EXIT(inx);
  /*  test and setup index  */
  N_GetPos(inx, index, inx);

  /*  test and setup typenumber  */
  N_GetPos(typenumber, type, inx);

  /*  dispatch on type  */
  return (aref_switch((unsigned)type, inx, baseL, index));
} /*  end N_OP_misc3()  */

/************************************************************************/
/*									*/
/*			N _ O P _ m i s c 4				*/
/*									*/
/*	CL:ASET opcode	op 373/7 (data, base typenumber, index)		*/
/*									*/
/************************************************************************/

LispPTR N_OP_misc4(LispPTR data, LispPTR base, LispPTR typenumber,
                   LispPTR inx, int alpha) {
  int index, type;

  if (alpha != 7) ERROR_EXIT(inx);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);

  /*  test and setup typenumber  */
  N_GetPos(typenumber, type, inx);

  aset_switch(type, inx);

doufn:
  ERROR_EXIT(inx);

} /*  end N_OP_misc4()  */

/***	N_OP_aref1   -- op 266   (array index)   ***/

LispPTR N_OP_aref1(LispPTR arrayarg, LispPTR inx) {
  LispPTR baseL;
  int index;
  OneDArray *arrayblk;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_ONED_ARRAY) ERROR_EXIT(inx);
  arrayblk = (OneDArray *)NativeAligned4FromLAddr(arrayarg);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);
  if (index >= arrayblk->totalsize) ERROR_EXIT(inx);
  index += arrayblk->offset;

  /*  setup base  */
  baseL = arrayblk->base;

  /*  dispatch on type  */
  return (aref_switch(arrayblk->typenumber, inx, baseL, index));
} /*  end N_OP_aref1()  */

/***	N_OP_aset1   -- op 267   (new-value array index)   ***/

/************************************************************************/
/*									*/
/*			N _ O P _ a s e t 1				*/
/*									*/
/*	1-dimensional array setter.					*/
/*									*/
/************************************************************************/

LispPTR N_OP_aset1(LispPTR data, LispPTR arrayarg, LispPTR inx) {
  OneDArray *arrayblk;
  LispPTR base;
  int index;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_ONED_ARRAY) ERROR_EXIT(inx);
  arrayblk = (OneDArray *)NativeAligned4FromLAddr(arrayarg);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);
  if (index >= arrayblk->totalsize) ERROR_EXIT(inx);
  index += arrayblk->offset;

  /*  setup base  */
  base = arrayblk->base;

  /*  dispatch on type  */
  aset_switch(arrayblk->typenumber, inx);

doufn:
  ERROR_EXIT(inx);

} /*  end N_OP_aset1()  */

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
  arrayblk = (LispArray *)NativeAligned4FromLAddr(arrayarg);
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

/***	N_OP_aset2   -- op 357   (new-value array index0 index1)   ***/

LispPTR N_OP_aset2(LispPTR data, LispPTR arrayarg, LispPTR inx0, LispPTR inx1) {
  LispArray *arrayblk;
  LispPTR base;
  int index, temp;
  int j;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_TWOD_ARRAY) ERROR_EXIT(inx1);
  arrayblk = (LispArray *)NativeAligned4FromLAddr(arrayarg);
  base = arrayblk->base;

  /*  test and setup index  */
  N_GetPos(inx1, temp, inx1);
  if (temp >= (j = arrayblk->Dim1)) ERROR_EXIT(inx1);
  N_GetPos(inx0, index, inx1);
  if (index >= arrayblk->Dim0) ERROR_EXIT(inx1);
  index *= j;
  index += temp;

  /*  dispatch on type  */
  aset_switch(arrayblk->typenumber, inx1);

doufn:
  ERROR_EXIT(inx1);

} /*  end N_OP_aset2()  */
