/* $Id: return.c,v 1.4 2001/12/24 01:09:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***********************************************************/
/*
                File Name :	return.c
                Including	:	OP_contextsw
                                        contextsw

                Created	:	May 1, 1987 Takeshi Shimizu
                Changed :	May 19 1987 take
                                Aug 27 1987 NMitani
                                Sep.02 1987 take
                                Sep.09 1987 take
                                Oct.23 1987 Take


*/
/***********************************************************/
#include <stdio.h>

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "stack.h"
#include "emlglob.h"
#include "lspglob.h"
#include "initatms.h"
#include "cell.h"
#include "return.h"
#include "testtooldefs.h"
#include "returndefs.h"
#include "commondefs.h"

/***********************************************************************/
/*
                Func Name :	OP_contextsw
                Created	:	Jul 3, 1987 Takeshi Shimizu
                changed 	AUG 25 1987 TAKE
                                aug 31 take
                                Aug 4  1987 NMitani
                                Oct 23 1987 Take
                                Nov 05 1987 Take(modify flags,del. whocalls
                                        and incall args)

                Desc	:	Execute ContextSW to FX specified as
                                offset from IFPGE by TOS .
*/
/***********************************************************************/

void OP_contextsw(void) {
#ifdef TRACE
  printf("OP_contextsw:\n");
#endif

  contextsw(TopOfStack & 0xffff, 1, 2);
  /* TOS will be smashed ?? I'm not sure .
        PC will be incremented 1. */

} /* OP_contextsw */

/************************************************************************/
/*									*/
/*			C O N T E X T S W				*/
/*									*/
/*	Context switch							*/
/*									*/
/************************************************************************/

void contextsw(DLword fxnum, DLword bytenum, DLword flags)

/* BYTEnum that you want increment PC
      after CONTEXTSW */
/* 0bit(MSB) ON: incall mode */
/* 1bit ON : call from OP_contextsw */
/* I don't know that it is the possible case that
       flags is 3 . */
{
  DLword *next68k;
  DLword *freeptr; /* point to STK to be FSB */

#ifdef TRACE
  printf("contextsw : %d \n", fxnum);
#endif

  if (fxnum != SubovFXP) {
    /* interrupt disable during executing [special] function
      invoked by contextsw(\KEYHANDLER,\RESETSTACK,FAULT)
     */
  }

  if (flags & 1) /* INCALL? */
    error("contextswitch sets Incall");
  else
    CURRENTFX->nopush = T;

  /* store PC */
  CURRENTFX->pc = (UNSIGNED)PC - (UNSIGNED)FuncObj + bytenum;

  /* TOS save */
  if (flags & 2) {
    PushStack(fxnum);
    CurrentStackPTR += 2;
  } else {
    PushCStack;
    CurrentStackPTR += 2;
  }

  CURRENTFX->nextblock = LOLOC(LAddrFromNative(CurrentStackPTR));

  /* FSB set */
  GETWORD(CurrentStackPTR) = STK_FSB_WORD;
  GETWORD(CurrentStackPTR + 1) = (((UNSIGNED)EndSTKP - (UNSIGNED)CurrentStackPTR) >> 1);
  if (0 == GETWORD(CurrentStackPTR + 1)) error("creating 0-long free stack block.");
#ifdef STACKCHECK
  if (EndSTKP < CurrentStackPTR) error("contextsw:Illegal ESP");
#endif

  Midpunt(fxnum); /* exchanging FX */

  next68k = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

  if (GETWORD(next68k) != STK_FSB_WORD) error("contextsw(): MP9316");
  freeptr = next68k;

  /* Merging FSB area */
  while (GETWORD(freeptr) == STK_FSB_WORD) EndSTKP = freeptr = freeptr + GETWORD(freeptr + 1);

#ifdef DEBUG
  printf("contextsw:ESTKP set ");
  laddr(EndSTKP);
#endif

  if (CURRENTFX->incall) {
    error("return to frame with incall bit ");
  } else {
    if (CURRENTFX->nopush) {
#ifdef DEBUG
      printf("context:after:nopush \n");
#endif

      CURRENTFX->nopush = NIL;
      CurrentStackPTR = next68k - 2;
      TopOfStack = *((LispPTR *)CurrentStackPTR);
      CurrentStackPTR -= 2;

    } else {
#ifdef DEBUG
      printf("context:after:3 \n");
#endif
      CurrentStackPTR = next68k - 2 /*-1*/; /* CHanged by Hayata */
    }

#ifdef STACKCHECK
    CHECKFX;
    if (EndSTKP < CurrentStackPTR) error("contextsw:Illegal ESP");
#endif
    FastRetCALL;
    return;
  }

} /* end contextsw */
