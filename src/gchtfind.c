/* $Id: gchtfind.c,v 1.3 1999/05/31 23:35:31 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "address.h"       // for LOLOC
#include "adr68k.h"        // for LAddrFromNative
#include "commondefs.h"    // for error
#include "gcdata.h"        // for GETGC, GCENTRY, ADDREF, DELREF, gc_ovfl
#include "gchtfinddefs.h"  // for enter_big_reference_count, htfind, modify_...
#include "gcrdefs.h"       // for disablegc1
#include "lispemul.h"      // for LispPTR, NIL, state, DLWORDSPER_CELL, ATOM_T
#include "lispmap.h"       // for HTCOLL_SIZE
#include "lspglob.h"       // for HTcoll, HTbigcount, HTmain
#include "lsptypes.h"      // for WORDPTR
#include "storagedefs.h"   // for newpage

#define Evenp(num, prim) (((num) % (prim)) == 0)
#ifdef BIGVM
/* HTCOLLMAX should be in half-entries, not in words */
#define HTCOLLMAX ((HTCOLL_SIZE / DLWORDSPER_CELL) - 16)
#else
#define HTCOLLMAX (HTCOLL_SIZE - 16)
#endif /* BIGVM */

/* GetLink gets a new entry from the GC collision table */
#define GetLink(var)                                               \
  do {                                                                \
    GCENTRY linkoff;                                      \
    linkoff = GETGC(HTcoll);                                       \
    if (linkoff == 0) {                                            \
      if ((linkoff = GETGC((GCENTRY *)HTcoll + 1)) >= HTCOLLMAX) { \
        disablegc1(NIL);                                           \
        return (NIL);                                              \
      }                                                           \
      GETGC((GCENTRY *)HTcoll + 1) = linkoff + 2;                  \
      (var) = (GCENTRY *)(HTcoll + linkoff);                       \
    } else {                                                       \
      GETGC(HTcoll) = GETGC((GCENTRY *)(HTcoll + linkoff + 1));    \
      (var) = (GCENTRY *)(HTcoll + linkoff);                       \
    }                                                              \
  } while (0)

#ifdef BIGVM
#define HTCNTSHIFT 17           /* amount to shift to get hash table count */
#define HTCNTMASK 0xFFFE0000    /* mask which masks off hash table count */
#define HTCNTSTKMASK 0XFFFF0000 /* mask for hash table count + stack bit */
#define HTSTKMASK 0x10000       /* mask for stack bit only */
#define HTHIMASK                                                \
  0x1FFE            /* mask of bits which contain high part of  \
                       pointer in hash table  FIXME change this \
                       to 1FFE when  pointers really go to      \
                       28 bits. JDS */
#define HTHISHIFT 1 /* high bits in hash table are shifted left 1 */
#else
#define HTCNTSHIFT 10       /* amount to shift to get hash table count */
#define HTCNTMASK 0xFC00    /* mask which masks off hash table count */
#define HTCNTSTKMASK 0XFE00 /* mask for hash table count + stack bit */
#define HTSTKMASK 0x0200    /* mask for stack bit only */
#define HTHIMASK                                                       \
  0x1FE             /* mask of bits which contain high part of pointer \
                    in hash table */
#define HTHISHIFT 1 /* high bits in hash table are shifted left 1 */
#endif              /* BIGVM */

/*  NewEntry is a macro for adding a new gc hash table entry;
        entry is pointer to hash table entry
        hiptr is the high point of the ref-cnted entry, shifted
        casep is one of ADDREF, DELREF, etc.
*/

/*
 * NewEntry is never called in the course of the reclamation.
 * Thus STKREF case is not needed.
 */
#define NewEntry(entry, hiptr, casep, ptr)                              \
  do {                                                                     \
    switch (casep) {                                                    \
      case ADDREF:                                                      \
        GETGC(entry) = (hiptr) | (2 << HTCNTSHIFT); /* set count = 2 */ \
        IncAllocCnt(1);                                                 \
        return NIL; /* not new 0 entry */                               \
      case DELREF:                                                      \
        GETGC(entry) = hiptr; /* set count = 0 */                       \
        IncAllocCnt(1);                                                 \
        return ptr; /* new 0 entry */                                   \
      default: error("GC error: new entry touches stack bit");          \
        return NIL; /* NOT REACHED */                                   \
    }                                                                   \
  } while (0)

