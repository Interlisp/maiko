/* $Id: dbgtool.c,v 1.4 2001/12/24 01:09:00 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***************************************************************/
/*
        file name :	 Kdbgtool.c

        For Debugging Aids

        Including :
                get_ivar_name(fx_addr68k , offset)

                date :   25 Aug 1987	NMitani
                changed: 09 Sep 1987 NMitani

*/
/***************************************************************/
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "emlglob.h"
#include "cell.h"
#include "stack.h"

#include "dbgtooldefs.h"
#include "kprintdefs.h"
#include "testtooldefs.h"
#include "uraidextdefs.h"

#define LOCAL_PVAR 0xFFFF
#ifdef BIGATOMS
#define VTY_IVAR 0x00000000
#define VTY_PVAR 0x80000000
#define VTY_FVAR 0xC0000000
#define NT_OFFSET_MASK 0xFFFFFFF
#define GetNTEntry(X) GetLongWord(X)
#define NAMETABLE LispPTR
#else
#define VTY_IVAR 0x0000
#define VTY_PVAR 0x8000
#define VTY_FVAR 0xC000
#define NT_OFFSET_MASK 0xFF
#define GetNTEntry(X) GETWORD(X)
#define NAMETABLE DLword
#endif

int BT_lines;
int BT_temp;
jmp_buf BT_jumpbuf;
#ifdef DOS
#define BTMAXLINE 24
/* DOS has a 25-line screen, and getchar discards ESC for some reason */
#define BT_morep                                              \
  do {                                              	      \
    if ((BT_temp != '!') && (++BT_lines > BTMAXLINE)) {         \
      printf("Press Return(Esc & Ret to quit, ! don't stop):"); \
      BT_temp = getch();                                        \
      fflush(stdin);                                            \
      BT_lines = 0;                                             \
      if (BT_temp == 27) longjmp(BT_jumpbuf, 1);                \
    }                                                           \
  } while (0)
#else /* DOS */
#define BTMAXLINE 30
#define BT_morep                                  \
  do {                                            \
    if (++BT_lines > BTMAXLINE) {                   \
      printf("Press Return(to quit Esc and Ret):"); \
      BT_temp = getchar();                          \
      fflush(stdin);                                \
      BT_lines = 0;                                 \
      if (BT_temp == 27) longjmp(BT_jumpbuf, 1);    \
    }                                               \
  } while (0)
#endif /* DOS */

/***************************************************************/
/*
        Func Name :	get_ivar_name

        Desc :		returns atom index of ivar on the given
                        FX.
                        If ivar is localvars then set *localivar
                        1 otherwise 0.

        Changed		25 Aug 1987	NMitani

*/
/***************************************************************/

