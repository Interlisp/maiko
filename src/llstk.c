/* $Id: llstk.c,v 1.5 2001/12/26 22:17:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/******************************************************************/
/*
        File Name :     llstk.c
        Desc.     :     Low Level stack operations
        Including :

        Edited by :     Takeshi Shimizu(March 14, 1988)

*/
/******************************************************************/
#include <stdio.h>        // for printf, putchar
#include <string.h>	  // for memset
#include "address.h"      // for LOLOC
#include "adr68k.h"       // for NativeAligned2FromStackOffset, StackOffsetFromNative
#include "commondefs.h"   // for error, warn
#include "dbgtooldefs.h"  // for sff
#include "emlglob.h"
#include "ifpage.h"       // for IFPAGE
#include "kprintdefs.h"   // for print
#include "lispemul.h"     // for DLword, state, CurrentStackPTR, CURRENTFX
#include "lispmap.h"      // for STK_HI, STK_OFFSET, S_POSITIVE
#include "llstkdefs.h"    // for blt, check_BF, check_FX, check_stack_rooms
#include "lspglob.h"      // for InterfacePage, Stackspace, STACKOVERFLOW_word
#include "lsptypes.h"     // for GETWORD
#include "return.h"       // for AFTER_CONTEXTSW, BEFORE_CONTEXTSW
#include "stack.h"        // for StackWord, Bframe, FX, frameex1, STKWORD
#include "storagedefs.h"  // for newpage
// #include "testtooldefs.h" // for print_atomname
extern int extended_frame;

/******************************************************************/
/*
        Func Name :     extendstack()
        Desc.     :     if LastStackAddr_word is exceeded,then allocate
                        one new lisppage for STACK area.

        Edited by :     Take(March 14, 1988)

*/
/******************************************************************/
static DLword *extendstack(void) {
  LispPTR easp;
  LispPTR scanptr;

  easp = InterfacePage->endofstack;

  if (easp < LOLOC(*LastStackAddr_word)) {
    if ((easp > LOLOC(*GuardStackAddr_word)) && ((*STACKOVERFLOW_word) == NIL)) {
      extended_frame = 1;
      ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->stackoverflow = 1;
      *STACKOVERFLOW_word = *PENDINGINTERRUPT_word = ATOM_T;
    }
    newpage(STK_OFFSET | (scanptr = easp + 2));
    /* I don't concern about DOLOCKPAGES */

    MAKEFREEBLOCK(NativeAligned2FromStackOffset(scanptr), DLWORDSPER_PAGE - 2);
    InterfacePage->endofstack = scanptr = easp + DLWORDSPER_PAGE;
    SETUPGUARDBLOCK(NativeAligned2FromStackOffset(InterfacePage->endofstack), 2);
    MAKEFREEBLOCK(NativeAligned2FromStackOffset(easp), 2);
    return ((DLword *)NativeAligned4FromStackOffset(scanptr));
  } else
    return (NIL);
} /* end extendstack */

