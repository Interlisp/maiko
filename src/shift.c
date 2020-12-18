/* $Id: shift.c,v 1.3 1999/05/31 23:35:42 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

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
#include "emlglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "arith.h"

#include "shiftdefs.h"
#include "mkcelldefs.h"

/*
 * XXX: it feels as though something is not clean here, looks like the
 * "int a" arguments are really LispPTR types, though perhaps it doesn't
 * matter.  NBriggs, May 2017
 */

/************************************************************
N_OP_llsh1
        entry		LLSH1		OPCODE[0340]
        return(a << 1)
************************************************************/
LispPTR N_OP_llsh1(int a) { N_ARITH_BODY_1_UNSIGNED(a, 1, <<); }

/************************************************************
N_OP_llsh8
        entry		LLSH8		OPCODE[0341]
        return(a << 8)
************************************************************/
LispPTR N_OP_llsh8(int a) { N_ARITH_BODY_1_UNSIGNED(a, 8, <<); }

/************************************************************
N_OP_lrsh1
        entry		LRSH1		OPCODE[0342]
        return(a >> 1)
************************************************************/
LispPTR N_OP_lrsh1(int a) { N_ARITH_BODY_1_UNSIGNED(a, 1, >>); }

/************************************************************
N_OP_lrsh8
        entry		LRSH8		OPCODE[0343]
        return(a >> 8)
************************************************************/
LispPTR N_OP_lrsh8(int a) { N_ARITH_BODY_1_UNSIGNED(a, 8, >>); }

/************************************************************
N_OP_lsh
        entry		LSH		OPCODE[0347]
        return(a <?> b)
************************************************************/
LispPTR N_OP_lsh(int a, int b) {
  register int arg, arg2;
  register int size;
  /*DLword	*wordp;*/

  N_GETNUMBER(b, size, do_ufn);
  N_GETNUMBER(a, arg2, do_ufn);

  if (size > 0) {
    if (size > 31) goto do_ufn;
    arg = arg2 << size;
    if ((arg >> size) != arg2) goto do_ufn;
  } else if (size < 0) {
    if (size < -31) goto do_ufn;
    arg = arg2 >> -size;
    /*** Commented out JDS 1/27/89:  This punts if you shifted ***/
    /*** ANY 1 bits off the right edge.  You CAN'T overflow    ***/
    /*** in this direction!!                                   ***/
    /*		if ((arg << -size) != arg2) goto do_ufn; */
  } else
    return (a);

  N_ARITH_SWITCH(arg);

do_ufn:
  ERROR_EXIT(b);
}
