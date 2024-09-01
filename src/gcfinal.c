/* $Id: gcfinal.c,v 1.3 1999/05/31 23:35:31 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*************************************************************************/
/*************************************************************************/
/*                                                                       */
/*                       File Name : gcfinal.c                      */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                                                                       */
/*           Functions :                                                 */
/*                       reclaimstackp();                                */
/*                       reclaimarrayblock(ptr);                         */
/*                       reclaimcodeblock() is in another file           */
/*                       releasingvmempage();                            */
/*			 deleteblock();					 */
/*			 linkblock();					 */
/*			 mergeforward();				 */
/*			 mergebackward();				 */
/*			 arrayblockmerger();				 */
/*			 checkarrayblock();				 */
/*			 findptrsbuffer();				 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/*  									 */
/*                                                                       */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include <stdio.h>        // for printf
#include "address.h"      // for HILOC
#include "adr68k.h"       // for NativeAligned4FromLAddr, LAddrFromNative
#include "array.h"        // for arrayblock, ARRAYBLOCKTRAILERCELLS, MAXBUCK...
#include "commondefs.h"   // for error
#include "gccodedefs.h"   // for reclaimcodeblock
#include "gcdata.h"       // for DELREF, REC_GCLOOKUP
#include "gchtfinddefs.h" // for htfind, rec_htfind
#include "gcfinaldefs.h"  // for arrayblockmerger, checkarrayblock, deleteblock
#include "lispemul.h"     // for LispPTR, NIL, T, POINTERMASK, DLword, ATOM_T
#include "llstkdefs.h"    // for decusecount68k
#include "lspglob.h"      // for FreeBlockBuckets_word, ArrayMerging_word
#include "lsptypes.h"     // for WORDPTR
#include "stack.h"        // for STACKP, FX

#ifdef NEVER
#define GetSegnuminColl(entry1) ((entry1 & 0x01fe) >> 1) /* segnum field */
#define GetLinkptr(entry) (entry & 0x0fffe)
#define GetCountinColl(entry1) ((entry1 & 0x0fc00) >> 10)
#define Oddp(num) (((num % 2) != 0) ? 1 : 0)
#define Evenp(num) (((num % 2) != 0) ? 0 : 1)
#define STK_HI 1 /* This value also */
#define PADDING 4
#define Boundp(frame_field) ((frame_field == 0) ? 1 : 0)
#endif /* NEVER */

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define Trailer(ldatum, datum68) ((ldatum) + 2 * ((datum68)->arlen - ARRAYBLOCKTRAILERCELLS))
#define BucketIndex(n) min(integerlength(n), MAXBUCKETINDEX)
#define FreeBlockChainN(n) ((POINTERMASK & *FreeBlockBuckets_word) + 2 * BucketIndex(n))

#ifndef BYTESWAP
#ifdef BIGVM
struct buf {
  LispPTR filepage;
  LispPTR vmempage;
  LispPTR buffernext;
  unsigned noreference : 1;
  unsigned usermapped : 1;
  unsigned iodirty : 1;
  unsigned unused : 1;
  unsigned sysnext : 28;
};
#else
struct buf {
  LispPTR filepage;
  LispPTR vmempage;
  LispPTR buffernext;
  unsigned noreference : 1;
  unsigned usermapped : 1;
  unsigned iodirty : 1;
  unsigned unused : 5;
  unsigned sysnext : 24;
};
#endif /* BIGVM */
#else
#ifdef BIGVM
struct buf {
  LispPTR filepage;
  LispPTR vmempage;
  LispPTR buffernext;
  unsigned sysnext : 28;
  unsigned unused : 1;
  unsigned iodirty : 1;
  unsigned usermapped : 1;
  unsigned noreference : 1;
};
#else
struct buf {
  LispPTR filepage;
  LispPTR vmempage;
  LispPTR buffernext;
  unsigned sysnext : 24;
  unsigned unused : 5;
  unsigned iodirty : 1;
  unsigned usermapped : 1;
  unsigned noreference : 1;
};
#endif /* BIGVM */
#endif /* BYTESWAP */

