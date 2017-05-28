/* This is G-file @(#) array3.c Version 2.9 (10/12/88). copyright Xerox & Fuji Xerox  */
static char *id = "@(#) array3.c	2.9 10/12/88";




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
/*			     A R R A Y 3 . C				*/
/*									*/
/*	Contains:	N_OP_aref1					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "arith.h"
#include "my.h"

/***	N_OP_aref1   -- op 266   (array index)   ***/
LispPTR N_OP_aref1(register LispPTR arrayarg, register LispPTR inx)
{

    register LispPTR baseL;
    register int type, index;
    register OneDArray *arrayblk;
    LispPTR temp;
    register int result;

	/*	for CREATECELL  */
    register DLword	*wordp;
    DLword	*createcell68k(unsigned int type);

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
    baseL = arrayblk->base;

  /*  disp on type  */
#ifdef OS4
     aref_switch(type, inx, baseL, index);
#else
    return ( aref_switch(type, inx, baseL, index) );
#endif

} /*  end N_OP_aref1()  */


