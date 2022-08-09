/* This is G-file @(#) array6.c Version 2.10 (4/21/92). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"
#include "adr68k.h"      // for Addr68k_from_LADDR
#include "array6defs.h"  // for N_OP_aset2
#include "emlglob.h"
#include "lispemul.h"    // for state, LispPTR, ERROR_EXIT
#include "lspglob.h"
#include "lsptypes.h"    // for LispArray, GetTypeNumber, TYPE_TWOD_ARRAY
#include "my.h"          // for N_GetPos, aset_switch

/***	N_OP_aset2   -- op 357   (new-value array index0 index1)   ***/
LispPTR N_OP_aset2(register LispPTR data, LispPTR arrayarg, LispPTR inx0, LispPTR inx1) {
  register LispArray *arrayblk;
  register LispPTR base;
  register int new;
  register int index, temp;
  int j;

  /*  verify array  */
  if (GetTypeNumber(arrayarg) != TYPE_TWOD_ARRAY) ERROR_EXIT(inx1);
  arrayblk = (LispArray *)Addr68k_from_LADDR(arrayarg);
  base = arrayblk->base;

  /*  test and setup index  */
  N_GetPos(inx1, temp, inx1);
  if (temp >= (j = arrayblk->Dim1)) ERROR_EXIT(inx1);
  N_GetPos(inx0, index, inx1);
  if (index >= arrayblk->Dim0) ERROR_EXIT(inx1);
  index *= j;
  index += temp;

  /*  disp on type  */
  aset_switch(arrayblk->typenumber, inx1);

doufn:
  ERROR_EXIT(inx1);

} /*  end N_OP_aset2()  */
