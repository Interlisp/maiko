/* This is G-file @(#) array4.c Version 2.7 (10/12/88). copyright Xerox & Fuji Xerox  */
static char *id = "@(#) array4.c	2.7 10/12/88";





/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
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
#include "arith.h"
#include "gc.h"
#include "my.h"

/***	N_OP_aset1   -- op 267   (new-value array index)   ***/


/************************************************************************/
/*									*/
/*			N _ O P _ a s e t 1				*/
/*									*/
/*	1-dimensional array setter.					*/
/*									*/
/************************************************************************/

N_OP_aset1(register LispPTR data, LispPTR arrayarg, register int inx)
{
    register int type;
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

      /*  setup typenumber  */
      type = 0xFF & arrayblk->typenumber;

      /*  setup base  */
      base = arrayblk->base;


      /*  disp on type  */
      aset_switch(type, inx);

doufn:		ERROR_EXIT(inx);

  } /*  end N_OP_aset1()  */

