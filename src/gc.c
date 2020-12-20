/* $Id: gc.c,v 1.3 1999/05/31 23:35:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

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
#include "lsptypes.h"
#include "emlglob.h"
#include "gcdata.h"

#include "gcdefs.h"
#include "gchtfinddefs.h"

/************************************************************

        entry		OP_gcref		OPCODE[025]

        1. alpha is ADDREF or DELREF, STKREF.
           TopOfStack is argued slot address.
        2. call gclookup with alpha and TopOfStack.
        3. if stk=0 and refcnt=0 of entry of HashMainTable,
           TopOfStack left alone.
           else replace TopOfStack with 0.
        4. increment PC by 2.

***********************************************************/

void OP_gcref(void) {
#ifdef TRACE
  printPC();
  printf("TRACE:OP_gcref()\n");
#endif
  GCLOOKUPV(TopOfStack, Get_code_BYTE(PC + 1), TopOfStack);
  PC += 2;
  return;
}
