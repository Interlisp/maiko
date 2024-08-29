/* $Id: fvar.c,v 1.3 1999/05/31 23:35:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"        // for NativeAligned4FromLAddr, NativeAligned2FromLAddr, LAddrFromNative
#include "byteswapdefs.h"  // for swapx
#include "commondefs.h"    // for error
#include "emlglob.h"
#include "fvardefs.h"      // for N_OP_fvar_, N_OP_fvarn, N_OP_stkscan, nati...
#include "gcdata.h"        // for GCLOOKUP, ADDREF, DELREF
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "lispmap.h"       // for ATOMS_HI, STK_HI
#include "lspglob.h"       // for Stackspace
#include "lsptypes.h"      // for DLword, LispPTR, GETBASEWORD, state, FRAME...
#include "stack.h"         // for fnhead, frameex1, GETNAMETABLE, frameex2

#define MaskShift(x) (((x) << 16) & SEGMASK)

#ifdef BIGATOMS
#define NT_OFFSET_MASK 0xFFFFFFF
#define NT_TYPE_MASK 0xFF000000
#define GetNTEntry(X) GetLongWord(X)
#define NAMETABLE LispPTR
#else
#define NT_OFFSET_MASK 0xFF
#define NT_TYPE_MASK 0xFF00
#define GetNTEntry(X) GETWORD(X)
#define NAMETABLE DLword
#endif

#define FVSTACK 2
#define FVGLOBAL 6

#ifdef BIGATOMS
#define FVIVARHI 0x0
#define FVPVARHI (0x80000000)
#define FVFVARHI (0xC0000000)
#else
#define FVIVARHI 0x0
#define FVPVARHI 0x8000
#define FVFVARHI 0xC000
#endif

#define ENDSTACKMARK 0xb

/******************************************************************************
nnewframe

        This routine is used by fvlookup and OP_stkscan.

        1. scan Name table in new function header.
        2. if found, set address of searching variable to chain address.
        3. return the type of var, on stack, or global. (fs: unused, removed)

******************************************************************************/

void nnewframe(struct frameex1 *newpfra2, DLword *achain, int name)
/* pointer to new frame extension */
/* pointer to 1st word of the searching
   FVAR slot in CurrentFrameExtension */
/* Atom index num. of target FVAR slot. */
{
  NAMETABLE *pindex; /* '90/06/06 osamu changed from DLword *
                               * index to indexs of new name table */
  UNSIGNED i;        /* temp for control */
  int nametablesize; /* NameTable size of current function header. */
  int ph;            /* alink temp, also phase */

newframe:
  /* assume that apframe1 points to the next frame to be scanned */
  ph = newpfra2->alink;
  if (ph == 0) error("alink = 0 in nnewframe");
  if (ph == ENDSTACKMARK) {
/* endStack */
#ifdef BIGATOMS
    if ((name & SEGMASK) != 0) /* New symbol */
    {
      int result = name + NEWATOM_VALUE_OFFSET;
      /*    printf("NEW-SYMBOL in nnewframe, 0x%x, result = 0x%x\n", name, result); */
      GETBASEWORD(achain, 1) = result >> 16;
      GETBASEWORD(achain, 0) = result & 0xFFFF;
    } else
#endif /* BIGATOMS */

#ifdef BIGVM
        if (name & SEGMASK) { /* It's a big-atom, so just offset from the base to get value */
      int result = name + NEWATOM_VALUE_OFFSET;
      GETBASEWORD(achain, 1) = result >> 16;
      GETBASEWORD(achain, 0) = result & 0xFFFF;
    } else { /* It's an "old" atom, so offset into the table of atoms */
      int result = (ATOMS_HI << 16) + (10 * name) + NEWATOM_VALUE_OFFSET;
      GETBASEWORD(achain, 1) = result >> 16;
      GETBASEWORD(achain, 0) = result & 0xFFFF;
    }
#else
    if (name >= 0x8000) {
      GETBASEWORD(achain, 1) = VALS_HI + 1;
      GETBASEWORD(achain, 0) = name * 2;
    } else {
      GETBASEWORD(achain, 1) = VALS_HI;
      GETBASEWORD(achain, 0) = name * 2;
    }
#endif /* BIGVM */
    return;
  }

  ph &= 0xFFFE; /* to mask off SLOW bit */
  newpfra2 = (struct frameex1 *)(-FRAMESIZE + Stackspace + ph);

  {                                  /* open new block to try and conserve address */
    struct fnhead *newpfn2; /* ptr to new fn header */

    if (newpfra2->validnametable) /* check VALIDNAMETABLE */
#ifdef BIGVM
      newpfn2 = (struct fnhead *)NativeAligned4FromLAddr(newpfra2->nametable);
    else
      newpfn2 = (struct fnhead *)NativeAligned4FromLAddr(newpfra2->fnheader);
#else
      newpfn2 = (struct fnhead *)NativeAligned4FromLAddr(
          ((newpfra2->hi2nametable) << 16 | newpfra2->lonametable));
    else
      newpfn2 = (struct fnhead *)NativeAligned4FromLAddr(
          ((newpfra2->hi2fnheader) << 16 | newpfra2->lofnheader));
#endif /* BIGVM */
    pindex = (NAMETABLE *)((DLword *)newpfn2 + FNHEADSIZE);
/* now pindex points 1st word of Nametable. */
#ifdef BIGATOMS
    nametablesize = (newpfn2->ntsize >> 1); /* ntsize is # of words in NT
                                               not # of element */
#else
    nametablesize = newpfn2->ntsize;
#endif
  }

  i = (UNSIGNED)(pindex + nametablesize);
  for (; (UNSIGNED)i > (UNSIGNED)pindex;) { /* searching in NewFuncHeader */
#ifdef BIGATOMS
/* These used to be GETWORDs, but NAMETABLE is a CELL in 3-byte */
#if 1
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 1;
      goto foundit;
    }
  cont2:
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 0;
      goto foundit;
    }