LispPTR get_ivar_name(struct frameex1 *fx_addr68k, DLword offset, int *localivar) {
  NAMETABLE *first_table;
  NAMETABLE *second_table;
  struct fnhead *fnobj;
  int i;
#ifdef BIGVM
  fnobj = (struct fnhead *)NativeAligned4FromLAddr((fx_addr68k)->fnheader);
#else
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(((int)(fx_addr68k)->hi2fnheader << 16) |
                                              (fx_addr68k)->lofnheader);
#endif /* BIGVM */
  if (fnobj->ntsize > 0) {
    /* name table exists */
    first_table = (NAMETABLE *)(fnobj + 1);
    second_table = (NAMETABLE *)((DLword *)first_table + fnobj->ntsize);

    for (i = 0; (i < fnobj->ntsize) && (GetNTEntry(second_table) != offset);
         first_table++, second_table++, i++)
      ;
    if ((i < fnobj->ntsize) && (GetNTEntry(first_table) != 0)) {
      /* target ivar was in name table */
      *localivar = 0;
      return ((LispPTR)(GetNTEntry(first_table)));
    } else {
/* Target ivar is in locar vars table */
#ifdef BIGATOMS
      first_table = (NAMETABLE *)(fnobj + 1) + (fnobj->ntsize);
      second_table =
          first_table + (((DLword *)fnobj + (fnobj->startpc >> 1) - (DLword *)first_table) >> 2);
#else
      first_table = (NAMETABLE *)(fnobj + 1) + (fnobj->ntsize << 1);
      second_table = first_table + (((DLword *)fnobj + (fnobj->startpc >> 1) - first_table) >> 1);
#endif
    }
  } else {
/* name table doesn't exist, so all ivars are in locar vars table */
#ifdef BIGATOMS
    int delta;
    first_table = (NAMETABLE *)(fnobj + 1) + (4 >> 1);
    delta = (DLword *)(((DLword *)fnobj) + (fnobj->startpc >> 1)) - (DLword *)first_table;
    second_table = first_table + (delta >> 2);
#else
    first_table = (NAMETABLE *)(fnobj + 1) + 4;
    second_table = first_table + (((DLword *)fnobj + (fnobj->startpc >> 1) - first_table) >> 1);
#endif
  }
#ifdef BIGATOMS
  while (*(second_table) != (VTY_IVAR | offset))
#else
  while (GETWORD(second_table) != (VTY_IVAR | offset))
#endif /* BIGATOMS */

  {
    first_table++;
    second_table++;
  }
  *localivar = 1;

#ifdef BIGATOMS
  return ((LispPTR)(*(first_table)));
#else
  return ((LispPTR)(GETWORD(first_table)));
#endif /* BIGATOMS */

} /* end get_ivar_name */

/***************************************************************/
/*
        Func Name :	get_pvar_name

        Desc :		returns atom index of pvar on the given
                        FX or LOCAL_PVAR if that pvar is localvars.

        Changed		26 Aug 1987	NMitani

*/
/***************************************************************/

LispPTR get_pvar_name(struct frameex1 *fx_addr68k, DLword offset) {
  NAMETABLE *first_table;
  NAMETABLE *second_table;
  struct fnhead *fnobj;
  int i;

#ifdef BIGVM
  fnobj = (struct fnhead *)NativeAligned4FromLAddr((fx_addr68k)->fnheader);
#else
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(((int)(fx_addr68k)->hi2fnheader << 16) |
                                              (fx_addr68k)->lofnheader);
#endif /* BIGVM */

  first_table = (NAMETABLE *)(fnobj + 1);
  second_table = (NAMETABLE *)((DLword *)first_table + fnobj->ntsize);
  if (first_table == second_table) return (LOCAL_PVAR); /* no name table */

  for (i = 0;
#ifdef BIGATOMS
       i < (fnobj->ntsize >> 1) && *(second_table) != (VTY_PVAR | offset);
#else
       i < fnobj->ntsize && GETWORD(second_table) != (VTY_PVAR | offset);
#endif
       first_table++, second_table++, i++) /* Do nothing */
    ;

#ifdef BIGATOMS
  if (i < (fnobj->ntsize >> 1)) return ((LispPTR) * (first_table));
#else
  if (i < fnobj->ntsize) return ((LispPTR)GETWORD(first_table));
#endif
  else
    return (LOCAL_PVAR); /* Pvar was local */
} /* end get_pvar_name */

/************************************************************************/
/*									*/
/*			g e t _ f n _ f v a r _ n a m e			*/
/*									*/
/*	Given (LISP) codeblock addr and an offset return fvar's name	*/
/*									*/
/************************************************************************/

LispPTR get_fn_fvar_name(struct fnhead *fnobj, DLword offset) {
  NAMETABLE *first_table;
  NAMETABLE *second_table;

  first_table = (NAMETABLE *)((DLword *)fnobj + fnobj->fvaroffset);
  second_table = (NAMETABLE *)((DLword *)first_table + fnobj->ntsize);
#ifdef BIGATOMS
  while (*(second_table) != (VTY_FVAR | offset))
#else
  while (GETWORD(second_table) != (VTY_FVAR | offset))
#endif /* BIGATOMS */

  {
    first_table++;
    second_table++;
  }
#ifdef BIGATOMS
  return ((LispPTR) * (first_table));
#else
  return ((LispPTR)GETWORD(first_table));
#endif /* BIGATOMS */

} /* end get_fvar_name */

