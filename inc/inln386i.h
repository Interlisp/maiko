/* $Id: inln386i.h,v 1.2 1999/01/03 02:06:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*			inlinedefs386i.c				*/
/*									*/
/*	Dispatch-loop macro definitions specific to the Sun386i		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


	/* use the swapx [inline] function to swap words in a dword */
#undef SWAP_WORDS
#define SWAP_WORDS(x) swapx(x)


	/* undefine these macros so we use the 386i inline code */
#undef Get_BYTE_PCMAC0
#undef Get_BYTE_PCMAC1
#undef Get_BYTE_PCMAC2
#undef Get_BYTE_PCMAC3

#undef Get_DLword_PCMAC0
#undef Get_DLword_PCMAC1
#undef Get_DLword_PCMAC2
#undef Get_DLword_PCMAC3

#define Get_BYTE_PCMAC0 Get_BYTE_PCMAC0fn()
#define Get_BYTE_PCMAC1 Get_BYTE_PCMAC1fn()
#define Get_BYTE_PCMAC2 Get_BYTE_PCMAC2fn()
#define Get_BYTE_PCMAC3 Get_BYTE_PCMAC3fn()

#define Get_DLword_PCMAC0 Get_DLword_PCMAC0fn()
#define Get_DLword_PCMAC1 Get_DLword_PCMAC1fn()
#define Get_DLword_PCMAC2 Get_DLword_PCMAC2fn()
#define Get_DLword_PCMAC3 Get_DLword_PCMAC3fn()

#undef DIFFERENCE
#undef PLUS2
#undef IDIFFERENCE
#undef IPLUS2

#define DIFFERENCE {						\
	fast_op_difference(POP_TOS_1);				\
	nextop1;						\
diff_err:							\
	asm("diff_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_POPPED_CALL_2(N_OP_difference, GET_POPPED);	\
	}
#define IDIFFERENCE {						\
	fast_op_idifference(POP_TOS_1);				\
	nextop1;						\
idiff_err:							\
	asm("idiff_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_POPPED_CALL_2(N_OP_idifference, GET_POPPED);	\
	}
#define IDIFFERENCE_N(n) {					\
	fast_op_idifferencen(n);				\
	nextop2;						\
idiffn_err:							\
	asm("idiffn_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_CALL_1d(N_OP_idifferencen, n);			\
	}

#define PLUS2 {							\
	fast_op_plus(POP_TOS_1);				\
	nextop1;						\
plus_err:							\
	asm("plus_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_POPPED_CALL_2(N_OP_plus2, GET_POPPED);		\
	}

#define IPLUS2 {						\
	fast_op_iplus(POP_TOS_1);				\
	nextop1;						\
iplus_err:							\
	asm("iplus_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_POPPED_CALL_2(N_OP_iplus2, GET_POPPED);		\
	}
#define IPLUS_N(n) {						\
	fast_op_iplusn(n);					\
	nextop2;						\
iplusn_err:							\
	asm("iplusn_err:");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	asm("popl %ecx");					\
	N_OP_CALL_1d(N_OP_iplusn, n);				\
	}

#undef GREATERP 
#define	GREATERP {						\
	fast_op_greaterp(POP_TOS_1);				\
	nextop1;						\
greaterp_err:							\
	asm("greaterp_err: popl	%ecx");					\
	N_OP_POPPED_CALL_2(N_OP_greaterp, GET_POPPED);		\
	}

#undef IGREATERP
#define	IGREATERP {						\
	fast_op_igreaterp(POP_TOS_1);				\
	nextop1;						\
igreaterp_err:							\
	asm("igreaterp_err: popl	%ecx");			\
	N_OP_POPPED_CALL_2(N_OP_igreaterp, GET_POPPED);		\
	}


#undef LRSH8
#define LRSH8 {							\
	fast_op_lrsh8();					\
	nextop1;						\
lrsh8_err:							\
	asm("lrsh8_err: ");					\
	N_OP_CALL_1(N_OP_lrsh8);				\
	}

#undef LRSH1
#define LRSH1 {							\
	fast_op_lrsh1();					\
	nextop1;						\
lrsh1_err:							\
	asm("lrsh1_err: ");					\
	N_OP_CALL_1(N_OP_lrsh1);				\
	}

#undef LLSH8
#define LLSH8 {							\
	fast_op_llsh8();					\
	nextop1;						\
llsh8_err:							\
	asm("llsh8_err: ");					\
	N_OP_CALL_1(N_OP_llsh8);				\
	}

#undef LLSH1
#define LLSH1 {							\
	fast_op_llsh1();					\
	nextop1;						\
llsh1_err:							\
	asm("llsh1_err: ");					\
	N_OP_CALL_1(N_OP_llsh1);				\
	}


#undef LOGOR
#define LOGOR { 						\
	fast_op_logor(POP_TOS_1);				\
	nextop1;						\
logor_err:							\
	asm("logor_err: popl	%ecx");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logor, GET_POPPED);		\
	}
 
#undef LOGAND
#define LOGAND {						\
	fast_op_logand(POP_TOS_1);				\
	nextop1;						\
logand_err:							\
	asm("logand_err: popl	%ecx");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logand, GET_POPPED);		\
	}

#undef LOGXOR
#define LOGXOR { 						\
	fast_op_logxor(POP_TOS_1);				\
	nextop1;						\
logxor_err:							\
	asm("logxor_err: popl	%ecx");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_logxor, GET_POPPED);		\
	}

#undef N_OP_ADDBASE 
#define N_OP_ADDBASE {						\
	fast_op_addbase(POP_TOS_1);				\
	nextop1;						\
addbase_err:							\
	asm("addbase_err: popl	%ecx");					\
	asm("addw $7,%bx");					\
	asm("rorl $15,%ebx");					\
	N_OP_POPPED_CALL_2(N_OP_addbase, GET_POPPED);		\
	}


#undef N_OP_LOLOC
#define N_OP_LOLOC	{ fast_op_loloc(); nextop1; }
#undef N_OP_HILOC
#define N_OP_HILOC	{ fast_op_hiloc(); nextop1; }

#undef N_OP_VAG2
#define N_OP_VAG2						\
  {								\
    asm("	subl	$4,%esi");				\
    asm("	movl	(%esi),%eax");				\
    asm("	roll	$16,%ebx");				\
    asm("	movw	%ax,%bx");				\
    asm("	rorl	$16,%ebx");				\
    nextop1; }