/*
 * RecNewEntry is called in the course of the reclamation.
 * Does not maintain the allocation count.
 */
#define RecNewEntry(entry, hiptr, casep, ptr)                           \
  do {                                                                     \
    switch (casep) {                                                    \
      case ADDREF:                                                      \
        GETGC(entry) = (hiptr) | (2 << HTCNTSHIFT); /* set count = 2 */ \
        return NIL;                               /* not new 0 entry */ \
      case DELREF:                                                      \
        GETGC(entry) = hiptr; /* set count = 0 */                       \
        return ptr;           /* new 0 entry */                         \
      case STKREF:            /* set refcnt to 1, stack bit to 1 */     \
        GETGC(entry) = (hiptr) | (1 << HTCNTSHIFT) | HTSTKMASK;         \
        return NIL;                                                     \
      default: error("GC error: new entry when turning off stack bit"); \
        return NIL; /* NOT REACHED */                                   \
    }                                                                   \
  } while (0)

/* ModEntry is a macro to modify an old gc hash table entry.
     entry is a pointer to the entry
     contents holds the old contents
     ptr is the pointer being counted
     casep is one of ADDREF, DELREF, etc.
     remove is a label to go to if the entry will go away

     It always return NIL, since cannot be creating a zero-count,
      no-stack-bit entry */
/*
 * ModEntry is never called in the course of the reclamation.
 * Thus STKREF and UNSTKREF cases are not needed.
 */
#define ModEntry(entry, contents, ptr, casep, remove)                                       \
  do {                                                                                         \
    if (((contents) & HTCNTMASK) == HTCNTMASK) { /* overflow; return non-zero */            \
      modify_big_reference_count(entry, casep, ptr);                                        \
      return NIL;                                                                           \
    }                                                                                       \
    switch (casep) {                                                                        \
      case ADDREF:                                                                          \
        (contents) += (1 << HTCNTSHIFT);                                                    \
        if (((contents) & HTCNTMASK) == HTCNTMASK) { /* overflow */                         \
          GETGC(entry) = contents;                                                          \
          enter_big_reference_count(ptr);                                                   \
          return NIL;                                                                       \
        }                                                                                   \
        if (((contents) & HTCNTSTKMASK) == (1 << HTCNTSHIFT)) {                             \
          DecAllocCnt(1);                                                                   \
          goto remove; /* NOLINT(bugprone-macro-parentheses) */                             \
        }                                                                                   \
        break;                                                                              \
      case DELREF:                                                                          \
        if (((contents) >> HTCNTSHIFT) == 0) error("attempt to decrement 0 reference count"); \
        (contents) -= (1 << HTCNTSHIFT);			                            \
        if (((contents) & HTCNTSTKMASK) == (1 << HTCNTSHIFT)) {                             \
          DecAllocCnt(1);                                                                   \
          goto remove; /* NOLINT(bugprone-macro-parentheses) */                             \
        }                                                                                   \
        break;                                                                              \
      default: error("GC error: mod entry touches stack bit");                              \
    }                                                                                       \
    GETGC(entry) = contents;                                                                \
    return NIL;                                                                             \
  } while (0)

/*
 * RecModEntry is called in the course of the reclamation.
 * Does not maintain the allocation count.
 */