/******************************************************************/
/*
        Func Name :     moveframe(oldfx68k)

        Edited by :     Take(March 14, 1988)
*/
/******************************************************************/
static LispPTR moveframe(FX *oldfx68k) {
  int size;
  DLword *next68k;
  DLword *new68k;
  int nametbl_on_stk = NIL;
  int at_eos = NIL;

  PreMoveFrameCheck(oldfx68k);
#ifdef FLIPCURSOR
  flip_cursorbar(10);
#endif

  size = FX_size(oldfx68k) + DLWORDSPER_CELL;
  S_CHECK(size > 0, "size of stack block < 0");
  next68k = NativeAligned2FromStackOffset(oldfx68k->nextblock);

tryfsb:
  if (FSBP(next68k)) {
    /* merge free blocks */
    new68k = next68k + FSB_size(next68k);
    if (FSBP(new68k)) {
      for (; FSBP(new68k); new68k = new68k + FSB_size(new68k))
        FSB_size(next68k) += FSB_size(new68k);
      new68k = (DLword *)oldfx68k;
      goto out;
    } else if (StackOffsetFromNative(new68k) == InterfacePage->endofstack) {
      if ((StackOffsetFromNative(new68k) > LOLOC(*GuardStackAddr_word)) &&
          ((*STACKOVERFLOW_word) == NIL))
        at_eos = T; /* search FSB in earlier STACK area by freestackblock */
      else if (extendstack() != NIL) {
        new68k = (DLword *)oldfx68k;
        goto out;
      } else {
        /* These lines are different from Original Code */
        return (0xFFFF); /* No space */
      }
    } /* else if end */
  }

  CHECK_FX(oldfx68k);

  S_CHECK(oldfx68k->usecount == 0, "use count > 0");
/* we don't check \INTERRUPTABLE */
#ifdef BIGVM
  if (oldfx68k->validnametable && ((oldfx68k->nametable >> 16) == STK_HI))
#else
  if (oldfx68k->validnametable && (oldfx68k->hi2nametable == STK_HI))
#endif /* BIGVM */
  {
/* frame contains a name table, so we care that the alignment
 of the new block be same as old */
#ifdef STACKCHECK
    {
      DLword n;
#ifdef BIGVM
      n = oldfx68k->nametable & 0xFFFF;
#else
      n = oldfx68k->lonametable;
#endif /* BIGVM */
      if ((n <= StackOffsetFromNative(oldfx68k)) && (n >= oldfx68k->nextblock)) {
        WARN("moveframe:check!!", sff(LAddrFromNative(oldfx68k)));
        return 0; /* ? */
      }
    }
#endif
    nametbl_on_stk = T;
    /* Find a free stack block */
    new68k = freestackblock(size, (StackWord *)oldfx68k,
                            (LAddrFromNative(oldfx68k) - DLWORDSPER_CELL) % DLWORDSPER_QUAD);
  } else
    new68k = freestackblock(size, (StackWord *)oldfx68k, -1); /* Not needed to align */

  if (new68k == 0) return (0xFFFF); /* exhausted */
  if (new68k < Stackspace) error("freestackblock returned gunk.");

  if (at_eos && ((UNSIGNED)new68k > (UNSIGNED)oldfx68k)) {
    /* extendstack already done in freestackblock */
    ((STKBLK *)new68k)->flagword = STK_FSB_WORD;
    if (((STKBLK *)new68k)->size == 0) error("0-long stack freeblock.");
    goto tryfsb;
  }

  /* copy frame and dummy bf pointer too */
  blt(new68k, (((DLword *)oldfx68k) - DLWORDSPER_CELL), size);

  ((Bframe *)new68k)->residual = T;
  new68k = new68k + DLWORDSPER_CELL; /* now NEW points to the FX */
  ((FX *)new68k)->nextblock = (StackOffsetFromNative(new68k) + size) - DLWORDSPER_CELL;
  /* (CHECK (fetch (BF CHECKED) of (fetch (FX BLINK) of OLDFRAME)))*/
  CHECK_BF((Bframe *)NativeAligned4FromStackOffset(GETBLINK(oldfx68k)));

  /* Set true BFptr,not residual */
  SETBLINK(new68k, GETBLINK(oldfx68k));

  if (nametbl_on_stk) {
    S_CHECK(((((UNSIGNED)new68k - (UNSIGNED)oldfx68k) >> 1) % DLWORDSPER_QUAD) == 0,
            "Misalignment of stack blocks, with nametable on stack");
#ifdef BIGVM
    ((FX *)new68k)->nametable += (((UNSIGNED)new68k - (UNSIGNED)oldfx68k) >> 1);
#else
    ((FX *)new68k)->lonametable += (((UNSIGNED)new68k - (UNSIGNED)oldfx68k) >> 1);
#endif
  }
  if (((Bframe *)DUMMYBF(oldfx68k))->residual) {
    MAKEFREEBLOCK(((DLword *)oldfx68k) - DLWORDSPER_CELL, size);
  } else {
    MAKEFREEBLOCK(oldfx68k, size - DLWORDSPER_CELL);
  }

out:
#ifdef FLIPCURSOR
  flip_cursorbar(10);
#endif

  return (S_POSITIVE | StackOffsetFromNative(new68k));
} /* moveframe end */

/******************************************************************/
/*
        Func Name :     do_stackoverflow(incallp)

                retval: If There is no space for stack then return 1
                                        else return 0
                incallp:
                        If Calling during function call,incallp=T
                        else NIL
        Edited by :     Take(March 28, 1988)
*/
/******************************************************************/
int do_stackoverflow(int incallp) {
  DLword newfx;
  DLword savenext;
  DLword *oldPVar;
  int movedistance;
#ifdef STACKCHECK
  LispPTR stackcontents;
  LispPTR TopIVAR;

  stackcontents = *((LispPTR *)CurrentStackPTR);
  TopIVAR = *((LispPTR *)IVar);
#endif

  /* Don't care PC,FuncObj, */
  /* if incall flag ON, don't care that IVar
       became residual, and it is pointed to by copied FX's BLINK */
  oldPVar = PVar;

  if (*NeedHardreturnCleanup_word) { warn("HardreturnCleanup in do_stackoverflow"); }
  if (incallp) { savenext = CURRENTFX->nextblock; /* save old nextblock */ }

  BEFORE_CONTEXTSW; /* Don't Use MIDPUNT and Don't care IFPAGE */

  /* Call MOVEFRAME directly */
  if ((newfx = (DLword)moveframe(CURRENTFX)) == 0xFFFF) {
    /* To make immediately call HARDRESET */
    Irq_Stk_Check = 0;
    Irq_Stk_End = 0;
    return (1); /* Whole space exhausted */
  }

  /* Return from MOVEFRAME directly */

  PVar = (DLword *)NativeAligned4FromStackOffset(newfx + FRAMESIZE);
  movedistance = ((UNSIGNED)PVar - (UNSIGNED)oldPVar) >> 1;
  AFTER_CONTEXTSW;

  if (incallp) {
    /* set next(it pointed to old IVar) with offset */
    CURRENTFX->nextblock = savenext + movedistance;

/* including Last Arg(kept in TOS */
#ifdef BIGVM
    S_CHECK(FuncObj == (struct fnhead *)NativeAligned4FromLAddr(CURRENTFX->fnheader),
            "in call, but stack frame doesn't match FN being executed.");
#else
    S_CHECK(FuncObj == (struct fnhead *)NativeAligned4FromLAddr((CURRENTFX->hi2fnheader << 16) |
                                                           CURRENTFX->lofnheader),
            "in call, but stack frame doesn't match FN being executed.");
#endif /* BIGVM */
    CHECK_FX(CURRENTFX);

    /* We should re-Set up IVAR,CURRENTFX->nextblock */
    IVar += movedistance;
  } /* incallp */

  return (0); /* Normal return */
  /* If  incallp ,we CAN continue executing FN or APPLY by just returning */
  /* new PVar will set in funcall */
} /* end do_stackoverflow */

