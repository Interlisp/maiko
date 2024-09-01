#ifndef MY_H
#define MY_H 1
/* @(#) my.h Version 2.15 (2/8/93). copyright venue   */

/************************************************************************/
/*									*/
/*				M Y . H					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include <sys/types.h>   // for u_char
#include "version.h"     // for BIGVM
#include "adr68k.h"      // for NativeAligned2FromLAddr, NativeAligned4FromLAddr, LAddrFromNative
#include "arith.h"       // for N_ARITH_SWITCH, N_GETNUMBER
#include "gchtfinddefs.h"// for htfind, rec_htfind, from gcdata.h macros
#include "gcdata.h"	 // for ADDREF, DELREF
#include "lispemul.h"    // for ERROR_EXIT, LispPTR, DLword, SEGMASK, state
#include "lispmap.h"     // for S_POSITIVE, S_CHARACTER, S_NEGATIVE
#include "lspglob.h"
#include "lsptypes.h"    // for GETBYTE, GETWORD, GetTypeNumber, TYPE_FLOATP
#include "mkcelldefs.h"  // for createcell68k

/************************************************************************/
/*									*/
/*			N _ M a k e F l o a t				*/
/*									*/
/*	Get a numeric argument as a floating-point number, converting	*/
/*	from SMALLP or FIXP, if necessary.				*/
/*									*/
/************************************************************************/
#define	N_MakeFloat(arg, dest, tos) do {				\
	switch (SEGMASK & (LispPTR)(arg)) {				\
	case S_POSITIVE:						\
		(dest) = (float)(0xFFFF & (LispPTR)(arg));		\
		break;							\
	case S_NEGATIVE:						\
		(dest) = (float)((int)(0xFFFF0000 | (LispPTR)(arg)));	\
		break;							\
	default:							\
		switch (GetTypeNumber(arg)) {				\
		  case TYPE_FLOATP: 					\
		    (dest) = *((float *)NativeAligned4FromLAddr(arg));	\
		    break; 						\
		  case TYPE_FIXP: 					\
		    (dest) = (float)(*((int *)NativeAligned4FromLAddr(arg))); \
		    break;						\
		  default: ERROR_EXIT(tos);				\
		}							\
	}								\
} while (0)



#define	N_GetPos(arg, dest, tos) do {					\
	if (((arg) & SEGMASK) == S_POSITIVE)				\
		(dest) = (arg) & 0xFFFF;				\
	else	{							\
	if (GetTypeNumber(arg) != TYPE_FIXP) ERROR_EXIT(tos);		\
	(dest) = *((int *)NativeAligned4FromLAddr(arg));		\
	if ((unsigned)(dest) & 0x80000000) ERROR_EXIT(tos);		\
	}								\
} while (0)

static inline LispPTR
aref_switch(unsigned type, LispPTR tos, LispPTR baseL, int index)
{
  switch (type)
    {
    case 38: /* pointer : 32 bits */
      return(*(((LispPTR *)NativeAligned4FromLAddr(baseL)) + index));

    case 20: { /* signed : 16 bits */
      DLword result = (GETWORD(((DLword *)NativeAligned2FromLAddr(baseL)) + index));
      if (result & 0x8000) return(result | S_NEGATIVE);
      else return(result | S_POSITIVE);
    }

    case 67: /* Character :  8 bits */
      return(S_CHARACTER | ((GETBYTE(((char *)NativeAligned2FromLAddr(baseL)) + index)) & 0xFF));

    case 22: {/* signed : 32 bits */
      int result = *(((int *)NativeAligned4FromLAddr(baseL)) + index);
      N_ARITH_SWITCH(result);
    }
    case 0: /* unsigned : 1 bit per element */
      return(S_POSITIVE | (((GETBYTE(((char *)NativeAligned2FromLAddr(baseL)) + (index >> 3))) >> (7 - (index & 7))) & 1));

    case 3: /* unsigned : 8 bits per element */
      return(S_POSITIVE | ((GETBYTE(((char *)NativeAligned2FromLAddr(baseL)) + index)) & 0xFF));

    case 4: /* unsigned : 16 bits per element */
      return(S_POSITIVE | ((GETWORD(((DLword *)NativeAligned2FromLAddr(baseL)) + index)) & 0xFFFF));

    case 54: /* Float : 32 bits */
      {
        float *cellp = (float *)createcell68k(TYPE_FLOATP);
        *cellp = *(float *)(NativeAligned4FromLAddr(baseL) + index);
        return (LAddrFromNative(cellp));
      }

    case 68: /* Character :  16 bits */
      return(S_CHARACTER | ((GETWORD(((DLword *)NativeAligned2FromLAddr(baseL)) + index)) & 0xFFFF));

    case 86: /* XPointer : 32 bits */
      return(*(((LispPTR *)NativeAligned4FromLAddr(baseL)) + index));

   default: /* Illegal or Unimplemented */
      ERROR_EXIT(tos);
    }/* end switch typenumber */
}