#define RecModEntry(entry, contents, ptr, casep, remove)                                    \
  do {                                                                                         \
    if (((contents) & HTCNTMASK) == HTCNTMASK) { /* overflow; return non-zero */            \
      modify_big_reference_count(entry, casep, ptr);                                        \
      return NIL;                                                                           \
    }                                                                                       \
    switch (casep) {                                                                        \
      case ADDREF:                                                                          \
        (contents) += (1 << HTCNTSHIFT);                                                    \
        if (((contents) & HTCNTMASK) == HTCNTMASK) {                                        \
          /* overflow */                                                                    \
          GETGC(entry) = contents;                                                          \
          enter_big_reference_count(ptr);                                                   \
          return NIL;                                                                       \
        }                                                                                   \
        break; /* check for possibly deleting entry */                                      \
      case DELREF:                                                                          \
        if (((contents) >> HTCNTSHIFT) == 0) error("attempt to decrement 0 reference count"); \
        (contents) -= (1 << HTCNTSHIFT);                                                    \
        break;                                                                              \
      case STKREF:                                                                          \
        GETGC(entry) = (contents) | HTSTKMASK;                                              \
        return NIL;                                                                         \
        /*                                                                                  \
                case UNSTKREF:                                                              \
                        contents = contents & ~ HTSTKMASK;                                  \
                        break;                                                              \
        */                                                                                  \
    }                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                                        \
    if (((contents) & HTCNTSTKMASK) == (1 << HTCNTSHIFT)) goto remove;                      \
    GETGC(entry) = contents;                                                                \
    return NIL;                                                                             \
  } while (0)

/************************************************************************/
/*									*/
/*	     e n t e r _ b i g _ r e f e r e n c e _ c o u n t		*/
/*									*/
/*	Add a new overflow entry, for a count that won't fit into	*/
/*	the field of a main GC table entry.				*/
/*									*/
/************************************************************************/

void enter_big_reference_count(LispPTR ptr) {
  struct gc_ovfl *oventry;
  LispPTR tmp;

  /* this kludge is apparently necessary. Odd pointers are
  illegal, but apparently some are reference counted. If you
  get an odd pointer, just ignore the low bit */

  ptr &= 0xfffffffe;

  oventry = (struct gc_ovfl *)HTbigcount;
  while (((tmp = oventry->ovfl_ptr) != ATOM_T) && (tmp != NIL))
  /* free area ? */
  {
    if (tmp == ptr) {
      error("ERROR : PTR already in overflow table.\n");
      oventry->ovfl_cnt += 0x10000; /* "Assure it lives forever" */
      return;
    } else
      ++oventry;
  }

  if (tmp == NIL) {
    if (Evenp(LAddrFromNative(oventry + 1), DLWORDSPER_PAGE)) {
      if ((UNSIGNED)oventry + 1 >= (UNSIGNED)HTcoll) error("GC big reference count table overflow");
      newpage(LAddrFromNative(oventry + 1));
    }
  }

  oventry->ovfl_cnt = MAX_GCCOUNT;
  oventry->ovfl_ptr = ptr;
  return;
}

/************************************************************************/
/*									*/
/*	    m o d i f y _ b i g _ r e f e r e n c e _ c o u n t		*/
/*									*/
/*	Modify an existing overflow entry.				*/
/*									*/
/************************************************************************/

void modify_big_reference_count(GCENTRY *entry, DLword casep, LispPTR ptr) {
  struct gc_ovfl *oventry;
  LispPTR tmp;

  /* ditto comment in entry_big_reference_count */
  if (ptr & 1) ptr &= 0xfffffffe;
  oventry = (struct gc_ovfl *)HTbigcount;
  while ((tmp = oventry->ovfl_ptr) != ptr)
    if (tmp == NIL) {
      error("refcnt previously overflowed, but not found in table.\n");
      return;
    } else
      ++oventry; /* increment by size of oventry structure */

  switch (casep) {
    case ADDREF: ++(oventry->ovfl_cnt); return;
    case DELREF:
      if (--(oventry->ovfl_cnt) < MAX_GCCOUNT) {
        /* fallen below threshold */
        ((struct hashentry *)GCPTR(entry))->count = MAX_GCCOUNT - 1;
        oventry->ovfl_ptr = ATOM_T; /* mark entry unused */
      }
      return;
    case STKREF:
      ((struct hashentry *)WORDPTR(entry))->stackref = 1;
      return;
      /*
              case UNSTKREF:
                ((struct hashentry *) WORDPTR(entry))->stackref = 0;
                return;
      */
  }
}

/************************************************************************/
/*									*/
/*				h t f i n d				*/
/*									*/
/*	Main entry for Ref-count manipulation:  Modify the reference	*/
/*	count for a lisp pointer.					*/
/*									*/
/*	casep is one of ADDREF, DELREF, STKREF				*/
/*									*/
/*	ADDREF = add 1							*/
/*	DELREF = subtract 1						*/
/*	STKREF = turn on stack bit					*/
/*	UNSTKREF = turn off stack bit					*/
/*									*/
/*	returns NIL if DELREF and the entry became			*/
/*	refcount = 0, stk bit off (only can happen on a *new* DELREF)	*/
/*	in which case it returns PTR					*/
/*									*/
/************************************************************************/

