/* $Id: inlnPS2.h,v 1.2 1999/01/03 02:06:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	(C) Copyright 1991, 1992 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*			   I N L I N E P S 2 . H			*/
/*									*/
/*	INLINE definitions for 386 machines, compiled with gcc.		*/
/*									*/
/*	This file consists of 3 sections:				*/
/*									*/
/*		inline static functions for use anywhere in Medley	*/
/*		(e.g., the byte-swapping functions)			*/
/*									*/
/*		#defines and static inline functions for the dispatch	*/
/*		loop (e.g., IDIFFERENCE), relying on the register	*/
/*		conventions that hold in that part of the code		*/
/*									*/
/*		#defines and static inline functions for other		*/
/*		specific files (e.g., the arithmetic functions,		*/
/*		free-variable lookup, etc.), relying on the register	*/
/*		conventions in the respective files.			*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*		   R E G I S T E R   C O N V E N T I O N S		*/
/*									*/
/*	The following register conventions hold in the dispatch loop,	*/
/*	and are set up by register ... asm("...") decls in xc.c:	*/
/*									*/
/*		esi	pccache	- the current PC			*/
/*		edi	cspcache - the current lisp stack ptr.		*/
/*		ebx	tscache - the top-of-stack item.		*/
/*									*/
/*									*/
/*	Rgister conventions within arithmetic functions in the files	*/
/*	arith2.c - arith4.c, etc.:					*/
/*									*/
/*		esi	first argument to the function			*/
/*		edi	second argument to the function			*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*    G E N E R A L - P U R P O S E   I N L I N E   F U N C T I O N S	*/
/*									*/
/*	These functions don't rely on register conventions.		*/
/*									*/
/************************************************************************/


#undef SWAP_WORDS
#define SWAP_WORDS swapx


extern inline const unsigned int swapx (unsigned int word)
 {
    asm("roll	$16,%0" : "=g" (word) : "0" (word));
    return(word);
 }



extern inline const unsigned int word_swap_longword (unsigned int word)
 {
    asm("roll	$16,%0" : "=r" (word) : "0" (word));

    return(word);
  }



extern inline const unsigned short byte_swap_word (unsigned short word)
 {
    asm("rolw	$8,%0" : "=r" (word) : "0" (word));

    return(word);
  }



extern inline void word_swap_page(unsigned short * page, int count)
  {
   asm volatile("\
	pushl %edi					\n\
	pushl %esi					\n\
	pushl %ecx					\n\
	cld");
    asm volatile("\
	movl %0,%%esi	// word pointer.		\n\
	movl %%esi,%%edi				\n\
	movl %1,%%ecx	// count" : : "g" (page), "g" (count));
    asm volatile("\
	lodsl						\n\
	rolw	$8,%ax					\n\
	roll	$16,%eax				\n\
	rolw	$8,%ax					\n\
	stosl						\n\
	loop	.-13					\n\
							\n\
	// epilogue.					\n\
	popl %ecx					\n\
	popl %esi					\n\
	popl %edi					\
	");

  }



/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

	/* undefine these macros so we use the 386i inline code */

#undef Get_BYTE_PCMAC0
#undef Get_BYTE_PCMAC1
#undef Get_BYTE_PCMAC2
#undef Get_BYTE_PCMAC3

#define Get_BYTE_PCMAC0 Get_BYTE_PCMAC0fn(pccache)
#define Get_BYTE_PCMAC1 Get_BYTE_PCMAC1fn(pccache)
#define Get_BYTE_PCMAC2 Get_BYTE_PCMAC2fn(pccache)
#define Get_BYTE_PCMAC3 Get_BYTE_PCMAC3fn(pccache)

