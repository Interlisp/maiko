#ifndef GCDATA_H
#define GCDATA_H 1
/* $Id: gc.h,v 1.3 2001/12/24 01:08:57 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1989-94 Venue. All Rights Reserved.               */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/************************************************************************/

/**********************************************************************/
/*
		File Name :     gc.h

		Define  for garbage collector   
 
*/
/**********************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */
#include "version.h" /* for USHORT */

#define ADDREF  0       /* for gclookup routine. */
#define DELREF  1       /* for gclookup routine. */
#define STKREF  2       /* for gclookup routine. */
#define UNSTKREF        3       /* for htfind function. (T.T.) */


#ifdef BIGVM
  /* 32-bit HTmain, HTcoll, etc. entries */
#define GETGC(x) *((LispPTR *) (x))
#define GCENTRY LispPTR
#define GCPTR
#define MAX_GCCOUNT     0x7FFF  /* = 32767 */
#else
  /* Old, 16-bit entries */
#define GETGC GETWORD
#define GCENTRY DLword
#define GCPTR(x) WORDPTR(x)
#define MAX_GCCOUNT     0x3F    /* = 63 */
#endif /* BIGVM */



   /* IncAllocCnt(n) decrements reclaim countdown by N
      and signals interrupt if GC should happen soon */

   /* IncAllocCnt is called only when *Reclaim_cnt_word != NIL */

#define IncAllocCnt(n) do { \
	if ((*Reclaim_cnt_word -= (n)) <= S_POSITIVE) { \
		/* time for GC */ \
		Irq_Stk_Check = Irq_Stk_End = 0; \
		*Reclaim_cnt_word = S_POSITIVE; \
	} \
  } while (0)

   /* DecAllocCnt only called when *Reclaim_cnt_word != NIL */

#define DecAllocCnt(n) do { *Reclaim_cnt_word += (n); } while (0)

#define FreeLink(link) do {                        \
	GETGC(link) = 0;                        \
	GETGC((link)+1) = GETGC(HTcoll);        \
	GETGC(HTcoll) = ((link) - HTcoll);      \
  } while (0)


  /* Given the contents of an HTMAIN or HTCOLL entry,
	 get the link pointer (i.e., turn off the low bit) */
#define GetLinkptr(entry)       ((entry) & 0x0fffffffe)


#define DelLink(link, prev, entry) do {                                    \
  if ((prev) != (GCENTRY *)0)                                           \
    {                                                                   \
      GETGC((GCENTRY *)(prev) + 1) = GETGC((GCENTRY *)(link) + 1);      \
    }                                                                   \
  else                                                                  \
    {                                                                   \
      GETGC((GCENTRY *)(entry)) = GETGC((GCENTRY *)(link) + 1) | 1;     \
    }                                                                   \
  FreeLink((GCENTRY *)(link));                                          \
  (link) = (GCENTRY *)(HTcoll + GetLinkptr(GETGC((GCENTRY *)(entry)))); \
  if (GETGC((GCENTRY *)(link) + 1) == 0)                                \
    {                                                                   \
      GETGC((GCENTRY *)(entry)) = GETGC((GCENTRY *)(link));             \
      FreeLink((GCENTRY *)(link));                                      \
    }                                                                   \
  } while (0)

#define RefCntP(ptr) (!(GetTypeEntry((ptr)) & TT_NOREF) &&              \
		      (*GcDisabled_word != ATOM_T))

#define GCLOOKUP(ptr, case) do {                                                \
	if (RefCntP(ptr)) {                                                  \
		if (*Reclaim_cnt_word != NIL)                                \
		  htfind(ptr, case);                                         \
		else                                                         \
		  rec_htfind(ptr, case);                                     \
	}                                                                    \
  } while (0)

#define GCLOOKUPV(ptr, case, val) do {                                          \
	if (RefCntP(ptr)) {                                                  \
		if (*Reclaim_cnt_word != NIL)                                \
                  (val) = htfind((ptr), (case));                             \
		else                                                         \
                  (val) = rec_htfind((ptr), (case));                         \
	} else (val) = NIL;                                                  \
  } while (0)

