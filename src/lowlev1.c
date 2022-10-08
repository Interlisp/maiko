/* $Id: lowlev1.c,v 1.3 1999/05/31 23:35:38 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"       // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#include "emlglob.h"
#include "lispemul.h"     // for LispPTR, state, DLword, POINTERMASK, ERROR_...
#include "lispmap.h"      // for S_POSITIVE
#include "lowlev1defs.h"  // for N_OP_getbitsnfd, N_OP_putbasen, N_OP_putbas...
#include "lspglob.h"
#include "lsptypes.h"     // for GETWORD

static const int mask_array[16] = {
    1,     3,     7,     0xf,   0x1f,   0x3f,   0x7f,   0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/************************************************************
 N_OP_putbitsnfd
        entry		PUTBITS.N.FD		OPCODE[0317]

        1.	<<Enter>>
                TopOfStack: NewValue
                *((int *)(CurrentStackPTR)):	 base address.
                alpha:	word offset.
                beta:		High nibble -- number of the first bit of the field
                                Low nibble  -- (number of the size of the fiel) - 1
        2.	Data is @[TopOfStack + alpha]
        3.	Shift and mask the data.
        4.	Set the data at [TopOfStack + alpha].
        5.	<<Exit>>
                return:	??

***********************************************************/
LispPTR N_OP_putbitsnfd(LispPTR base, LispPTR data, int word_offset,
                        int beta) {
  DLword *pword;
  int shift_size, field_size, fmask;

#ifdef CHECK
  if (base > POINTERMASK) { error("getbits: base out of range"); }
  if (beta > 0xFF) { error("bad beta argument to PUTBITS"); }
#endif

  if ((SEGMASK & data) != S_POSITIVE) { ERROR_EXIT(data); }

  pword = NativeAligned2FromLAddr(base + word_offset);
  field_size = 0xF & beta;
  shift_size = 15 - (beta >> 4) - field_size;
  fmask = mask_array[field_size] << shift_size;
  GETWORD(pword) = ((data << shift_size) & fmask) | (GETWORD(pword) & (~fmask));

  return (base);
}

/************************************************************
 N_OP_getbitsnfd
        entry		GETBITS.N.FD		OPCODE[0312]

        1.	<<Enter>>
                TopOfStack: base address.
                alpha:	word offset.
                beta:	High nibble -- number of the first bit of the field
                        Low nibble  -- (number of the size of the field) - 1
        2.	Data is @[TopOfStack + alpha]
        3.	Shift and mask the data.
        4.	<<Exit>>
                return:		hi - S_POSITIVE
                                lo - mask & shifted data

***********************************************************/
LispPTR N_OP_getbitsnfd(int base_addr, int word_offset, int beta) {
  DLword *pword;
  short first;
  short size;

  pword = NativeAligned2FromLAddr(base_addr + word_offset);
  size = 0xF & beta;
  first = beta >> 4;

#ifdef CHECK
  if (base_addr > POINTERMASK) { error("getbits: base out of range"); }
  if (first + size > 15) { error("getbits beta too big"); }
#endif

  return (S_POSITIVE | ((GETWORD(pword) >> (16 - (first + size + 1))) & mask_array[size]));
}

/************************************************************
 N_OP_putbasen
        entry		PUTBASE.N		OPCODE[0315]

        1.	<<Enter>>
                TopOfStack:	Low Word - replace data
                *(CurrentStackPTR): base address.
                alpha: offset.
        2.	if high word of TopOFStack is not SMALLPL,
                then call ufn2incS,
                else replace (base address + offset) with data.
        3.	increment PC by 2.
        4.	<<Exit>>
                return:	base address.
                (Called only by Native code)
***********************************************************/

LispPTR N_OP_putbasen(LispPTR base, LispPTR tos, int n) {
  base = POINTERMASK & base;
  if ((SEGMASK & tos) != S_POSITIVE) {
    ERROR_EXIT(tos);
  } else {
    GETWORD(NativeAligned2FromLAddr(base + n)) = GetLoWord(tos);
    return (base);
  }
}

/************************************************************
 N_OP_putbaseptrn
        entry		PUTBASEPTR.N		OPCODE[0316]

        1.	<<Enter>>
                TopOfStack:	replace data (2 words)
                *(CurrentStackPTR): base address.
                alpha: offset.
        2.	replace (base address + offset) with data.
        3.	save base address to TopOfStack
        3.	increment PC by 2.
        4.	<<Exit>>
                return:	base address.
                (Called only by Native code)
***********************************************************/

LispPTR N_OP_putbaseptrn(LispPTR base, LispPTR tos, int n) {
  base = POINTERMASK & base;
  *NativeAligned4FromLAddr(base + n) = tos & POINTERMASK;
  return (base);
}