/******************************************************************/
/*
        Func Name :     freestackblock(n,sart,align)
        Desc.     :     Search the FSB has specified size n or more
                        Return useful area's ptr.
                                   If there is no space for STACK,return 0

        Edited by :     take(15-Jul-87)
                        take(11-Apr-88)
*/
/******************************************************************/

DLword *freestackblock(DLword n, StackWord *start68k, int align)
/* size you want(in DLword) */
/* searching will start68k at here */
/* if Negative,it needn't align */
{
  int wantedsize;
  StackWord *scanptr68k;
  STKBLK *freeptr68k;
  StackWord *easp68k;
  DLword freesize;

  DLword *extendstack(void);

  if (n % 2) error("asking for odd-length stack block");

  /* compute actually size you needed */
  wantedsize = n + STACKAREA_SIZE + MINEXTRASTACKWORDS;

  easp68k = (StackWord *)(NativeAligned2FromStackOffset(InterfacePage->endofstack));

  /*** DEBUG ***/
  S_CHECK(n > 2, "asking for block < 2 words long");
  S_CHECK(start68k != 0, "start68k = 0");
  S_CHECK(start68k >= (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase),
          "start68k before stack base");

STARTOVER:
  if (start68k)
    scanptr68k = start68k;
  else
    scanptr68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase);

SCAN:
  switch ((unsigned)(STKWORD(scanptr68k)->flags)) {
    case STK_FSB: goto FREESCAN;
    case STK_GUARD:
      if ((UNSIGNED)scanptr68k < (UNSIGNED)easp68k) goto FREESCAN;
      if (start68k) {
        scanptr68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase);
        goto SCAN;
      } else
        goto NEWPAGE;
    case STK_FX:
      scanptr68k = (StackWord *)NativeAligned2FromStackOffset(((FX *)scanptr68k)->nextblock);
      break;
    default: {
#ifdef STACKCHECK
      StackWord *orig68k = scanptr68k;
#endif
      while (STKWORD(scanptr68k)->flags != STK_BF) {
        S_WARN(STKWORD(scanptr68k)->flags == STK_NOTFLG, "NOTFLG not on", (void *)scanptr68k);
        scanptr68k = (StackWord *)(((DLword *)scanptr68k) + DLWORDSPER_CELL);
      }

#ifdef STACKCHECK
      if (((Bframe *)scanptr68k)->residual) {
        if (scanptr68k != orig68k) {
          WARN("freestackblock:scanptr68k !=org", printf(":0x%x\n", LAddrFromNative(scanptr68k)));
          return 0; /* ? */
        }
      } else {
        if (((Bframe *)scanptr68k)->ivar != StackOffsetFromNative(orig68k)) {
          WARN("BF doesn't point TopIVAR", printf(":0x%x\n", LAddrFromNative(scanptr68k)));
          return 0; /* ? */
        }
      }
#endif
      /* Used to be a +=, but SunOS4/Sparc compiles it wrong */
      scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
      break;
    }
  } /* end switch(scanptr68k */

NEXT:
  if (scanptr68k != start68k) {
    S_CHECK((UNSIGNED)scanptr68k <= (UNSIGNED)easp68k, "scan ptr past end of stack");
    goto SCAN;
  }
NEWPAGE:
  easp68k = (StackWord *)extendstack();
  if (easp68k)
    goto STARTOVER;
  else {
    warn("freestackblock:StackFull MP9319");
    return (0);
  }

FREESCAN:
  freeptr68k = (STKBLK *)scanptr68k;
  freesize = FSB_size(freeptr68k);
