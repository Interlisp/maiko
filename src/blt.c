/* $Id: blt.c,v 1.3 1999/05/31 23:35:24 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *
 *	Author :  Takeshi Shimizu
 *
 */
/******************************************************************/
/*
                File Name  :	blt.c
                Including  :	OP_blt

                Created    :	jul 9, 1987 by T.Shimizu
*/
/******************************************************************/

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "stack.h"
#include "emlglob.h"
#include "lspglob.h"
#include "cell.h"

#include "bltdefs.h"

/*
  N_OP_blt takes 3 arguments.
        STK-1 has destination's pointer.
        STK has source's pointer.
        TOS has number of words to be translated.
*/

LispPTR N_OP_blt(LispPTR destptr, LispPTR sourceptr, LispPTR wordcount) {
  DLword *source68k;
  DLword *dest68k;
  int nw;

  if ((wordcount & SEGMASK) != S_POSITIVE) ERROR_EXIT(wordcount);
  nw = wordcount & 0xffff;

  source68k = NativeAligned2FromLAddr(sourceptr) + nw;
  dest68k = NativeAligned2FromLAddr(destptr) + nw;

  while (nw) {
    (GETWORD(--dest68k)) = GETWORD(--source68k);
    nw--;
  }

  return (wordcount);
} /* end N_OP_blt */
