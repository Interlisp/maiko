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

#include <stdio.h>
#include "lispemul.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "lspglob.h"
#include "stack.h"
#include "cell.h"
#include "ifpage.h"
#include "gcdata.h"
#include "array.h"

#include "gcfinaldefs.h"
#include "commondefs.h"
#include "gccodedefs.h"
#include "gchtfinddefs.h"
#include "llstkdefs.h"

#ifdef NEVER
#define GetSegnuminColl(entry1) ((entry1 & 0x01fe) >> 1) /* segnum field */
#define GetLinkptr(entry) (entry & 0x0fffe)
#define GetCountinColl(entry1) ((entry1 & 0x0fc00) >> 10)
#define Oddp(num) (((num % 2) != 0) ? 1 : 0)
#define Evenp(num) (((num % 2) != 0) ? 0 : 1)
#define STK_HI 1 /* This value also */
#define WORDSPERCELL 2
#define PADDING 4
#define Boundp(frame_field) ((frame_field == 0) ? 1 : 0)
#endif /* NEVER */

#define min(a, b) ((a > b) ? b : a)
#define Trailer(ldatum, datum68) (ldatum + 2 * (datum68->arlen - ARRAYBLOCKTRAILERCELLS))
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
  };
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
  bptr = (struct buf *)Addr68k_from_LADDR(*System_Buffer_List_word);
  while (LADDR_from_68k(bptr) != NIL) {
    if (ptr == bptr->vmempage)
      return (LADDR_from_68k(bptr));
    else
      bptr = (struct buf *)Addr68k_from_LADDR(bptr->sysnext);
  };
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
  register struct buf *bptr;
  register LispPTR bufferptr = findptrsbuffer(ptr);

  if (bufferptr == NIL) return (NIL); /* Not in use, OK to reclaim it */

  bptr = (struct buf *)Addr68k_from_LADDR(bufferptr);
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
    bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
    btrailer = (struct arrayblock *)Addr68k_from_LADDR(Trailer(base, bbase));
    bfwd = (struct arrayblock *)Addr68k_from_LADDR(bbase->fwd);
    bbwd = (struct arrayblock *)Addr68k_from_LADDR(bbase->bkwd);
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
      /* Remaining tests only for free list. */
      return (NIL);
    else if ((bbwd->fwd != base) || (bfwd->bkwd != base)) {
      error("ARRAYBLOCK links fouled\n");
    } else {
      fbl = FreeBlockChainN(bbase->arlen);
      rover = tmprover = (LispPTR *)Addr68k_from_LADDR(fbl);
      /* GETBASEPTR */
      if ((*rover & POINTERMASK) == NIL) error("Free Block's bucket empty\n");
      do {
        if ((*rover & POINTERMASK) == base) return (NIL);
        checkarrayblock((*rover & POINTERMASK), T, NIL);
        rbase = (struct arrayblock *)Addr68k_from_LADDR(*rover & POINTERMASK);
      } while (((*rover = rbase->fwd) & POINTERMASK) != (*tmprover & POINTERMASK));
      return (NIL);
    };
  };
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
  bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
  if ((bbase->arlen >= MINARRAYBLOCKSIZE) && (bbase->fwd != NIL)) {
    fwd = bbase->fwd;
    fbbase = (struct arrayblock *)Addr68k_from_LADDR(fwd);
    bkwd = bbase->bkwd;
    bbbase = (struct arrayblock *)Addr68k_from_LADDR(bkwd);
    fbl = FreeBlockChainN(bbase->arlen);
    freeblock = (LispPTR *)Addr68k_from_LADDR(fbl);
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
    bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
    if (bbase->arlen < MINARRAYBLOCKSIZE)
      checkarrayblock(base, T, NIL);
    else {
      fbl = FreeBlockChainN(bbase->arlen);
      freeblock = (LispPTR *)Addr68k_from_LADDR(POINTERMASK & fbl);
      freeblocklsp = POINTERMASK & (*freeblock);
      if (freeblocklsp == NIL) {
        bbase->fwd = base;
        bbase->bkwd = base;
      } else {
        fbbase = (struct arrayblock *)Addr68k_from_LADDR(freeblocklsp);
        bbase->fwd = freeblocklsp;
        bbase->bkwd = fbbase->bkwd;
        tmpbase = (struct arrayblock *)Addr68k_from_LADDR(fbbase->bkwd);
        tmpbase->fwd = base;
        fbbase->bkwd = base;
      };
      *freeblock = base;
      checkarrayblock(base, T, T);
    };
  };
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
  bbase = (struct arrayblock *)Addr68k_from_LADDR(block);
  dbase = (struct abdum *)WORDPTR(bbase);
  dbase->abflags = FREEARRAYFLAGWORD;
  bbase->arlen = length;
  trailer = Trailer(block, bbase);
  bbase = (struct arrayblock *)Addr68k_from_LADDR(trailer);
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
  bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
  bnbase = (struct arrayblock *)Addr68k_from_LADDR(nbase);
  arlens = bbase->arlen;
  narlens = bnbase->arlen;
  secondbite = MAXARRAYBLOCKSIZE - arlens;
  if (narlens > secondbite) {
    arlens = MAXARRAYBLOCKSIZE;
    narlens = narlens - secondbite;
    minblocksize =
        ((*Hunk_word == ATOM_T) ? (ARRAYBLOCKOVERHEADCELLS + MAXCELLSPERHUNK) : MINARRAYBLOCKSIZE);
    if (narlens < minblocksize) {
      shaveback = narlens - minblocksize;
      narlens = minblocksize;
      arlens += shaveback;
      secondbite += shaveback;
    };
    linkblock(makefreearrayblock(nbase + 2 * secondbite, narlens));
    narlens = 0;
  };
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

  ptrailer = (struct arrayblock *)Addr68k_from_LADDR(base - ARRAYBLOCKTRAILERWORDS);
  if (base == NIL)
    return (NIL);
  else if ((*ArrayMerging_word == NIL) ||
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
  bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
  nbase = base + 2 * (bbase->arlen);
  bnbase = (struct arrayblock *)Addr68k_from_LADDR(nbase);
  if ((*ArrayMerging_word == NIL) ||
      ((base == NIL) ||
       (checkarrayblock(base, T, T) ||
        ((nbase == *ArrayFrLst_word) ||
         ((nbase == *ArrayFrLst2_word) ||
          (checkarrayblock(nbase, (!(nbinuse = bnbase->inuse)), NIL) || nbinuse))))))
    return (NIL);
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

  base = (struct arrayblock *)Addr68k_from_LADDR(ptr - ARRAYBLOCKHEADERWORDS);
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
  };