FREE:
  scanptr68k = (StackWord *)(((DLword *)freeptr68k) + freesize);
  if (freesize == 0) error("FREESIZE = 0");

  switch ((unsigned)(STKWORD(scanptr68k)->flags)) {
    case STK_FSB: freesize = freesize + FSB_size(scanptr68k); goto FREE;

    case STK_GUARD:
      if ((UNSIGNED)scanptr68k < (UNSIGNED)easp68k) {
        freesize = freesize + FSB_size(scanptr68k);
        goto FREE;
      }
      break;

    default: break;

  } /* end switch(scanp.. */

  if (freesize >= wantedsize) {
    if ((align < 0) || (align == (StackOffsetFromNative(freeptr68k) % DLWORDSPER_QUAD)))
      wantedsize = MINEXTRASTACKWORDS;
    else
      wantedsize = MINEXTRASTACKWORDS + DLWORDSPER_CELL;

    scanptr68k = (StackWord *)(((DLword *)freeptr68k) + wantedsize);

    SETUPGUARDBLOCK(scanptr68k, n);
    MAKEFREEBLOCK(freeptr68k, wantedsize);
    MAKEFREEBLOCK(((DLword *)scanptr68k) + n, freesize - wantedsize - n);
    return ((DLword *)scanptr68k);
  } else
    MAKEFREEBLOCK(freeptr68k, freesize);

  goto NEXT;
} /* freestackblock end */

/******************************************************************/
/*
        Func Name :     decusecount68k(frame)
        Desc.     :     Search the FSB has specified size n or more
                        Return useful are ptr.

        Edited by :     take(March 14, 1988)
*/
/******************************************************************/
#define BF_size(ptr68k) ((StackOffsetFromNative(ptr68k)) - ((Bframe *)(ptr68k))->ivar + 2)

void decusecount68k(FX *frame68k) {
  DLword *alink68k;
  Bframe *blink68k;
  DLword *clink68k;
  /*** DLword *ivar68k; */
  int size;

  if (FX_INVALIDP(frame68k)) return;
  CHECK_FX(frame68k);
  /* I don't check if \INTERRUPTABLE is NIL */
  while (StackOffsetFromNative(frame68k)) {
    if (frame68k->usecount != 0) {
      frame68k->usecount--;
      return;
    } else {
      alink68k = NativeAligned2FromStackOffset(GETALINK(frame68k));
      blink68k = (Bframe *)NativeAligned4FromStackOffset(GETBLINK(frame68k));
      clink68k = NativeAligned2FromStackOffset(GETCLINK(frame68k));

      size = FX_size(frame68k);

      if (((Bframe *)DUMMYBF(frame68k))->residual) { /* this frame has dummy BF */
        MAKEFREEBLOCK(((DLword *)frame68k) - DLWORDSPER_CELL, size + DLWORDSPER_CELL);
      } else {
        MAKEFREEBLOCK(frame68k, size);
      }

      if (blink68k->usecnt != 0) {
        blink68k->usecnt--;
      } else {
        /***    ivar68k=NativeAligned2FromStackOffset(blink68k->ivar);
            GETWORD(ivar68k)=STK_FSB_WORD;
            GETWORD(ivar68k+1)=ivar68k -(DLword *)blink68k +2; **/

        MAKEFREEBLOCK(NativeAligned2FromStackOffset(blink68k->ivar), BF_size(blink68k));
      }
      if (alink68k != clink68k) decusecount68k((FX *)alink68k);

      frame68k = (FX *)clink68k;

    } /* else end */

  } /*while end */
} /* decusecount68k end */

#ifdef ORG_FILPCORSORBAR
extern DLword *EmCursorBitMap68K;

#ifdef XWINDOW
extern int Current_Hot_X, Current_Hot_Y;
#endif /* XWINDOW */

extern struct cursor CurrentCursor;
void flip_cursorbar(int n)
{
  GETWORD(EmCursorBitMap68K + n) = ~(GETWORD(EmCursorBitMap68K + n));

#ifdef XWINDOW
  /* JDS 011213 When using current_hot_y, remember fn does 15-it! */
  Set_XCursor(Current_Hot_X, 15 - Current_Hot_Y);
#endif /* XWINDOW */
}
#else
extern short *DisplayRegion68k;
extern int DisplayRasterWidth;

void flip_cursorbar(int n) {
  short *word;
  word = DisplayRegion68k + (n * DisplayRasterWidth);
  GETWORD(word) ^= 0xFFFF;
}
#endif

/**************************************************************/
/*
                blt(dest,source,size)
*/
/**************************************************************/
void blt(DLword *dest68k, DLword *source68k, int nw) {
  /******* OLD def ,
   Due to C compiler's bug, we can't use pre-decrement for val
    source68k += nw;
    dest68k += nw;
    while(nw)
    {
      GETWORD(--dest68k)= GETWORD(--source68k);
      nw--;
     }
  **** OLD def ****/
  source68k = source68k + nw - 1;
  dest68k = dest68k + nw - 1;
  while (nw--) { GETWORD(dest68k--) = GETWORD(source68k--); }
}

