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
#include "gcfinaldefs.h"  // for checkarrayblock
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
#define Trailer(ldatum, datum68) ((ldatum) + DLWORDSPER_CELL * ((datum68)->arlen - ARRAYBLOCKTRAILERCELLS))
#define BucketIndex(n) min(integerlength(n), MAXBUCKETINDEX)
#define FreeBlockChainN(n) ((POINTERMASK & *FreeBlockBuckets_word) + 2 * BucketIndex(n))

/*
 * Declaration of buffer must be identical layout to Lisp BUFFER datatype in PMAP.
 */
#ifndef BYTESWAP
#ifdef BIGVM
struct buffer {
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
struct buffer {
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
struct buffer {
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
struct buffer {
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

static int integerlength(unsigned int n) {
  int p = 0;

  if (n <= 2) return (n); /* easy case */
  if (n >= 65536) {
    n >>= 16;
    p += 16;
  }
  if (n >= 256) {
    n >>= 8;
    p += 8;
  }
  if (n >= 16) {
    n >>= 4;
    p += 4;
  }
  if (n >= 4) {
    n >>= 2;
    p += 2;
  }
  if (n >= 2) {
    p += 1;
  }
  return (p + 1);
}

/************************************************************************/
/*									*/
/*			f i n d p t r s b u f f e r			*/
/*									*/
/*	Given a pointer to a VMEMPAGEP, see if it is pointed to by	*/
/*	any BUFFER.  If so, return the BUFFER's pointer. Otherwise,	*/
/*	return NIL.							*/
/*									*/
/************************************************************************/

static LispPTR findptrsbuffer(LispPTR ptr) {
  LispPTR buf;
  struct buffer *buf_np;
  buf = *System_Buffer_List_word;
  while (buf != NIL) {
    buf_np = (struct buffer *)NativeAligned4FromLAddr(buf);
    if (ptr == buf_np->vmempage) {
      return (buf);
    }
    buf = buf_np->sysnext;
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
  LispPTR buffer = findptrsbuffer(ptr);
  struct buffer *buffer_np;

  if (buffer == NIL) return (NIL); /* Not in use, OK to reclaim it */

  buffer_np = (struct buffer *)NativeAligned4FromLAddr(buffer);
  buffer_np->noreference = T; /* Mark the buffer free to use ?? */
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
  struct arrayblock *base_np, *trailer_np;
  struct arrayblock *fwd_np, *bkwd_np, *rbase;
  LispPTR fbl;
  LispPTR *rover, *tmprover;
#ifdef ARRAYCHECK
  if (T)
#else
  if (*Array_Block_Checking_word != NIL)
#endif
  {
    base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
    trailer_np = (struct arrayblock *)NativeAligned4FromLAddr(Trailer(base, base_np));
#if 0
    printf("cblock: 0x%x free: %x onfreelist: %x pw: %x arlen %d\n",
           base, free, onfreelist, base_np->password, base_np->arlen);
#endif
    if (base_np->password != ARRAYBLOCKPASSWORD) {
      printarrayblock(base);
      error("ARRAYBLOCK password wrong\n");
      return(T);
    } else if (base_np->inuse == free) {
      printarrayblock(base);
      error("ARRAYBLOCK INUSE bit set wrong\n");
      return(T);
    } else if (trailer_np->password != ARRAYBLOCKPASSWORD) {
      printarrayblock(base);
      error("ARRAYBLOCK trailer password wrong\n");
      return(T);
    } else if (base_np->arlen != trailer_np->arlen) {
      printarrayblock(base);
      error("ARRAYBLOCK Header and Trailer length don't match\n");
      return(T);
    } else if (trailer_np->inuse == free)
    /* This is not original source.(in original,
       trailer_np -> base_np) maybe, this is correction. */
    {
      printarrayblock(base);
      error("ARRAYBLOCK Trailer INUSE bit set wrong\n");
      return(T);
    } else if (!onfreelist || (base_np->arlen < MINARRAYBLOCKSIZE))
      return (NIL);
    /* Remaining tests only for free list. */
    fwd_np = (struct arrayblock *)NativeAligned4FromLAddr(base_np->fwd);
    bkwd_np = (struct arrayblock *)NativeAligned4FromLAddr(base_np->bkwd);
    if ((bkwd_np->fwd != base) || (fwd_np->bkwd != base)) {
      error("ARRAYBLOCK links fouled\n");
      return(T);
    } else {
      fbl = FreeBlockChainN(base_np->arlen);
      rover = tmprover = (LispPTR *)NativeAligned4FromLAddr(fbl);
      /* GETBASEPTR */
      if ((*rover & POINTERMASK) == NIL) {
        error("Free Block's bucket empty\n");
        return(T);
      }
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
/*
 * Removes "base", a block from the free list and
 * adjusts the forward and backward pointers of the blocks behind and
 * ahead of the deleted block.
 * The forward and backward pointers of this deleted block are left
 * dangling - as in the Lisp implementation. Also does not affect the
 * inuse bit in header and trailer.
 */
static void deleteblock(LispPTR base) {
  struct arrayblock *base_np, *f_np, *b_np;
  LispPTR f, b, fbl, freeblock;
  LispPTR *fbl_np;
  base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
  if ((base_np->arlen >= MINARRAYBLOCKSIZE) && (base_np->fwd != NIL)) {
    f = base_np->fwd;
    f_np = (struct arrayblock *)NativeAligned4FromLAddr(f);
    b = base_np->bkwd;
    b_np = (struct arrayblock *)NativeAligned4FromLAddr(b);
    fbl = FreeBlockChainN(base_np->arlen);
    fbl_np = (LispPTR *)NativeAligned4FromLAddr(fbl);
    freeblock = POINTERMASK & *fbl_np;
    if (base == f) {
      if (base == freeblock)
        *fbl_np = NIL;
      else
        error("GC error:deleting last list # FREEBLOCKLIST\n");
      return;
    } else if (base == freeblock)
      *fbl_np = f;
    f_np->bkwd = b;
    b_np->fwd = f;
  }
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
/*
 * Links a block onto the free list for a particular size range.
 * The free list is maintained as a doubly linked circular list accessed
 * from the block pointed to by the free list bucket for the size.
 * If there are no blocks in the free list bucket then the forward and
 * backward pointers of the newly added block point to the block itself.
 */
static LispPTR linkblock(LispPTR base) {
  struct arrayblock *base_np, *freeblock_np, *tail_np;
  LispPTR fbl, freeblock;
  LispPTR *fbl_np;

  if (*FreeBlockBuckets_word == NIL)
    return (base);

  base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
  if (base_np->arlen < MINARRAYBLOCKSIZE) {
    checkarrayblock(base, T, NIL);
    return (base);
  }

  /* lisp pointer to bucket for size */
  fbl = FreeBlockChainN(base_np->arlen);
  /* native pointer to bucket */
  fbl_np = (LispPTR *)NativeAligned4FromLAddr(POINTERMASK & fbl);
  /* lisp pointer to first free block on chain */
  freeblock = POINTERMASK & (*fbl_np);
  if (freeblock == NIL) { /* no blocks already in chain */
    base_np->fwd = base;
    base_np->bkwd = base;
  } else {
    /* set up new block to be first free block on the chain */
    freeblock_np = (struct arrayblock *)NativeAligned4FromLAddr(freeblock);
    /* link new block forward to free block */
    base_np->fwd = freeblock;
    /* new block's backward link becomes free block's backward link  */
    base_np->bkwd = freeblock_np->bkwd;
    /* get the tail location (backward pointer of freelist head) */
    tail_np = (struct arrayblock *)NativeAligned4FromLAddr(freeblock_np->bkwd);
    /* set its forward pointer to new block */
    tail_np->fwd = base;
    /* and the update the free block's backward link to new block */
    freeblock_np->bkwd = base;
  }
  /* new block becomes the head of the free list */
  *fbl_np = base;
  checkarrayblock(base, T, T); /* free, and on free list */
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
  struct arrayblock *block_np, *trailer_np;
  struct abdum *flags_np;
  block_np = (struct arrayblock *)NativeAligned4FromLAddr(block);
  /* this is an appropriate place to test whether the block that
     is about to be freed contains words that look like valid
     array header/trailer pairs as data.  This may result in
     false positives, but could help if there's a real smash happening.
  */
  /* struct abdum's abflags is a DLword and does not account for
     the BYTESWAP setup (as arrayblock does), so use WORDPTR to
     pick the correct word of the cell
  */
  flags_np = (struct abdum *)WORDPTR(block_np);
  flags_np->abflags = FREEARRAYFLAGWORD;
  block_np->arlen = length;
  trailer = Trailer(block, block_np);
  trailer_np = (struct arrayblock *)NativeAligned4FromLAddr(trailer);
  flags_np = (struct abdum *)WORDPTR(trailer_np);
  flags_np->abflags = FREEARRAYFLAGWORD;
  trailer_np->arlen = length;
  return (block);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
static LispPTR arrayblockmerger(LispPTR base, LispPTR nbase) {
  DLword arlens, narlens, secondbite, minblocksize, shaveback;
  struct arrayblock *base_np, *nbase_np;
  base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
  nbase_np = (struct arrayblock *)NativeAligned4FromLAddr(nbase);
  arlens = base_np->arlen;
  narlens = nbase_np->arlen;
  secondbite = MAXARRAYBLOCKSIZE - arlens;
  /* There are three cases for merging the blocks
   * (1) the total size of the two blocks is less than max:
   *     merge into a single block
   * (2) creating a max size block leaves a viable leftover block:
   *     move the boundary to make a max block and a leftover block
   * (3) creating a max size block leaves a non-viable leftover block
   *     move the boundary to make a big block and a minimum size leftover block
   */
  if (base + (DLWORDSPER_CELL * arlens) != nbase) {
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
    linkblock(makefreearrayblock(nbase + DLWORDSPER_CELL * secondbite, narlens));
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

/*
 * merges this block into the block behind it, unless there are
 * disqualifying conditions:
 *     merging is turned off or
 *     this is the first block in array space or
 *     this is the first block in the 2nd array space or
 *     the block behind it is in use
 * in which case it is linked onto the freelist (fwd and backward pointers)
 * and added to the free block chain by size.
 * If it can be merged, 
 */
LispPTR mergebackward(LispPTR base) {
  LispPTR pbase;
  struct arrayblock *ptrailer_np;

  if (base == NIL)
    return (NIL);
  /* back up to get the trailer of the previous block */
  ptrailer_np = (struct arrayblock *)NativeAligned4FromLAddr(base - ARRAYBLOCKTRAILERWORDS);
  /* check that there are no disqualifying conditions for merging with previous block */
  if ((*ArrayMerging_word == NIL) ||
           ((base == *ArraySpace_word) || ((base == *ArraySpace2_word) || (ptrailer_np->inuse == T))))
    return (linkblock(base));
  /* back up to the header of the previous block */
  pbase = base - DLWORDSPER_CELL * ptrailer_np->arlen;
  /* check that it is free, but skip free list checks */
  checkarrayblock(pbase, T, NIL);
  /* remove it from the free list */
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
  struct arrayblock *base_np, *nbase_np;
  if (*ArrayMerging_word == NIL) return NIL;
  if (base == NIL) return NIL;
  if (checkarrayblock(base, T, T)) return NIL;

  base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
  nbase = base + DLWORDSPER_CELL * (base_np->arlen);
  if (nbase == *ArrayFrLst_word || nbase == *ArrayFrLst2_word) return NIL;

  nbase_np = (struct arrayblock *)NativeAligned4FromLAddr(nbase);
  nbinuse = nbase_np->inuse;
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
/*
 * The pointer passed is to the data of the block, not the array block
 * header.
 */
LispPTR reclaimarrayblock(LispPTR ptr) {
  LispPTR tmpptr, btrailer;
  struct arrayblock *base_np;
  LispPTR *tmpp;
  int reclaim_p;

  reclaim_p = T;
#ifdef ARRAYCHECK
  checkarrayblock(ptr - ARRAYBLOCKHEADERWORDS, NIL, NIL);
#endif /* ARRAYCHECK */

  base_np = (struct arrayblock *)NativeAligned4FromLAddr(ptr - ARRAYBLOCKHEADERWORDS);
#ifdef ARRAYCHECK
  if (HILOC(ptr) < FIRSTARRAYSEGMENT) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error(
        "Bad array block reclaimed [not in array space].\nContinue with 'q' but save state ASAP. "
        "\n");
    return (T);
  } else if (ARRAYBLOCKPASSWORD != base_np->password) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error("Bad array block reclaimed [password wrong].\nContinue with 'q' but save state ASAP. \n");
    return (T);
  } else if (base_np->inuse == NIL) {
    printarrayblock(ptr - ARRAYBLOCKHEADERWORDS);
    error(
        "Bad array block reclaimed [block not in use].\nContinue with 'q' but save state ASAP. \n");
    return (T);
  }
#else
  /* Normal case, just tell the guy something's wrong: */
  if ((HILOC(ptr) < FIRSTARRAYSEGMENT) ||
      ((ARRAYBLOCKPASSWORD != base_np->password) || (base_np->inuse == NIL))) {
    error("Bad array block reclaimed--continue with 'q' but save state ASAP. \n");
    return (T);
  }
#endif /* ARRAYCHECK */

  switch (base_np->gctype) {
    case PTRBLOCK_GCT: {
      btrailer = (ptr - 2) + DLWORDSPER_CELL * (base_np->arlen - ARRAYBLOCKTRAILERCELLS);
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
    mergeforward(mergebackward(makefreearrayblock(ptr - ARRAYBLOCKHEADERWORDS, base_np->arlen)));
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
  struct arrayblock *base_np, *trailer_np, *ptrailer_np;
  LispPTR *addr;
  LispPTR pbase, nbase;

  base_np = (struct arrayblock *)NativeAligned4FromLAddr(base);
  trailer_np = (struct arrayblock *)NativeAligned4FromLAddr(Trailer(base, base_np));
  ptrailer_np = (struct arrayblock *)NativeAligned4FromLAddr(base - ARRAYBLOCKTRAILERWORDS);

  nbase = base + DLWORDSPER_CELL * base_np->arlen;
  pbase = base - DLWORDSPER_CELL * ptrailer_np->arlen;

  printf("This array block: 0x%x.  Previous: 0x%x.  Next: 0x%x.\n", base, pbase, nbase);
  printf("        password: 0x%x     gctype: 0x%x   in use: %d\n", base_np->password,
         base_np->gctype, base_np->inuse);
  if (!base_np->inuse)
    printf("      Free list: fwd 0x%x bkwd 0x%x\n", base_np->fwd, base_np->bkwd);
  printf("  Header  Length: %d cells.\n\n", base_np->arlen);
  printf(" Trailer  Length: %d cells.\n\n", trailer_np->arlen);

  addr = ((LispPTR *)base_np) - 20;
  for (; addr < (LispPTR *)base_np; addr++) printf("%16p (0x%8x) %8x\n", (void *)addr, LAddrFromNative(addr), *addr);
  printf("%16p (0x%8x) %8x <- array header\n", (void *)addr, LAddrFromNative(addr), *addr);
  addr++;
  for (; addr < (LispPTR *)base_np + 20; addr++) printf("%16p (0x%8x) %8x\n", (void *)addr, LAddrFromNative(addr), *addr);
  printf(". . .\n");

  addr = ((LispPTR *)trailer_np) - 20;
  for (; addr < (LispPTR *)trailer_np; addr++) printf("%16p (0x%8x) %8x\n", (void *)addr, LAddrFromNative(addr), *addr);
  printf("%16p (0x%8x) %8x <- array trailer\n", (void *)addr, LAddrFromNative(addr), *addr);
  addr++;
  for (; addr < (LispPTR *)trailer_np + 20; addr++) printf("%16p (0x%8x) %8x\n", (void *)addr, LAddrFromNative(addr), *addr);
}

static void printfreeblockchainhead(int index)
{
  LispPTR fbl, freeblock;
  LispPTR *fbl_np;

  fbl = POINTERMASK & ((*FreeBlockBuckets_word) + (DLWORDSPER_CELL * index));
  fbl_np = (LispPTR *)NativeAligned4FromLAddr(fbl);
  /* lisp pointer to free block on chain */
  freeblock = POINTERMASK & (*fbl_np);
  if (freeblock == NIL) { /* no blocks in chain */
    printf("Free block chain (bucket %d): NIL\n", index);
  } else {
    printf("Free block chain(bucket %d): 0x%x\n", index, freeblock);
  }
}

void printfreeblockchainn(int arlen)
{
  if (arlen >= 0) {
    printfreeblockchainhead(BucketIndex(arlen));
    return;
  } else
    for (int i = 0; i <= MAXBUCKETINDEX; i++) {
      printfreeblockchainhead(i);
    }
}
