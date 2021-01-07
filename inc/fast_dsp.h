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

	/* Sun 3 */
#ifdef mc68020
#undef nextop0
#undef nextop1
#undef nextop2
#undef nextop3
#undef nextop4
/* JRB - fast case is now nextop1 */
#define nextop0 { fast0_dispatcher(); goto nextopcode; }
#define nextop1 { fast1_dispatcher(); goto nextopcode; }
#define nextop2 { fast1_dispatcher2(); }
#define nextop3 { PCMACL += 2; nextop1; }
#define nextop4 { PCMACL += 3; nextop1; }
#endif

	/* Sun 386i */
#ifdef I386
#undef nextop0
#undef nextop1
#undef nextop2
#undef nextop3
#undef nextop4
/* JRB - fast case is now nextop1 */
#define nextop0 { fast0_dispatcher(); goto nextopcode; }
#define nextop1 { fast1_dispatcher(); goto nextopcode; }
#define nextop2 { fast2_dispatcher(); }
#define nextop3 { PCMACL += 2; nextop1; }
#define nextop4 { PCMACL += 3; nextop1; }
#endif


	/* ISC 386 using gcc */
#ifdef ISC
#undef nextop0
#undef nextop1
#undef nextop2
#undef nextop3
#undef nextop4

#define nextop0 { fast0_dispatcher(); goto nextopcode; }
#define nextop1 { fast1_dispatcher(); goto nextopcode; }
#define nextop2 { fast2_dispatcher(); goto nextopcode;}
#define nextop3 { fast3_dispatcher(); goto nextopcode;}
#define nextop4 { fast4_dispatcher(); goto nextopcode;}

#define fast0_dispatcher() \
    asm volatile("						\n\
//	leal	-1(%0),%%eax					\n\
/	xorb	$3,%%al						\n\
/	movzbl	(%%eax),%%eax					\n\
	movzbl	-1(%0),%%eax \n\
	jmp	*optable(,%%eax,4)" : : "r" (pccache): "ax");

#define fast1_dispatcher() \
    asm volatile("						\n\
/	movl	%0,%%eax					\n\
/	xorb	$3,%%al						\n\
	movzbl	(%0),%%eax \n\
	incl	%0						\n\
/	movzbl	(%%eax),%%eax					\n\
	jmp	*optable(,%%eax,4)" : "=r" (pccache) : "0" (pccache): "ax");

#define fast2_dispatcher() \
    asm volatile("						\n\
/	leal	1(%0),%%eax					\n\
	movzbl	1(%0),%%eax \n\
	addl	$2,%0						\n\
/	xorb	$3,%%al						\n\
/	movzbl	(%%eax),%%eax					\n\
	jmp	*optable(,%%eax,4)" :"=r" (pccache) : "0" (pccache): "ax");

#define fast3_dispatcher() \
    asm volatile("						\n\
/	leal	2(%0),%%eax					\n\
	movzbl	2(%0),%%eax \n\
	addl	$3,%0						\n\
/	xorb	$3,%%al						\n\
/	movzbl	(%%eax),%%eax					\n\
	jmp	*optable(,%%eax,4)" :"=r" (pccache) : "0" (pccache): "ax");
#define fast4_dispatcher() \
    asm volatile("						\n\
/	leal	3(%0),%%eax					\n\
	movzbl	3(%0),%%eax \n\
	addl	$4,%0						\n\
/	xorb	$3,%%al						\n\
/	movzbl	(%%eax),%%eax					\n\
	jmp	*optable(,%%eax,4)" :"=r" (pccache) : "0" (pccache): "ax");
	
#endif

#endif /* OPDISP */





	/* ISC 386 using gcc with turbo assembler (DOS version) */
#ifdef DOS
#undef nextop0
#undef nextop1
#undef nextop2
#undef nextop3
#undef nextop4

#define nextop0 { fast0_dispatcher(); goto nextopcode; }
#define nextop1 { fast1_dispatcher(); goto nextopcode; }
#define nextop2 { fast2_dispatcher(); goto nextopcode;}
#define nextop3 { fast3_dispatcher(); goto nextopcode;}
#define nextop4 { fast4_dispatcher(); goto nextopcode;}