/************* The following procedure is common !! **************************/

int integerlength(unsigned int n) {
  int cnt;
  if (n <= 2)
    return (n);
  else {
    cnt = 1;
    do {
      cnt++;
      n = (n >> 1);
    } while (n != 1);
    return (cnt);
  }
}

/************* The above procedure is common !! **************************/

/************************************************************************/
/*									*/
/*			f i n d p t r s b u f f e r			*/
/*									*/
/*	Given a pointer to a VMEMPAGEP, see if it is pointed to by	*/
/*	any BUFFER.  If so, return the BUFFER's pointer. Otherwise,	*/
/*	return NIL.							*/
/*									*/
/************************************************************************/

LispPTR findptrsbuffer(LispPTR ptr) {
  struct buf *bptr;
  bptr = (struct buf *)NativeAligned4FromLAddr(*System_Buffer_List_word);
  while (LAddrFromNative(bptr) != NIL) {
    if (ptr == bptr->vmempage)
      return (LAddrFromNative(bptr));
    else
      bptr = (struct buf *)NativeAligned4FromLAddr(bptr->sysnext);
  }
  return (NIL);
}

/************************************************************************/
/*									*/
/*		    r e l e a s i n g v m e m p a g e			*/
/*									*/
/*	Called when ptr, a VMEMPAGEP, is about to be reclaimed by the	*/
/*	GC.  Returns T if it is NOT OK TO RECLAIM THE VMEMPAGEP.	*/
/*	Otherwise, returns NIL.  It won't be OK to reclaim when		*/
/*	the VMEMPAGEP is being used as a buffer.			*/
/*									*/
/************************************************************************/

LispPTR releasingvmempage(LispPTR ptr) {
  struct buf *bptr;
  LispPTR bufferptr = findptrsbuffer(ptr);

  if (bufferptr == NIL) return (NIL); /* Not in use, OK to reclaim it */

  bptr = (struct buf *)NativeAligned4FromLAddr(bufferptr);
  bptr->noreference = T; /* Mark the buffer free to use ?? */
  return (ATOM_T);
}

/************************************************************************/
/*									*/
/*		      c h e c k a r r a y b l o c k			*/
/*									*/
/*	Given an array block, do consistency checks on it.		*/
/*									*/
/************************************************************************/