extern inline const unsigned int Get_BYTE_PCMAC0fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("leal	-1(%1),%0 \n\
	xorl	$3,%0 \n\
	movzbl	(%0),%0 \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline const unsigned int Get_BYTE_PCMAC1fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("movl	%1,%0 \n\
	xorl	$3,%0 \n\
	movzbl	(%0),%0 \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline const unsigned int Get_BYTE_PCMAC2fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("leal	1(%1),%0 \n\
	xorl	$3,%0 \n\
	movzbl	(%0),%0 \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline const unsigned int Get_BYTE_PCMAC3fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("leal	2(%1),%0 \n\
	xorl	$3,%0 \n\
	movzbl	(%0),%0 \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }


#undef Get_DLword_PCMAC0
#undef Get_DLword_PCMAC1
#undef Get_DLword_PCMAC2
#undef Get_DLword_PCMAC3


#define Get_DLword_PCMAC0 Get_DLword_PCMAC0fn(pccache)
#define Get_DLword_PCMAC1 Get_DLword_PCMAC1fn(pccache)
#define Get_DLword_PCMAC2 Get_DLword_PCMAC2fn(pccache)
#define Get_DLword_PCMAC3 Get_DLword_PCMAC3fn(pccache)

extern inline const unsigned int Get_DLword_PCMAC0fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("movl	%1,%%edx \n\
	xorb	$3,%%dl \n\
	movzbl	(%%edx),%%eax \n\
	leal	-1(%1),%%edx	\n\
	xorb	$3,%%dl	\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline const unsigned int Get_DLword_PCMAC1fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	1(%1),%%edx \n\
	xorb	$3,%%dl \n\
	movzbl	(%%edx),%%eax \n\
	leal	(%1),%%edx	\n\
	xorb	$3,%%dl	\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }


extern inline const unsigned int Get_DLword_PCMAC2fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	2(%1),%%edx \n\
	xorb	$3,%%dl \n\
	movzbl	(%%edx),%%eax \n\
	leal	1(%1),%%edx	\n\
	xorb	$3,%%dl	\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline const unsigned int Get_DLword_PCMAC3fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	3(%1),%%edx \n\
	xorb	$3,%%dl \n\
	movzbl	(%%edx),%%eax \n\
	leal	2(%1),%%edx	\n\
	xorb	$3,%%dl	\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }


#undef Get_Pointer_PCMAC0
#undef Get_Pointer_PCMAC1
#undef Get_Pointer_PCMAC2

#define Get_Pointer_PCMAC0 Get_Pointer_PCMAC0fn(pccache)
#define Get_Pointer_PCMAC1 Get_Pointer_PCMAC1fn(pccache)
#define Get_Pointer_PCMAC2 Get_Pointer_PCMAC2fn(pccache)


extern inline const unsigned int Get_Pointer_PCMAC0fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	-1(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movzbl	(%%edx),%%eax	\n\
	shll	$16,%%eax	\n\
	leal	1(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%al	\n\
	leal	(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline const unsigned int Get_Pointer_PCMAC1fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movzbl	(%%edx),%%eax	\n\
	shll	$16,%%eax	\n\
	leal	2(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%al	\n\
	leal	1(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }


extern inline const unsigned int Get_Pointer_PCMAC2fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("leal	1(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movzbl	(%%edx),%%eax	\n\
	shll	$16,%%eax	\n\
	leal	3(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%al	\n\
	leal	2(%1),%%edx	\n\
	xorb	$3,%%dl		\n\
	movb	(%%edx),%%ah	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }



#undef DIFFERENCE
#undef IDIFFERENCE

#define DIFFERENCE {						\
	fast_op_difference(POP_TOS_1);				\
	fast1_dispatcher();						\
diff_err:							\
	asm volatile("diff_err:");					\
	asm volatile("addb $7,%bl");					\
	asm volatile("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_difference, GET_POPPED);	\
	}

extern inline void fast_op_difference(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl	\n\
	jne	diff_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	diff_err	\n\
	subl	%%ebx,%%eax	\n\
	jo	diff_err	\n\
	rorl	$15,%%eax	\n\
	orl	$917504,%%eax	\n\
	movl	%%eax,%%ebx	\
    " :  : "g" (value) : "ax" );

  }

#define IDIFFERENCE {						\
	fast_op_idifference(POP_TOS_1);				\
	fast1_dispatcher();						\
idiff_err:							\
	asm volatile("idiff_err:");				\
	asm volatile("addb $7,%bl");					\
	asm volatile("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_idifference, GET_POPPED);	\
	}

extern inline void fast_op_idifference(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl	\n\
	jne	idiff_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	idiff_err	\n\
	subl	%%ebx,%%eax	\n\
	jo	idiff_err	\n\
	rorl	$15,%%eax	\n\
	orl	$917504,%%eax	\n\
	movl	%%eax,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }

#undef IDIFFERENCE_N
#define IDIFFERENCE_N(n) {					\
	fast_op_idifferencen(n);				\
	fast2_dispatcher();						\
idiffn_err:							\
	asm("idiffn_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	N_OP_CALL_1d(N_OP_idifferencen, n);			\
	}

extern inline void fast_op_idifferencen(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl		\n\
	jne	idiffn_err	\n\
	subl	%%eax,%%ebx	\n\
	jo	idiffn_err	\n\
	rorl	$15,%%ebx	\n\
	orl	$917504,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }

#undef PLUS2
#undef IPLUS2

#define PLUS2 {							\
	fast_op_plus(POP_TOS_1);				\
	fast1_dispatcher();						\
plus_err:							\
	asm volatile("plus_err:");					\
	asm volatile("addw $7,%bx");					\
	asm volatile("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_plus2, GET_POPPED);		\
	}
extern inline void fast_op_plus(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl	\n\
	jne	plus_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	plus_err	\n\
	addl	%%ebx,%%eax	\n\
	jo	plus_err	\n\
	rorl	$15,%%eax	\n\
	orl	$917504,%%eax	\n\
	movl	%%eax,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }


#define IPLUS2 {						\
	fast_op_iplus(POP_TOS_1);				\
	fast1_dispatcher();						\
iplus_err:							\
	asm volatile("iplus_err:");					\
	asm volatile("addw $7,%bx");					\
	asm volatile("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_iplus2, GET_POPPED);		\
	}
extern inline void fast_op_iplus(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl	\n\
	jne	iplus_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	iplus_err	\n\
	addl	%%ebx,%%eax	\n\
	jo	iplus_err	\n\
	rorl	$15,%%eax	\n\
	orl	$917504,%%eax	\n\
	movl	%%eax,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }


#undef IPLUS_N
#define IPLUS_N(n) {						\
	fast_op_iplusn(n);					\
	fast2_dispatcher();						\
iplusn_err:							\
	asm("iplusn_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	N_OP_CALL_1d(N_OP_iplusn, n);				\
	}

extern inline void fast_op_iplusn(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%eax	\n\
	roll	$15,%%ebx	\n\
	subb	$7,%%bl	\n\
	jne	iplusn_err	\n\
	addl	%%ebx,%%eax	\n\
	jo	iplusn_err	\n\
	rorl	$15,%%eax	\n\
	orl	$917504,%%eax	\n\
	movl	%%eax,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }


#undef GREATERP 
#define	GREATERP {						\
	fast_op_greaterp(POP_TOS_1);				\
	fast1_dispatcher();						\
greaterp_err:							\
	asm("greaterp_err:");					\
	N_OP_POPPED_CALL_2(N_OP_greaterp, GET_POPPED);		\
	}

extern inline void fast_op_greaterp(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	movl	%%ebx,%%edx	\n\
	roll	$15,%%edx	\n\
	subb	$7,%%dl	\n\
	jne	greaterp_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	greaterp_err	\n\
	xorl	%%ebx,%%ebx	\n\
	cmpl	%%edx,%%eax	\n\
	jle	greater_no	\n\
	movl	$76,%%ebx	\n\
greater_no:    " :  : "g" (value)  );

  }


#undef IGREATERP
#define	IGREATERP {						\
	fast_op_igreaterp(POP_TOS_1);				\
	fast1_dispatcher();						\
igreaterp_err:							\
	asm("igreaterp_err: ");			\
	N_OP_POPPED_CALL_2(N_OP_igreaterp, GET_POPPED);		\
	}

extern inline void fast_op_igreaterp(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	movl	%%ebx,%%edx	\n\
	roll	$15,%%edx	\n\
	subb	$7,%%dl	\n\
	jne	igreaterp_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	igreaterp_err	\n\
	xorl	%%ebx,%%ebx	\n\
	cmpl	%%edx,%%eax	\n\
	jle	igreater_no	\n\
	movl	$76,%%ebx	\n\
igreater_no:    " : : "g" (value)  );

  }


#undef LRSH8
#define LRSH8 {							\
    asm volatile("\
	movl	%%ebx,%%eax	\n\
	roll	$16,%%eax	\n\
	cmpw	$0xe,%%ax	\n\
	jne	lrsh8_err	\n\
	shrw	$8,%%bx	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
lrsh8_err:							\
	asm("lrsh8_err: ");					\
	N_OP_CALL_1(N_OP_lrsh8);				\
	}

#undef LRSH1
#define LRSH1 {							\
    asm volatile("\
	movl	%%ebx,%%eax	\n\
	roll	$16,%%eax	\n\
	cmpw	$0xe,%%ax	\n\
	jne	lrsh1_err	\n\
	shrw	$1,%%bx	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
lrsh1_err:							\
	asm("lrsh1_err: ");					\
	N_OP_CALL_1(N_OP_lrsh1);				\
	}

#undef LLSH8
#define LLSH8 {							\
    asm volatile("\
	movl	%%ebx,%%eax	\n\
	roll	$16,%%eax	\n\
	cmpw	$0xe,%%ax	\n\
	jne	llsh8_err	\n\
	shlw	$8,%%bx	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
llsh8_err:							\
	asm("llsh8_err: ");					\
	N_OP_CALL_1(N_OP_llsh8);				\
	}

#undef LLSH1
#define LLSH1 {							\
    asm volatile("\
	movl	%%ebx,%%eax	\n\
	roll	$16,%%eax	\n\
	cmpw	$0xe,%%ax	\n\
	jne	llsh1_err	\n\
	shlw	$1,%%bx	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
llsh1_err:							\
	asm("llsh1_err: ");					\
	N_OP_CALL_1(N_OP_llsh1);				\
	}


#undef LOGOR
#define LOGOR { 						\
	fast_op_logor(POP_TOS_1);				\
	fast1_dispatcher();						\
logor_err:							\
	asm("logor_err:");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logor, GET_POPPED);		\
	}

extern inline void fast_op_logor(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	cmpb	$7,%%bl	\n\
	jne	logor_err	\n\
	roll	$15,%%eax	\n\
	cmpb	$7,%%al	\n\
	jne	logor_err	\n\
	orl	%%eax,%%ebx	\n\
	rorl	$15,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }

 
#undef LOGAND
#define LOGAND {						\
	fast_op_logand(POP_TOS_1);				\
	fast1_dispatcher();						\
logand_err:							\
	asm("logand_err: ");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logand, GET_POPPED);		\
	}

extern inline void fast_op_logand(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	cmpb	$7,%%bl	\n\
	jne	logand_err	\n\
	roll	$15,%%eax	\n\
	cmpb	$7,%%al	\n\
	jne	logand_err	\n\
	andl	%%eax,%%ebx	\n\
	rorl	$15,%%ebx	\n\
    " : : "g" (value) : "ax" );

  }


#undef LOGXOR
#define LOGXOR { 						\
	fast_op_logxor(POP_TOS_1);				\
	fast1_dispatcher();						\
logxor_err:							\
	asm("logxor_err:");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logxor, GET_POPPED);		\
	}

extern inline void fast_op_logxor(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	cmpb	$7,%%bl	\n\
	jne	logxor_err	\n\
	roll	$15,%%eax	\n\
	subb	$7,%%al	\n\
	jne	logxor_err	\n\
	xorl	%%eax,%%ebx	\n\
	rorl	$15,%%ebx	\n\
    " :  : "g" (value) : "ax" );

  }



#undef N_OP_ADDBASE 
#define N_OP_ADDBASE {						\
	fast_op_addbase(POP_TOS_1);				\
	fast1_dispatcher();						\
addbase_err:							\
	asm("addbase_err: ");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_addbase, GET_POPPED);		\
	}
extern inline void fast_op_addbase(LispPTR value)
  {
    asm volatile("\
	movl	%0,%%eax	\n\
	roll	$15,%%ebx	\n\
	cmpb	$7,%%bl	\n\
	jne	addbase_err	\n\
	sarl	$15,%%ebx	\n\
	andl	$0xFFFFFF,%%eax	\n\
	addl	%%eax,%%ebx	\n\
    " : : "g" (value) : "ax" );

  }




#undef N_OP_LOLOC
#define N_OP_LOLOC	{  \
    asm volatile(" \
	andl	$0x0000FFFF,%0 \n\
	orl	$0x000E0000,%0" : "=r" (tscache) : "0" (tscache)); \
     nextop1; }

#undef N_OP_HILOC
#define N_OP_HILOC	{ \
    asm volatile(" \
	shrl	$16,%0	\n\
	andl	$0x0000FFFF,%0 \n\
	orl	$0x000E0000,%0" : "=r" (tscache) : "0" (tscache)); \
     nextop1; }

