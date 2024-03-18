/* $Id: findkey.c,v 1.3 1999/05/31 23:35:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"
#include "stack.h"
#include "lspglob.h"
#include "adr68k.h"
#include "testtooldefs.h"
#include "findkeydefs.h"

/***********************************************************************/
/*
                File Name :	findkey.c

                Desc	:

                                Date :		Mar. 29 88
                                Edited by :	Bob Krivacic
                Including :	N_OP_findkey


*/
/**********************************************************************/

LispPTR N_OP_findkey(LispPTR tos, int byte) {
  LispPTR *ptr;
  DLword *find_end;
  DLword arg_nth;

#ifdef TRACE
  printPC();
  printf("TRACE : N_OP_findkey \n");
#endif

  if (CURRENTFX->alink & 1) { /* slow case */
    find_end = NativeAligned2FromStackOffset(CURRENTFX->blink - 4);
  } else { /*  Fast cae */
    find_end = ((DLword *)CURRENTFX) - 2 - 4;
  }

  arg_nth = byte + 1;

  for (ptr = (LispPTR *)(IVar + ((byte * 2) - 2)); (UNSIGNED)find_end >= (UNSIGNED)ptr;
       ptr += 2, arg_nth += 2) {
    if (*ptr == tos) { /* KEY founded */
      return (S_POSITIVE | arg_nth);
    }
  } /* for end */

  /* No matched */

  return (NIL_PTR);

} /* end N_OP_findkey() */
