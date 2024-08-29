/* $Id: gccode.c,v 1.3 1999/05/31 23:35:30 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/* File Name : gccode.c						*/
/*									*/
/************************************************************************/
/*									*/
/* Creation Date : Sep-25-1987						*/
/* Written by Tomoru Teruuchi						*/
/* Edit by Larry Masinter						*/
/************************************************************************/
/*									*/
/* Functions :								*/
/* reclaimcodeblock();							*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
/* \Tomtom								*/
/************************************************************************/

#include <stdio.h>       // for sprintf
#include "address.h"     // for LOLOC, HILOC
#include "adr68k.h"      // for NativeAligned4FromLAddr
#include "commondefs.h"  // for error
#include "gccodedefs.h"  // for code_block_size, reclaimcodeblock
#include "gcdata.h"      // for REC_GCLOOKUP, DELREF, ADDREF
#include "gchtfinddefs.h" // for htfind, rec_htfind
#include "lspglob.h"     // for Deleted_Implicit_Hash_Slot_word, UFNTable
#include "lsptypes.h"    // for LispPTR, NIL, UFN, Get_code_BYTE, POINTERMASK
#include "stack.h"       // for fnhead

#define min(a, b) (((a) > (b)) ? (b) : (a))

#define ENDOFX 0
#define GCONST 111

#define Reprobefn(bits, index) ((((bits) ^ ((bits) >> 8)) & min(63, index)) | 1)
#define Fn16bits(a, b) (((a) + (b)) & 0x0ffff)
#define Hashingbits(item) (HILOC(item) ^ (((LOLOC(item) & 0x1fff) << 3) ^ (LOLOC(item) >> 9)))
#define Getikvalue(base, index) (*(LispPTR *)NativeAligned4FromLAddr((base) + ((index) << 1)))

#ifndef BYTESWAP
typedef struct implicit_key_hash_table {
  LispPTR base;
  unsigned last_index : 16;
  unsigned num_slots : 16;
  unsigned num_keys : 16;
  unsigned null_slots : 16;
  LispPTR key_accessor;
} Ikhashtbl;
#else
typedef struct implicit_key_hash_table {
  LispPTR base;
  unsigned num_slots : 16;
  unsigned last_index : 16;
  unsigned null_slots : 16;
  unsigned num_keys : 16;
  LispPTR key_accessor;
} Ikhashtbl;
#endif

extern unsigned int oplength[256];
#ifdef BIGVM
/* Table of opcode lengths for 4-byte atom opcode cases */
#define LONGEST_OPCODE 5
unsigned int oplength[256] = {
    0, 0, 0, 0, 0, 1, 4, 2, 4, 4, 4, 4, 4, 5, 0, 0, 0, 2, 0, 0, 1, 1, 0, 4, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 2, 9, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    4, 0, 1, 1, 0, 0, 0, 4, 0, 0, 0, 0, 1, 1, 2, 4, 9, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 4, 0, 1, 1, 0, 1, 1, 2, 9, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0};
#elif defined(BIGATOMS)
/* Table of opcode lengths for 3-byte atom opcode cases */
#define LONGEST_OPCODE 4
unsigned int oplength[256] = {
    0, 0, 0, 0, 0, 1, 3, 2, 3, 3, 3, 3, 3, 4, 0, 0, 0, 2, 0, 0, 1, 1, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 2, 9, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    3, 0, 1, 1, 0, 0, 0, 3, 0, 0, 0, 0, 1, 1, 2, 3, 9, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 3, 0, 1, 1, 0, 1, 1, 2, 9, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0};
#else
/* Table of opcode lengths for old, 2-byte atom opcodes. */
#define LONGEST_OPCODE 3
unsigned int oplength[256] = {
    0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 3, 0, 0, 0, 2, 0, 0, 1, 1, 0, 2, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 2, 9, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 1, 1, 2, 3, 9, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 2, 0, 1, 1, 0, 1, 1, 2, 9, 0, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0};
#endif /* BIGATOMS */

typedef ByteCode *InstPtr;

/************************************************************************/
/*									*/
/*		    m a p _ c o d e _ p o i n t e r s			*/
/*									*/
/*	Find all pointers in a block of compiled code (they're the	*/
/*	the args to the GCONST opcode), and change the reference	*/
/*	count according to "casep".  Complains if it hits an unknown	*/
/*	opcode.								*/
/*									*/
/************************************************************************/