#undef N_OP_VAG2
#define N_OP_VAG2						\
  {								\
    asm("	subl	$4,%edi");				\
    asm("	movl	(%edi),%eax");				\
    asm("	roll	$16,%ebx");				\
    asm("	movw	%ax,%bx");				\
    asm("	rorl	$16,%ebx");				\
    nextop1; }


/***********/
/* Fast function call */
#define asm_label_check_interrupt() asm volatile("check_interrupt:");

/* * These are the pieces of the function-call code.  lbl is the label
 	to use to defeat the dead-code eliminator, lpfx is the string to
	be used to disambiguate interior labels in the asm code, and nargs
	is the number of args in this FN call (e.g. FN0 => 0) */

#define fn_section1(lbl,lpfx,nargs) \
  struct fnhead *LOCFNCELL;					\
  int defcell_word;						\
  int NEXTBLOCK;						\
  int result;							\
  int RESTARGS;						\
    asm volatile("							\n\
	movl %%esi,%%eax	// fn_atom_index = 			\n\
	xorb $3,%%al		// get_AtomNo_PCMAC1			\n\
	movzbl (%%eax),%%edx						\n\
	sall $16,%%edx							\n\
	leal 1(%%esi),%%eax						\n\
	xorb $3,%%al							\n\
	movb (%%eax),%%dh						\n\
	leal 2(%%esi),%%eax						\n\
	xorb $3,%%al							\n\
	movb (%%eax),%%dl						\n\
	movl %%edx,%0		// fn_atom_index is in edx now.		\n\
	" : "=g" (fn_atom_index) : : "ax", "cx", "dx");			\

