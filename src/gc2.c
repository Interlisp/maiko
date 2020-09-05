/* $Id: gc2.c,v 1.3 1999/05/31 23:35:30 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: gc2.c,v 1.3 1999/05/31 23:35:30 sybalsky Exp $ Copyright (C) Venue";

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

#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "emlglob.h"
#include "address.h"
#include "adr68k.h"

#include "gc2defs.h"
#include "gcscandefs.h"

#ifdef GCC386
#include "inlnPS2.h"
#endif /* GCC386 */

/**********************************************************************/
/*
                Func Name : OP_gcscan1
*/
/**********************************************************************/

void OP_gcscan1(void) {
  DLword gcscan1(register int probe);

#ifdef TRACE
  printPC();
  printf("TRACE: OP_gcscan1()\n");
#endif
  if ((TopOfStack & SEGMASK) == S_POSITIVE) { TopOfStack = gcscan1(LOLOC(TopOfStack)); }
  if (TopOfStack != NIL) TopOfStack |= S_POSITIVE;
  PC++;
} /* OP_gcscan1 end */

/**********************************************************************/
/*
                Func Name : OP_gcscan2
*/
/**********************************************************************/

void OP_gcscan2(void) {
  DLword gcscan2(register int probe);

#ifdef TRACE
  printPC();
  printf("TRACE: OP_gcscan2()\n");
#endif
  if ((TopOfStack & SEGMASK) == S_POSITIVE) { TopOfStack = gcscan2(LOLOC(TopOfStack)); }
  if (TopOfStack != NIL) TopOfStack |= S_POSITIVE;
  PC++;
} /* OP_gcscan2 end */
