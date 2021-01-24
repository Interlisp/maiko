/* $Id: inlnPS2.h,v 1.2 1999/01/03 02:06:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	(C) Copyright 1991, 1992 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
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
/*	Register conventions within arithmetic functions in the files	*/
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