#define fnx_section1(lbl,lpfx,nargs) \
  struct fnhead *LOCFNCELL;					\
  int defcell_word;						\
  int NEXTBLOCK;						\
  int result, num_args;							\
  int RESTARGS;						\
    asm volatile("							\n\
	leal 0(%%esi),%%eax			 			\n\
	xorb $3,%%al							\n\
	movzbl (%%eax),%%edx						\n\
	movl	%%edx,%1	// num_args = Get_Byte_PCMAC1;		\n\
	leal 1(%%esi),%%eax	// fn_atom_index = 			\n\
	xorb $3,%%al		// get_AtomNo_PCMAC1			\n\
	movzbl (%%eax),%%edx						\n\
	sall $16,%%edx							\n\
	leal 2(%%esi),%%eax						\n\
	xorb $3,%%al							\n\
	movb (%%eax),%%dh						\n\
	leal 3(%%esi),%%eax						\n\
	xorb $3,%%al							\n\
	movb (%%eax),%%dl						\n\
	movl %%edx,%0		// fn_atom_index is in edx now.		\n\
	" : "=g" (fn_atom_index), "=g" (num_args) : : "ax", "cx", "dx");			\

#define fn_section1a(lbl,lpfx,nargs)					\
    asm volatile("							\n\
	testl $16711680,%%edx						\n\
	je ." lpfx "118							\n\
	addl %%edx,%%edx	// new atom case			\n\
	addl Lisp_world,%%edx						\n\
	addl $8,%%edx							\n\
	jmp ." lpfx "119						\n\
	.align 4							\n\
." lpfx "118:								\n\
	sall $2,%%edx							\n\
	addl Defspace,%%edx						\n\
." lpfx "119:								\n\
	movl %%edx,%0	// to -92					\n\
	" : "=g" (fn_defcell) : "g" (fn_atom_index) : "ax", "cx", "dx"); \
									\
    asm volatile("							\n\
	movl (%%edx),%%eax						\n\
	movl %%eax,%0	// to -32 (must remain, it's used by C below)	\n\
	testl %%eax,%%eax	// if defcell_word>0			\n\
	jnl ." lpfx "120						\n\
	testl $0x4000000,%%eax	/ check for needs-byte-swapping		\n\
	jnz  ." lpfx "swap	/ if so, go do it out of line.		\n\
	" : "=g" (defcell_word) : "g" (fn_defcell) : "ax", "dx");	\