static LispPTR map_code_pointers(LispPTR codeblock, short int casep) {
  InstPtr codeptr;
  unsigned int opnum;
  unsigned int len;
  struct fnhead *fnbase;
  fnbase = (struct fnhead *)NativeAligned4FromLAddr(codeblock);
  codeptr = ((InstPtr)fnbase) + fnbase->startpc;

#ifdef RESWAPPEDCODESTREAM
  if (!fnbase->byteswapped) byte_swap_code_block(fnbase);
#endif

  while (T) {
    switch (opnum = Get_code_BYTE(codeptr)) {
      case ENDOFX: /* -X- */ return (NIL);
      case GCONST: /* GCONST */
#ifdef BIGVM
      {
        LispPTR reclaimed = (Get_code_BYTE(codeptr + 1) << 24) |
                            (Get_code_BYTE(codeptr + 2) << 16) | (Get_code_BYTE(codeptr + 3) << 8) |
                            Get_code_BYTE(codeptr + 4);
#else
      {
        LispPTR reclaimed = (Get_code_BYTE(codeptr + 1) << 16) | (Get_code_BYTE(codeptr + 2) << 8) |
                            Get_code_BYTE(codeptr + 3);
#endif /* BIGVM */
        if (reclaimed != codeblock)
        /*			      {htfind(reclaimed, casep);} */
        {
          REC_GCLOOKUP(reclaimed, casep);
        }
      }
    }
    if ((len = oplength[opnum]) >
        LONGEST_OPCODE) { /* len > biggest possible marks an unknown opcode */
      char errtext[200];
      sprintf(errtext,
              "Unrecognized bytecode (0%o) at offset 0%to in code block x%x,x%x; continue to use "
              "UFN length",
              opnum, codeptr - (InstPtr)fnbase, (codeblock >> 16) & 0xFF, codeblock & 0xFFFF);
      error(errtext);
      oplength[opnum] = len = (((UFN *)UFNTable) + (opnum))->byte_num;
    }
    codeptr += len + 1;
  }
}

/************************************************************************/
/*									*/
/*		    r e m i m p l i c i t k e y h a s h			*/
/*									*/
/*	Remove a fn defn from the implicit-key hash table of defns	*/
/*									*/
/************************************************************************/

/* JRB - These values are xpointers; their high bytes are not set and
        shouldn't be looked at */
#define getikkey(value) ((*(LispPTR *)NativeAligned4FromLAddr(value)) & POINTERMASK)

static LispPTR remimplicitkeyhash(LispPTR item, LispPTR ik_hash_table) {
  Ikhashtbl *ik_htable;
  LispPTR reprobe, bits, limits, index, base, value;
  ik_htable = (Ikhashtbl *)NativeAligned4FromLAddr(ik_hash_table);
  bits = Hashingbits(item);
  limits = ik_htable->last_index;
  index = (bits & limits);
  base = ik_htable->base;
  value = Getikvalue(base, index);
  if (value != *Deleted_Implicit_Hash_Slot_word) {
    if (value != NIL) {
      if (item == getikkey(value)) { goto found; }
    } else
      return (NIL);
  }
  reprobe = Reprobefn(bits, limits);
lp:
  index = Fn16bits(index, reprobe) & limits;
  value = Getikvalue(base, index);
  if (value != *Deleted_Implicit_Hash_Slot_word) {
    if (value != NIL) {
      if (item == getikkey(value)) { goto found; }
    } else
      return (NIL);
  }
  goto lp;
found:
  /*
      htfind(*Deleted_Implicit_Hash_Slot_word, ADDREF);
      htfind(Getikvalue(base, index), DELREF);
  */
  REC_GCLOOKUP(*Deleted_Implicit_Hash_Slot_word, ADDREF);
  REC_GCLOOKUP(Getikvalue(base, index), DELREF);
  Getikvalue(base, index) = *Deleted_Implicit_Hash_Slot_word;
  (ik_htable->num_keys)--;
  return (T);
}

/************************************************************************/
/*									*/
/*		    r e c l a i m c o d e b l o c k			*/
/*									*/
/*	Reclaim an array block that contains compiled code.  When	*/
/*	this happens, we need to decrement the reference counts for	*/
/*									*/
/*		* The frame name, which may be a string, list, etc.	*/
/*		* Any GCONSTs in the code (constants, internal fns,	*/
/*		  etc.), since they're no longer needed.		*/
/*									*/
/************************************************************************/

LispPTR reclaimcodeblock(LispPTR codebase) {
  struct fnhead *fnbase;
  if ((*Closure_Cache_Enabled_word != NIL) &&
      (remimplicitkeyhash(codebase, *Closure_Cache_word) != NIL)) {
    return (T);
  }
  fnbase = (struct fnhead *)NativeAligned4FromLAddr(codebase);
  REC_GCLOOKUP((POINTERMASK & fnbase->framename), DELREF);
  if (fnbase->startpc != 0) map_code_pointers(codebase, DELREF);
  return (NIL);
}

/************************************************************************/
/*									*/
/*		    c o d e _ b l o c k _ s i z e			*/
/*									*/
/*	Given a native pointer to a code block, return its size.	*/
/*									*/
/************************************************************************/

int code_block_size(long unsigned int codeblock68k) {
  InstPtr codeptr, initcodeptr;
  unsigned int opnum;
  unsigned int len;
  struct fnhead *fnbase;
  fnbase = (struct fnhead *)codeblock68k;
  initcodeptr = codeptr = ((InstPtr)fnbase) + fnbase->startpc;
  while (T) {
    switch (opnum = Get_BYTE(codeptr)) {
      case ENDOFX: /* -X- */ return (codeptr - initcodeptr);
    }
    if ((len = oplength[opnum]) >
        LONGEST_OPCODE) { /* len > biggest possible marks an unknown opcode */
      char errtext[200];
      sprintf(errtext,
              "Unrecognized bytecode (0%o) at offset 0%to in code block x%x,x%x; continue to use "
              "UFN length",
              opnum, codeptr - (InstPtr)fnbase, (int)((codeblock68k >> 16) & 0xFF), (int)(codeblock68k & 0xFFFF));
      error(errtext);
      oplength[opnum] = len = (((UFN *)UFNTable) + (opnum))->byte_num;
    }
    codeptr += len + 1;
  }
}
