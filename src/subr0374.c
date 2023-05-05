/* $Id: subr0374.c,v 1.3 1999/05/31 23:35:43 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 * This doesn't appear to be used anywhere.
 * Adjusted result to be LispPTR and return value to NIL instead of
 * being an int/return 0.
 *
 * NBriggs, May 2017
 */

/********************************************************/
/*
        subr_k_trace()

                subr----0130 for maiko trace
                first argument is base address of
                error message in Lisp.
                second argument is length of message.
*/
/********************************************************/

#include <stdio.h>
#include "lispemul.h"
#include "adr68k.h"
#include "lspglob.h"

#include "subr0374defs.h"

LispPTR subr_k_trace(LispPTR *args) {
  int len;
  char *base;

  len = 0xFFFF & args[1];
  base = (char *)NativeAligned2FromLAddr(args[0]);
  while (len-- > 0) putc(*base++, stderr);
  putc('\n', stderr);
  return (NIL);
}
