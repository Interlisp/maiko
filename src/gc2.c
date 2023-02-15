/* $Id: gc2.c,v 1.3 1999/05/31 23:35:30 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/**********************************************************************/
/*
                File Name:	gc2.c
                Desc: implement opcode SCAN1,SCAN2,GCRECLAIMCELL


                Including :	OP_scan1
                                OP_scan2
                                OP_gcreccell

*/
/**********************************************************************/

#include <stdio.h>         // for printf
#include "address.h"       // for LOLOC
#include "emlglob.h"
#include "gc2defs.h"       // for OP_gcscan1, OP_gcscan2
#include "gcscandefs.h"    // for gcscan1, gcscan2
#include "lispemul.h"      // for state, TopOfStack, NIL, PC, SEGMASK
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"
#include "testtooldefs.h"  // for printPC

/**********************************************************************/
/*
                Func Name : OP_gcscan1
*/
/**********************************************************************/

void OP_gcscan1(void) {
  int scan;
#ifdef TRACE
  printPC();
  printf("TRACE: OP_gcscan1()\n");
#endif
  if ((TopOfStack & SEGMASK) == S_POSITIVE) {
    scan = gcscan1(LOLOC(TopOfStack));
    TopOfStack = (scan == -1) ? NIL : scan | S_POSITIVE;
  } else {
    printf("OP_gcscan1: not a number\n");
  }
  PC++;
} /* OP_gcscan1 end */

/**********************************************************************/
/*
                Func Name : OP_gcscan2
*/
/**********************************************************************/

void OP_gcscan2(void) {
  int scan;
#ifdef TRACE
  printPC();
  printf("TRACE: OP_gcscan2()\n");
#endif
  if ((TopOfStack & SEGMASK) == S_POSITIVE) {
    scan = gcscan2(LOLOC(TopOfStack));
    TopOfStack = (scan == -1) ? NIL : scan | S_POSITIVE;
  }
  PC++;
} /* OP_gcscan2 end */