/************************************************************************/
/*									*/
/*			g e t _ f v a r _ n a m e			*/
/*									*/
/*	Given an FX and fvar offset, return the atom# for the fvar.	*/
/*									*/
/************************************************************************/

LispPTR get_fvar_name(struct frameex1 *fx_addr68k, DLword offset) {
#ifdef BIGVM
  return (get_fn_fvar_name((struct fnhead *)NativeAligned4FromLAddr((fx_addr68k)->fnheader), offset));
#else
  return (get_fn_fvar_name((struct fnhead *)NativeAligned4FromLAddr(
                               ((int)(fx_addr68k)->hi2fnheader << 16) | (fx_addr68k)->lofnheader),
                           offset));
#endif /* BIGVM */
} /* end get_fvar_name */

/************************************************************************/
/*									*/
/*				s f					*/
/*									*/
/*	Dump the contents of a single frame, given the address of 	*/
/*	the frame as a NATIVE address.					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int sf(struct frameex1 *fx_addr68k) {
  Bframe *bf;
  DLword *next68k;
  DLword *ptr;
  DLword *ptrhi;
  DLword *ptrlo;
  LispPTR atomindex;
  int i;
  DLword npvar;
  DLword max_npvar;
  LispPTR pvarindex;
  DLword nfvar;
  struct fnhead *fnobj;
  int localivar;
  LispPTR ivarindex;

  BT_lines = 0;

  if ((UNSIGNED)fx_addr68k == 0) return (-1);
  if ((fx_addr68k)->flags != STK_FX) {
    printf("Invalid FX 0x%x, flags = 0x%x.\n", LAddrFromNative(fx_addr68k), (fx_addr68k)->flags);
    return (-1);
  }

  if (((fx_addr68k)->alink & 1) == 0) { /* FAST */
    bf = (Bframe *)(((DLword *)fx_addr68k) - 2);
  } else { /* SLOW */
    bf = (Bframe *)NativeAligned4FromStackOffset((fx_addr68k)->blink);
  }

  /* Print IVARs */
  printf("IVAR -------\n");
  BT_morep;

  ptr = NativeAligned2FromStackOffset(bf->ivar);
  i = 0;
  while (ptr != (DLword *)bf) {
    ptrlo = ptr + 1;
    printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptr), GETWORD(ptrlo));
    ivarindex = get_ivar_name(fx_addr68k, i++, &localivar);
    if (localivar == 1) printf("*local* ");
    print_atomname(ivarindex);
    printf("  ");
    print(*(LispPTR *)ptr);
    putchar('\n');
    BT_morep;
    ptr += 2;
  }
  putchar('\n');
  BT_morep;
  printf("## STACK BF at 0x%x ##\n", (LispPTR)LAddrFromNative(bf));
  BT_morep;

  /* print BF  */
  if (bf->flags != 4) {
    printf("Invalid frame, NOT a BX\n");
    return (-1);
  }
  putchar('[');
  if (bf->residual) printf("Res, ");
  if (bf->padding) printf("Pad, ");
  printf("cnt=%d ]\n", bf->usecnt);
  BT_morep;
  printf("ivar : 0x%x\n", bf->ivar);
  BT_morep;

  printf(">> Bf's ivar says 0x%x vs. IVar says 0x%x\n", bf->ivar + STK_OFFSET,
         LAddrFromNative(IVar));
  BT_morep;

  atomindex = get_framename(fx_addr68k);
  printf("Fname is ");
  print(atomindex);
  printf("\n");
  BT_morep;

  /***** printout FX ****/
  printf("## STACK FX at 0x%x ##\n", LAddrFromNative(fx_addr68k));
  BT_morep;

  if ((fx_addr68k)->flags != 6) {
    printf("Invalid frame, NOT FX\n");
    return (-1);
  }

  putchar('[');
  if ((fx_addr68k)->fast) printf("F, ");
  if ((fx_addr68k)->incall) printf("incall, ");
  if (fx_addr68k->validnametable) printf("V, ");
  printf("cnt = %d ]\n", fx_addr68k->usecount);
  BT_morep;

  printf(" #alink           0x%x ", fx_addr68k->alink);

  if (fx_addr68k->alink & 1)
    printf("[SLOWP]\n");
  else
    printf("\n");

  BT_morep;

