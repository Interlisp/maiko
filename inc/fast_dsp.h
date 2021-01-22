#ifndef FAST_DSP_H
#define FAST_DSP_H 1
/* $Id: fast_dsp.h,v 1.2 1999/01/03 02:05:59 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* 	These are the Macros Used to generate fast dispatch inline code. 
*/


/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1991 Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	  F A S T   O P C O D E   D I S P A T C H   M A C R O S		*/
/*									*/
/*	These macros generate better opcode-dispatch code than the	*/
/*	native compiler will.  The difference may be only one or	*/
/*	two instructions, but in the inner loop, that's a LOT.		*/
/*									*/
/*	To add a new architecture, you must define 5 macros:		*/
/*									*/
/*		nextop0 - for single-byte opcodes			*/
/*		nextop1 - skip a byte and grab an opcode		*/
/*		nextop2 - skip 2 bytes and grab an opcode		*/
/*		nextop3 - skip 3 bytes and grab an opcode		*/
/*		nextop4 - skip 4 bytes and grab an opcode		*/
/*									*/
/*	(These macros are already defined naively, so undef them.)	*/
/*									*/
/*	For existing implementations, these often expand out to		*/
/*	calls to inline functions.					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/


#ifdef OPDISP	/* Only do any of this if OPDISP is set. */

/* We only need to redefine nextop0 as the others build upon it. */
#undef nextop0

#define nextop0 goto *optable[Get_BYTE_PCMAC0]

#endif /* OPDISP */

#endif /* FAST_DSP_H */