#define aset_switch(type, tos)						\
do {									\
   switch (type) {							\
      case 38: /* pointer : 32 bits */					\
	GCLOOKUP(*(((LispPTR *)NativeAligned4FromLAddr(base)) + index), DELREF); \
	GCLOOKUP(data, ADDREF);						\
        *(((LispPTR *)NativeAligned4FromLAddr(base)) + index) = data;	\
        return(data);							\
      case 20: { /* signed : 16 bits */					\
        DLword new = data & 0xFFFF;					\
        if ((((data & SEGMASK) == S_POSITIVE) && ((data & 0x8000) == 0)) ||\
	    (((data & SEGMASK) == S_NEGATIVE) && (data & 0x8000)))	\
        GETWORD(((DLword *)NativeAligned2FromLAddr(base)) + index) = new;		\
        else ERROR_EXIT(tos);						\
        return(data);							\
        }								\
      case 67: { /* Character :  8 bits */				\
        DLword new = data & 0xFFFF;					\
        if ((data & SEGMASK) != S_CHARACTER) ERROR_EXIT(tos);		\
        if (new > 0xFF) ERROR_EXIT(tos);				\
        GETBYTE(((char *)NativeAligned2FromLAddr(base)) + index) = (u_char)new; \
        return(data);							\
      }									\
      case 22: { /* signed : 32 bits */					\
	int new;							\
        N_GETNUMBER(data, new, doufn);					\
        *(((int *)NativeAligned4FromLAddr(base)) + index) = new;	\
        return(data);							\
      }									\
      case 0: { /* unsigned : 1 bit per element */			\
        int new;							\
        N_GetPos(data, new, tos);					\
        if (new > 1) ERROR_EXIT(tos);					\
        if (new) {							\
          new = (1 << (7 - (index & 7)));				\
          GETBYTE(((u_char *)NativeAligned2FromLAddr(base)) + (index >> 3)) |= (u_char)new; \
        }								\
        else {								\
          new = 0xFF - (1 << (7 - (index & 7)));			\
          GETBYTE(((u_char *)NativeAligned2FromLAddr(base)) + (index >> 3)) &= (u_char)new; \
        }								\
        return(data);							\
      }									\
      case 3: { /* unsigned : 8 bits per element */			\
        int new;							\
        N_GetPos(data, new, tos);					\
        if (new > 0xFF) ERROR_EXIT(tos);				\
        GETBYTE(((char *)NativeAligned2FromLAddr(base)) + index) = (u_char)new; \
        return(data);							\
      }									\
      case 4: { /* unsigned : 16 bits per element */			\
        int new;							\
        N_GetPos(data, new, tos); 					\
        if (new > 0xFFFF) ERROR_EXIT(tos);				\
        GETWORD(((DLword *)NativeAligned2FromLAddr(base)) + index) = (DLword)new; \
        return(data);							\
      }									\
      case 54: /* Float : 32 bits */					\
        if (GetTypeNumber(data) != TYPE_FLOATP) ERROR_EXIT(tos);	\
        *(((int *)NativeAligned4FromLAddr(base)) + index) = *((int *)NativeAligned4FromLAddr(data)); \
        return(data);							\
      case 68: {/* Character :  16 bits */				\
        DLword new;							\
        if ((data & SEGMASK) != S_CHARACTER) ERROR_EXIT(tos); 		\
        new = data & 0xFFFF;						\
        GETWORD(((DLword *)NativeAligned2FromLAddr(base)) + index) = new; \
        return(data);							\
      }									\
      case 86: /* XPointer : 32 bits */					\
        *(((LispPTR *)NativeAligned4FromLAddr(base)) + index) = data;	\
        return(data);							\
      default: /* Illegal or Unimplemented */				\
        ERROR_EXIT(tos);						\
    }/* end switch typenumber */					\
} while (0)
#endif /* MY_H */