#ifdef BIGVM
  printf(" fnhead   0x%x \n", fx_addr68k->fnheader);
  BT_morep;
#else
  printf(" fnheadlo         0x%x \n", fx_addr68k->lofnheader);
  BT_morep;
  printf(" hi1,hi2 fnhead   0x%x , 0x%x \n", fx_addr68k->hi1fnheader, fx_addr68k->hi2fnheader);
  BT_morep;
#endif /* BIGVM */
  printf(" nextblock        0x%x \n", fx_addr68k->nextblock);
  BT_morep;
  printf(" pc               0x%x \n", fx_addr68k->pc);
  BT_morep;
#ifdef BIGVM
  printf(" nametbl  0x%x \n", fx_addr68k->nametable);
  BT_morep;
#else
  printf(" lonametbl        0x%x \n", fx_addr68k->lonametable);
  BT_morep;
  printf(" hi1,hi2 nametbl  0x%x , 0x%x  \n", fx_addr68k->hi1nametable, fx_addr68k->hi2nametable);
  BT_morep;
#endif /* BIGVM */
  printf(" #blink           0x%x \n", fx_addr68k->blink);
  BT_morep;
  printf(" #clink           0x%x \n", fx_addr68k->clink);
  BT_morep;

/* added by NMitani 26 Aug 87 */

#ifdef BIGVM
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(fx_addr68k->fnheader);
#else
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(((int)fx_addr68k->hi2fnheader << 16) |
                                              fx_addr68k->lofnheader);
#endif                                /* BIGVM */
  max_npvar = npvar = fnobj->nlocals; /* npvar is number of Pvars */
  if (fnobj->fvaroffset)
#ifdef BIGATOMS
    nfvar = *((NAMETABLE *)((DLword *)fnobj + fnobj->fvaroffset + fnobj->ntsize)) & NT_OFFSET_MASK;
#else
    nfvar = GETWORD((NAMETABLE *)((DLword *)fnobj + fnobj->fvaroffset + fnobj->ntsize)) &
            NT_OFFSET_MASK;
