#ifndef ADDRESS_H
#define ADDRESS_H 1
/* $Id: address.h,v 1.2 1999/01/03 02:05:51 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/**********************************************************************/
/*
 		File Name :	address.h

		Address Manipulate Macros(for LISP pointer)

 					Date :		December 8, 1986
 					Edited by :	Takeshi Shimizu
 					Changed :	Dec.22.86 (take)
					Changed :	Jan.14.87(take)
					Changed :	Apr.20.87(mitani)
							Sep.02.87 take
							(add parenthesises)
*/
/**********************************************************************/

/* NOTE: These MACRO should be used for the pointers in LISP SYSOUT */
#define LLSH(datum, n)			((datum) << (n))
#define LRSH(datum, n)			((datum) >> (n))

#define HILOC(ptr)			(LRSH(((unsigned int)(ptr) & SEGMASK),16))
#define LOLOC(ptr)			((unsigned int)(ptr) & 0x0ffff)

#define VAG2(hi,lo)			(LispPTR)(LLSH((hi),16) | (lo))


/* NOTE: argument off must be WORD offset */
#define ADDBASE(ptr,off)		((UNSIGNED)(ptr) + (off))
#define GETBASE(ptr,off)		(GETWORD(((DLword *)(ptr)) + (off)))



/* Following MACRO defs. is related with POINTER which is defined as ACCESSFNS in Interlisp(LLNEW)  */
#ifdef BIGVM
#define POINTER_PAGE(datum)	(((unsigned int)(datum) & 0x0fffff00) >> 8 )
#define POINTER_SEGMENT(datum)		HILOC(datum)
#define POINTER_PAGEBASE(datum)	((datum) & 0x0fffff00)
#else
#define POINTER_PAGE(datum)	(((unsigned int)(datum) & 0x0ffff00) >> 8 )
#define POINTER_SEGMENT(datum)		HILOC(datum)
#define POINTER_PAGEBASE(datum)	((datum) & 0x0ffff00)
#endif /* BIGVM */
#endif /* ADDRESS_H */
