/* $Id: ufn.c,v 1.2 1999/01/03 02:07:41 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/******************************************************************/
/*

                File Name  :	ufn.c(for use with SYSOUT)
                Including  :	ufn

                Created    :	jun 8, 1987 by T.Shimizu


*/
/******************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

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

#ifdef C_ONLY

#define GetUFNEntry(num) (((UFN *)UFNTable) + (num))

/******************************************************************/
/*

                Func Name  :	ufn
                Desc  :		call Lisp UFN func
                                TopOfStack;
                                PC ;

                Created    :	May 29, 1987 by T.Shimizu
                Changed	   :	Jun 17 1987 take
                                Aug 25 1987 take
                                Sep  4  1987 NMitani
                                Oct 07 1987 take
                                Nov 13 1987 take +8 & Simple STKCHK
                                Nov 25 1987 take(tmp_fn)
*/
/******************************************************************/

void ufn(DLword bytecode)
{
  DefCell *defcell68k; /* Definition Cell PTR */
  int pv_num;          /* scratch for pv */
  UFN *entry68k;
  struct fnhead *tmp_fn;
  unsigned int arg_num; /* Num of args */
  int rest;             /* use for alignments */

#ifdef TRACE
  printPC();
  print_atomname("TRACE : ufn() 0%o ", bytecode); /* XXX: this is WRONG */
#endif

  PushCStack;

  entry68k = (UFN *)GetUFNEntry(bytecode); /*changing I/F 17-jun*/

#ifdef TRACE
  if (entry68k->atom_name)
    printf("Atom number: %d\n", 0xffff && entry68k->atom_name);
  else
    error("UNF not specified");
#endif

  switch (entry68k->byte_num) {
    case 0: break;
    case 1: /*PushStack(SPOS_HI  | Get_BYTE(PC+1));*/
      CurrentStackPTR += 2;
      GETWORD(CurrentStackPTR) = SPOS_HI;
      GETWORD(CurrentStackPTR + 1) = (DLword)Get_code_BYTE(PC + 1);
/* I think we don't have to shift alpha byte eight bit before save it. */
#ifdef DEBUG
      printf("***ufn: case 1\n");
#endif
      break;

    case 2: /*PushStack(S_POSITIVE |Get_DLword(PC+1));*/
      CurrentStackPTR += 2;
      GETWORD(CurrentStackPTR) = SPOS_HI;
      GETWORD(CurrentStackPTR + 1) = (DLword)((Get_code_BYTE(PC + 1) << 8) | Get_code_BYTE(PC + 2));

#ifdef DEBUG
      printf("***ufn: case 2\n");
#endif
      break;
    default: error("ufn : Bad UFN MP 9351 "); break;
  }

  /* Get Next Block offset form OPCODE byte */
  CURRENTFX->nextblock =
      (LAddrFromNative(CurrentStackPTR) & 0x0ffff) - (entry68k->arg_num << 1) + 2 /** +1 **/;

  /* Setup IVar */
  IVar = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

#ifdef LISPTRACE
  print(entry68k->atom_name);
  printf(": ");
  {
    int cnt;
    for (cnt = 0; cnt < arg_num; cnt++) {
      printf(" IVAR%d :", cnt);
      print(*((LispPTR *)(IVar + (cnt * 2))));
    }
  }
  printf("\n");
#endif
  /* Set PC to the Next Instruction and save into FX */
  CURRENTFX->pc = ((UNSIGNED)PC - (UNSIGNED)FuncObj) + entry68k->byte_num + 1;

  defcell68k = (DefCell *)GetDEFCELL68k(entry68k->atom_name);

  if (defcell68k->ccodep == 0) { /* This LispFunc is NOT compiled object . We must use Interpreter*/
    printf("UFN: UFN func isn't compiled OBJ \n");
    defcell68k = (DefCell *)GetDEFCELL68k(ATOM_INTERPRETER);
    PushStack(TopOfStack); /* Move AtomIndex to CSTK */
  }

  /* Nov 25 87 take */
  tmp_fn = (struct fnhead *)NativeAligned4FromLAddr(defcell68k->defpointer);

  /* stack overflow check STK_SAFE is redundant?? */
  if ((UNSIGNED)(CurrentStackPTR + tmp_fn->stkmin + STK_SAFE) >= (UNSIGNED)StkLimO) {
    /**printf("#### STKOVER in UFN case\n");**/

    DOSTACKOVERFLOW(entry68k->arg_num, entry68k->byte_num); /* After STKOVR, retry current OPCODE */
  }

  FuncObj = tmp_fn;

  if (FuncObj->na >= 0) {
    /* This Function is Spread Type */
    /* Arguments on Stack Adjustment  */
    rest = entry68k->arg_num - FuncObj->na;

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
  /*  *(++CurrentStackPTR) = CURRENTFX->nextblock ; */

  /* Set up FX */
  GETWORD(CurrentStackPTR) = FX_MARK;

  ((struct frameex1 *)CurrentStackPTR)->alink = LAddrFromNative(PVar);
  PVar = (DLword *)CurrentStackPTR + FRAMESIZE;
#ifdef BIGVM
  ((struct frameex1 *)CurrentStackPTR)->fnheader = (defcell68k->defpointer);
#else
  ((struct frameex1 *)CurrentStackPTR)->lofnheader = (defcell68k->defpointer) & 0x0ffff;
  ((struct frameex1 *)CurrentStackPTR)->hi2fnheader = ((defcell68k->defpointer) & SEGMASK) >> 16;
#endif /* BIGVM */

  CurrentStackPTR = PVar;

  /* Set up PVar area */
  pv_num = FuncObj->pv + 1;

  while (pv_num > 0) {
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    pv_num--;
  }

  /*  CurrentStackPTR ++ ;*/

  /* Set PC points New Function's first OPCODE */
  PC = (ByteCode *)((UNSIGNED)FuncObj + FuncObj->startpc);

  /** TopOfStack = (FuncObj->startpc -1) >> 1;**/

} /* end ufn */

#endif
