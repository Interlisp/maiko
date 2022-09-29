/* $Id: intcall.c,v 1.3 1999/05/31 23:35:34 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"       // for StackOffsetFromNative, NativeAligned4FromLAddr
#include "cell.h"         // for definition_cell, GetDEFCELL68k
#include "emlglob.h"
#include "intcalldefs.h"  // for cause_interruptcall
#include "llstkdefs.h"    // for do_stackoverflow
#include "lspglob.h"
#include "lsptypes.h"     // for GETWORD
#include "returndefs.h"   // for contextsw
#include "stack.h"        // for state, CurrentStackPTR, DLword, FX, FuncObj
#include "tosfns.h"       // for SWAPPED_FN_CHECK

void cause_interruptcall(unsigned int atom_index)
/* Atomindex for Function you want to invoke */
{
  struct definition_cell *defcell68k; /* Definition Cell PTR */
  short pv_num;                       /* scratch for pv */
  struct fnhead *tmp_fn;
  int rest; /* use for alignments */

  CURRENTFX->nopush = T;
  CURRENTFX->nextblock = StackOffsetFromNative(CurrentStackPTR) + 4;
  PushCStack; /* save TOS */

  /* Setup IVar */
  IVar = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

  /* Set PC to the Next Instruction and save into pre-FX */
  CURRENTFX->pc = ((UNSIGNED)PC - (UNSIGNED)FuncObj);

  /* Get DEFCELL 68k address */
  defcell68k = (struct definition_cell *)GetDEFCELL68k(atom_index);

  /* Interrupt FN should be compiled code */
  tmp_fn = (struct fnhead *)NativeAligned4FromLAddr(defcell68k->defpointer);

  /* This used to be >=, but I think that was a change from earlier,
     when it was originally >.  I changed it back on 2/2/98 to see
     if that fixes stack overflow trouble.  --JDS */
  if ((UNSIGNED)(CurrentStackPTR + tmp_fn->stkmin + STK_SAFE) > (UNSIGNED)EndSTKP) {
    /*printf("Interrupt:$$ STKOVER when ");
    print(atom_index);
    printf(" was called  *****\n");*/
    DOSTACKOVERFLOW(0, -1);
  }
  FuncObj = tmp_fn;
  SWAPPED_FN_CHECK; /* Check for need to re-swap code stream */
  if (FuncObj->na >= 0) {
    /* This Function is Spread Type */
    /* Arguments on Stack Adjustment  */
    rest = 0 - (FuncObj->na);

    while (rest < 0) {
      PushStack(NIL_PTR);
      rest++;
    }
    CurrentStackPTR -= (rest << 1);
  } /* if end */

  /* Set up BF */
  CurrentStackPTR += 2;
  GETWORD(CurrentStackPTR) = BF_MARK;
  GETWORD(CurrentStackPTR + 1) = CURRENTFX->nextblock;
  CurrentStackPTR += 2;

  /* Set up FX */
  GETWORD(CurrentStackPTR) = FX_MARK;

  /* Now SET new FX */
  /* Make it SLOWP */
  ((FX *)CurrentStackPTR)->alink = StackOffsetFromNative(PVar) + 1;
  ((FX *)CurrentStackPTR)->blink = StackOffsetFromNative(DUMMYBF(CurrentStackPTR));
  ((FX *)CurrentStackPTR)->clink = StackOffsetFromNative(PVar);
  PVar = (DLword *)CurrentStackPTR + FRAMESIZE;
#ifdef BIGVM
  ((FX *)CurrentStackPTR)->fnheader = (defcell68k->defpointer);
#else
  ((FX *)CurrentStackPTR)->lofnheader = (defcell68k->defpointer) & 0x0ffff;
  ((FX *)CurrentStackPTR)->hi2fnheader = ((defcell68k->defpointer) & SEGMASK) >> 16;
#endif /* BIGVM */

  CurrentStackPTR = PVar;

  /* Set up PVar area */
  pv_num = FuncObj->pv + 1;
  while (pv_num > 0) {
    *((LispPTR *)CurrentStackPTR) = 0x0ffffffff;
    CurrentStackPTR += DLWORDSPER_CELL;
    *((LispPTR *)CurrentStackPTR) = 0x0ffffffff;
    CurrentStackPTR += DLWORDSPER_CELL;
    pv_num--;
  }

  /* Set PC points New Function's first OPCODE */
  PC = (ByteCode *)FuncObj + FuncObj->startpc;
  CURRENTFX->nextblock = StackOffsetFromNative(CurrentStackPTR);
  MAKEFREEBLOCK(CurrentStackPTR, ((UNSIGNED)EndSTKP - (UNSIGNED)CurrentStackPTR) >> 1);
} /* end */