#endif /* ARRAYCHECK */

  switch (base->gctype) {
    case PTRBLOCK_GCT: {
      btrailer = (ptr - 2) + 2 * (base->arlen - ARRAYBLOCKTRAILERCELLS);
      tmpptr = ptr;
      do {
        tmpp = (LispPTR *)Addr68k_from_LADDR(tmpptr);
        /* GCLOOKUP(0x8000,DELREF, *tmpp);*/ /* added 8-Oct-87 TT */
        REC_GCLOOKUP(*tmpp, DELREF);
        *tmpp = NIL;
        tmpptr += 2;
      } while (tmpptr != btrailer);
      break;
    };
    case CODEBLOCK_GCT:
      reclaim_p = ((reclaimcodeblock(ptr) == NIL) ? T : NIL);

      /* default:   No Action */
  };
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
  register STACKP *stkp;
  register FX *fxp;
  stkp = (STACKP *)Addr68k_from_LADDR(ptr);
  fxp = (FX *)Addr68k_from_StkOffset(stkp->edfxp);
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
  struct arrayblock *bfwd, *bbwd, *rbase;
  LispPTR *addr, *tmprover;

  LispPTR pbase, nbase;

  bbase = (struct arrayblock *)Addr68k_from_LADDR(base);
  btrailer = (struct arrayblock *)Addr68k_from_LADDR(Trailer(base, bbase));
  ptrailer = (struct arrayblock *)Addr68k_from_LADDR(base - ARRAYBLOCKTRAILERWORDS);
  bfwd = (struct arrayblock *)Addr68k_from_LADDR(bbase->fwd);
  bbwd = (struct arrayblock *)Addr68k_from_LADDR(bbase->bkwd);

  nbase = base + 2 * bbase->arlen;
  pbase = base - 2 * ptrailer->arlen;

  printf("This array block: 0x%x.  Previous: 0x%x.  Next: 0x%x.\n", base, pbase, nbase);
  printf("          Length: %d cells.\n\n", bbase->arlen);

  addr = ((LispPTR *)bbase) - 20;
  for (; addr < (LispPTR *)bbase; addr++) printf("%p	%8x\n", addr, *addr);
  printf("%p	%8x <- array header\n", addr, *addr);
  addr++;
  for (; addr < (LispPTR *)bbase + 20; addr++) printf("%p	%8x\n", addr, *addr);
  printf(". . .\n");

  addr = ((LispPTR *)btrailer) - 20;
  for (; addr < (LispPTR *)btrailer; addr++) printf("%p	%8x\n", addr, *addr);
  printf("%p	%8x <- array trailer\n", addr, *addr);
  addr++;
  for (; addr < (LispPTR *)btrailer + 20; addr++) printf("%p	%8x\n", addr, *addr);
}