#define fast0_dispatcher() \
    asm volatile("						\n\
	lea	eax,-1[%0]					\n\
	xor	al,3						\n\
	movzx	eax, BYTE PTR [eax]					\n\
;	mov	dword ptr save_opcode, eax	\n\
;	mov dword ptr save_pc, esi	\n\
;	mov dword ptr save_tos, ebx	\n\
;	mov	dword ptr save_tsptr, edi	\n\
;	mov dword ptr save_esp, esp		\n\
;	mov	dword ptr save_ebp, ebp		\n\
	jmp	OFFSET CODE32:optable[eax*4]" : "=r" (pccache) : "0" (pccache): "ax");

#define fast1_dispatcher() \
    asm volatile("						\n\
	mov	eax,%0					\n\
	xor	al,3						\n\
	inc	%0						\n\
	movzx	eax, BYTE PTR [eax]					\n\
;	mov	dword ptr save_opcode, eax	\n\
;	mov dword ptr save_pc, esi	\n\
;	mov dword ptr save_tos, ebx	\n\
;	mov	dword ptr save_tsptr, edi	\n\
;	mov dword ptr save_esp, esp		\n\
;	mov	dword ptr save_ebp, ebp		\n\
	jmp	OFFSET CODE32:optable[eax*4]" : "=r" (pccache) : "0" (pccache): "ax");

#define fast2_dispatcher() \
    asm volatile("						\n\
	lea	eax,1[%0]					\n\
	add	%0,2						\n\
	xor	al,3						\n\
	movzx	eax, BYTE PTR [eax]					\n\
;	mov	dword ptr save_opcode, eax	\n\
;	mov dword ptr save_pc, esi	\n\
;	mov dword ptr save_tos, ebx	\n\
;	mov	dword ptr save_tsptr, edi	\n\
;	mov dword ptr save_esp, esp		\n\
;	mov	dword ptr save_ebp, ebp		\n\
	jmp	OFFSET CODE32:optable[eax*4]" : "=r" (pccache) : "0" (pccache): "ax");

#define fast3_dispatcher() \
    asm volatile("						\n\
	lea	eax,2[%0]					\n\
	add	%0,3						\n\
	xor	al,3						\n\
	movzx	eax, BYTE PTR [eax]					\n\
;	mov	dword ptr save_opcode, eax	\n\
;	mov dword ptr save_pc, esi	\n\
;	mov dword ptr save_tos, ebx	\n\
;	mov	dword ptr save_tsptr, edi	\n\
;	mov dword ptr save_esp, esp		\n\
;	mov	dword ptr save_ebp, ebp		\n\
	jmp	OFFSET CODE32:optable[eax*4]" : "=r" (pccache) : "0" (pccache): "ax");

#define fast4_dispatcher() \
    asm volatile("						\n\
	lea	eax,3[%0]					\n\
	add	%0,4						\n\
	xor	al,3						\n\
	movzx	eax, BYTE PTR [eax]					\n\
;	mov	dword ptr save_opcode, eax	\n\
;	mov dword ptr save_pc, esi	\n\
;	mov dword ptr save_tos, ebx	\n\
;	mov	dword ptr save_tsptr, edi	\n\
;	mov dword ptr save_esp, esp		\n\
;	mov	dword ptr save_ebp, ebp		\n\
	jmp	OFFSET CODE32:optable[eax*4]" : "=r" (pccache) : "0" (pccache): "ax");
	
#endif /* DOS */







#ifdef SPARCDISP
#undef nextop0
#undef nextop1
#undef nextop2
#undef nextop3
#undef nextop4
#define nextop0 { fast_dispatcher(table, Get_BYTE(PCMAC)); goto nextopcode; }
#define nextop_n(n) {							\
	PCMACL += n;							\
	nextop0; 							\
}
#define nextop1 { nextop_n(1); }
#define nextop2 { nextop_n(2); }
#define nextop3 { nextop_n(3); }
#define nextop4 { nextop_n(4); }
#endif /* SPARCDISP */









#endif /* FAST_DSP_H */
