/* This is G-file @(#) array2.c Version 2.9 (10/12/88). copyright Xerox & Fuji Xerox  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*			    A R R A Y 2 . C				*/
/*									*/
/*	Contains N_OP_misc4, the ASET opcode.				*/
/*									*/
/************************************************************************/
#include "version.h"
#include "array2defs.h"  // for N_OP_misc4
#include "emlglob.h"
#include "lispemul.h"    // for state, LispPTR, ERROR_EXIT
#include "lspglob.h"
#include "lsptypes.h"
#include "my.h"          // for N_GetPos, aset_switch

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
