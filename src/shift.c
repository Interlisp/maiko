/* $Id: shift.c,v 1.3 1999/05/31 23:35:42 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "arith.h"      // for N_GETNUMBER, N_ARITH_SWITCH
#include "emlglob.h"
#include "lispemul.h"   // for state, ERROR_EXIT, LispPTR
#include "lspglob.h"
#include "lsptypes.h"
#include "shiftdefs.h"  // for N_OP_llsh1, N_OP_llsh8, N_OP_lrsh1, N_OP_lrsh8

/*
 * XXX: it feels as though something is not clean here, looks like the
 * "int a" arguments are really LispPTR types, though perhaps it doesn't
 * matter.  NBriggs, May 2017 -- Yes. Replaced. NBriggs, Aug 2022
 */

/************************************************************
N_OP_llsh1
        entry		LLSH1		OPCODE[0340]
        return(a << 1)
************************************************************/
LispPTR N_OP_llsh1(LispPTR a) {
  int arg1;

  N_GETNUMBER(a, arg1, du_ufn);
  arg1 <<= 1;
  N_ARITH_SWITCH(arg1);

 du_ufn:
  ERROR_EXIT(a);
}

/************************************************************
N_OP_llsh8
        entry		LLSH8		OPCODE[0341]
        return(a << 8)
************************************************************/
LispPTR N_OP_llsh8(LispPTR a) {
  int arg1;

  N_GETNUMBER(a, arg1, du_ufn);
  arg1 <<= 8;
  N_ARITH_SWITCH(arg1);

 du_ufn:
  ERROR_EXIT(a);
}

/************************************************************
N_OP_lrsh1
        entry		LRSH1		OPCODE[0342]
        return(a >> 1)
************************************************************/
LispPTR N_OP_lrsh1(LispPTR a) {
  int arg1;

  N_GETNUMBER(a, arg1, du_ufn);
  arg1 = (unsigned)arg1 >> 1;
  N_ARITH_SWITCH(arg1);

 du_ufn:
  ERROR_EXIT(a);

}

/************************************************************
N_OP_lrsh8
        entry		LRSH8		OPCODE[0343]
        return(a >> 8)
************************************************************/
LispPTR N_OP_lrsh8(LispPTR a) {
  int arg1;

  N_GETNUMBER(a, arg1, du_ufn);
  arg1 = (unsigned)arg1 >> 8;
  N_ARITH_SWITCH(arg1);

 du_ufn:
  ERROR_EXIT(a);
}

/************************************************************
N_OP_lsh
        entry		LSH		OPCODE[0347]
        return(a <?> b)
************************************************************/
LispPTR N_OP_lsh(LispPTR a, LispPTR b) {
  int arg, arg2;
  int size;
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
