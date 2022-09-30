/* $Id: gcarray.c,v 1.3 1999/05/31 23:35:30 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*************************************************************************/
/*                                                                       */
/*                      File Name : gcarray.c                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions :                                                 */
/*                       LispPTR aref1(array, index);                    */
/*			 DLword find_symbol(char_base,offset,            */
/*						 length,hashtbl);        */
/*                       DLword get_package_atom(                        */
/*                               char_base,charlen,packname,             */
/*                                            packlen,externalp);        */
/*									 */
/*			LispPTR with_symbol(charbase, charlen, 		 */
/*					    hashtable, result_fixp);	 */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/* The function "aref1" is the accessor of oned_array.                   */
/* The functions "find_symbol" and "get_package_atom" are implemented    */
/*     to access the atom through the package mechanism.                 */
/*									 */
/*	with_symbol is a C subr implementation of LLPACKAGE's 		 */
/*	WITH-SYMBOL macro:  It returns as its result the symbol found,	 */
/*	or NIL if none, and in the result_fixp, -1 if not found.	 */
/*									 */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include <stdio.h>         // for printf
#include <string.h>        // for strncmp
#include "adr68k.h"        // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#include "array.h"         // for arrayheader
#include "car-cdrdefs.h"   // for car, cdr
#include "cell.h"          // for PNCell, PLCell, GetPnameCell, GetPropCell
#include "commondefs.h"    // for error
#include "debug.h"         // for PACKAGE
#include "gcarraydefs.h"   // for aref1, find_symbol, get_package_atom, with...
#include "lispemul.h"      // for LispPTR, DLword, NIL, SEGMASK, T
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"       // for Package_from_Index_word
#include "lsptypes.h"      // for GETBYTE, GETWORD
#include "mkatomdefs.h"    // for compare_chars, compare_lisp_chars, compute...
#include "testtooldefs.h"  // for find_package_from_name

/*** not currently used -FS
        #define min(a,b)		((a > b)?b:a)
        #define Trailer(ldatum,datum68)	(ldatum+2*(datum68->arlen - ARRAYBLOCKTRAILERCELLS))
        #define BucketIndex(n)		min(integerlength(n),MAXBUCKETINDEX)
        #define FreeBlockChainN(n)	((POINTERMASK & *FreeBlockBuckets_word)+2*BucketIndex(n))
 ***/

#define Rehash_factor(hash, tablelen) (((hash) % ((tablelen)-2)) + 1)
#define Symbol_hash_reprobe(hash, rehashfactor, tablelen) (((hash) + (rehashfactor)) % (tablelen))
#define Entry_hash(strlen, sxhash) \
  (((((((strlen) ^ (sxhash)) ^ ((sxhash) >> 8)) ^ ((sxhash) >> 16)) ^ ((sxhash) >> 19)) % 254) + 2)

/************************************************************************/
/*									*/
/*			Package hashtable structure			*/
/*									*/
/*	2 per package, for looking up internal and external symbols.	*/
/*									*/
/************************************************************************/

struct hashtable {
  LispPTR table;
  LispPTR hash;
  LispPTR size;
  LispPTR free;
  LispPTR deleted;
};