#else
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 1;
      goto foundit;
    }
  cont2:
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 2;
      goto foundit;
    }
  cont3:
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 3;
      goto foundit;
    }
  cont4:
    if ((int)*((NAMETABLE *)pindex++) == (int)name) {
      ph = 0;
      goto foundit;
    }
#endif

#else
    if (GETWORD((NAMETABLE *)pindex++) == (DLword)name) {
      ph = 1;
      goto foundit;
    }
  cont2:
    if (GETWORD((NAMETABLE *)pindex++) == (DLword)name) {
      ph = 2;
      goto foundit;
    }
  cont3:
    if (GETWORD((NAMETABLE *)pindex++) == (DLword)name) {
      ph = 3;
      goto foundit;
    }
  cont4:
    if (GETWORD((NAMETABLE *)pindex++) == (DLword)name) {
      ph = 0;
      goto foundit;
    }
#endif
    continue;
  foundit : {
    int fvartype;   /* probing fvar vartype */
    int fvaroffset; /* probing fvar varoffset */
    DLword *ppvar;  /* ptr to probing var candidate */
#ifdef BIGATOMS
    fvartype = (int)*(pindex + nametablesize - 1);
#else
    fvartype = GETWORD(pindex + nametablesize - 1);
#endif /* BIGATOMS */

    fvaroffset = NT_OFFSET_MASK & fvartype;
    fvaroffset <<= 1;
    fvartype &= NT_TYPE_MASK;
    switch (fvartype) {
      case FVPVARHI: /* 0x8000 or 0x80000000(BIGATOMS) */
        ppvar = FRAMESIZE + (DLword *)newpfra2 + fvaroffset;
        /* ppvar points to argued Pvar */
        if (WBITSPTR(ppvar)->xMSB) /* check UNBOUND (if *ppvar is negative , unbound) */
          switch (ph) {
            case 0: continue;
            case 1:
              goto cont2;
              /* case 2 : goto cont3;
              case 3 : goto cont4; */
          }
        GETBASEWORD(achain, 1) = STK_HI;
        GETBASEWORD(achain, 0) = 0xFFFF & LAddrFromNative(ppvar);
        /* save High word of PVAR slot address to FVAR slot */
        /* achain points to target FVAR slot */
        return;
      case FVFVARHI: /* 0xC000 or 0xC0000000(BIGATOMS) */
        ppvar = FRAMESIZE + (DLword *)newpfra2 + fvaroffset;
        if (WBITSPTR(ppvar)->LSB) goto endlookfor;
        /* Not Found in new FnHeader, scan next one. */
        *((int *)achain) = *((int *)ppvar);
        /* save address of FX to FVAR slot */
        /* achain points to target FVAR slot */
        return;
      case FVIVARHI: /* 0x000: */
        ppvar = -1 + (DLword *)newpfra2;
        /* ppvar points to IVAR field in Basic  frame */
        GETBASEWORD(achain, 1) = STK_HI;
        GETBASEWORD(achain, 0) = GETWORD(ppvar) + fvaroffset;
        return;
      default: error("Stack corrupted: bad value in name table");
    } /* end switch */
  }   /* end if */
  }   /* end for */

endlookfor:
  goto newframe; /* scan the next one */
}

/**************************************************************************
 nfvlookup:

        This routine is used by only OP_fvarn.
                ( in addition to N_OP_fvar_() )
        1. get Atom index number of target fvar slot.
        2. call fvlookfor.

****************************************************************************/
void nfvlookup(struct frameex1 *apframe1, DLword *achain,
               struct fnhead *apfnhead1)