/**************************************************************/
/*
                stack_check(start68k)
                for DEBUG
*/
/**************************************************************/
#ifdef FSBCHECK
  struct big_fsbs {
    DLword offset;
    DLword size;
  } bigFSB[100];
  int bigFSBindex;
#endif

void stack_check(StackWord *start68k) {
  StackWord *scanptr68k;
  StackWord *endstack68k;
  DLword *top_ivar;
  DLword save_nextblock;
  DLword savestack1, savestack2;
  DLword setflg = NIL;
  DLword freesize;

#ifdef FSBCHECK
  bigFSBindex = 0;
  memset((char *)bigFSB, 0, sizeof(bigFSB));
#endif

  if ((CURRENTFX->nextblock != StackOffsetFromNative(CurrentStackPTR)) || (!FSBP(CurrentStackPTR))) {
    if ((DLword *)CURRENTFX >= CurrentStackPTR) {
      WARN("CURRENTFX >= CurrentStackPTR??\n",
           printf("CURRENTFX=0x%x,CurrentStackPTR=0x%x\n", LAddrFromNative(CURRENTFX),
                  LAddrFromNative(CurrentStackPTR)));
    }
    setflg = T;
    printf("set CURRENTFX->nextblock in debugger. But it will be reset after this check \n");
    save_nextblock = CURRENTFX->nextblock;
    savestack1 = GETWORD(CurrentStackPTR + 2);
    savestack2 = GETWORD(CurrentStackPTR + 3);
    CURRENTFX->nextblock = StackOffsetFromNative(CurrentStackPTR + 2);
    GETWORD(CurrentStackPTR + 2) = STK_FSB_WORD;
    GETWORD(CurrentStackPTR + 3) = (((UNSIGNED)EndSTKP - (UNSIGNED)(CurrentStackPTR + 2)) >> 1);
  }

  if (start68k)
    scanptr68k = start68k;
  else
    scanptr68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase);
  endstack68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->endofstack);

  if (STKWORD(endstack68k)->flags != STK_GUARD) printf("?? endstack is not GUARD BLK\n");

  while (scanptr68k < endstack68k) {
    switch ((unsigned)(STKWORD(scanptr68k)->flags)) {
      case STK_FSB:
        freesize = FSB_size(scanptr68k);
        if (freesize == 0) { warn("FSB freesize = 0!"); }
#ifdef FSBCHECK
        if (freesize > STACKAREA_SIZE + MINEXTRASTACKWORDS) {
          if (bigFSBindex < 100) {
            bigFSB[bigFSBindex].offset = StackOffsetFromNative(scanptr68k);
            bigFSB[bigFSBindex].size = freesize;
            bigFSBindex++;
          }
        }
#endif
        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);
        putchar('F');
        break;

      case STK_GUARD:
        freesize = FSB_size(scanptr68k);
        if (freesize == 0) { warn("Guard block freesize = 0!"); }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);
        putchar('G');
        break;

      case STK_FX:
        CHECK_FX((FX *)scanptr68k);
        scanptr68k = (StackWord *)NativeAligned2FromStackOffset(((FX *)scanptr68k)->nextblock);
        putchar('X');
        break;

      default:
        top_ivar = (DLword *)scanptr68k;
        while (STKWORD(scanptr68k)->flags != STK_BF) {
          if (STKWORD(scanptr68k)->flags != STK_NOTFLG) {
            WARN("StackCheck:!=STK_NOTFLG", printf("content:0x%x\n", GETWORD((DLword *)scanptr68k)));
          }
          scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        } /* while end */
        CHECK_BF((Bframe *)scanptr68k);
        if (((Bframe *)scanptr68k)->residual) {
          if ((DLword *)scanptr68k != top_ivar)
            printf("Residual has real IVAR:0x%x\n", LAddrFromNative(scanptr68k));
        } else {
          if (((Bframe *)scanptr68k)->ivar != StackOffsetFromNative(top_ivar))
            printf("BF doesn't point TopIVAR:0x%x\n", LAddrFromNative(scanptr68k));
        }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        putchar('B');
        break;

    } /*switch end */

    if (scanptr68k != start68k) {
      if (scanptr68k > endstack68k) {
        WARN("scanptr exceeded end stack",
             printf("scanptr68k=%p endstack68k=%p", (void *)scanptr68k, (void *)endstack68k));
      }
    }
  } /* while end */

#ifdef FSBCHECK
  if (bigFSBindex != 0) {
    int i;

    printf("\nBIG FSB(s):\n");

    for (i = 0; i < bigFSBindex; i++) {
      printf("Offset: 0x%x , Size: 0x%x\n", bigFSB[i].offset, bigFSB[i].size);
    }
  }
#endif
  printf("\nStack Check done\n");
  if (setflg) {
    CURRENTFX->nextblock = save_nextblock;
    GETWORD(CurrentStackPTR + 2) = savestack1;
    GETWORD(CurrentStackPTR + 3) = savestack2;
  }

} /*stack_check end */