LispPTR checkarrayblock(LispPTR base, LispPTR free, LispPTR onfreelist) {
  struct arrayblock *bbase, *btrailer;
  struct arrayblock *bfwd, *bbwd, *rbase;
  LispPTR fbl;
  LispPTR *rover, *tmprover;
#ifdef ARRAYCHECK
  if (T)
#else
  if (*Array_Block_Checking_word != NIL)
#endif
  {
    bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
    btrailer = (struct arrayblock *)NativeAligned4FromLAddr(Trailer(base, bbase));
    if (bbase->password != ARRAYBLOCKPASSWORD) {
      printarrayblock(base);
      error("ARRAYBLOCK password wrong\n");
    } else if (bbase->inuse == free) {
      printarrayblock(base);
      error("ARRAYBLOCK INUSE bit set wrong\n");
    } else if (btrailer->password != ARRAYBLOCKPASSWORD) {
      printarrayblock(base);
      error("ARRAYBLOCK trailer password wrong\n");
    } else if (bbase->arlen != btrailer->arlen) {
      printarrayblock(base);
      error("ARRAYBLOCK Header and Trailer length don't match\n");
    } else if (btrailer->inuse == free)
    /* This is not original source.(in original,
       btrailer -> bbase) maybe, this is correction. */
    {
      printarrayblock(base);
      error("ARRAYBLOCK Trailer INUSE bit set wrong\n");
    } else if (!onfreelist || (bbase->arlen < MINARRAYBLOCKSIZE))
      return (NIL);
    /* Remaining tests only for free list. */
    bfwd = (struct arrayblock *)NativeAligned4FromLAddr(bbase->fwd);
    bbwd = (struct arrayblock *)NativeAligned4FromLAddr(bbase->bkwd);
    if ((bbwd->fwd != base) || (bfwd->bkwd != base)) {
      error("ARRAYBLOCK links fouled\n");
    } else {
      fbl = FreeBlockChainN(bbase->arlen);
      rover = tmprover = (LispPTR *)NativeAligned4FromLAddr(fbl);
      /* GETBASEPTR */
      if ((*rover & POINTERMASK) == NIL) error("Free Block's bucket empty\n");
      do {
        if ((*rover & POINTERMASK) == base) return (NIL);
        checkarrayblock((*rover & POINTERMASK), T, NIL);
        rbase = (struct arrayblock *)NativeAligned4FromLAddr(*rover & POINTERMASK);
      } while (((*rover = rbase->fwd) & POINTERMASK) != (*tmprover & POINTERMASK));
      return (NIL);
    }
  }
  return (NIL);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR deleteblock(LispPTR base) {
  struct arrayblock *bbase, *fbbase, *bbbase;
  LispPTR fwd, bkwd, fbl, freeblocklsp;
  LispPTR *freeblock;
  bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
  if ((bbase->arlen >= MINARRAYBLOCKSIZE) && (bbase->fwd != NIL)) {
    fwd = bbase->fwd;
    fbbase = (struct arrayblock *)NativeAligned4FromLAddr(fwd);
    bkwd = bbase->bkwd;
    bbbase = (struct arrayblock *)NativeAligned4FromLAddr(bkwd);
    fbl = FreeBlockChainN(bbase->arlen);
    freeblock = (LispPTR *)NativeAligned4FromLAddr(fbl);
    freeblocklsp = POINTERMASK & *freeblock;
    if (base == fwd) {
      if (base == freeblocklsp)
        *freeblock = NIL;
      else
        error("GC error:deleting last list # FREEBLOCKLIST\n");
      return (NIL);
    } else if (base == freeblocklsp)
      *freeblock = fwd;
    fbbase->bkwd = bkwd;
    bbbase->fwd = fwd;
  }
  return (NIL);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR linkblock(LispPTR base) {
  struct arrayblock *bbase, *fbbase, *tmpbase;
  LispPTR fbl, freeblocklsp;
  LispPTR *freeblock;
  if (*FreeBlockBuckets_word != NIL) {
    bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
    if (bbase->arlen < MINARRAYBLOCKSIZE)
      checkarrayblock(base, T, NIL);
    else {
      fbl = FreeBlockChainN(bbase->arlen);
      freeblock = (LispPTR *)NativeAligned4FromLAddr(POINTERMASK & fbl);
      freeblocklsp = POINTERMASK & (*freeblock);
      if (freeblocklsp == NIL) {
        bbase->fwd = base;
        bbase->bkwd = base;
      } else {
        fbbase = (struct arrayblock *)NativeAligned4FromLAddr(freeblocklsp);
        bbase->fwd = freeblocklsp;
        bbase->bkwd = fbbase->bkwd;
        tmpbase = (struct arrayblock *)NativeAligned4FromLAddr(fbbase->bkwd);
        tmpbase->fwd = base;
        fbbase->bkwd = base;
      }
      *freeblock = base;
      checkarrayblock(base, T, T);
    }
  }
  return (base);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR makefreearrayblock(LispPTR block, DLword length) {
  LispPTR trailer;
  struct arrayblock *bbase;
  struct abdum *dbase;
  bbase = (struct arrayblock *)NativeAligned4FromLAddr(block);
  dbase = (struct abdum *)WORDPTR(bbase);
  dbase->abflags = FREEARRAYFLAGWORD;
  bbase->arlen = length;
  trailer = Trailer(block, bbase);
  bbase = (struct arrayblock *)NativeAligned4FromLAddr(trailer);
  dbase = (struct abdum *)WORDPTR(bbase);
  dbase->abflags = FREEARRAYFLAGWORD;
  bbase->arlen = length;
  return (block);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
LispPTR arrayblockmerger(LispPTR base, LispPTR nbase) {
  DLword arlens, narlens, secondbite, minblocksize, shaveback;
  struct arrayblock *bbase, *bnbase;
  bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
  bnbase = (struct arrayblock *)NativeAligned4FromLAddr(nbase);
  arlens = bbase->arlen;
  narlens = bnbase->arlen;
  secondbite = MAXARRAYBLOCKSIZE - arlens;
  /* There are three cases for merging the blocks
   * (1) the total size of the two blocks is less than max:
   *     merge into a single block
   * (2) creating a max size block leaves a viable leftover block:
   *     move the boundary to make a max block and a leftover block
   * (3) creating a max size block leaves a non-viable leftover block
   *     move the boundary to make a big block and a minimum size leftover block
   */
  if (base + (2 * arlens) != nbase) {
    error("Attempt to merge non-adjacent blocks in array space\n");
  }
  if (narlens > secondbite) { /* (2) or (3) */
    arlens = MAXARRAYBLOCKSIZE;
    narlens = narlens - secondbite;
    minblocksize =
        ((*Hunk_word == ATOM_T) ? (ARRAYBLOCKOVERHEADCELLS + MAXCELLSPERHUNK) : MINARRAYBLOCKSIZE);
    if (narlens < minblocksize) { /* (3) */
      shaveback = narlens - minblocksize;
      narlens = minblocksize;
      arlens += shaveback;
      secondbite += shaveback;
    }
    linkblock(makefreearrayblock(nbase + 2 * secondbite, narlens));
    narlens = 0;
  }
  return (linkblock(makefreearrayblock(base, arlens + narlens)));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR mergebackward(LispPTR base) {
  LispPTR pbase;
  struct arrayblock *ptrailer;

  if (base == NIL)
    return (NIL);
  ptrailer = (struct arrayblock *)NativeAligned4FromLAddr(base - ARRAYBLOCKTRAILERWORDS);
  if ((*ArrayMerging_word == NIL) ||
           ((base == *ArraySpace_word) || ((base == *ArraySpace2_word) || (ptrailer->inuse == T))))
    return (linkblock(base));
  pbase = base - 2 * ptrailer->arlen;
  checkarrayblock(pbase, T, NIL);
  deleteblock(pbase);
  return (arrayblockmerger(pbase, base));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR mergeforward(LispPTR base) {
  LispPTR nbase, nbinuse;
  struct arrayblock *bbase, *bnbase;
  if (*ArrayMerging_word == NIL) return NIL;
  if (base == NIL) return NIL;
  if (checkarrayblock(base, T, T)) return NIL;

  bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
  nbase = base + 2 * (bbase->arlen);
  if (nbase == *ArrayFrLst_word || nbase == *ArrayFrLst2_word) return NIL;

  bnbase = (struct arrayblock *)NativeAligned4FromLAddr(nbase);
  nbinuse = bnbase->inuse;
  if (checkarrayblock(nbase, !nbinuse, NIL)) return NIL;
  if (nbinuse) return (NIL);
  deleteblock(nbase);
  deleteblock(base);
  return (arrayblockmerger(base, nbase));
}

/************************************************************************/
/*									*/
/*		    r e c l a i m a r r a y b l o c k			*/
/*									*/
/*	Reclaim a block of storage in the array-space heap.		*/
/*									*/
/************************************************************************/

LispPTR reclaimarrayblock(LispPTR ptr) {
  LispPTR tmpptr, btrailer;
  struct arrayblock *base;
  LispPTR *tmpp;
  int reclaim_p;

  reclaim_p = T;
#ifdef ARRAYCHECK
  checkarrayblock(ptr - ARRAYBLOCKHEADERWORDS, NIL, NIL);
#endif /* ARRAYCHECK */

  base = (struct arrayblock *)NativeAligned4FromLAddr(ptr - ARRAYBLOCKHEADERWORDS);
#ifdef ARRAYCHECK
  if (HILOC(ptr) < FIRSTARRAYSEGMENT) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error(
        "Bad array block reclaimed [not in array space].\nContinue with 'q' but save state ASAP. "
        "\n");
    return (T);
  } else if (ARRAYBLOCKPASSWORD != base->password) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error("Bad array block reclaimed [password wrong].\nContinue with 'q' but save state ASAP. \n");
    return (T);
  } else if (base->inuse == NIL) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error(
        "Bad array block reclaimed [block not in use].\nContinue with 'q' but save state ASAP. \n");
    return (T);
  }
#else
  /* Normal case, just tell the guy something's wrong: */
  if ((HILOC(ptr) < FIRSTARRAYSEGMENT) ||
      ((ARRAYBLOCKPASSWORD != base->password) || (base->inuse == NIL))) {
    error("Bad array block reclaimed--continue with 'q' but save state ASAP. \n");
    return (T);
  }
#endif /* ARRAYCHECK */

  switch (base->gctype) {
    case PTRBLOCK_GCT: {
      btrailer = (ptr - 2) + 2 * (base->arlen - ARRAYBLOCKTRAILERCELLS);
      tmpptr = ptr;
      do {
        tmpp = (LispPTR *)NativeAligned4FromLAddr(tmpptr);
        /* GCLOOKUP(0x8000,DELREF, *tmpp);*/ /* added 8-Oct-87 TT */
        REC_GCLOOKUP(*tmpp, DELREF);
        *tmpp = NIL;
        tmpptr += 2;
      } while (tmpptr != btrailer);
      break;
    }
    case CODEBLOCK_GCT:
      reclaim_p = ((reclaimcodeblock(ptr) == NIL) ? T : NIL);

      /* default:   No Action */
  }
  if (reclaim_p == T)
    mergeforward(mergebackward(makefreearrayblock(ptr - ARRAYBLOCKHEADERWORDS, base->arlen)));
  return (T);
}

/************************************************************************/
/*									*/
/*		      r e c l a i m s t a c k p				*/
/*									*/
/*	Reclaim a STACKP, which contains a stack pointer.		*/
/*									*/
/************************************************************************/

LispPTR reclaimstackp(LispPTR ptr) /* This is the entry function */
                                   /*  in stack reclaiming */
{
  STACKP *stkp;
  FX *fxp;
  stkp = (STACKP *)NativeAligned4FromLAddr(ptr);
  fxp = (FX *)NativeAligned4FromStackOffset(stkp->edfxp);
  decusecount68k(fxp); /* decrement the use-count for the frame it uses */
  return (NIL);        /* and let the normal reclaimer reclaim it */
}

/************************************************************************/
/*									*/
/*			p r i n t a r r a y b l o c k			*/
/*									*/
/*	Print an array block's contents, for debugging work.		*/
/*									*/
/************************************************************************/

void printarrayblock(LispPTR base) {
  struct arrayblock *bbase, *btrailer, *ptrailer;
  LispPTR *addr;

  LispPTR pbase, nbase;

  bbase = (struct arrayblock *)NativeAligned4FromLAddr(base);
  btrailer = (struct arrayblock *)NativeAligned4FromLAddr(Trailer(base, bbase));
  ptrailer = (struct arrayblock *)NativeAligned4FromLAddr(base - ARRAYBLOCKTRAILERWORDS);

  nbase = base + 2 * bbase->arlen;
  pbase = base - 2 * ptrailer->arlen;

  printf("This array block: 0x%x.  Previous: 0x%x.  Next: 0x%x.\n", base, pbase, nbase);
  printf("          Length: %d cells.\n\n", bbase->arlen);

  addr = ((LispPTR *)bbase) - 20;
  for (; addr < (LispPTR *)bbase; addr++) printf("%p	%8x\n", (void *)addr, *addr);
  printf("%p	%8x <- array header\n", (void *)addr, *addr);
  addr++;
  for (; addr < (LispPTR *)bbase + 20; addr++) printf("%p	%8x\n", (void *)addr, *addr);
  printf(". . .\n");

  addr = ((LispPTR *)btrailer) - 20;
  for (; addr < (LispPTR *)btrailer; addr++) printf("%p	%8x\n", (void *)addr, *addr);
  printf("%p	%8x <- array trailer\n", (void *)addr, *addr);
  addr++;
  for (; addr < (LispPTR *)btrailer + 20; addr++) printf("%p	%8x\n", (void *)addr, *addr);
}