/* pointer to current frame extension */
/* pointer to 1st word of the searching
   FVAR slot in CurrentFrameExtension */
/* pointer to current function header */
{
  DLword *pfh;  /* pointer to current function header */
  int paoffset; /* 2word offset in PVAR AREA */

  pfh = (DLword *)apfnhead1;
  paoffset = ((UNSIGNED)achain - (UNSIGNED)PVar) >> 2;
/* slot to looked for, 2word offset from PVar */
#ifdef BIGATOMS
  nnewframe(apframe1, achain, (*((LispPTR *)((DLword *)pfh + apfnhead1->fvaroffset) + paoffset -
                                 apfnhead1->nlocals)));
#else
  nnewframe(apframe1, achain,
            (GETWORD(pfh + (apfnhead1->fvaroffset + paoffset - apfnhead1->nlocals))));
#endif
}

/*************************************************************************
 N_OP_fvarn
        entry of OPCODE[120b-127b]: FVAR, FVARX

        1. save TopOfStack to evaluation stack.
        2. set address of searching FVAR slot to chain.
        3. call lookfor. (It sets some content to FVAR slot)
        4. get some address by calculation of content of FVAR slot.
        5. set the address to TopOfStack.
**************************************************************************/
LispPTR N_OP_fvarn(int n)
/* n is word offset */

{
  DLword *chain; /* keep FVAR slot2 in CurrentFrameExtension */

  chain = PVar + n;

  if (WBITSPTR(chain)->LSB) {
    /* check 15bit of FVAR slot1 in CurrentFrameExtension.
       0: bound
       1: unbound */
    nfvlookup(CURRENTFX, chain, FuncObj);
  }

  return (GetLongWord(
      NativeAligned4FromLAddr(POINTERMASK & (((GETBASEWORD(chain, 1)) << 16) | GETBASEWORD(chain, 0)))));
}

/******************************************************************************

N_OP_stkscan
        entry	STKSCAN		OPCODE[057]

        <<Enter>>
        TopOfStack:	Low word - Atom index number of variable to be scanned.
        <<Exit>>
        TopOfStack:	Address of found value.

        1. call fvlookup.
        2. Set *chain to TopOfStack.
        3. Increment Pc by 1.

******************************************************************************/

LispPTR N_OP_stkscan(LispPTR tos) {
  int scratch;
  scratch = tos;
  nnewframe(CURRENTFX, (DLword *)&scratch, POINTERMASK & scratch);
  return (swapx(scratch));
}

/**************************************************
N_OP_fvar_

        Entry:	FVAR_		opcode[0143]


***************************************************/

LispPTR N_OP_fvar_(LispPTR tos, int n) {
  DLword *ppvar;    /* pointer to argued Fvar slot in pvar area */
  DLword *pfreeval; /* pointer to argued free value */

  ppvar = PVar + n;

  if (WBITSPTR(ppvar)->LSB) /* check unbound ? */
  {                         /* unbound */
    nfvlookup(CURRENTFX, ppvar, FuncObj);
  }

  pfreeval = NativeAligned2FromLAddr(MaskShift((GETWORD(ppvar + 1))) | GETWORD(ppvar));

  if (((0xFF & GETWORD(ppvar + 1)) != STK_HI)) {
    GCLOOKUP(*((LispPTR *)pfreeval), DELREF);
    GCLOOKUP(tos, ADDREF);
  }

  *((LispPTR *)pfreeval) = tos;
  return (tos);
}

#ifndef BIGATOMS

#define VALS_HI_RET(x) ((int)(x) << 17) + VALS_HI + ((unsigned short)(x) >> 15)

#define STK_HI_RET(x) (((int)(x) << 16) | 1 | ((unsigned int)(x) >> 16))


#else

#ifdef BIGVM
#define VALS_HI_RET(x)                                                                            \
  ((((int)(x)&SEGMASK) == 0) ? (swapx((ATOMS_HI << 16) + (10 * (int)(x)) + NEWATOM_VALUE_OFFSET)) \
                             : (swapx((int)(x) + NEWATOM_VALUE_OFFSET)))
#else
#define VALS_HI_RET(x)                                                                    \
  ((((int)(x)&SEGMASK) == 0) ? (((int)(x) << 17) + VALS_HI + ((unsigned short)(x) >> 15)) \
                             : (swapx((int)(x) + NEWATOM_VALUE_OFFSET)))
#endif /* BIGVM */

#define STK_HI_RET(x) (((unsigned int)(x) << 16) | 1 | ((unsigned int)(x) >> 16))

#endif /* BIGATOMS */