/**************************************************************/
/*
                walk_stack(start68k)
                for DEBUG

       Walk the stack, printing information about what we
       see as we go.  Unlike stack_check, this prints frame
       names, alink/clink/next values, free-block lengths, etc.

*/
/**************************************************************/
void walk_stack(StackWord *start68k) {
  StackWord *scanptr68k;
  StackWord *endstack68k;
  DLword *top_ivar;
  DLword save_nextblock;
  DLword savestack1, savestack2;
  DLword setflg = NIL;
  DLword freesize;

  if ((CURRENTFX->nextblock != StackOffsetFromNative(CurrentStackPTR)) || (!FSBP(CurrentStackPTR))) {
    if ((DLword *)CURRENTFX >= CurrentStackPTR) {
      WARN("CURRENTFX >= CurrentStackPTR??\n",
           printf("CURRENTFX=0x%x,CurrentStackPTR=0x%x\n", LAddrFromNative(CURRENTFX),
                  LAddrFromNative(CurrentStackPTR)));
    }
    setflg = T;
    printf("set CURRENTFX->nextblock in debugger. But it will be reset after this check \n");
    save_nextblock = CURRENTFX->nextblock;
    savestack1 = GETWORD(CurrentStackPTR + 2);
    savestack2 = GETWORD(CurrentStackPTR + 3);
    CURRENTFX->nextblock = StackOffsetFromNative(CurrentStackPTR + 2);
    GETWORD(CurrentStackPTR + 2) = STK_FSB_WORD;
    GETWORD(CurrentStackPTR + 3) = (((UNSIGNED)EndSTKP - (UNSIGNED)(CurrentStackPTR + 2)) >> 1);
  }

  /* Start from where caller specifies, (as real address!); if addr=0 */
  /* start from stackbase. */

  if (start68k) {
    scanptr68k = (StackWord *)((unsigned long)start68k & -2);
    printf("Starting at 0x%tx.", (DLword *)scanptr68k - Stackspace);
  } else {
    scanptr68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase);
    printf("Stack base = 0x%tx.", (DLword *)scanptr68k - Stackspace);
  }

  endstack68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->endofstack);

  printf("  End of stack = 0x%tx.\n\n", (DLword *)endstack68k - Stackspace);

  if (STKWORD(endstack68k)->flags != STK_GUARD)
    printf("?? endstack is not GUARD BLK\nendstack = %p, flags = %d\n\n", (void *)endstack68k,
           STKWORD(endstack68k)->flags);

  while (scanptr68k < endstack68k) {
    switch ((unsigned)(STKWORD(scanptr68k)->flags)) {
      /* Free stack block */
      case STK_FSB:
        freesize = FSB_size(scanptr68k);
        printf("%04tx:  Free block (len %d/0x%x)\n", (DLword *)scanptr68k - Stackspace, freesize,
               freesize);

        if (freesize == 0) { freesize = 2; }

        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);

        break;

      case STK_GUARD:
        freesize = FSB_size(scanptr68k);
        printf("%04tx:  Guard block (len %d/0x%x)\n", (DLword *)scanptr68k - Stackspace, freesize,
               freesize);
        if (freesize == 0) { freesize = 2; }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);

        break;

      case STK_FX:
        CHECK_FX((FX *)scanptr68k);

        printf("%04tx:  ", (DLword *)scanptr68k - Stackspace);

        {
          FX *fx = (FX *)scanptr68k;
          struct fnhead *fnobj;
#ifdef BIGVM
          fnobj = (struct fnhead *)NativeAligned4FromLAddr(fx->fnheader);
#else
          fnobj =
              (struct fnhead *)NativeAligned4FromLAddr(((int)fx->hi2fnheader << 16) | fx->lofnheader);
#endif /* BIGVM */
          print(fnobj->framename);
          printf("\talink: 0x%04x, clink: 0x%04x, next: 0x%04x\n", fx->alink, fx->clink,
                 fx->nextblock);
        }

        {
          Bframe *dummybf, *mtmp;
          int mblink;

          /* Now make sure the FX is connected to */
          /* a Basic Frame.                       */

          dummybf = (Bframe *)DUMMYBF(scanptr68k);

          /* Check for connection via BLINK field: */
          if (StackOffsetFromNative(dummybf) != GETBLINK(scanptr68k)) {
            mblink = GETBLINK(scanptr68k);
            mtmp = (Bframe *)NativeAligned4FromStackOffset(mblink);
            if ((dummybf->residual == NIL) || (dummybf->ivar != mtmp->ivar))
              printf("       [Bad residual]\n");
          }
        }

        scanptr68k = (StackWord *)NativeAligned2FromStackOffset(((FX *)scanptr68k)->nextblock);
        break;

      default:
        top_ivar = (DLword *)scanptr68k;
        while (STKWORD(scanptr68k)->flags != STK_BF) {
          if (STKWORD(scanptr68k)->flags != STK_NOTFLG) {
            printf("%04tx:  Bad BF IVAR 0x%x\n", (DLword *)scanptr68k - Stackspace,
                   GETWORD((DLword *)scanptr68k));
          }
          scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        } /* while end */
        /* CHECK_BF(scanptr68k); */
        {
          Bframe *bf = (Bframe *)scanptr68k;
          printf("%04tx:  BF  usecnt %d, resid %d, padding %d, ivar 0x%04x\n",
                 (DLword *)scanptr68k - Stackspace, bf->usecnt, bf->residual, bf->padding,
                 bf->ivar);

          if (((Bframe *)scanptr68k)->ivar != StackOffsetFromNative(top_ivar))
            printf("       [but top_ivar = 0x%04tx]\n", top_ivar - Stackspace);
        }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        break;

    } /*switch end */

    if (scanptr68k != start68k) {
      if (scanptr68k > endstack68k) {
        WARN("scanptr exceeded end stack",
             printf("scanptr68k=%p endstack68k=%p", (void *)scanptr68k, (void *)endstack68k));
      }
    }
  } /* while end */

