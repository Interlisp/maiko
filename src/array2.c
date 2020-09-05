/* This is G-file @(#) array2.c Version 2.9 (10/12/88). copyright Xerox & Fuji Xerox  */
static char *id = "@(#) array2.c	2.9 10/12/88";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*			    A R R A Y 2 . C				*/
/*									*/
/*	Contains N_OP_misc4, the ASET opcode.				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "gc.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "my.h"

#include "array2defs.h"
#include "gchtfinddefs.h"

/************************************************************************/
/*									*/
/*			N _ O P _ m i s c 4				*/
/*									*/
/*	CL:ASET opcode	op 373/7 (data, base typenumber, index)		*/
/*									*/
/************************************************************************/

LispPTR N_OP_misc4(register LispPTR data, register LispPTR base, register LispPTR typenumber,
                   register LispPTR inx, int alpha) {
  register int new;
  register int index;
  int type;

  if (alpha != 7) ERROR_EXIT(inx);

  /*  test and setup index  */
  N_GetPos(inx, index, inx);

  /*  test and setup typenumber  */
  N_GetPos(typenumber, type, inx);

  aset_switch(type, inx);

doufn:
  ERROR_EXIT(inx);

} /*  end N_OP_misc4()  */
