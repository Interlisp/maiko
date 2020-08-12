/* $Id: tos1defs.h,v 1.2 1999/01/03 02:06:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*		T O P - O F - S T A C K   D E F I N I T I O N S		*/
/*									*/
/*	TOPOFSTACK	cached top of stack value.			*/
/*	CSTKPTR		points to where TOPOFSTACK should be stored.	*/
/*									*/
/************************************************************************/

#ifndef BYTESWAP
	/********************************************************/
	/*   Normal byte-order definitions, for e.g., 68020s	*/
	/********************************************************/

/* These are the TOS manipulation Macros */

#define HARD_PUSH(x)	*(CSTKPTRL++) = x
#define PUSH(x)		{HARD_PUSH(TOPOFSTACK); TOPOFSTACK = x;}
#define POP		TOPOFSTACK = *(--CSTKPTRL)
#define GET_TOS_1	*(CSTKPTR - 1)
#define GET_TOS_2	*(CSTKPTR - 2)
#define GET_POPPED	*CSTKPTR
#define POP_TOS_1	*(--CSTKPTRL)
#define TOPOFSTACK	tscache
#define FIXTOS1		CSTKPTRL++
#define GET_TOS_1_HI	*((DLword *)(CSTKPTR - 1))
#define GET_TOS_1_LO	*((DLword *)(CSTKPTR - 1)+1)

#else

	/********************************************************/
	/*	Byte-swapped definitions, for e.g., 80386s	*/
	/********************************************************/

/* These are the TOS manipulation Macros */

#define HARD_PUSH(x)	*(CSTKPTRL++) = x
#define PUSH(x)		{HARD_PUSH(TOPOFSTACK); TOPOFSTACK = x;}
#define POP		TOPOFSTACK = *(--CSTKPTRL)
#define GET_TOS_1	*(CSTKPTR - 1)
#define GET_TOS_2	*(CSTKPTR - 2)
#define GET_POPPED	*CSTKPTR
#define POP_TOS_1	*(--CSTKPTRL)
#define TOPOFSTACK	tscache
#define FIXTOS1		CSTKPTRL++
#define GET_TOS_1_HI	GETWORD((DLword *)(CSTKPTR - 1))
#define GET_TOS_1_LO	GETWORD((DLword *)(CSTKPTR - 1)+1)

#endif /* BYTESWAP */


/* OPCODE interface routines */

#define StackPtrSave	{CurrentStackPTR = (DLword *) (CSTKPTR-1);}
#define StackPtrRestore	{CSTKPTRL = ((LispPTR *) CurrentStackPTR)+1;}



#define EXT	{ PC=pccache-1;					\
		  TopOfStack=TOPOFSTACK;			\
		  StackPtrSave; }

#define RET	{ pccache=PC+1;					\
		  StackPtrRestore;				\
		  TOPOFSTACK = TopOfStack; }

#define NRET	{ RET; nextop0; }


#define RET_FROM_NATIVE							\
{									\
	POP;								\
	PCMACL += 1;							\
	table = optable;						\
}


#ifdef NATIVETRAN
#define NATIVE_NEXTOP0	{						\
		if (BCE_CURRENTFX->native)  { RET_TO_NATIVE; }		\
		nextop0;						\
		}
#else
#define NATIVE_NEXTOP0 nextop0
#endif


#define CALL_NATIVE(defcell, num_args)					\
{									\
	setivar_pvar_jmp(CSTKPTR + num_args, PVAR,			\
		((int *)(GET_NATIVE_ADDR(defcell)))[num_args+(6+2)]);	\
}

#define CALL_NATIVE2(defcell, num_args)					\
{									\
	setivar_pvar_jmp(CSTKPTR + num_args, PVAR,			\
		((int *)(GET_NATIVE_ADDR(defcell)))			\
		[(num_args <= -6) ? ((6+2)-6) : (num_args+(6+2))]);	\
}


#define RET_TO_NATIVE	{						\
	register int native_addr;					\
	register struct fnhead *LocFuncObj = FuncObj;			\
	if (native_addr =						\
		((int *)(GET_NATIVE_ADDR(LocFuncObj)))			\
		[((UNSIGNED) PCMAC - (UNSIGNED) LocFuncObj)+(6+2)])	\
		{							\
		HARD_PUSH(TOPOFSTACK);					\
		setivar_pvar_jmp(IVAR, PVAR, native_addr);		\
		};							\
	nextop0;							\
	}