#define REC_GCLOOKUP(ptr, case) do { if (RefCntP(ptr)) rec_htfind(ptr, case); } while (0)
#define REC_GCLOOKUPV(ptr, case, val) do {                                      \
	if (RefCntP(ptr))                                                    \
          (val) = rec_htfind((ptr), (case));                                 \
	else                                                                 \
          (val) = NIL;                                                       \
  } while (0)

#define FRPLPTR(old , new) do { \
		GCLOOKUP(new, ADDREF); \
		GCLOOKUP(old, DELREF); \
		(old) = (new) ; } while (0)


#ifndef BYTESWAP
	/********************************************************/
	/*   Normal byte-order definitions, for e.g., 68020s    */
	/********************************************************/
#ifdef BIGVM  
struct   hashentry
  { /* GC hashtable entry */
    unsigned short count        :15;
    unsigned short stackref     :1;
    unsigned short segnum       :15;
    unsigned short collision    :1;
  };

struct  htlinkptr
  {     /* overlay access method */
    LispPTR     contents;
  };

struct  htcoll
  { /* GC collision table entry */
    LispPTR     free_ptr ;
    LispPTR     next_free ;
  };

struct  gc_ovfl
  {
    LispPTR       ovfl_ptr ;
    unsigned int  ovfl_cnt ;
  };

struct  htoverflow 
  {     /* July-23-1987 by TT */
    unsigned    pcase   :4;
    unsigned    ptr     :28;
  };
#else
struct   hashentry
  { /* GC hashtable entry */
    USHORT count        :6;
    USHORT stackref     :1;
    USHORT segnum       :8;
    USHORT collision    :1;
  };

struct  htlinkptr
  {     /* overlay access method */
    DLword      contents;
  };

struct  htcoll
  { /* GC collision table entry */
    DLword      free_ptr ;
    DLword      next_free ;
  };

struct  gc_ovfl
  {
    LispPTR       ovfl_ptr ;
    unsigned int  ovfl_cnt ;
  };

struct  htoverflow 
  {     /* July-23-1987 by TT */
    unsigned    pcase   :8;
    unsigned    ptr     :24;
  };
#endif /* BIGVM */

#else

	/********************************************************/
	/*      Byte-swapped definitions, for e.g., 80386s      */
	/********************************************************/
#ifdef BIGVM
struct   hashentry
  { /* GC hashtable entry */
    USHORT collision    :1;
    USHORT segnum       :15;
    USHORT stackref     :1;
    USHORT count        :15;
  };

struct  htlinkptr
  {     /* overlay access method */
    LispPTR     contents;
  };

struct  htcoll
  { /* GC collision table entry */
    LispPTR     free_ptr ;
    LispPTR     next_free ;
  };

struct  gc_ovfl
  {
    LispPTR       ovfl_ptr ;
    unsigned int  ovfl_cnt ;
  };

struct  htoverflow 
  {     /* July-23-1987 by TT */
    unsigned    ptr     :28;
    unsigned    pcase   :4;
  };
#else
struct   hashentry
  { /* GC hashtable entry */
    USHORT collision    :1;
    USHORT segnum       :8;
    USHORT stackref     :1;
    USHORT count        :6;
  };

struct  htlinkptr
  {     /* overlay access method */
    DLword      contents;
  };

struct  htcoll
  { /* GC collision table entry */
    DLword      next_free ;
    DLword      free_ptr ;
  };

struct  gc_ovfl
  {
    LispPTR       ovfl_ptr ;
    unsigned int  ovfl_cnt ;
  };

struct  htoverflow 
  {     /* July-23-1987 by TT */
    unsigned    ptr     :24;
    unsigned    pcase   :8;
  };
#endif /* BIGVM */

#endif /*  BYTESWAP */
#endif /* GCDATA_H */