/* The end of macros & structure for medley version */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR aref1(LispPTR array, int index) {
  LispPTR retval = 0;
  LispPTR base;
  short typenumber;
  struct arrayheader *actarray;

  actarray = (struct arrayheader *)NativeAligned4FromLAddr(array);
  if (index >= actarray->totalsize) {
    printf("Invalid index in GC's AREF1:  0x%x\n", index);
    printf(" Array size limit:  0x%x\n", actarray->totalsize);
    printf(" Array ptr: 0x%x\n", array);
    printf(" Array 68K ptr: %p\n", (void *)actarray);
    printf("base:     0x%x\n", actarray->base);
    printf("offset:   0x%x\n", actarray->offset);
    printf("type #:   0x%x\n", actarray->typenumber);
    printf("fill ptr: 0x%x\n", actarray->fillpointer);
    error("index out of range in GC's AREF1.");
  }
  index += actarray->offset;
  typenumber = actarray->typenumber;
  base = actarray->base;
  switch (typenumber) {
    case 3: /* unsigned 8bits */
      retval = (GETBYTE(((char *)NativeAligned2FromLAddr(base)) + index)) & 0x0ff;
      retval |= S_POSITIVE;
      break;
    case 4: /* unsigned 16bits */
      retval = (GETWORD(((DLword *)NativeAligned2FromLAddr(base)) + index)) & 0x0ffff;
      retval |= S_POSITIVE;
      break;
    case 38: retval = (*(((LispPTR *)NativeAligned4FromLAddr(base)) + index)); break;
    default: error("Not Implemented in gc's aref1 (other types)");
  }
  return (retval);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR find_symbol(const char *char_base, DLword offset, DLword length, LispPTR hashtbl, DLword fatp,
                    DLword lispp)

/* T => the "chars" coming in are 16-bit */
/* T => the incoming chars are in LISP space */
{
  DLword hashval, ehashval, h2, ehash, indexvar;
  int arraylen;
  struct hashtable *hashtbladdr;
#ifdef BIGATOMS
  LispPTR vecs, hashes;
#endif /* BIGATOMS */

  LispPTR vec, hash;
  struct arrayheader *vec68k;
  int fatpnamep;

  if (!hashtbl) return (0xffffffff);

  if (lispp)
    hashval = compute_lisp_hash(char_base, offset, length, fatp);
  else
    hashval = compute_hash(char_base, offset, length);

  ehashval = Entry_hash(length, hashval);
  hashtbladdr = (struct hashtable *)NativeAligned4FromLAddr(hashtbl);

  /* Move our string ptr up by offset, allowing for fatness */
  if (fatp)
    char_base += (offset << 1);
  else
    char_base += offset;

#ifdef BIGATOMS
  vecs = hashtbladdr->table;
  hashes = hashtbladdr->hash;

loop_thru_hashtables:

  vec = car(vecs);
  vecs = cdr(vecs);
  hash = car(hashes);
  hashes = cdr(hashes);
  vec68k = (struct arrayheader *)NativeAligned4FromLAddr(vec);
  arraylen = vec68k->totalsize;
  if (arraylen == 0) return (0xffffffff); /*kludge TAKE*/
  h2 = Rehash_factor(hashval, arraylen);
  indexvar = (hashval % arraylen);
#else
  vec = hashtbladdr->table;
  hash = hashtbladdr->hash;
  vec68k = (struct arrayheader *)NativeAligned4FromLAddr(vec);
  arraylen = vec68k->totalsize;
  if (arraylen == 0) return (0xffffffff); /*kludge TAKE*/
  h2 = Rehash_factor(hashval, arraylen);
  indexvar = (hashval % arraylen);
#endif /* BIGATOMS */

retry:
  /* the aref1 returns a smallp, which is always <256, so trim it */
  while (ehashval != (ehash = 0xFF & aref1(hash, indexvar))) {
    if (ehash == NIL) { /* Ran out of entries in this table; try next or fail */
#ifdef BIGATOMS
      if (hashes == NIL) return (0xffffffff); /* Last table.  Fail. */
      goto loop_thru_hashtables;
#else
      return (0xffffffff);
#endif /* BIGATOMS */
    }
    indexvar = Symbol_hash_reprobe(indexvar, h2, arraylen);
  }
  /*   if ((indexvar&0xffff) != NIL) */
  {
    LispPTR index;
    PNCell *pnptr;
    char *pname_base;

    index = aref1(vec, indexvar);
    if ((index & SEGMASK) == S_POSITIVE) index &= 0xFFFF;

    pnptr = (PNCell *)GetPnameCell(index);
    fatpnamep = ((PLCell *)GetPropCell(index))->fatpnamep;
    pname_base = (char *)NativeAligned2FromLAddr(pnptr->pnamebase);
    if ((length == GETBYTE(pname_base)) &&
        (T == ((lispp) ? compare_lisp_chars((pname_base + 1 + fatpnamep), char_base, length,
                                            fatpnamep, fatp)
                       : compare_chars((pname_base + 1 + fatpnamep), char_base, length)))) {
      return (index);
    } else {
      indexvar = Symbol_hash_reprobe(indexvar, h2, arraylen);
      goto retry;
    }
  }
  /*   else return(0xffffffff); */ /* can't find */
}

/************************************************************************/
/*									*/
/*			g e t _ p a c k a g e _ a t o m			*/
/*									*/
/*	Try to look up the given symbol in the given package.  If 	*/
/*	you find it, return the atom number.  Otherwise, return -1.	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR get_package_atom(const char *char_base, DLword charlen, const char *packname, DLword packlen,
                         int externalp) {
  int packindex;
  PACKAGE *packaddr;
  /* LispPTR hashtbladdr; */
  LispPTR index;

  /* For convenience, recognize the common package nicknames: */

  if (0 == strncmp(packname, "XCL", packlen))
    packindex = find_package_from_name("XEROX-COMMON-LISP", 17);
  else if (0 == strncmp(packname, "SI", packlen))
    packindex = find_package_from_name("SYSTEM", 6);
  else if (0 == strncmp(packname, "CL", packlen))
    packindex = find_package_from_name("LISP", 4);
  else if (0 == strncmp(packname, "XCLC", packlen))
    packindex = find_package_from_name("COMPILER", 8);

  /**** else if (0 == strncmp(packname, "KEYWORD", packlen))
      packindex = 7;***/

  else
    packindex = find_package_from_name(packname, packlen);

  if (packindex < 0) {
    printf("getting package index failed %s:%s\n", packname, char_base);
    return (0xffffffff);
  }

  /* if (packindex != 7)  Not necessary (Take)*/
  packaddr = (PACKAGE *)NativeAligned4FromLAddr(aref1(*Package_from_Index_word, packindex));
  /* else packaddr = (PACKAGE *)NativeAligned4FromLAddr(
                          *Keyword_Package_word);	*/
  /* hashtbladdr =	((externalp == T)?(packaddr->EXTERNAL_SYMBOLS):
                           (packaddr->INTERNAL_SYMBOLS));
   return( find_symbol(char_base, 0, charlen, hashtbladdr, 0, 0) );*/

  if ((index = find_symbol(char_base, 0, charlen, packaddr->EXTERNAL_SYMBOLS, 0, 0)) != 0xffffffff)
    return (index);
  else
    return (find_symbol(char_base, 0, charlen, packaddr->INTERNAL_SYMBOLS, 0, 0));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR with_symbol(LispPTR char_base, LispPTR offset, LispPTR charlen, LispPTR fatp,
                    LispPTR hashtbl, LispPTR result) {
  char *charbase68k = (char *)NativeAligned2FromLAddr(char_base);
  LispPTR *resultptr = (LispPTR *)NativeAligned4FromLAddr(result);
  DLword chars = charlen & 0xFFFF; /* charlen must be a SMALLP! */
  DLword offst = offset & 0xFFFF;
  int symbol; /* Where the symbol goes pro tem */

  symbol = find_symbol(charbase68k, offst, chars, hashtbl, (DLword)fatp, (DLword)1);

  if (symbol == -1) { /* Not found.  Signal that with -1 in result fixp */
    *resultptr = -1;
    return (NIL);
  }

  *resultptr = 3;
  return (symbol);

} /* End of with_symbol */