#endif /* BIGATOMS */

  else
    nfvar = 0;

  if (fx_addr68k == CURRENTFX) {
    ptr = PVar;
    i = 0;
    while (npvar-- > 0) {
      ptrhi = ptr;
      ptrlo = ptr + 1;
      printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
      if ((pvarindex = get_pvar_name(fx_addr68k, i++)) == LOCAL_PVAR)
        printf("*local* [pvar%d] ", (i - 1));
      else
        print_atomname(pvarindex);
      if (GETWORD(ptr) == 0xFFFF) {
        printf("  [variable not bound]\n");
        BT_morep;
      } else {
        printf("  ");
        print(*(LispPTR *)ptr);
        putchar('\n');
        BT_morep;
      }
      ptr += 2;
    }
    i = max_npvar;
    while (nfvar && nfvar-- >= max_npvar) {
      ptrhi = ptr;
      ptrlo = ptr + 1;
      printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
      if (0xFFFF == GETWORD(ptrhi)) {
        printf("[not looked up]  ");
        print_atomname(get_fvar_name(fx_addr68k, i));
        putchar('\n');
        BT_morep;
      } else if ((0xFFFF & GETWORD(ptrlo)) == 1) {
        printf("[fvar  ");
        print_atomname(get_fvar_name(fx_addr68k, i));
        printf("  on stack]  ");
        print(*(LispPTR *)NativeAligned4FromLAddr(((int)(0x0F & GETWORD(ptrlo)) << 16) | GETWORD(ptrhi)));
        putchar('\n');
        BT_morep;
      } else {
        printf("[fvar  ");
        print_atomname(get_fvar_name(fx_addr68k, i));
        printf("  top value ]   ");
        print(*(LispPTR *)NativeAligned4FromLAddr(((int)(0xFFF & GETWORD(ptrlo)) << 16) | GETWORD(ptrhi)));
        putchar('\n');
        BT_morep;
      }
      ptr += 2;
      i++;
    }
    if (fx_addr68k->alink == 11) /* for contextsw */
      next68k = NativeAligned2FromStackOffset(fx_addr68k->nextblock);

    else
      next68k = CurrentStackPTR;
    while (ptr < next68k) {
      ptrhi = ptr;
      ptrlo = ptr + 1;
      printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
      print(*(LispPTR *)ptr);
      ptr += 2;
      putchar('\n');
      BT_morep;
    }
    printf("this frame is last !!\n");
    BT_morep;
    return (-1);
  }

  next68k = NativeAligned2FromStackOffset(fx_addr68k->nextblock);
  ptr = (DLword *)(fx_addr68k + 1);

  i = 0;
  while (npvar-- > 0) {
    ptrhi = ptr;
    ptrlo = ptr + 1;
    printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
    if ((pvarindex = get_pvar_name(fx_addr68k, i++)) == LOCAL_PVAR)
      printf("*local* [pvar%d] ", (i - 1));
    else
      print_atomname(pvarindex);
    if (GETWORD(ptr) == 0xFFFF) {
      printf("  [variable not bound]\n");
      BT_morep;
    } else {
      printf("  ");
      print(*(LispPTR *)ptr);
      putchar('\n');
      BT_morep;
    }
    ptr += 2;
  }
  i = max_npvar;

  while (nfvar && nfvar-- >= max_npvar) {
    ptrhi = ptr;
    ptrlo = ptr + 1;
    printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
    if (0xFFFF == GETWORD(ptrhi)) {
      printf("[not looked up]  ");
      print_atomname(get_fvar_name(fx_addr68k, i));
      putchar('\n');
      BT_morep;
    } else if ((0xFFFF & GETWORD(ptrlo)) == 1) {
      printf("[fvar  ");
      print_atomname(get_fvar_name(fx_addr68k, i));
      printf("  on stack]  ");
      print(*(LispPTR *)NativeAligned4FromLAddr(((int)(0x0F & GETWORD(ptrlo)) << 16) | GETWORD(ptrhi)));
      putchar('\n');
      BT_morep;
    } else {
      printf("[fvar  ");
      print_atomname(get_fvar_name(fx_addr68k, i));
      printf("  top value ]   ");
      print(*(LispPTR *)NativeAligned4FromLAddr(((int)(0x0F & GETWORD(ptrlo)) << 16) | GETWORD(ptrhi)));
      putchar('\n');
      BT_morep;
    }
    ptr += 2;
    i++;
  }

  while (next68k > ptr) {
    ptrhi = ptr;
    ptrlo = ptr + 1;
    printf(" %6x : 0x%4x  0x%4x  ", LAddrFromNative(ptr), GETWORD(ptrhi), GETWORD(ptrlo));
    putchar('\n');
    BT_morep;
    ptr += 2;
  }
  return (0);
}

/************************************************************************/
/*									*/
/*				b t					*/
/*									*/
/*	Print the names of the frames on the stack.  Equivalent to	*/
/*	the Lisp DEBUGGER's BT! command.				*/
/*									*/
/*		Changed		26 Aug 1987	NMitani			*/
/*		Changed   May 4, 1988	Take(for URaid)			*/
/*									*/
/************************************************************************/