#ifdef FSBCHECK
  if (bigFSBindex != 0) {
    int i;

    printf("\nBIG FSB(s):\n");

    for (i = 0; i < bigFSBindex; i++) {
      printf("Offset: 0x%x , Size: 0x%x\n", bigFSB[i].offset, bigFSB[i].size);
    }
  }
#endif
  printf("\nStack Check done\n");
  if (setflg) {
    CURRENTFX->nextblock = save_nextblock;
    GETWORD(CurrentStackPTR + 2) = savestack1;
    GETWORD(CurrentStackPTR + 3) = savestack2;
  }

} /* walk_stack end */

/**************************************************************/
/*
                quick_stack_check()
                for DEBUGING using FNSTKCHECK
*/
/**************************************************************/
int quick_stack_check(void) {
  StackWord *start68k;
  StackWord *scanptr68k;
  StackWord *endstack68k;
  DLword *top_ivar;
  DLword save_nextblock;
  DLword savestack1, savestack2;
  DLword setflg = NIL;
  DLword freesize;

#ifdef FSBCHECK
  bigFSBindex = 0;
  memset((char *)bigFSB, 0, sizeof(bigFSB));
#endif

  if ((CURRENTFX->nextblock != StackOffsetFromNative(CurrentStackPTR)) || (!FSBP(CurrentStackPTR))) {
    if ((DLword *)CURRENTFX >= CurrentStackPTR) {
      WARN("CURRENTFX >= CurrentStackPTR??\n",
           printf("CURRENTFX=0x%x,CurrentStackPTR=0x%x\n", LAddrFromNative(CURRENTFX),
                  LAddrFromNative(CurrentStackPTR)));
      return(1);
    }
    setflg = T;
    save_nextblock = CURRENTFX->nextblock;
    savestack1 = GETWORD(CurrentStackPTR + 2);
    savestack2 = GETWORD(CurrentStackPTR + 3);
    CURRENTFX->nextblock = StackOffsetFromNative(CurrentStackPTR + 2);
    GETWORD(CurrentStackPTR + 2) = STK_FSB_WORD;
    GETWORD(CurrentStackPTR + 3) = (((UNSIGNED)EndSTKP - (UNSIGNED)(CurrentStackPTR + 2)) >> 1);
  }

  scanptr68k = start68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->stackbase);
  endstack68k = (StackWord *)NativeAligned2FromStackOffset(InterfacePage->endofstack);

  if (STKWORD(endstack68k)->flags != STK_GUARD) printf("?? endstack is not GUARD BLK\n");

  while (scanptr68k < endstack68k) {
    switch ((unsigned)(STKWORD(scanptr68k)->flags)) {
      case STK_FSB:
        freesize = FSB_size(scanptr68k);
        if (freesize == 0) {
          warn("FSB freesize = 0!");
          return(1);
        }
#ifdef FSBCHECK
        if (freesize > STACKAREA_SIZE + MINEXTRASTACKWORDS) {
          if (bigFSBindex < 100) {
            bigFSB[bigFSBindex].offset = StackOffsetFromNative(scanptr68k);
            bigFSB[bigFSBindex].size = freesize;
            bigFSBindex++;
          }
        }
#endif
        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);
        break;
      case STK_GUARD:
        freesize = FSB_size(scanptr68k);
        if (freesize == 0) {
          warn("Guard block freesize = 0!");
          return(1);
        }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + freesize);
        break;
      case STK_FX:
        CHECK_FX((FX *)scanptr68k);
        scanptr68k = (StackWord *)NativeAligned2FromStackOffset(((FX *)scanptr68k)->nextblock);
        break;
      default:
        top_ivar = (DLword *)scanptr68k;
        while (STKWORD(scanptr68k)->flags != STK_BF) {
          if (STKWORD(scanptr68k)->flags != STK_NOTFLG) {
            warn("StackCheck:!=STK_NOTFLG");
            printf("content:0x%x\n", GETWORD((DLword *)scanptr68k));
            return(1);
          }
          scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        } /* while end */
        CHECK_BF((Bframe *)scanptr68k);
        if (((Bframe *)scanptr68k)->residual) {
          if ((DLword *)scanptr68k != top_ivar)
            printf("Residual has real IVAR:0x%x\n", LAddrFromNative(scanptr68k));
        } else {
          if (((Bframe *)scanptr68k)->ivar != StackOffsetFromNative(top_ivar))
            printf("BF doesn't point TopIVAR:0x%x\n", LAddrFromNative(scanptr68k));
        }
        scanptr68k = (StackWord *)((DLword *)scanptr68k + DLWORDSPER_CELL);
        break;
    } /*switch end */
    if (scanptr68k != start68k) {
      if (scanptr68k > endstack68k) {
        WARN("scanptr exceeded end stack",
             printf("scanptr68k=%p endstack68k=%p", (void *)scanptr68k, (void *)endstack68k));
      }
    }
  } /* while end */
