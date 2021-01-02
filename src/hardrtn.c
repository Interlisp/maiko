/* $Id: hardrtn.c,v 1.4 2001/12/24 01:09:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/********************************************************************/
/*
        File Name :	hardrtn.c

        Edited by	Takeshi Shimizu
        Date:		10-MAY-88

        RET VAL: If success: 0
                 else (NO SPACE in STACK)  1 -> should cause HARDRESET

*/
/********************************************************************/
#include <stdio.h>
#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "adr68k.h"
#include "address.h"
#include "lspglob.h"
#include "emlglob.h"
#include "cell.h"
#include "stack.h"
#include "return.h"

#include "hardrtndefs.h"
#include "commondefs.h"
#include "llstkdefs.h"

#define MAKE_FXCOPY(fx68k)                                                                   \
  {                                                                                          \
    BEFORE_CONTEXTSW;                                                                        \
    if ((fx68k = (FX *)make_FXcopy(fx68k)) == 0) { return (1); /* Whole space exhausted */ } \
    AFTER_CONTEXTSW;                                                                         \
    CHECK_FX(fx68k);                                                                         \
  }
static FX *make_FXcopy(register FX *fx68k) {
  register int size;
  int nametbl_on_stk = NIL;
  register DLword *new68k;
  register Bframe *retbf68k;

#ifdef FLIPCURSOR
  flip_cursorbar(5);
#endif

  CHECK_FX(fx68k);
  size = FX_size(fx68k) + DLWORDSPER_CELL;
#ifdef BIGVM
  if (fx68k->validnametable && ((fx68k->nametable >> 16) == STK_HI))
#else
  if (fx68k->validnametable && (fx68k->hi2nametable == STK_HI))
#endif /* BIGVM */
  {
/* frame contains a name table, so we care that the alignment
 of the new block be same as old */
#ifdef STACKCHECK
    {
      DLword n;
#ifdef BIGVM
      n = fx68k->nametable & 0xFFFF;
#else
      n = fx68k->lonametable;
#endif /* BIGVM */
      if ((n <= StkOffset_from_68K(fx68k)) && (n >= fx68k->nextblock))
        error("hardreturn:nametable check");
    }
#endif
    nametbl_on_stk = T;
    /* Find a free stack block */
    new68k = freestackblock(size, (StackWord *)CURRENTFX,
                            (StkOffset_from_68K(fx68k) - DLWORDSPER_CELL) % DLWORDSPER_QUAD);
  } /*if end */
  else
    new68k = freestackblock(size, (StackWord *)CURRENTFX, -1); /* No align */

  if (new68k == 0) return (0); /* No more space for STACK */

  /* blt(dest,source,size) */
  blt(new68k, (((DLword *)fx68k) - DLWORDSPER_CELL), size);

  ((Bframe *)new68k)->residual = T;
  new68k = new68k + DLWORDSPER_CELL; /* now NEW points to the FX */
  ((FX *)new68k)->nextblock = (StkOffset_from_68K(new68k) + size) - DLWORDSPER_CELL;
  retbf68k = (Bframe *)Addr68k_from_StkOffset(GETBLINK(fx68k));
  /* Set true BFptr,not the residual */
  SETBLINK(new68k, GETBLINK(fx68k));
  ((FX *)new68k)->usecount = 0;
  CHECK_BF(retbf68k);

#ifdef BIGVM
  if (nametbl_on_stk) ((FX *)new68k)->nametable += (((UNSIGNED)new68k - (UNSIGNED)fx68k) >> 1);
#else
  if (nametbl_on_stk) ((FX *)new68k)->lonametable += (((UNSIGNED)new68k - (UNSIGNED)fx68k) >> 1);
#endif
  /* increment use count of basic frame of returnee because
  we made another FX which points to it */
  retbf68k->usecnt++;
  SET_FASTP_NIL(fx68k);
  /* increment use count of CLINK of returnee
     because we made a copy of returnee */
  incusecount68k((FX *)Addr68k_from_StkOffset(GETCLINK(fx68k)));

  if (GETCLINK(fx68k) != GETALINK(fx68k)) {
    incusecount68k((FX *)Addr68k_from_StkOffset(GETALINK(fx68k)));
  }

  decusecount68k(fx68k); /* if usecon==0  -> FSB */
  SETACLINK(CURRENTFX, StkOffset_from_68K(new68k));
  CHECK_FX((FX *)new68k);
  CHECK_FX(CURRENTFX);
#ifdef STACKCHECK
  stack_check(0);
#endif
#ifdef FLIPCURSOR
  flip_cursorbar(5);
#endif

  return ((FX *)new68k);
} /* make_FXcopy end */

/********************************************************************/
/*
        Func Name :	slowreturn()

        Edited by	Takeshi Shimizu
        DATE:		10-MAY-88

        RET VAL : If 0 success
                  If 1  NO STACK SPACE->uraid-> HARDRESET
                  NBriggs, Aug 11 2020: I see no return (1) case!
*/
/********************************************************************/