lbl:									\
    asm volatile("							\n\
/	.align 4	/ this isn't usually the target of a branch.	\n\
	andl $16777215,%%eax						\n\
	movl %%eax,%1							\n\
	addl %%eax,%%eax						\n\
	addl Lisp_world,%%eax						\n\
	movl %%eax,%0		// to -80				\n\
	" : "=g" (LOCFNCELL), "=g" (defcell_word) : : "cx");		\
									\
    asm volatile("							\n\
	movw MachineState+20,%%dx	// currentfx pc =		\n\
	incw %%dx							\n\
	movl %%esi,%%ecx		// PCMAC -			\n\
	subw %%dx,%%cx		// funcobj				\n\
	addw $4,%%cx		// + opcode size			\n\
	movl MachineState+4,%%edx					\n\
	movw %%cx,-12(%%edx)	// save in frame			\n\
	movzwl 2(%%eax),%%edx						\n\
	addl %%edx,%%edx						\n\
	movl MachineState+32,%%eax					\n\
	subl %%edx,%%eax						\n\
	movl %%eax,MachineState+28 // Irq_Stk_Check =			\n\
	cmpl %%eax,%%edi						\n\
	jg check_interrupt	//goto check_interrupt			\n\
	" :  : "g" (LOCFNCELL) : "ax", "dx", "cx");

