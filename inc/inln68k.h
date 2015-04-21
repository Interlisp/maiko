/* $Id: inln68k.h,v 1.2 1999/01/03 02:06:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


/* 	inlinedefs68K.h */
/* 	Bob Krivacic 2/23/88 */

/* 	These are the Macros used to generate inline 68K assembly code 
	to implement the opcodes.  Used by both dispatch 
	loop and native code.
*/

#undef DIFFERENCE
#undef PLUS2
#undef GREATERP
#undef IDIFFERENCE
#undef IPLUS2
#undef IGREATERP
#undef LOGOR
#undef LOGAND
#undef LOGXOR
#undef LRSH8
#undef LRSH1
#undef LLSH8
#undef LLSH1
#undef N_OP_ADDBASE
#undef N_OP_LOLOC
#undef N_OP_HILOC
#undef N_OP_VAG2
#undef LISTP
#undef NTYPEX
#undef TYPEP
#undef SWAP_WORDS
/*
#undef FN3
#undef RETURN
*/ 

#define SWAP_WORDS(x) swapx(x)
 
#define DIFFERENCE {						\
	TOPOFSTACK = op_difference(POP_TOS_1, TOPOFSTACK);	\
	nextop1;						\
diff_err:							\
	asm("diff_err:");					\
	N_OP_POPPED_CALL_2(N_OP_difference, GET_POPPED);	\
	}

#define PLUS2 {							\
	TOPOFSTACK = op_plus(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
plus_err:							\
	asm("plus_err:");					\
	N_OP_POPPED_CALL_2(N_OP_plus2, GET_POPPED);		\
	}

#define	GREATERP {						\
	TOPOFSTACK = op_greaterp(POP_TOS_1, TOPOFSTACK);	\
	nextop1;						\
greaterp_err:							\
	asm("greaterp_err:");					\
	N_OP_POPPED_CALL_2(N_OP_greaterp, GET_POPPED);		\
	}

#define IDIFFERENCE {						\
	TOPOFSTACK = op_difference(POP_TOS_1, TOPOFSTACK);	\
	}

#define IPLUS2 {							\
	TOPOFSTACK = op_plus(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
	}

#define	IGREATERP {						\
	TOPOFSTACK = op_greaterp(POP_TOS_1, TOPOFSTACK);	\
	nextop1;						\
	}


#define LOGOR { 						\
	TOPOFSTACK = op_logor(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
logor_err:							\
	asm("logor_err:");					\
	N_OP_POPPED_CALL_2(N_OP_logor, GET_POPPED);		\
	}
 
#define LOGAND {						\
	TOPOFSTACK = op_logand(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
logand_err:							\
	asm("logand_err:");					\
	N_OP_POPPED_CALL_2(N_OP_logand, GET_POPPED);		\
	}

#define LOGXOR {						\
	TOPOFSTACK = op_logxor(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
logxor_err:							\
	asm("logxor_err:");					\
	N_OP_POPPED_CALL_2(N_OP_logxor, GET_POPPED);		\
	}


#define LRSH8 {							\
	TOPOFSTACK = op_lrsh8(TOPOFSTACK);			\
	nextop1;						\
lrsh8_err:							\
	asm("lrsh8_err:");					\
	N_OP_CALL_1(N_OP_lrsh8);				\
	}
	
#define LRSH1 {							\
	TOPOFSTACK = op_lrsh1(TOPOFSTACK);			\
	nextop1;						\
lrsh1_err:							\
	asm("lrsh1_err:");					\
	N_OP_CALL_1(N_OP_lrsh1);				\
	}

#define LLSH8 {							\
	TOPOFSTACK = op_llsh8(TOPOFSTACK);			\
	nextop1;						\
llsh8_err:							\
	asm("llsh8_err:");					\
	N_OP_CALL_1(N_OP_llsh8);				\
	}

#define LLSH1 {							\
	TOPOFSTACK = op_llsh1(TOPOFSTACK);			\
	nextop1;						\
llsh1_err:							\
	asm("llsh1_err:");					\
	N_OP_CALL_1(N_OP_llsh1);				\
	}

#define N_OP_ADDBASE {						\
	TOPOFSTACK = addbase(POP_TOS_1, TOPOFSTACK);		\
	nextop1;						\
	}


#define N_OP_LOLOC	{ TOPOFSTACK = loloc(TOPOFSTACK); nextop1; }
#define N_OP_HILOC	{ TOPOFSTACK = hiloc(TOPOFSTACK); nextop1; }
#define N_OP_VAG2	{ TOPOFSTACK = vag2(POP_TOS_1, TOPOFSTACK); nextop1; }

#define LISTP		{ TOPOFSTACK = listp(TOPOFSTACK); nextop1;}
#define NTYPEX		{ TOPOFSTACK = ntypex(TOPOFSTACK); nextop1; }
#define TYPEP(n)	{ TOPOFSTACK = typep(TOPOFSTACK, Get_BYTE(PCMAC+1)); nextop2; }