/******************************************************************************
native_newframe

        1. scan Name table in new function header.
        2. if found, set address of searching variable to chain address.
        3. return the pointer

******************************************************************************/
LispPTR native_newframe(int slot)
/* index of FVAR slot. */
{
  struct frameex2 *newpfra2; /* pointer to new frame extension */
  DLword *achain;            /* pointer to 1st word of the searching
                                       FVAR slot in CurrentFrameExtension */
  int name;                  /* Atom# of target FVAR slot. */

  { /* LOCAL temp regs */
    int rslot = slot;
    struct fnhead *fnobj = FuncObj;
    LispPTR *pvar = (LispPTR *)PVar;

#ifdef BIGATOMS
    name = (int)*((LispPTR *)((DLword *)fnobj + fnobj->fvaroffset) + rslot - fnobj->nlocals);
#else
    name = GETWORD((DLword *)fnobj + (fnobj->fvaroffset + rslot - fnobj->nlocals));
#endif

    newpfra2 = (struct frameex2 *)((DLword *)pvar - FRAMESIZE);
    achain = (DLword *)(pvar + rslot);
  }

  {
    NAMETABLE *pindex; /* index to indexs of new name table */
    int i;             /* temp for control */
    int nametablesize; /* NameTable size of current fnhdr */
    int alink;

  natnewframe:
    /* assume that apframe1 points to the next frame to be scanned */

    alink = newpfra2->alink;
    if (alink == 0) error("alink is 0 in native_newframe");
    if (alink == ENDSTACKMARK) { /* End of stack, so return top-level-value */
      /*	    if (name & SEGMASK) printf("native_newframe returning name 0x%x, result
         0x%x.\n",
                              name, VALS_HI_RET(name)); */
      return (*((LispPTR *)achain) = VALS_HI_RET(name));
    }
    newpfra2 = (struct frameex2 *)(-FRAMESIZE + Stackspace + (alink & 0xFFFE));

    {                                  /* open new block to try and conserve address */
      struct fnhead *newpfn2; /* ptr to new fn header */

      newpfn2 = GETNAMETABLE(newpfra2);

      pindex = (NAMETABLE *)(((DLword *)newpfn2) + FNHEADSIZE);
/* now pindex points 1st word of Nametable. */
#ifdef BIGATOMS
      nametablesize = (newpfn2->ntsize >> 1); /* ntsize is # of words in NT
                                             nametablesize is # of items in NT */
#else
      nametablesize = newpfn2->ntsize;
#endif /* 					    */
    }

    for (i = nametablesize; --i >= 0;) {
/* searching in NewFuncHeader */
#ifdef BIGATOMS
      if ((LispPTR) * ((NAMETABLE *)pindex++) == (LispPTR)name) {
#else
      if (GETWORD((NAMETABLE *)pindex++) == (DLword)name) {
#endif /* BIGATOMS    */


        int fvartype;   /* probing fvar vartype */
        int fvaroffset; /* probing fvar varoffset */
        DLword *ppvar;  /* ptr to probing var candidate */
#ifdef BIGATOMS
        fvartype = (int)*(pindex + nametablesize - 1);
#else
        fvartype = GETWORD(pindex + nametablesize - 1);
#endif /* BIGATOMS */

        fvaroffset = (NT_OFFSET_MASK & fvartype) << 1;
        fvartype &= NT_TYPE_MASK;
        switch (fvartype) {
          case FVPVARHI: /* 0x8000 or 0x80000000(NEWATOM): */
            ppvar = FRAMESIZE + (DLword *)newpfra2 + fvaroffset;
            /* ppvar points to argued Pvar */
            if (WBITSPTR(ppvar)->xMSB) /* check UNBOUND (if *ppvar is negative , unbound) */
            {
              continue;
            }
            /* save High word of PVAR slot address to FVAR slot */
            /* achain points to target FVAR slot */
            return (*((LispPTR *)achain) = STK_HI_RET(LAddrFromNative(ppvar)));
          case FVFVARHI: /* 0xC000 or 0xC0000000(NEWATOM S) */
            ppvar = FRAMESIZE + (DLword *)newpfra2 + fvaroffset;
            if (WBITSPTR(ppvar)->LSB) { goto endlookfor; }
            /* Not Found in new FuncHeader, scan next one. */
            /* save address of frame extension to FVAR slot */
            /* achain points to target FVAR slot */
            return (*((int *)achain) = *((int *)ppvar));
          case FVIVARHI: /* 0x0000000: */
            ppvar = (DLword *)newpfra2 - 1;
            /* ppvar points to IVAR field in Basic frame */
            return (*((LispPTR *)achain) = STK_HI_RET(GETWORD(ppvar) + fvaroffset));
          default: error("Stack corrupted: bad value in name table");
        } /* end switch */
      }   /* end if */
    }     /* end for */

  endlookfor:
    goto natnewframe; /* scan the next one */
  }
}