LispPTR htfind(LispPTR ptr, int casep) {
  GCENTRY *entry, *link, *prev;
  GCENTRY entry_contents, hiptr;

  /* if the NOREF bit is on in the type table entry, do
  not reference count this pointer. Used for non-reference
  counted types like symbols, and also when the GC is
  disabled. */

  /*
   * Following two tests were moved into GCLOOKUP macro
   * for efficiency.
   */
  /*
      if (GetTypeEntry(ptr) & TT_NOREF) return NIL;
  */
  /* if *GcDisabled_word is T then do nothing */
  /* FS:  this test should not be needed (because type table should
          be cleared).  Also, this test seems to cause an infinite
          ucode loop in remimplicitkeyhash on the 386i		*/

  /*    if(*GcDisabled_word == ATOM_T) return(NIL); */

  /* GC hash table entries have the high 8 bits of the
  pointer stored in the middle. Set up hiptr to have
  the high bits of the pointer ready to store or test
  against */

  hiptr = (((UNSIGNED)ptr) >> (16 - HTHISHIFT)) & HTHIMASK;

  /* entry points at the place in the main hash table
  where this pointer is stored. The 'hash' isn't one really;
  it just uses the low bits of the pointer. */

  entry = HTmain + (LOLOC(ptr) >> 1);

  entry_contents = GETGC(entry);

  if (entry_contents == 0) NewEntry(entry, hiptr, casep, ptr);
  /* NewEntry returns */

  if (entry_contents & 1) { /* low bit means a collision entry */
    /* entry_contents-1 removes low bit */
    link = HTcoll + (entry_contents - 1);
    prev = 0;
    goto newlink;
  }

  if (hiptr == (entry_contents & HTHIMASK)) {
    ModEntry(entry, entry_contents, ptr, casep, delentry);
    /* ModEntry returns or will go to delentry */
  }

  /* new collision */

  GetLink(link);
  GetLink(prev);
  GETGC((GCENTRY *)prev + 1) = 0;
  GETGC((GCENTRY *)prev) = entry_contents;
  GETGC((GCENTRY *)link + 1) = prev - HTcoll;
  GETGC((GCENTRY *)entry) = (link - HTcoll) + 1;

  NewEntry(link, hiptr, casep, ptr);
/* NewEntry returns */

delentry:
  GETGC(entry) = 0;
  return NIL;

/* start here when a collision is detected. link is a pointer to
  the entry in the collision table, prev is the previous collision
  entry or 0 if this is the first one. */

newlink:
  entry_contents = GETGC(link);
  if (hiptr == (entry_contents & HTHIMASK)) {
    ModEntry(link, entry_contents, ptr, casep, dellink);
    /* ModEntry returns or goes to dellink */
  }

  /* collision didn't match  */
  entry_contents = GETGC((GCENTRY *)link + 1);
  if (entry_contents == 0) goto nolink;

  /* try the next link in the collision table */
  prev = link;
  link = HTcoll + entry_contents;
  goto newlink;

dellink:
  if (prev)
    GETGC((GCENTRY *)prev + 1) = GETGC((GCENTRY *)link + 1);
  else
    GETGC((GCENTRY *)entry) = (GETGC((GCENTRY *)link + 1)) | 1;
  FreeLink(link);
  link = HTcoll + (GETGC((GCENTRY *)entry)) - 1;
  if (GETGC(link + 1) == 0) {
    GETGC((GCENTRY *)entry) = GETGC((GCENTRY *)link);
    FreeLink(link);
  }
  return NIL;

nolink: /* no match */

  GetLink(link);
  GETGC((GCENTRY *)link + 1) = GETGC((GCENTRY *)entry) - 1;
  GETGC((GCENTRY *)entry) = (link - HTcoll) + 1;
  NewEntry(link, hiptr, casep, ptr);
  /* NewEntry will return */
}