#define fn_section2(lpfx) \
    asm volatile("							\n\
	movl %%ecx,MachineState						\n\
	subl Stackspace,%%ecx						\n\
	shrl $1,%%ecx	// NEXTBLOCK in -88				\n\
	movl %%ecx,%0							\n\
	movl MachineState+4,%%edx					\n\
	movw %%cx,-10(%%edx)	// currentfx->nextblock =		\n\
	movl %%ebx,(%%edi)	// HARD_PUSH tos			\n\
	addl $4,%%edi							\n\
	" : "=g" (NEXTBLOCK) : : "ax", "dx", "cx");			\
									\
    asm volatile("							\n\
	movl %1,%%ecx	// if LOCFNCELL->na				\n\
	cmpw $0,(%%ecx)		//  >= 0				\n\
	jl ." lpfx "122							\n\
				// then					\n\
	movswl (%%ecx),%%ecx	// RESTARGS = #args(0)			\n\
	" : "=g" (RESTARGS) : "g" (LOCFNCELL) : "ax", "dx", "cx");

	/* Between these sections comes some code that subtracts */
	/* the true arg count from %ecx.  It's safe to use eax if you need it */

#define fn_section3(lpfx,nargs) 					\
    asm volatile("							\n\
	jle ." lpfx "124	// if < 0, skip loop.			\n\
	.align 4							\n\
." lpfx "125:			// while RESTARGS<0			\n\
	xorl %%eax,%%eax		// storing NILs			\n\
	cld			// so stosl increments edi		\n\
	rep				// repeat for ecx count		\n\
	stosl				// store them.			\n\
." lpfx "124:								\n\
	sall $2,%%ecx		//   RESTARGS				\n\
	addl %%ecx,%%edi	// ecx is <= 0 here....			\n\
	" : "=g" (RESTARGS) : "g" (LOCFNCELL) : "ax", "dx", "cx");	\
									\
    asm volatile("							\n\
." lpfx "122:				// na <0 ??			\n\
	movl %1,%%ecx	// HARDPUSH(BFMARK | NEXTBLOCK)			\n\
	orl $-2147483648,%%ecx						\n\
	movl %%ecx,(%%edi)						\n\
	movl MachineState+4,%%edx	// * CSTKPTR =			\n\
	subl Stackspace,%%edx	// FXMARK<<16 |				\n\
	shrl $1,%%edx	// StkOffsetfrom68k(PVAR).			\n\
	orl $-1073741824,%%edx						\n\
	movl %%edx,4(%%edi)						\n\
									\n\
	movl %3,%%eax	// fnheader =					\n\
	roll	$16,%%eax	// swapx(defcell_word)			\n\
	movl %%eax,8(%%edi)						\n\
	leal 24(%%edi),%%edi	// CSTKPTRL = 				\n\
	movl %%edi,MachineState+4 // PVAR = CSTKPTR			\n\
									\n\
	movl %2,%%edx	// result =					\n\
	movswl 6(%%edx),%%ecx	// LOCFNCELL->pv			\n\
	incl %%ecx		// pv is (# of quadwords)-1, so inc it.	\n\
	shll $1,%%ecx		// and * 2 to get 32-bit words worth	\n\
	cld			// so stosl increments edi		\n\
	movl $-1,%%eax		// The UNBOUND_VALUE to be pushed	\n\
	rep								\n\
	stosl								\n\
." lpfx "127:								\n\
	leal 4(%%edi),%%edi	// CSTKPTRL += 1			\n\
	movl %2,%%ecx	// PCMACL = LOCFNCELL				\n\
	movzwl 4(%%ecx),%%ecx	// + LOCFNCELL-> startpc		\n\
	movl %2,%%edx							\n\
	leal 1(%%ecx,%%edx),%%edx					\n\
	movl %%edx,%%esi	//replace above 2 inst			\n\
	movl %2,%%eax	//FuncObj = LOCFNCELL				\n\
	movl %%eax,MachineState+20					\n\
	" : "=&g" (result) : "g" (NEXTBLOCK),  "g" (LOCFNCELL),		\
	"g" (defcell_word) : "ax", "dx", "cx");				\
									\
    fast0_dispatcher();	/* can't be nextop0, or the following */	\
			/* code winds up dead and removed.    */	\
    asm volatile("							\n\
	.align 4							\n\
." lpfx "swap:		/ Come here if we need to byte-swap the fn	\n\
	pushl	%%eax							\n\
	pushl	%%eax							\n\
	call	byte_swap_code_block	/ swap it			\n\
	popl	%%eax							\n\
	popl	%%eax							\n\
	jmp	%l0" : : "" (&&lbl));					\
    asm volatile("							\n\
	.align 4							\n\
." lpfx "swap:		/ Come here if we need to byte-swap the fn	\n\
	call	byte_swap_ 						\n\
." lpfx "120:");


