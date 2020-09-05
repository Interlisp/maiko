/* This is G-file @(#) array6.c Version 2.10 (4/21/92). copyright Xerox & Fuji Xerox  */
static char *id = "@(#) array6.c	2.10 4/21/92";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "gcdata.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "my.h"

#include "array6defs.h"
#include "gchtfinddefs.h"

/***	N_OP_aset2   -- op 357   (new-value array index0 index1)   ***/
LispPTR N_OP_aset2(register LispPTR data, LispPTR arrayarg, LispPTR inx0, LispPTR inx1) {
  register int type;
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

  /*  setup typenumber  */
  type = 0xFF & arrayblk->typenumber;

  /*  disp on type  */
  aset_switch(type, inx1);

doufn:
  ERROR_EXIT(inx1);

} /*  end N_OP_aset2()  */