void bt1(FX *startFX) {
  FX *fx;
  struct fnhead *fnobj;
  FX *get_nextFX(FX * fx);
  int fnum = 0;
  BT_lines = 0;
  fx = startFX;
  URaid_FXarray[fnum] = fx;
  printf("%3d : ", fnum++);
#ifdef BIGVM
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(fx->fnheader);
#else
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(((int)fx->hi2fnheader << 16) | fx->lofnheader);
#endif /* BIGVM */
  printf("   0x%x : ", LAddrFromNative(fx));
  print(fnobj->framename);
  putchar('\n');
  BT_morep;
  while ((fnobj->framename != ATOM_T) && (fx->alink != 11)) {
    if (fnum > URMAXFXNUM - 1) {
      /* Internal buf overflow,more than 100 stacks */
      printf("***There are more than 100 stack frames.\n");
      printf(
          "If you want to continue, Uraid will smash its internal table for FX pointer. Do you "
          "accept?(Y or N)\n");
      {
        int c;
        c = getchar();
        fflush(stdin);
        if ((c == 'Y') || (c == 'y')) {
          fnum = 0;
          memset(URaid_FXarray, 0, URMAXFXNUM * 4);
        } else
          goto bt1_exit;
      }
    }

    fx = get_nextFX(fx);
#ifdef BIGVM
    fnobj = (struct fnhead *)NativeAligned4FromLAddr(fx->fnheader);
#else
    fnobj = (struct fnhead *)NativeAligned4FromLAddr(((int)fx->hi2fnheader << 16) | fx->lofnheader);
#endif /* BIGVM */
    URaid_FXarray[fnum] = fx;
    printf("%3d : ", fnum++);
    printf("   0x%x : ", LAddrFromNative(fx));
    print(fnobj->framename);
    putchar('\n');
    BT_morep;
  }
  if (fnobj->framename != ATOM_T) {
    printf(">>root frame for contextsw<<\n");
    putchar('\n');
    BT_morep;
  }

bt1_exit:
  URaid_ArrMAXIndex = fnum - 1;
  URaid_currentFX = 0;
} /* end bt */

void bt(void) { bt1(CURRENTFX); }

/***************************************************************/
/*
        Func Name :	btvv

        Desc :		dumps the all stack frame name.

        Changed		4 Sep 1987	NMitani

*/
/***************************************************************/

/************************************************************************/
/*									*/
/*				b t v v					*/
/*									*/
/*	Print all stack frames, with variables.  Equivalent to the	*/
/*	Lisp DEBUGGER's BTV! command.					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void btvv(void) {
  struct frameex1 *fx_addr68k;
  LispPTR atomindex;
  FX *get_nextFX(FX * fx);

  fx_addr68k = CURRENTFX;

  atomindex = get_framename(fx_addr68k);

  while ((atomindex != ATOM_T) && (fx_addr68k->alink != 11)) {
    sf(fx_addr68k);
    fx_addr68k = get_nextFX(fx_addr68k);
    atomindex = get_framename(fx_addr68k);
  }

  sf(fx_addr68k);

  printf("\n BTV! end ********\n");

} /*end btvv*/

/************************************************************************/
/*									*/
/*				s f f					*/
/*									*/
/*	Given the address of a lisp stack frame-extension (FX)		*/
/*	as a LISP (not native!) address, print the stack frame.		*/
/*									*/
/*									*/
/*							NMitani		*/
/************************************************************************/

void sff(LispPTR laddr) { sf((FX *)NativeAligned4FromLAddr(laddr)); }

#ifdef BIGATOMS
/*****************************************************************/
/* Nametable debugging aid                                       */
/*                                                               */
/* nt(index): print function header and name table               */
/*            in defcell of Atom index 'index'                   */
/*****************************************************************/
#define FNOVERHEADWORDS (8)

void nt(LispPTR index)
/* atom index */
{
  struct fnhead *fnobj;
  DefCell *defcell68k;

  defcell68k = (DefCell *)GetDEFCELL68k(index);
  fnobj = (struct fnhead *)NativeAligned4FromLAddr(defcell68k->defpointer);

  /* check if it's the same index ??*/
  if (index != (fnobj->framename)) {
    printf("DEFCELL says it is ");
    print_atomname(index);
    printf("\n But Func OBJ says ");
    print_atomname(fnobj->framename);
    putchar('\n');
    return;
  }

  printf("***DUMP Func Header << ");
  printf("start at 0x%x lisp address(%p 68k)\n", LAddrFromNative(fnobj), (void *)fnobj);
  print(index);
  putchar('\n');

  ntheader(fnobj);
}