#define fn_section4(nargs)						\
	{ /* it's not a CCODEP */					\
	  fn_num_args = nargs	/* argcount*/;				\
	  fn_opcode_size = FN_OPCODE_SIZE;				\
	  fn_apply = 0;							\
	  goto op_fn_common;						\
	}								\



#undef FN0
#define FN0 \
  {									\
    fn_section1(L0120,"0",0);						\
    fn_section1a(L0120,"0",0);						\
    asm volatile("							\n\
	leal 4(%edi),%ecx	// newivar = CSTKPTR-argcount+1");	\
    fn_section2("0");							\
	/* decl %%ecx's would go here, 1 per arg */			\
    fn_section3("0",0);							\
    fn_section4(0);							\
  }


#undef FN1
#define FN1 \
  {									\
    fn_section1(L1120,"1",1);						\
    fn_section1a(L1120,"1",1);						\
    asm volatile("							\n\
	leal (%edi),%ecx	// newivar = CSTKPTR-argcount+1");	\
    fn_section2("1");							\
    asm volatile("decl %ecx");						\
    fn_section3("1",1);							\
    fn_section4(1);							\
  }

#undef FN2
#define FN2 \
  {									\
    fn_section1(L2120,"2",2);						\
    fn_section1a(L2120,"2",2);						\
    asm volatile("							\n\
	leal -4(%edi),%ecx	// newivar = CSTKPTR-argcount+1");	\
    fn_section2("2");							\
    asm volatile("subl $2,%ecx");					\
    fn_section3("2",2);							\
    fn_section4(2);							\
  }

