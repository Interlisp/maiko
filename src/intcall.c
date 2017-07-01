/* $Id: intcall.c,v 1.3 1999/05/31 23:35:34 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: intcall.c,v 1.3 1999/05/31 23:35:34 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "stack.h"
#include "llstk.h"
#include "return.h"
#include "emlglob.h"
#include "lspglob.h"
#include "initatms.h"
#include "cell.h"
#include "tosfns.h"

void cause_interruptcall(register unsigned int atom_index)
/* Atomindex for Function you want to invoke */
{
  register struct definition_cell *defcell68k; /* Definition Cell PTR */
  register short pv_num;                       /* scratch for pv */
  register struct fnhead *tmp_fn;
  int rest; /* use for arignments */

  CURRENTFX->nopush = T;
  CURRENTFX->nextblock = StkOffset_from_68K(CurrentStackPTR) + 4;
  PushCStack; /* save TOS */

  /* Setup IVar */
  IVar = Addr68k_from_StkOffset(CURRENTFX->nextblock);

  /* Set PC to the Next Instruction and save into pre-FX */
  CURRENTFX->pc = ((UNSIGNED)PC - (UNSIGNED)FuncObj);

  /* Get DEFCELL 68k address */
  defcell68k = (struct definition_cell *)GetDEFCELL68k(atom_index);

  /* Interrupt FN should be compiled code */
  tmp_fn = (struct fnhead *)Addr68k_from_LADDR(defcell68k->defpointer);

  /* This used to be >=, but I think that was a change from earlier,
     when it was originally >.  I changed it back on 2/2/98 to see
     if that fixes stack overflow toruble.  --JDS */
  if ((UNSIGNED)(CurrentStackPTR + tmp_fn->stkmin + STK_SAFE) > (UNSIGNED)EndSTKP) {
    /*printf("Intrrupt:$$ STKOVER when ");
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
  ((FX *)CurrentStackPTR)->alink = StkOffset_from_68K(PVar) + 1;
  ((FX *)CurrentStackPTR)->blink = StkOffset_from_68K(DUMMYBF(CurrentStackPTR));
  ((FX *)CurrentStackPTR)->clink = StkOffset_from_68K(PVar);
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
  CURRENTFX->nextblock = StkOffset_from_68K(CurrentStackPTR);
  MAKEFREEBLOCK(CurrentStackPTR, ((UNSIGNED)EndSTKP - (UNSIGNED)CurrentStackPTR) >> 1);
} /* end */
