#ifndef ADR68K_H
#define ADR68K_H 1
/* $Id: adr68k.h,v 1.2 1999/01/03 02:05:52 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/*
 *	Copyright (C) 1987 by Fuji Xerox Co., Ltd. All rights reserved.
 *
 *	Author	:	Takeshi Shimizu
 *			Hiroshi Hayata
 */

/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/

/**********************************************************************/
/*
		Func name :	adr68k.h
		Translate 68k address to Lisp or Lisp to 68k

		Date :		January 16, 1987
		Create :	Takeshi Shimizu
*/
/**********************************************************************/

#include <stddef.h>
#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"

static inline LispPTR LAddrFromNative(void *NAddr)
{
  if ((uintptr_t)NAddr & 1) {
    printf("Misaligned pointer in LAddrFromNative %p\n", NAddr);
  }
  return ((DLword *)NAddr) - Lisp_world;
}

static inline DLword *NativeAligned2FromLAddr(LispPTR LAddr)
{
  return (Lisp_world + LAddr);
}

static inline LispPTR *NativeAligned4FromLAddr(LispPTR LAddr)
{
  if (LAddr & 1) {
    printf("Misaligned pointer in NativeAligned4FromLAddr 0x%x\n", LAddr);
  }
  return (LispPTR *)(Lisp_world + LAddr);
}

static inline LispPTR *NativeAligned4FromLPage(LispPTR LPage)
{
  return (LispPTR *)(Lisp_world + (LPage << 8));
}

static inline DLword StackOffsetFromNative(void *SAddr)
{
  /* Stack offsets are expressed as an offset in DLwords from the stack base */
  ptrdiff_t hoffset = (DLword *)SAddr - Stackspace;
  if (hoffset > 0xffff) {
    printf("Stack offset is too large: 0x%tx\n", hoffset);
  }
  return (DLword)hoffset;
}

static inline DLword *NativeAligned2FromStackOffset(DLword StackOffset)
{
  return Stackspace + StackOffset;
}

static inline LispPTR *NativeAligned4FromStackOffset(DLword StackOffset)
{
  return (LispPTR *)(Stackspace + StackOffset);
}

static inline LispPTR LPageFromNative(void *NAddr)
{
  if ((uintptr_t)NAddr & 1) {
    printf("Misaligned pointer in LPageFromNative %p\n", NAddr);
  }
  return (((DLword *)NAddr) - Lisp_world) >> 8;
}
/*  translate 68k ptr to Lisp DLword address */
#define LADDR_from_68k(ptr68k)	((LispPTR)(((UNSIGNED)(ptr68k) - (UNSIGNED)Lisp_world) >>1))


/*  translate 68k ptr to Lisp Page number   */
#define LPAGE_from_68k(ptr68k)	(LADDR_from_68k(ptr68k) >> 8)


/* Translate Lisp_address to 68K address	*/
/*	Lisp_addr: word offset		*/
#define Addr68k_from_LADDR(Lisp_addr)	(Lisp_world + (Lisp_addr))


/* translate LispPage to 68k address */
#define Addr68k_from_LPAGE(Lisp_page)	(Addr68k_from_LADDR(((Lisp_page) << 8) ))
/* Stack Offset Macros */

#define StkOffset_from_68K(ptr68k)\
	((LispPTR)(((UNSIGNED)(ptr68k) - (UNSIGNED)Stackspace) >>1))

#define Addr68k_from_StkOffset(stkoffset)\
	(Stackspace + (stkoffset))
#endif /* ADR68K_H */
