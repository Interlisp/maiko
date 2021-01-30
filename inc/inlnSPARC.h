/* $Id: inlnSPARC.h,v 1.2 1999/01/03 02:06:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/



/* 	inlinedefsSPARC.h */
/* 	Bob Krivacic, Steve Purcell  */

/* 	These are the Macros Used to generate inline SPARC assembly code 
	to implement the opcodes.  Used by both dispatch 
	loop and native code.
*/

/* this is *very* carefully written to generate optimal sun4 assembly code. Be very careful modifying it if you must do so. scp*/

/*
	Get_BYTE(PCMAC+1);			\
xc.c:		#define PCMAC pccache
xc.c:		register InstPtr pccache;
xc.c:		typedef ByteCode *InstPtr;  CHANGED TO
xc.c:		typedef BYTECODE *InstPtr;
lispemul.h:	typedef char ByteCode;
lispemul.h:	#define Get_BYTE(byteptr)	(((BYTECODE *)(byteptr))->code)
lispemul.h:	typedef struct {unsigned code : 8;} BYTECODE;
 */

#define NSMALLP(x) (((x) >> 17) ^ 7)
#define NSMALLP_RANGE(x) (((x << 15) >>15) ^ x)

#define UNBOX_SMALLP(sour)  ( ((int)sour << 15) >> 15 )

#define HARD_CASE(fn) {SV; TOPOFSTACK = fn(POP_TOS_1, TOPOFSTACK, ufn_2); nextop1;}

#define UNBOX_ELSE(sour, dest, otherwise){			\
	if (NSMALLP(sour))					\
	    if (GetTypeNumber(sour) != TYPE_FIXP) otherwise;	\
	    else dest = FIXP_VALUE(sour);			\
	else dest = UNBOX_SMALLP(sour);}

#define UNBOX_ELSE_UFN(sour, dest) UNBOX_ELSE(sour, dest, goto op_ufn);

#define BOX_INTO(result, dest){					\
	if (NSMALLP_RANGE(result))/* dest = box_fixp(result);*/	\
		{register LispPTR *wordp; 			\
		wordp = (LispPTR *) createcell68k(TYPE_FIXP);	\
		*((int *)wordp) = result;			\
		dest = (LADDR_from_68k(wordp));	}		\
	else dest = (( (unsigned)result <<15) >>15) | S_POSITIVE;}


#define ARITH_OP(op, exceptions, handler) 			\
	{register int arg1, arg2, result;			\
	arg1 = GET_TOS_1;		 /* w/o side effect */	\
	if(!NSMALLP(TOPOFSTACK) && !NSMALLP(arg1)) {		\
		arg2 = UNBOX_SMALLP(TOPOFSTACK);		\
		arg1 = UNBOX_SMALLP(arg1);			\
		result = arg1 op arg2;				\
		BOX_INTO(result, TOPOFSTACK);			\
		POP_TOS_1;					\
		nextop1;}					\
	N_OP_CALL_2(handler); }

#define BINARY_OP(exp, exceptions) {				\
	register int arg1, arg2, result;			\
	arg1 = GET_TOS_1;		 /* w/o side effect */	\
	UNBOX_ELSE_UFN(TOPOFSTACK, arg2);			\
	UNBOX_ELSE_UFN(arg1, arg1);				\
	result = exp;						\
	if (exceptions) goto op_ufn;				\
	BOX_INTO(result, TOPOFSTACK);				\
	POP_TOS_1; nextop1;}

#undef GREATERP 
#undef IGREATERP

#define GREATERP {						\
	register int arg1, arg2, result;			\
	arg1 = GET_TOS_1;		 /* w/o side effect */	\
	UNBOX_ELSE_UFN(TOPOFSTACK, arg2);			\
	UNBOX_ELSE_UFN(arg1, arg1);				\
	TOPOFSTACK = (arg1>arg2? ATOM_T : NIL_PTR);		\
	POP_TOS_1; nextop1;}

#define IGREATERP {						\
	register int arg1, arg2, result;			\
	arg1 = GET_TOS_1;		 /* w/o side effect */	\
	UNBOX_ELSE_UFN(TOPOFSTACK, arg2);			\
	UNBOX_ELSE_UFN(arg1, arg1);				\
	TOPOFSTACK = (arg1>arg2? ATOM_T : NIL_PTR);		\
	POP_TOS_1; nextop1;}

#define OVERFLOW(a,b,r) (   (int)((r^a) & (a^~b))    < 0)
#define SOVERFLOW(a,b,r) (   (int)((r^a) & (a^b))    < 0)
/* overflow is ((arg1>0) == (arg2>0)) && ((result>0) != (arg1>0)) */ 

#undef IPLUS2
#undef PLUS2
#define	IPLUS2		ARITH_OP(+, OVERFLOW(arg1,arg2,result), N_OP_iplus2)
#define	PLUS2		ARITH_OP(+, OVERFLOW(arg1,arg2,result), N_OP_plus2)

#undef IDIFFERENCE 
#undef DIFFERENCE 
#define	IDIFFERENCE	ARITH_OP(-, SOVERFLOW(arg1,arg2,result), N_OP_idifference)
#define	DIFFERENCE	ARITH_OP(-, SOVERFLOW(arg1,arg2,result), N_OP_difference)

#undef LOGOR
#undef LOGAND
#undef LOGXOR

#define LOGOR		BINARY_OP(arg1 | arg2, 0)
#define LOGAND		BINARY_OP(arg1 & arg2, 0)
#define LOGXOR		BINARY_OP(arg1 ^ arg2, 0)

#define UNARY_OP(exp, exceptions) {				\
	register int arg, result; 						\
	UNBOX_ELSE_UFN(TOPOFSTACK, arg);			\
	result = exp;						\
	if (exceptions) goto op_ufn;				\
	BOX_INTO(result, TOPOFSTACK);				\
	nextop1;}

#undef LRSH8
#undef LRSH1
#define LRSH8	UNARY_OP((unsigned)arg >> 8, 0)
#define LRSH1	UNARY_OP((unsigned)arg >> 1, 0)

#undef LLSH8
#undef LLSH1 
#define LLSH8	UNARY_OP(arg << 8, ((arg >> 24)!=0))
#define LLSH1	UNARY_OP(arg << 1, (arg < 0))

#undef ADDBASE
#define ADDBASE {						\
	register int arg1, arg2;				\
	UNBOX_ELSE_UFN(TOPOFSTACK, arg2);			\
	TOPOFSTACK = POP_TOS_1 + arg2;				\
	nextop1;}