void ntheader(struct fnhead *fnobj) {
  LispPTR *localnt1;
  int localntsize;
  LispPTR fnobj_lisp;

  fnobj_lisp = LAddrFromNative((DLword *)fnobj);
  printf("0x%08x: 0x%08x  stkmin\n", fnobj_lisp, fnobj->stkmin);
  printf("0x%08x: 0x%08x  na\n", fnobj_lisp + 1, fnobj->na);
  printf("0x%08x: 0x%08x  pv\n", fnobj_lisp + 2, fnobj->pv);
  printf("0x%08x: 0x%08x  startpc\n", fnobj_lisp + 3, fnobj->startpc);
  printf("0x%08x: 0x%08x  argtype\n", fnobj_lisp + 4, fnobj->argtype);
  printf("0x%08x: 0x%08x  framename ", fnobj_lisp + 5, fnobj->framename);
  print(fnobj->framename);
  putchar('\n');
  printf("0x%08x: 0x%08x  ntsize\n", fnobj_lisp + 6, fnobj->ntsize);
  printf("0x%08x: 0x%08x  nlocals", fnobj_lisp + 7, fnobj->nlocals);
  printf("  0x%08x  fvaroffset\n", fnobj->fvaroffset);

  nt1((LispPTR *)((DLword *)fnobj + FNOVERHEADWORDS), fnobj->ntsize, "Name Table");
  localnt1 =
      (LispPTR *)((DLword *)fnobj + FNOVERHEADWORDS + (fnobj->ntsize ? (2 * fnobj->ntsize) : 4));
  localntsize = ((DLword *)fnobj + (fnobj->startpc >> 1) - (DLword *)localnt1) >> 1;
  nt1(localnt1, localntsize, "Local args");
}

void nts(struct frameex1 *fxp) {
  struct fnhead *nt;

#ifdef BIGVM
  if (fxp->validnametable)
    nt = (struct fnhead *)NativeAligned4FromLAddr(fxp->nametable);
  else
    nt = (struct fnhead *)NativeAligned4FromLAddr(fxp->fnheader);
#else
  if (fxp->validnametable)
    nt = (struct fnhead *)NativeAligned4FromLAddr(((fxp->hi2nametable) << 16 | fxp->lonametable));
  else
    nt = (struct fnhead *)NativeAligned4FromLAddr(((fxp->hi2fnheader) << 16 | fxp->lofnheader));
#endif /* BIGVM */
  ntheader(nt);
}

#define VARTYPE_FVAR (3)
#define VARTYPE_PVAR (2)
#define VARTYPE_IVAR (0)

#define VAROFFSET(X) ((X) & 0xFFFFFFF)

void nt1(LispPTR *start, int size, char *str) {
  LispPTR *endp, *entry2p;

  printf("\n---- %s ----\n", str);
  size = size >> 1; /* originally size is # of word. now size is # of int */
  endp = start + size;
  while (start < endp) {
    entry2p = start + size;
    printf("0x%06x: 0x%08x", LAddrFromNative((DLword *)start), *start);
    printf("  0x%06x: 0x%08x", LAddrFromNative((DLword *)entry2p), *entry2p);
    if (*start != 0) {
      if ((*entry2p >> 30) == VARTYPE_FVAR)
        printf(" FVAR");
      else if ((*entry2p >> 30) == VARTYPE_PVAR)
        printf(" PVAR");
      else if ((*entry2p >> 30) == VARTYPE_IVAR)
        printf(" IVAR");
      else
        printf(" ????");
      printf("%2d  ", VAROFFSET(*entry2p));
      print_atomname(*start);
    }
    putchar('\n');
    start++;
  }
}

#endif