#ifdef FSBCHECK
  if (bigFSBindex != 0) {
    int i;

    printf("\nBIG FSB(s):\n");

    for (i = 0; i < bigFSBindex; i++) {
      printf("Offset: 0x%x , Size: 0x%x\n", bigFSB[i].offset, bigFSB[i].size);
    }
  }
#endif

  if (setflg) {
    CURRENTFX->nextblock = save_nextblock;
    GETWORD(CurrentStackPTR + 2) = savestack1;
    GETWORD(CurrentStackPTR + 3) = savestack2;
  }
  return(0);

} /* quick_stack_check end */

/************************************************************************/
/*									*/
/*			      C H E C K _ F X				*/
/*									*/
/*	Consistency checks on a Frame Extension; used when the		*/
/*	STACKCHECK compile switch is on.				*/
/*									*/
/*	The function just returns if things are OK; calls error		*/
/*	if there are problems.						*/
/*									*/
/************************************************************************/

void check_FX(FX *fx68k) {
  Bframe *dummybf, *mtmp;
  int mblink;

  /* The FX better really be an FX */

  if (((FX *)(fx68k))->flags != STK_FX) error("CheckFX:NOT FX");

  /* Make sure the NEXTBLOCK field of the FX */
  /* Points BEYOND the start of the FX; some */
  /* stack blocks have been corrupted this   */
  /* way.  --JDS 2/3/98                      */

  if (fx68k->nextblock < StackOffsetFromNative(fx68k) /*+FRAMESIZE*/) {
    error("FX's nextblock field < the FFX.");
  }

  /* Now make sure the FX is connected to */
  /* a Basic Frame.                       */

  dummybf = (Bframe *)DUMMYBF(fx68k);

  /* Check for connection via BLINK field: */
  if (StackOffsetFromNative(dummybf) == GETBLINK(fx68k)) return;

  mblink = GETBLINK(fx68k);
  mtmp = (Bframe *)NativeAligned4FromStackOffset(mblink);
  if ((dummybf->residual != NIL) && (dummybf->ivar == mtmp->ivar))
    return;
  else
    error("CheckFX:bad residual case");

} /* END check_FX */

/************************************************************************/
/*									*/
/*			   C H E C K _ B F				*/
/*									*/
/*	Check a Basic Frame (BF) for consistency.  This is used		*/
/*	when STACKCHECK is defined.					*/
/*									*/
/************************************************************************/

void check_BF(Bframe *bf68k) {
  Bframe *iptr68k;

  /* For starterd, it must BE a BF */
  if (bf68k->flags != STK_BF) error("checkBF:not BF1");

  /* If the frame is residual (whatever that means), it's OK */
  if (bf68k->residual == T)
    return;
  else {
    if (bf68k->ivar & 1)
      error("IVAR is ODD in a BF");
    else
      for (iptr68k = (Bframe *)NativeAligned4FromStackOffset(bf68k->ivar);
           iptr68k <= (Bframe *)(((DLword *)bf68k) - 2); iptr68k++) /* inc 2DLword */
      {
        /* Make sure none of the "ivar" slots have stack-type */
        /* bits set. */
        if (iptr68k->flags != STK_NOTFLG) warn("CheckBF:Stack-bits set in IVAR");
      }
  }

} /* end check_BF */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int check_stack_rooms(FX *fx68k) {
  DLword *freeptr68k;

  CHECK_FX(fx68k);
  freeptr68k = NativeAligned2FromStackOffset(fx68k->nextblock);
  if (!FSBP(freeptr68k)) error("check_stack_rooms:  nextblock doesn't point to an FSB");
  return (FSB_size(freeptr68k));

} /* end check_stack_rooms */