int slowreturn(void) {
  register DLword *next68k;
  register DLword *freeptr;
  register Bframe *currentBF;
  register FX *returnFX;

  S_CHECK(SLOWP(CURRENTFX), "CURRENTFX not SLOWP");

  /* Get returnee's FX from CURRENTFX->alink , It's SLOWP case */
  returnFX = (FX *)Addr68k_from_StkOffset(CURRENTFX->alink - 11);

  if ((CURRENTFX->alink & 0xFFFE) != CURRENTFX->clink) { /* ALINK != CLINK */
#ifdef STACKCHECK
    printf("A!=C\n");
#endif
    /* return to CLINK fx */
    SETALINK(CURRENTFX, CURRENTFX->clink);
    decusecount68k(returnFX);
    returnFX = (FX *)Addr68k_from_StkOffset(CURRENTFX->clink - FRAMESIZE);
  }

  if (returnFX->usecount != 0) { /* COPY returnee's FX */
#ifdef STACKCHECK
    printf("returnFX->usecount !=0\n");
#endif
    MAKE_FXCOPY(returnFX); /* smashes returnFX */
  }

retry: /* this is retry entry after MAKE_FXCOPY etc */

  next68k = Addr68k_from_StkOffset(returnFX->nextblock);
  currentBF = (Bframe *)Addr68k_from_StkOffset(CURRENTFX->blink);

  if (GETWORD(next68k) == STK_FSB_WORD) {
  another:
    freeptr = ((DLword *)CURRENTFX) - 2;
    if (BFRAMEPTR(freeptr)->residual) {
      if (BFRAMEPTR(currentBF)->usecnt == 0) {
        /* make FREEBLOCK for real BF */
        GETWORD(IVar) = STK_FSB_WORD;
        GETWORD(IVar + 1) = (((UNSIGNED)currentBF - (UNSIGNED)IVar) >> 1) + 2;
        if (0 == GETWORD(IVar + 1)) error("creating 0-len block");
      } else
        BFRAMEPTR(currentBF)->usecnt--;
    } else
      freeptr = IVar; /* reset free ptr: not residual case */

    /* free FX + BF(dummy or real) */
    GETWORD(freeptr) = STK_FSB_WORD;
    GETWORD(freeptr + 1) = ((UNSIGNED)EndSTKP - (UNSIGNED)freeptr) >> 1;
    if (0 == GETWORD(freeptr + 1)) error("creating 0-len block");
    S_CHECK(EndSTKP >= freeptr, "EndSTKP < freeptr!");
    S_CHECK(FSB_size(freeptr) >= MINEXTRASTACKWORDS,
            "free block < min size, after deciding it fits.");

    PVar = ((DLword *)returnFX) + FRAMESIZE;
    /*  Now right CURRENTFX(PVar) is set */

    if (GETWORD(next68k) != STK_FSB_WORD) error("OP_return: MP9316");

    freeptr = next68k;
    while (GETWORD(freeptr) == STK_FSB_WORD) EndSTKP = freeptr = freeptr + GETWORD(freeptr + 1);

    if (CURRENTFX->incall) { /* this frame is  Interrupted */
      error("Stack error: INCALL bit found in returnee frame");
    } else {
      if (CURRENTFX->nopush) {
        CURRENTFX->nopush = NIL;
        CurrentStackPTR = next68k - 2;
        TopOfStack = *((LispPTR *)CurrentStackPTR);
        CurrentStackPTR -= 2;

      } else
        CurrentStackPTR = next68k - 2;

      S_CHECK(EndSTKP >= freeptr, "EndSTKP < freeptr");

      FastRetCALL;
#ifdef LISPTRACE
      printf("TRACE: return from ");
      print(fnobj->framename);
      printf(" :<= ");
      print(TopOfStack);
      printf("\n");
#endif
      return (0); /* normal return */
    }

  } /* FSB end */
  else if (next68k != IVar) {
#ifdef STACKCHECK
    printf("next68k != IVar and not FSB\n");
#endif
    MAKE_FXCOPY(returnFX); /* smashes returnFX */
    goto retry;
  } else if (BFRAMEPTR(currentBF)->usecnt != 0) {
#ifdef STACKCHECK
    printf("currentBF->usecnt != 0");
#endif
    MAKE_FXCOPY(returnFX); /* smashes returnFX */
    goto retry;
  } else {
    if ((next68k == IVar) || (BFRAMEPTR(currentBF)->usecnt == 0))
      goto another;
    else
      error("Shouldn't");
  }
  error("Control reached end of slowreturn()!");
  return (NIL); /* NOT REACHED */
} /* slowreturn end */

#define MAXSAFEUSECOUNT 200

void incusecount68k(FX *fx68k) {
  StackWord *scanptr68k;

  if (FX_INVALIDP(fx68k)) return;

  CHECK_FX(fx68k);

  if ((++(fx68k->usecount)) > MAXSAFEUSECOUNT)
    error("MP9324:Stack frame use count maximum exceeded");

  scanptr68k = (StackWord *)Addr68k_from_StkOffset(fx68k->nextblock);
  switch (STKWORD(scanptr68k)->flags) {
    case STK_NOTFLG:
      while (STKWORD(scanptr68k)->flags != STK_BF)
        scanptr68k = (StackWord *)(((DLword *)scanptr68k) + DLWORDSPER_CELL);
      break;

    case STK_BF: break;

    default:
      return;
      /* break; */
  } /* switch end */

  /* DEBUG */
  S_CHECK(
      (((Bframe *)scanptr68k)->residual == T) || (((Bframe *)scanptr68k)->ivar == fx68k->nextblock),
      "BF not residual, and IVER /= nextblock of prior frame");

  scanptr68k = (StackWord *)(((DLword *)scanptr68k) + DLWORDSPER_CELL);

  if (STKWORD(scanptr68k)->flags == STK_FX) {
    CHECK_FX((FX *)scanptr68k);
    SET_FASTP_NIL(scanptr68k);
  }

} /* incusecount68k end */
