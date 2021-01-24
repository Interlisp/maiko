/* $Id: inlndos.h,v 1.2 1999/01/03 02:06:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


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

extern inline  unsigned int Get_BYTE_PCMAC0fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("lea	%0,-1[%1] \n\
	xor	%0,3 \n\
	movzx	%0,BYTE PTR [%0] \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline  unsigned int Get_BYTE_PCMAC1fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("lea	%0,[%1] \n\
	xor	%0,3 \n\
	movzx	%0,BYTE PTR [%0] \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline  unsigned int Get_BYTE_PCMAC2fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("lea	%0,1[%1] \n\
	xor	%0,3 \n\
	movzx	%0,BYTE PTR [%0] \n\
	" : "=r" (word) : "r" (pccache) );
    return(word);
 }

extern inline  unsigned int Get_BYTE_PCMAC3fn (pccache)
unsigned int pccache;
 {
    register unsigned int word;
    asm("lea	%0,2[%1] \n\
	xor	%0,3 \n\
	movzx	%0,BYTE PTR [%0] \n\
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

extern inline  unsigned int Get_DLword_PCMAC0fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("mov	edx,%1 \n\
	xor	dl,3 \n\
	movzx	eax, byte ptr [edx] \n\
	lea	edx,-1[%1]	\n\
	xor	dl,3	\n\
	mov	ah, byte ptr [edx]	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline  unsigned int Get_DLword_PCMAC1fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("lea	edx,1[%1] \n\
	xor	dl,3 \n\
	movzx	eax, byte ptr [edx] \n\
	lea	edx,[%1]	\n\
	xor	dl,3	\n\
	mov	ah, byte ptr [edx]	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }


extern inline  unsigned int Get_DLword_PCMAC2fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("lea	edx,2[%1] \n\
	xor	dl,3 \n\
	movzx	eax, byte ptr [edx] \n\
	lea	edx,1[%1]	\n\
	xor	dl,3	\n\
	mov	ah, byte ptr [edx]	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline  unsigned int Get_DLword_PCMAC3fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("lea	edx,3[%1] \n\
	xor	dl,3 \n\
	movzx	eax, byte ptr [edx] \n\
	lea	edx,2[%1]	\n\
	xor	dl,3	\n\
	mov	ah, byte ptr [edx]	\n\
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
    asm("lea	edx,-1[%1]	\n\
	xor	dl,3		\n\
	movzx	eax,byte ptr [edx]	\n\
	shl	eax,16	\n\
	lea	edx,1[%1]	\n\
	xor	dl,3		\n\
	mov	al,[edx]	\n\
	lea	edx,[%1]	\n\
	xor	dl,3		\n\
	mov	ah,[edx]	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }

extern inline const unsigned int Get_Pointer_PCMAC1fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("lea	edx,[%1]	\n\
	xor	dl,3		\n\
	movzx	eax,byte ptr [edx]	\n\
	shl	eax,16	\n\
	lea	edx,2[%1]	\n\
	xor	dl,3		\n\
	mov	al,[edx]	\n\
	lea	edx,1[%1]	\n\
	xor	dl,3		\n\
	mov	ah,[edx]	\n\
	" : "=r" (word) : "r" (pccache) : "dx" );
    return(word);
 }


extern inline const unsigned int Get_Pointer_PCMAC2fn(pccache)
unsigned int pccache;
 {
    register unsigned int word asm("ax");
    asm("lea	edx,1[%1]	\n\
	xor	dl,3		\n\
	movzx	eax,byte ptr [edx]	\n\
	shl	eax,16	\n\
	lea	edx,3[%1]	\n\
	xor	dl,3		\n\
	mov	al,[edx]	\n\
	lea	edx,2[%1]	\n\
	xor	dl,3		\n\
	mov	ah,[edx]	\n\
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
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_difference, GET_POPPED);	\
	}

extern inline  void fast_op_difference(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	sub	bl,7	\n\
	jne	diff_err	\n\
	rol	eax,17	\n\
	sub	al,7	\n\
	jne	diff_err	\n\
	sub	eax,ebx	\n\
	jo	diff_err	\n\
	ror	eax,15	\n\
	or	eax,917504	\n\
	mov	ebx,eax	\
    " :  : "g" (value) : "ax" );

  }

#define IDIFFERENCE {						\
	fast_op_idifference(POP_TOS_1);				\
	fast1_dispatcher();						\
idiff_err:							\
	asm volatile("idiff_err:");				\
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_idifference, GET_POPPED);	\
	}

extern inline  void fast_op_idifference(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	sub	bl,7	\n\
	jne	idiff_err	\n\
	rol	eax,17	\n\
	sub	al,7	\n\
	jne	idiff_err	\n\
	sub	eax,ebx	\n\
	jo	idiff_err	\n\
	ror	eax,15	\n\
	or	eax,917504	\n\
	mov	ebx,eax	\
    " :  : "g" (value) : "ax" );

  }


#undef IDIFFERENCE_N
#define IDIFFERENCE_N(n) {					\
	fast_op_idifferencen(n);				\
	fast2_dispatcher();						\
idiffn_err:							\
	asm("idiffn_err2:");			\
	asm volatile("add ebx,eax	; undo the sub");	\
	asm("idiffn_err:");					\
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_CALL_1d(N_OP_idifferencen, n);			\
	}

extern inline  void fast_op_idifferencen(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0			\n\
	rol	eax,15			\n\
	rol	ebx,15			\n\
	sub	bl,7			\n\
	jne	idiffn_err			\n\
	sub	ebx,eax			\n\
	jo	idiffn_err2			\n\
	ror	ebx,15			\n\
	or	ebx,917504			\n\
    " :  : "g" (value) : "ax" );

  }

#undef PLUS2
#undef IPLUS2

#define PLUS2 {							\
	fast_op_plus(POP_TOS_1);				\
	fast1_dispatcher();						\
plus_err:							\
	asm volatile("plus_err:");					\
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_plus2, GET_POPPED);		\
	}
extern inline  void fast_op_plus(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	sub	bl,7	\n\
	jne	plus_err	\n\
	rol	eax,15	\n\
	sub	al,7	\n\
	jne	plus_err	\n\
	add	eax,ebx	\n\
	jo	plus_err	\n\
	ror	eax,15	\n\
	or	eax,917504	\n\
	mov	ebx,eax	\n\
    " :  : "g" (value) : "ax" );

  }


#define IPLUS2 {						\
	fast_op_iplus(POP_TOS_1);				\
	fast1_dispatcher();						\
iplus_err:							\
	asm volatile("iplus_err:");					\
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_iplus2, GET_POPPED);		\
	}
extern inline  void fast_op_iplus(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	sub	bl,7	\n\
	jne	iplus_err	\n\
	rol	eax,15	\n\
	sub	al,7	\n\
	jne	iplus_err	\n\
	add	eax,ebx	\n\
	jo	iplus_err	\n\
	ror	eax,15	\n\
	or	eax,917504	\n\
	mov	ebx,eax	\n\
    " :  : "g" (value) : "ax" );

  }


#undef IPLUS_N
#define IPLUS_N(n) {						\
	fast_op_iplusn(n);					\
	fast2_dispatcher();						\
iplusn_err:							\
	asm("iplusn_err:");					\
	asm volatile("add bl,7");					\
	asm volatile("ror ebx,15");					\
	N_OP_CALL_1d(N_OP_iplusn, n);				\
	}

extern inline  void fast_op_iplusn(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	eax,15	\n\
	rol	ebx,15	\n\
	sub	bl,7	\n\
	jne	iplusn_err	\n\
	add	eax,ebx	\n\
	jo	iplusn_err	\n\
	ror	eax,15	\n\
	or	eax,917504	\n\
	mov	ebx,eax	\n\
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

extern inline  void fast_op_greaterp(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	mov	edx,ebx	\n\
	rol	edx,15	\n\
	sub	dl,7	\n\
	jne	greaterp_err	\n\
	rol	eax,15	\n\
	sub	al,7	\n\
	jne	greaterp_err	\n\
	xor	ebx,ebx	\n\
	cmp	eax,edx	\n\
	jle	greater_no	\n\
	mov	ebx,76	\n\
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

extern inline  void fast_op_igreaterp(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	mov	edx,ebx	\n\
	rol	edx,15	\n\
	sub	dl,7	\n\
	jne	igreaterp_err	\n\
	rol	eax,15	\n\
	sub	al,7	\n\
	jne	igreaterp_err	\n\
	xor	ebx,ebx	\n\
	cmp	eax,edx	\n\
	jle	igreater_no	\n\
	mov	ebx,76	\n\
igreater_no:    " : : "g" (value)  );

  }


#undef LRSH8
#define LRSH8 {							\
    asm volatile("\
	mov	eax,ebx	\n\
	rol	eax,16	\n\
	cmp	ax,0eh	\n\
	jne	lrsh8_err	\n\
	shr	bx,8	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
lrsh8_err:							\
	asm("lrsh8_err: ");					\
	N_OP_CALL_1(N_OP_lrsh8);				\
	}

#undef LRSH1
#define LRSH1 {							\
    asm volatile("\
	mov	eax,ebx	\n\
	rol	eax,16	\n\
	cmp	ax,0eh	\n\
	jne	lrsh1_err	\n\
	shr	bx,1	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
lrsh1_err:							\
	asm("lrsh1_err: ");					\
	N_OP_CALL_1(N_OP_lrsh1);				\
	}

#undef LLSH8
#define LLSH8 {							\
    asm volatile("\
	mov	eax,ebx	\n\
	rol	eax,16	\n\
	cmp	ax,0eh	\n\
	jne	llsh8_err	\n\
	or  bh,bh	\n\
	jne llsh8_err	\n\
	shl	bx,8	\n\
    " : : : "ax" );	\
	fast1_dispatcher();						\
llsh8_err:							\
	asm("llsh8_err: ");					\
	N_OP_CALL_1(N_OP_llsh8);				\
	}

#undef LLSH1
#define LLSH1 {							\
    asm volatile("\
	mov	eax,ebx	\n\
	rol	eax,16	\n\
	cmp	ax,0eh	\n\
	jne	llsh1_err	\n\
	test  bh,80h	\n\
	jne llsh1_err	\n\
	shl	bx,1	\n\
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
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_logor, GET_POPPED);		\
	}

extern inline  void fast_op_logor(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	cmp	bl,7	\n\
	jne	logor_err	\n\
	rol	eax,15	\n\
	cmp	al,7	\n\
	jne	logor_err	\n\
	or	ebx,eax	\n\
	ror	ebx,15	\n\
    " :  : "g" (value) : "ax" );

  }

 
#undef LOGAND
#define LOGAND {						\
	fast_op_logand(POP_TOS_1);				\
	fast1_dispatcher();						\
logand_err:							\
	asm("logand_err: ");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_logand, GET_POPPED);		\
	}

extern inline  void fast_op_logand(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	cmp	bl,7	\n\
	jne	logand_err	\n\
	rol	eax,15	\n\
	cmp	al,7	\n\
	jne	logand_err	\n\
	and	ebx,eax	\n\
	ror	ebx,15	\n\
    " : : "g" (value) : "ax" );

  }


#undef LOGXOR
#define LOGXOR { 						\
	fast_op_logxor(POP_TOS_1);				\
	fast1_dispatcher();						\
logxor_err:							\
	asm("logxor_err:");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_logxor, GET_POPPED);		\
	}

extern inline  void fast_op_logxor(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	cmp	bl,7	\n\
	jne	logxor_err	\n\
	rol	eax,15	\n\
	sub	al,7	\n\
	jne	logxor_err	\n\
	xor	ebx,eax	\n\
	ror	ebx,15	\n\
    " :  : "g" (value) : "ax" );

  }



#undef N_OP_ADDBASE 
#define N_OP_ADDBASE {						\
	fast_op_addbase(POP_TOS_1);				\
	fast1_dispatcher();						\
addbase_err:							\
	asm("addbase_err: ");					\
	asm volatile("ror ebx,15");					\
	N_OP_POPPED_CALL_2(N_OP_addbase, GET_POPPED);		\
	}
extern inline  void fast_op_addbase(LispPTR value)
  {
    asm volatile("\
	mov	eax,%0	\n\
	rol	ebx,15	\n\
	cmp	bl,7	\n\
	jne	addbase_err	\n\
	sar	ebx,15	\n\
	and	eax,0ffffffh	\n\
	add	ebx,eax	\n\
    " : : "g" (value) : "ax" );

  }




#undef N_OP_LOLOC
#define N_OP_LOLOC	{  \
    asm volatile(" \
	and	%0,0ffffh \n\
	or	%0,0e0000h" : "=r" (tscache) : "0" (tscache)); \
     nextop1; }

#undef N_OP_HILOC
#define N_OP_HILOC	{ \
    asm volatile(" \
	shr	%0,16	\n\
	and	%0,0ffffh \n\
	or	%0,0e0000h" : "=r" (tscache) : "0" (tscache)); \
     nextop1; }

#undef N_OP_VAG2
#define N_OP_VAG2						\
  {								\
    asm("	sub	edi,4");				\
    asm("	mov	eax,[edi]");				\
    asm("	rol	ebx,16");				\
    asm("	mov	bx,ax");				\
    asm("	ror	ebx,16");				\
    nextop1; }