#undef FN3
#define FN3 \
  {									\
    fn_section1(L3120,"3",3);						\
    fn_section1a(L3120,"3",3);						\
    asm volatile("							\n\
	leal -8(%edi),%ecx	// newivar = CSTKPTR-argcount+1");	\
    fn_section2("3");							\
    asm volatile("subl $3,%ecx");					\
    fn_section3("3",3);							\
    fn_section4(3);							\
  }


#undef FN4
#define FN4 \
  {									\
    fn_section1(L4120,"4",4);						\
    fn_section1a(L4120,"4",4);						\
    asm volatile("							\n\
	leal -12(%edi),%ecx	// newivar = CSTKPTR-argcount+1");	\
    fn_section2("4");							\
    asm volatile("subl $4,%ecx");					\
    fn_section3("4",4);							\
    fn_section4(4);							\
  }

#undef xFNX
#define xFNX \
  {									\
    fnx_section1(Lx120,"x",4);						\
    fn_section1a(Lx120,"x",4);						\
    asm volatile("							\n\
	movl	%0,%%eax	// get num_args				\n\
	leal	-4(,%%eax,4),%%eax					\n\
	negl	%%eax							\n\
	leal (%%edi,%%eax),%%ecx	// newivar = CSTKPTR-argcount+1" : : "g" (num_args));	\
    fn_section2("x");							\
    asm volatile("subl %0,%%ecx" : : "g" (num_args));			\
    fn_section3("x",4);							\
	{ /* it's not a CCODEP */					\
	  fn_num_args = num_args;	/* argcount*/			\
	  fn_opcode_size = FNX_OPCODE_SIZE;				\
	  fn_apply = 0;							\
	  goto op_fn_common;						\
	}								\
  }





/************************************************************************/
/*									*/
/*				o p r e t u r n				*/
/*									*/
/*	Inline version of the RETURN opcode.				*/
/*									*/
/************************************************************************/

inline const extern void opreturn()
  {
    asm volatile("							\n\
	movl MachineState+4,%%edx					\n\
	movzwl -20(%%edx),%%edx					\n\
	testb $1,%%dl					\n\
	jne  opret_fail					\n\
	movl MachineState,%%edi					\n\
	movl Stackspace,%%ecx					\n\
	leal 0(%%ecx,%%edx,2),%%eax					\n\
	movl %%eax,MachineState+4					\n\
	leal -22(%%eax),%%edx					\n\
	xorb $2,%%dl					\n\
	movzwl (%%edx),%%edx					\n\
	leal 0(%%ecx,%%edx,2),%%edx					\n\
	movl %%edx,MachineState					\n\
	movl -16(%%eax),%%ecx					\n\
	roll	$16,%%ecx					\n\
	andl $16777215,%%ecx					\n\
	addl %%ecx,%%ecx					\n\
	addl Lisp_world,%%ecx					\n\
	movl %%ecx,MachineState+20					\n\
	movzwl -12(%%eax),%%eax					\n\
	leal 1(%%eax,%%ecx),%%esi					\n\
	movzwl 2(%%ecx),%%edx					\n\
	addl %%edx,%%edx					\n\
	movl MachineState+24,%%eax				\n\
	subl %%edx,%%eax					\n\
	movl %%eax,MachineState+28				\n\
	cmpl %%eax,%%edi					\n\
	jg  check_interrupt					\n\
	cmpl $0,MachineState+32					\n\
	jle  check_interrupt					\n\
	movl MachineState+24,%%ecx				\n\
	movl %%ecx,MachineState+32				\n\
								\n\
	leal	-1(%%esi),%%eax					\n\
	xorb	$3,%%al						\n\
	movzbl	(%%eax),%%eax					\n\
	jmp	*optable(,%%eax,4)				\n\
					\n\
	.align 4					\n\
opret_fail:" : : : "ax", "dx", "cx");
  }