/************************************************************************/
/*									*/
/*			r e c _ h t f i n d				*/
/*									*/
/*	Version of HTFIND used during reclaims (part of GC process)	*/
/*	Same purpose, but doesn't increment the GC count-down, and	*/
/*	DELREF can add 0-refcount entries to the table.			*/
/*									*/
/************************************************************************/

LispPTR rec_htfind(LispPTR ptr, int casep) {
  GCENTRY *entry, *link, *prev;
  GCENTRY entry_contents, hiptr;

  /* if the NOREF bit is on in the type table entry, do
  not reference count this pointer. Used for non-reference
  counted types like symbols, and also when the GC is
  disabled. */
  /*
   * Following two tests were moved into GCLOOKUP macro
   * for efficiency.
   */
  /*
      if (GetTypeEntry(ptr) & TT_NOREF)
                  return NIL;
  */
  /* if *GcDisabled_word is T then do nothing */
  /* FS:  this test should not be needed (because type table should
      be cleared).  Also, this test seems to cause an infinite
      ucode loop in remimplicitkeyhash on the 386i		*/

  /*    if(*GcDisabled_word == ATOM_T) return(NIL); */

  /* GC hash table entries have the high 8 bits of the
  pointer stored in the middle. Set up hiptr to have
  the high bits of the pointer ready to store or test
  against */

  hiptr = (((unsigned int)ptr) >> (16 - HTHISHIFT)) & HTHIMASK;

  /* entry points at the place in the main hash table
  where this pointer is stored. The 'hash' isn't one really;
  it just uses the low bits of the pointer. */

  entry = HTmain + (LOLOC(ptr) >> 1);

  entry_contents = GETGC(entry);

  if (entry_contents == 0) RecNewEntry(entry, hiptr, casep, ptr);
  /* NewEntry returns */

  if (entry_contents & 1) { /* low bit means a collision entry */
    /* entry_contents-1 removes low bit */
    link = HTcoll + (entry_contents - 1);
    prev = 0;
    goto newlink;
  }

  if (hiptr == (entry_contents & HTHIMASK)) {
    RecModEntry(entry, entry_contents, ptr, casep, delentry);
    /* ModEntry returns or will go to delentry */
  }

  /* new collision */

  GetLink(link);
  GetLink(prev);
  GETGC((GCENTRY *)prev + 1) = 0;
  GETGC((GCENTRY *)prev) = entry_contents;
  GETGC((GCENTRY *)link + 1) = prev - HTcoll;
  GETGC((GCENTRY *)entry) = (link - HTcoll) + 1;

  RecNewEntry(link, hiptr, casep, ptr);

delentry:
  GETGC(entry) = 0;
  return NIL;

/* start here when a collision is detected. link is a pointer to
  the entry in the collision table, prev is the previous collision
  entry or 0 if this is the first one. */

newlink:
  entry_contents = GETGC(link);
  if (hiptr == (entry_contents & HTHIMASK)) {
    RecModEntry(link, entry_contents, ptr, casep, dellink);
    /* ModEntry returns or goes to dellink */
  }
  /* collision didn't match  */
  entry_contents = GETGC(link + 1);
  if (entry_contents == 0) { goto nolink; }
  /* try the next link in the collision table */
  prev = link;
  link = HTcoll + entry_contents;
  goto newlink;

dellink:
  if (prev)
    GETGC((GCENTRY *)prev + 1) = GETGC((GCENTRY *)link + 1);
  else
    GETGC((GCENTRY *)entry) = (GETGC((GCENTRY *)link + 1)) | 1;

  FreeLink(link);
  link = HTcoll + ((GETGC((GCENTRY *)entry)) - 1);
  if (GETGC((GCENTRY *)link + 1) == 0) {
    GETGC((GCENTRY *)entry) = GETGC((GCENTRY *)link);
    FreeLink(link);
  }
  return NIL;

nolink: /* no match */

  GetLink(link);
  GETGC((GCENTRY *)link + 1) = GETGC((GCENTRY *)entry) - 1;
  GETGC((GCENTRY *)entry) = (link - HTcoll) + 1;
  RecNewEntry(link, hiptr, casep, ptr);
}
