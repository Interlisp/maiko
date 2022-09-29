/* $Id: lowlev2.c,v 1.3 1999/05/31 23:35:38 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
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
#include "lispemul.h"     // for state, LispPTR, ERROR_EXIT, SEGMASK, POINTE...
#include "lispmap.h"      // for S_POSITIVE, S_NEGATIVE
#include "lowlev2defs.h"  // for N_OP_addbase, N_OP_getbasebyte, N_OP_putbas...
#include "lspglob.h"
#include "lsptypes.h"     // for GETBYTE, GetTypeNumber, TYPE_FIXP

/*** NOTE: these routines likely not called (see inlinedefsC.h) ***/

/************************************************************
 N_OP_addbase
        entry		ADDBASE		OPCODE[0320]

        1.	<<Enter>>
                TopOfStack: offset
                *(CurrentStackPTR): base address
        2.	if High word of TopOfStack is SMALLPL or SMALLNEG,
                then add base address and offset and set result to TopOfStack.
                else call ufn2incs.
        4.	<<Exit>>
                return: new address

***********************************************************/

LispPTR N_OP_addbase(LispPTR base, LispPTR offset) {
  base = POINTERMASK & base;
  switch ((SEGMASK & offset)) {
    case S_POSITIVE: return (base + (offset & 0x0000FFFF));
    case S_NEGATIVE: return (base + (offset | 0xFFFF0000));
    default:
      switch ((GetTypeNumber(offset))) {
        case TYPE_FIXP:
          /* overflow or underflow isn't check */
          return (base + *(int *)NativeAligned4FromLAddr(offset));
        default: /* floatp also */ ERROR_EXIT(offset);
      } /* end switch */
  }     /* end switch */
}

/************************************************************
 N_OP_getbasebyte
        entry		GETBASEBYTE		OPCODE[0302]

        1.	<<Enter>>
                *(--CurrentStackPTR): base address.
                TopOfStack:	Low word - byte offset.
        2.	if high word of TopOfStack is not SMALLPL,
                then	call ufn2incS.
                else	fetch 8 bits word at (base address + byte offset).
        4.	<<Exit>>
                return:		Least Low Byte - fetched data

***********************************************************/

LispPTR N_OP_getbasebyte(LispPTR base_addr, LispPTR byteoffset) {
  switch ((SEGMASK & byteoffset)) {
    case S_POSITIVE: byteoffset = byteoffset & 0x0000FFFF; break;
    case S_NEGATIVE: byteoffset = byteoffset | 0xFFFF0000; break;
    default:
      switch ((GetTypeNumber(byteoffset))) {
        case TYPE_FIXP: byteoffset = *((int *)NativeAligned4FromLAddr(byteoffset)); break;
        default: /* floatp also fall thru */ ERROR_EXIT(byteoffset);
      } /* end switch */
      break;
  } /* end switch */
  return ((0xFF & (GETBYTE((char *)NativeAligned2FromLAddr((POINTERMASK & base_addr)) + byteoffset))) |
          S_POSITIVE);
}

/************************************************************
 N_OP_putbasebyte
        entry		PUTBASEBYTE		OPCODE[0307]

        1.	<<Enter>>
                TopOfStack:	Least Low Byte - replace data.
                *((int *)(CurrentStackPTR-1)): byte offset.
                *((int *)(CurrentStackPTR-2)): base address.
        4.	<<Exit>>
                return:		Least Low Byte - replace data ?

***********************************************************/

LispPTR N_OP_putbasebyte(LispPTR base_addr, LispPTR byteoffset, LispPTR tos) {
  if (((SEGMASK & tos) != S_POSITIVE) || ((unsigned short)tos >= 256)) ERROR_EXIT(tos);
  switch ((SEGMASK & byteoffset)) {
    case S_POSITIVE: byteoffset &= 0x0000FFFF; break;
    case S_NEGATIVE: byteoffset |= 0xFFFF0000; break;
    default:
      /* ucode and ufn don't handle displacement not smallp */
      ERROR_EXIT(tos);
  } /* end switch */
  GETBYTE(((char *)NativeAligned2FromLAddr(POINTERMASK & base_addr)) + byteoffset) = 0xFF & tos;
  return (tos);
}
