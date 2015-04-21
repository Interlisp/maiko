/* $Id: native.h,v 1.2 1999/01/03 02:06:18 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

/* ******

#include "lispemul.h"
#include "emlglob.h"
#include "address.h"
#include "adr68k.h"
#include "stack.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "cell.h"

****** */

/* ************************************************************************* */
/* **************		  EMULATOR MACROS 		************ */
/* ************************************************************************* */


typedef char ByteCode;
typedef unsigned short  DLword;
typedef unsigned int  LispPTR;

typedef struct  consstr { 
		  unsigned cdr_code : 8 ;
		  unsigned car_field : 24 ;
	} ConsCell;

#define CDR_INDIRECT           0
#define CDR_NIL				128
#define CDR_ONPAGE			128


struct state{
	DLword	*ivar;
	DLword	*pvar;
};

#define ATOM_T	0114	/* T's AtomIndex Number 114Q */
#define NIL_PTR		0   /* from cell.h 24-mar-87 take */
#define NOBIND_PTR	1
#define FRAMESIZE	10		/* size of frameex1: 10 words */
#define FNHEADSIZE	8		/* size of fnhead: 8 words */

#define GET_NATIVE_ADDR(fnobject)					\
		*((int *) ((int)fnobject + fnobject->startpc - 4))

#define CALL_NATIVE(defcell, num_args)  \
	asmgoto(((int *)(GET_NATIVE_ADDR(defcell)))[num_args+(6+2)]);


#define CURRENTFX ((struct  frameex1 *)((DLword *) PVar - FRAMESIZE))
#define	IVar	MachineState.ivar
#define PVar	MachineState.pvar

#define GetLongWord(address)		(*((int *) (address)))
#define LADDR_from_68k(ptr68k)	(((unsigned int)(ptr68k) - (unsigned int)Lisp_world) >>1)
#define Addr68k_from_LADDR(Lisp_addr)	(Lisp_world + (Lisp_addr))
#define StkOffset_from_68K(ptr68k)\
	(((unsigned int)(ptr68k) - (unsigned int)Stackspace) >>1)

#define Addr68k_from_StkOffset(stkoffset)\
	(Stackspace + (stkoffset))
#define POINTER_PAGEBASE(datum)	((datum) & 0x0ffff00)

#define GetDTD(typnum)	(DTDspace + ((typnum)<<4))
#define GetTypeEntry(address)       *(MDStypetbl + ((((int)address) & 0x0ffff00)>>9))
#define GetTypeNumber(address)     ((*(MDStypetbl +((((int)address) & 0x0ffff00)>>9))) & 2047)
#define Listp(address)    (GetTypeNumber(address) == TYPE_LISTP)

#define  BF_MARK	0x8000
#define  BF_MARK32	0x80000000
#define  FX_MARK	0xc000
#define  FX_MARK_NATIVE 0xc800
#define  STK_SAFE	32	/* added with stkmin */


struct fnhead{
	DLword		stkmin;	/* ?? */
	short		na;	/* Numbers of arguments */
	short		pv;	/* ?? */
	DLword		startpc;
			/* head of ByteCodes, DLword offset from stkmin */
	unsigned	native :1;	/* native translated? */
	unsigned	nil1 :1 ; 	/* not used */
	unsigned	argtype : 2;	/* ?? */
	unsigned	nil2 :4 ; 	/* not used */
	unsigned	framename : 24;	/* index in AtomSpace */
	DLword		ntsize;		/* size of NameTable */
	unsigned		nlocals :8;	/* ?? */
	unsigned		fvaroffset :8;	
			/* DLword offset from head of NameTable */
	/* NameTable of variavle length is follwing with this structure. */
};


typedef struct frameex1{
	unsigned	flags	:3;
	unsigned	fast	:1;
	unsigned	native	:1;	/* This frame treats N-func */
	unsigned	incall	:1;
	unsigned	validnametable	:1;
			/* 0: look for FunctionHeader 
			   1: look for NameTable on this FrameEx */
	unsigned	nopush	:1;
	unsigned	usecount :8;
	DLword	alink;		/* alink pointer (Low addr) */ 
	DLword	lofnheader;	/* pointer to FunctionHeader (Low addr) */
	unsigned hi1fnheader : 8; /* pointer to FunctionHeader (Hi1 addr) */
	unsigned hi2fnheader : 8; /* pointer to FunctionHeader (Hi2 addr) */
	DLword	nextblock;	/* pointer to FreeStackBlock */
	DLword	pc;		/* Program counter */
	DLword	lonametable;	/* pointer to NameTable of this FrameEx (Low addr) */
	unsigned hi1nametable :8;	/* pointer to NameTable of this FrameEx (Hi1 addr) */
	unsigned hi2nametable :8;	/* pointer to NameTable of this FrameEx (Hi2 addr) */
	DLword	blink;		/* blink pointer (Low addr) */
	DLword	clink;		/* clink pointer (Low addr) */
} FX ;

typedef struct frameex2{
	unsigned	flags	:3;
	unsigned	fast	:1;
	unsigned	native	:1;	/* This frame treats N-func */
	unsigned	incall	:1;
	unsigned	validnametable	:1;
			/* 0: look for FunctionHeader 
			   1: look for NameTable on this FrameEx */
	unsigned	nopush	:1;
	unsigned	usecount :8;
	DLword	alink;		/* alink pointer (Low addr) */ 
	LispPTR	fnheader;	/* pointer to FunctionHeader (swapped) */
	DLword	nextblock;	/* pointer to FreeStackBlock */
	DLword	pc;		/* Program counter */
	LispPTR	nativera;	/* address of native ra */
	DLword	blink;		/* blink pointer (Low addr) */
	DLword	clink;		/* clink pointer (Low addr) */
} FX2 ;




typedef struct basic_frame {
	unsigned	flags : 3 ;
	unsigned	nil   : 3 ;
	unsigned	residual :1 ;
	unsigned	padding  : 1 ;
	unsigned	usecnt   : 8 ;
	DLword		ivar ;

} Bframe ;


/* Structure for DTD */
struct dtd {
	DLword dtd_name ;
	DLword dtd_size ;
	LispPTR dtd_free ;
	unsigned unuse	:2 ;
	unsigned dtd_obsolate :1 ;
	unsigned dtd_finalizable :1 ;
	unsigned dtd_lockedp : 1 ;
	unsigned dtd_hunkp : 1 ;
	unsigned dtd_gctype :2 ;
	unsigned dtd_descrs : 24;
	LispPTR dtd_typespecs ;
	LispPTR dtd_ptrs ;
	int  dtd_oldcnt;
	DLword dtd_cnt0 ;
	DLword dtd_nextpage ;
	DLword dtd_typeentry ;
	DLword dtd_supertype ;
};




#define TYPE_SMALLP			1
#define TYPE_FIXP			2
#define TYPE_FLOATP			3 
#define TYPE_LITATOM			4 
#define TYPE_LISTP			5 
#define TYPE_ARRAYP			6 
#define TYPE_STRINGP			7 
#define TYPE_STACKP			8
#define TYPE_CHARACTERP			9
#define TYPE_VMEMPAGEP			10
#define TYPE_STREAM			11
 
#define TYPE_BITMAP			12
#define TYPE_COMPILED_CLOSURE		13
#define TYPE_ONED_ARRAY			14
#define TYPE_TWOD_ARRAY			15
#define TYPE_GENERAL_ARRAY		16


typedef struct compiled_closure {
		unsigned int nil1	: 8 ;
		unsigned int def_ptr	: 24; /* function */
		unsigned int nil2	: 8 ;
		unsigned int env_ptr	: 24; /* environment */
  } CClosure ;

typedef struct definition_cell {
	unsigned	ccodep	:	1 ;
	unsigned	fastp	:	1 ;
	unsigned	argtype	:	2 ;
	unsigned	pseudocodep :	1 ;
	unsigned	nil	:	3 ;
	unsigned	defpointer :	24;

} DefCell ;

#define GetDEFCELL68k(index)	((LispPTR *)Defspace + (index) )
#define GetVALCELL68k(index)	((LispPTR *)Valspace + (index))

#define S_POSITIVE	0xE0000
#define S_NEGATIVE	0xF0000
#define S_CHARACTER	0x70000


/* ************************************************************************* */
/* **************		 NATIVE ONLY MACROS 		************ */
/* ************************************************************************* */



/************************************************************************/
/*	 TOP OF STACK OPERATIONS		 			*/
/************************************************************************/

#define PUSH(x)		{*((LispPTR *) CSTKPTR++) = (LispPTR) x;}
#define PUSH16(x)	*((DLword *) CSTKPTR++) = x;
#define PUSH16s(x, y) 	{ PUSH16(x); PUSH16(y); }
#define PUSH_SWAPED(x)	{ register LispPTR temp;\
			  temp = x; \
			  PUSH16(temp); \
			  PUSH16(swapx(temp)); \
			}



#define POP		*((LispPTR *) --CSTKPTR)
#define TOS		*((LispPTR *) CSTKPTR-1)
#define COPY_TOP 	TOS
#define SAVE_PUSH_TOP 	TOS
#define PREV_TOS 	*((LispPTR *) CSTKPTR-2)
#define GET_POPPED	*CSTKPTR
#define GET_POPPED_2	*((LispPTR *) CSTKPTR+1)
#define LSTACK		(CSTKPTR - 1)


/************************************************************************/
/*	 MACROS TO SAVE & RESTORE STATE FOR OP_xx OPCODE ROUTINES	*/
/* 	 TO CALL OLD STYLE OPCODE ROUTINE				*/
/************************************************************************/

#define NATIVE_EXT(call_pc)						\
{	PC  =  (ByteCode *) call_pc;					\
	TopOfStack = POP;						\
	CurrentStackPTR = (DLword *) (CSTKPTR-1);			\
}

#define NATIVE_RET							\
{	CSTKPTR = (LispPTR *) CurrentStackPTR + 1;			\
	PUSH(TopOfStack);						\
}

#define CALL_OP_FN(callpc, nextpc, opcodefn)	{			\
	NATIVE_EXT(callpc);						\
	opcodefn();							\
	NATIVE_RET;							\
	if (nextpc != (unsigned int) PC) {				\
		QUIT_NATIVE(PC); 					\
	}								\
}

/************************************************************************/
/*	 RETURN TO DISPATCH TO EXECUTE NEW FRAME 			*/
/************************************************************************/

#define NATIVE_CURRENTFX ((struct frameex1 *)((DLword *) PVAR - FRAMESIZE))

#define C_RETURN_TO_DISPATCH						\
{									\
	asmgoto(&c_ret_to_dispatch); 					\
}

/************************************************************************/
/* 	RETURN TO DISPATCH TO EXECUTE OPCODE & RETURN TO NATIVE CODE	*/
/************************************************************************/

#define BCE(ret_pc)	{						\
	setpc_jmp(ret_pc, &ret_to_unimpl); 				\
}

/************************************************************************/
/* 	RETURN TO DISPATCH TO EXECUTE OPCODE & STAY IN EMULATOR		*/
/************************************************************************/

#define QUIT_NATIVE(ret_pc)						\
{ 									\
	setpc_jmp(ret_pc, &ret_to_dispatch); 				\
}

/************************************************************************/
/* 	RETURN TO DISPATCH TO EXECUTE FN CALL 				*/
/************************************************************************/

#define RETURN_TO_FN_CALL(ret_pc, golabel)				\
{ 									\
	setpc_jmp(ret_pc, &golabel); 					\
}

/************************************************************************/
/* 	RETURN TO DISPATCH TO EXECUTE UFN CALL 				*/
/************************************************************************/

#define CALL_UFN(call_pc, opcode)					\
{ 									\
	setpc_jmp(call_pc, &ret_to_ufn); 				\
}


/************************************************************************/
/* 	RETURN TO DISPATCH TO EXECUTE TIMER INTERRUPT 			*/
/************************************************************************/

#define RETURN_TO_TIMER(call_pc)					\
{ 									\
	setpc_jmp(call_pc, &ret_to_timer); 				\
}

/************************************************************************/
/* 	STACK OVERFLOW & TIMER CHECKS 					*/
/************************************************************************/

#define STK_MIN(fnobj) ((fnobj->stkmin+STK_SAFE) << 1)

#define STK_END_COMPUTE(stk_end,fnobj) 					\
		( (int)stk_end - STK_MIN(fnobj) )


#define CLR_IRQ 							\
	{Irq_Stk_Check = STK_END_COMPUTE((Irq_Stk_End = (int) EndSTKP), \
					 FuncObj);			\
	}

#define STACK_ONLY_CHECK(stkmin)					\
{	if ((int) CSTKPTR > (EndSTKP - ((stkmin+STK_SAFE) << 1))){	\
		IVar = (DLword *) IVAR;				\
		PVar = (DLword *) PVAR;				\
		TopOfStack = POP;						\
		CurrentStackPTR = (DLword *) (CSTKPTR-1);			\
		if (do_stackoverflow(0)) {				\
			printf("REAL STACK OVERFLOW\n");		\
			asmgoto(&ret_to_dispatch);			\
			}						\
		CSTKPTR = (LispPTR *) CurrentStackPTR + 1;		\
		PUSH(TopOfStack);					\
		IVAR = CSTKPTR + entry_pc;				\
		PVAR = (LispPTR *) PVar;				\
		}							\
}
	
#define TIMER_STACK_CHECK(pc)						\
{	if ( (int) CSTKPTR > Irq_Stk_Check ) {if(pc ==1) {printf("before timer exit\n"); do_brk();} RETURN_TO_TIMER(pc);} }

/************************************************************************/
/* 	FUNCTION ENTRY SETUP	 					*/
/************************************************************************/

/* 	The code generator must expand this differently, depending on
  	the number of paramaters available to the function.
*/

#define framesetup(x, stkmin, swapped_func_obj)	{ 			\
  register int NEXTBLOCK;						\
  {									\
   register struct frameex1 *LocFX;					\
   LocFX = NATIVE_CURRENTFX;						\
   LocFX->nextblock = NEXTBLOCK = StkOffset_from_68K(IVAR);		\
  }									\
  IVar = (DLword *) IVAR ;						\
  Irq_Stk_Check = ( (int)Irq_Stk_End - ( (stkmin+STK_SAFE) << 1) );	\
   									\
 /* Set up BF */							\
  PUSH((BF_MARK << 16) | NEXTBLOCK);					\
  PUSH((FX_MARK_NATIVE << 16) | StkOffset_from_68K(PVAR)); 		\
  PUSH(swapped_func_obj); 						\
  (DLword *) PVAR = PVar = (DLword *) CSTKPTR = (((DLword *) CSTKPTR) + (FRAMESIZE-4)) ;	\
} /* end framesetup */


/************************************************************************/
/* 	FUNCTION CALL & RETURN 						*/
/************************************************************************/

#define fncall_self(args, pc_assign, newpcval, golabel) {		\
  NATIVE_CURRENTFX->pc = newpcval;					\
  IVAR = CSTKPTR - args;						\
  pc_assign;								\
  goto golabel;								\
} /* end fncall */


#define newdefcell  ((struct fnhead *) DATUM68K)
#define fncall_other(args, call_args, pc_assign, currpc, newpcval, atom_index, fn_def_cell_addr_68K, return_label) {\
 /* register struct fnhead *newdefcell;	*/					\
  if ( (((DefCell *)fn_def_cell_addr_68K)->ccodep ) &&			\
	  ( newdefcell = (struct fnhead *)Addr68k_from_LADDR(		\
	   ((DefCell *) fn_def_cell_addr_68K)->defpointer))->native) 	\
	{		 						\
	 NATIVE_CURRENTFX->pc = newpcval;				\
	 FuncObj = (struct fnhead *)newdefcell;				\
	 IVAR = CSTKPTR - args;						\
	 pc_assign;							\
	 CALL_NATIVE(newdefcell, -call_args);				\
	};								\
   RETURN_TO_FN_CALL(currpc, return_label);				\
} /* end fncall */




#define envcall_native(retpcval, args, fncell, native_addr_slot, environment) {\
	FuncObj = (struct fnhead *) fncell;				\
	native_closure_env = environment;				\
	NATIVE_CURRENTFX->pc = retpcval;				\
	IVAR = CSTKPTR - args;						\
	IF (args > 5) {(int) PC = -args; CALL_NATIVE(fncell, -6);}	\
	CALL_NATIVE(fncell, -args);					\
} /* end envcall */

#define returnFX ((struct frameex2 *) DATUM68K)

#define return_op(pcval, swapped_func_obj, ret_result, slow_ret_result)	\
{									\
 /* *** op_return(pcval,swapped_func_obj); */ 				\
 register DLword alink; 						\
 alink = NATIVE_CURRENTFX->alink;					\
 if ( alink & 1 )  { slow_ret_result; BCE(pcval); }			\
 ret_result;	/* NOTE: this smashes the BF word if fn called with 0 args */\
 CSTKPTR = IVAR + 1; 							\
 returnFX = (struct frameex2 *) 					\
	((DLword *) 							\
	    (PVAR = (LispPTR *) Addr68k_from_StkOffset(alink))		\
	    - FRAMESIZE); 						\
 IVAR = (LispPTR *) Addr68k_from_StkOffset(*((DLword *)returnFX -1)) ; 	\
 IVar = (DLword *) IVAR;						\
 PVar = (DLword *) PVAR;						\
 if (returnFX->native) {						\
	 if (returnFX->fnheader == swapped_func_obj) 			\
	   {(unsigned int) PC = (unsigned int) returnFX->pc;		\
		goto switchlabel; 					\
	   }								\
	 else 								\
	   {register struct fnhead *newfncell;				\
	    newfncell = FuncObj = (struct fnhead *)			\
	      Addr68k_from_LADDR(0x0ffffff & swapx(returnFX->fnheader));\
	    CALL_NATIVE(newfncell, (unsigned int) returnFX->pc); 	\
	   }								\
	}								\
 else									\
	{register struct fnhead *LocFnObj;				\
	    FuncObj = LocFnObj = (struct fnhead *)			\
	    Addr68k_from_LADDR(0x0ffffff & swapx(returnFX->fnheader));	\
 	Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,LocFnObj);		\
	if (((int)(CSTKPTR) > Irq_Stk_Check) || (Irq_Stk_End <= 0)) 	\
		RETURN_TO_TIMER(returnFX->pc + (int) FuncObj);		\
	C_RETURN_TO_DISPATCH;						\
	}								\
} /* return_op end */ 


/************************************************************************/
/* 	MACORS FOR OPCODES 						*/
/************************************************************************/


#define MYARGCOUNT							\
( ( ( 						\
	 (	((NATIVE_CURRENTFX->alink & 1) == 0)			\
	 ?	(int) ((LispPTR *)NATIVE_CURRENTFX - 1)		\
	 :	(int) (Stackspace + NATIVE_CURRENTFX->blink)	\
	 )										\
	  - (int) IVAR) >> 2)  )

#define N_OP_CHECKAPPLY(tos, abs_pc) {					\
  register DefCell *defcell;						\
  defcell = (DefCell *) GetDEFCELL68k(tos & 0xffff) ; 			\
  if  (!(  defcell->ccodep  && ((tos & 0xffff0000) == 0)	&&	\
	( ( defcell->argtype == 0 ) || ( defcell->argtype == 2 ) ) ) ) 	\
  	BCE(abs_pc);							\
}

#define N_OP_TYPEMASK(n) 						\
(	( ( ((DLword)GetTypeEntry(TEMPREG = TOS)) & (DLword)n) == 0)	\
    	? NIL_PTR							\
	: TEMPREG							\
)

#define GETBASE_N(ptr, n)\
	( *((DLword *)Addr68k_from_LADDR((0xFFFFFF & ptr) + n)))


#define GETBASEPTR_N(ptr, n)\
	(0xFFFFFF & *((LispPTR *)Addr68k_from_LADDR((0xFFFFFF & ptr) + n)))

#define N_OP_PUTBASEPTR_N(tos_1, tos, n)				\
	*((LispPTR *)((DLword *)Addr68k_from_LADDR(0xffffff & tos_1) + n)) = tos;



#define N_OP_PUTBASE_N(tos_1, tos, n, error_label)			\
{									\
	if ( ((unsigned short)(swapx(TEMPREG = (LispPTR)tos))) != (S_POSITIVE >> 16))	\
		goto error_label;					\
	*((DLword *)Addr68k_from_LADDR(0xffffff & tos_1) + n) = (DLword) TEMPREG;	\
}

#define N_OP_GETBITS_N_FD(tos, offset, bit_mask, shift_amount)	\
	(	 							\
	 (( ( *((DLword *)Addr68k_from_LADDR(0xFFFFFF & tos) + offset))	\
			>> shift_amount )				\
			& bit_mask )					\
	)


#define N_OP_PUTBITS_N_FD(tos_1, tos, offset, bit_mask, shift_amount, error_label)\
{register LispPTR toscache, base;					\
	if ( ((unsigned short)(swapx(toscache = (LispPTR)tos))) != (S_POSITIVE >> 16))	\
		goto error_label;					\
	(DLword *) DATUM68K  = (DLword *)Addr68k_from_LADDR(base = 0xffffff & tos_1) + offset;\
	*((DLword *)DATUM68K )= 						\
		(  (toscache << shift_amount) & 			\
		   (bit_mask << shift_amount)) |			\
		(*((DLword *)DATUM68K ) & (~(bit_mask << shift_amount)));\
}

#define N_OP_GETBASEBYTE(tos_1, tos, error_label)			\
((	((TEMPREG = (TOS_CACHE = tos) & 0xffff0000) == S_POSITIVE)\
	?	(*((char *) Addr68k_from_LADDR(0xffffff & tos_1) + (unsigned short) TOS_CACHE))\
	:(	(TEMPREG == S_NEGATIVE)\
		?	(*((char *) Addr68k_from_LADDR(0xffffff & tos_1) + (0xffff0000 | TOS_CACHE)))\
		:(	( GetTypeNumber(TOS_CACHE) == TYPE_FIXP )\
			?	(*((char *) Addr68k_from_LADDR(0xffffff & tos_1) + \
					*((int *)Addr68k_from_LADDR(TOS_CACHE))))\
			:	asmgoto(error_label)\
		)\
	)\
))


#define N_OP_PUTBASEBYTE(tos_2, tos_1, tos, error_label)		\
{register LispPTR toscache, base;					\
	toscache = tos;							\
	TEMPREG = tos_1;						\
	base = 0xffffff & tos_2;					\
	if(	((0xFFFF0000 & toscache ) != S_POSITIVE) || 		\
		((unsigned short)toscache >= 256))			\
		goto error_label;					\
	switch( (0xFFFF0000 & TEMPREG) ){				\
	 case S_POSITIVE:						\
	 	TEMPREG &=  0x0000FFFF;					\
	 	break;							\
	 case S_NEGATIVE:						\
		TEMPREG |=  0xFFFF0000;					\
		break;							\
	 default:							\
			goto error_label;				\
	}								\
	*((char*)Addr68k_from_LADDR(0xFFFFFF & base) + TEMPREG) =	\
		 0xFF & toscache;					\
}

#define N_OP_CAR(tos, error_label) 					\
 	(Listp(TOS_CACHE = tos) 					\
 	?	(							\
		  (((ConsCell *)					\
		   (DATUM68K = (LispPTR *)(Addr68k_from_LADDR(TOS_CACHE))))\
		  ->cdr_code == CDR_INDIRECT) 				\
		 ?	((LispPTR) ( ((ConsCell *)			\
		Addr68k_from_LADDR( ((ConsCell *)DATUM68K)->car_field))->car_field))\
		 : ((LispPTR)(((ConsCell *)DATUM68K)->car_field))	\
		)							\
 	:	( (TOS_CACHE == NIL_PTR)				\
		? TOS_CACHE 						\
		:							\
		( ( TOS_CACHE == ATOM_T)				\
		? TOS_CACHE 						\
		: asmgoto(error_label) 					\
		)							\
		)							\
	)


#define N_OP_CDR(tos, error_label)  					\
  	(Listp(TOS_CACHE = tos) 					\
 	?	( ((TEMPREG = (LispPTR) 				\
		   ( ((ConsCell *)					\
		   (DATUM68K = ((LispPTR *)(Addr68k_from_LADDR(TOS_CACHE)))))\
		  ->cdr_code)) == CDR_NIL)				\
		 ? (LispPTR) NIL_PTR					\
		 : (LispPTR) ( (TEMPREG > CDR_ONPAGE)			\
		 ?							\
			/* cdr-samepage */				\
			(POINTER_PAGEBASE(TOS_CACHE) + 			\
			((TEMPREG & 127) << 1))				\
		 : (LispPTR) ( (TEMPREG == CDR_INDIRECT)		\
		 ? ((LispPTR) cdr (((ConsCell *)DATUM68K)->car_field))	\
		 : (LispPTR) ((ConsCell *)(Addr68k_from_LADDR 		\
		(POINTER_PAGEBASE(TOS_CACHE) + (TEMPREG << 1))))->car_field\
		)							\
		)							\
		)							\
	:	(LispPTR) ( (TOS_CACHE == NIL_PTR) ? NIL_PTR : asmgoto(error_label))  \
	)

#define N_OP_CDDR(tos, error_label)  					\
  	(Listp(TOS_CACHE = tos) 					\
 	?	( ((TEMPREG = (LispPTR) 				\
		   ( ((ConsCell *)					\
		   (DATUM68K = ((LispPTR *)(Addr68k_from_LADDR(TOS_CACHE)))))\
		  ->cdr_code)) == CDR_NIL)				\
		 ? (LispPTR) NIL_PTR					\
		 : (LispPTR) ( (TEMPREG > CDR_ONPAGE)			\
		 ?							\
			/* cdr-samepage */				\
			(SAME_PAGE_CDR)				\
		 : (LispPTR) ( (TEMPREG == CDR_INDIRECT)		\
		 ? N_OP_CDR(cdr(((ConsCell *)DATUM68K)->car_field),error_label)\
		 : N_OP_CDR(						\
			((ConsCell *)					\
				(Addr68k_from_LADDR 			\
					(POINTER_PAGEBASE(TOS_CACHE) + 	\
						(TEMPREG << 1)		\
					)				\
				)					\
			)->car_field					\
			, error_label)					\
		)							\
		)							\
		)							\
	:	(LispPTR) ( (TOS_CACHE == NIL_PTR) ? NIL_PTR : asmgoto(error_label))  \
	)

#define SAME_PAGE_CDR							\
/* take CDR of List Cell */						\
( 		((TEMPREG = (LispPTR) 					\
		   ( ((ConsCell *)					\
		   (DATUM68K = (LispPTR *)				\
				(((int)DATUM68K & 0xfffffe00) |		\
				(((int) TEMPREG & 127) << 2))		\
		   ))							\
		  ->cdr_code)) == CDR_NIL)				\
		 ? (LispPTR) NIL_PTR					\
		 : (LispPTR) ( (TEMPREG > CDR_ONPAGE)			\
		 ?	/* cdr-samepage */				\
			(POINTER_PAGEBASE(TOS_CACHE) + 			\
			 ((TEMPREG & 127) << 1))			\
		 : (LispPTR) ( (TEMPREG == CDR_INDIRECT)		\
		 ? ((LispPTR) cdr (((ConsCell *)DATUM68K)->car_field))	\
		 : (LispPTR) ((ConsCell *)(Addr68k_from_LADDR 		\
		(POINTER_PAGEBASE(TOS_CACHE) + (TEMPREG << 1))))->car_field\
		)							\
		)							\
)


#define N_OP_FVAR(slot, dl_slot)					\
(	GetLongWord(Addr68k_from_LADDR(swapx(				\
	( ( ((DLword *)PVAR)[dl_slot] & 1 )				\
	? native_newframe(slot)						\
	: PVAR[slot]							\
)))))	


#define N_OP_UNBIND(tos)						\
/* {register LispPTR SAVE_TOS = tos; CSTKPTR = (LispPTR *) N_OP_unbind(CSTKPTR); PUSH(SAVE_TOS);} */ \
	nop_unbind(tos);

#define N_OP_DUNBIND							\
/*  { CSTKPTR = (LispPTR *) N_OP_dunbind(CSTKPTR); }  THIS MAY NOT WORK	*/		\
	nop_dunbind();

#define N_OP_CLARITHEQUAL(tos_1, tos, error_addr)			\
(	(((TEMPREG = tos) & 0xfffe0000) == (S_POSITIVE & 0xfffe0000))	\
	? 	((TEMPREG == tos_1) ? ATOM_T : NIL_PTR)			\
	:	((((int) DATUM68K = GetTypeNumber(TEMPREG)) == TYPE_FIXP)\
		?	((TEMPREG == tos_1) ? ATOM_T : NIL_PTR)		\
		:	(((int) DATUM68K == TYPE_FLOATP)		\
			?	((TEMPREG == tos_1) ? ATOM_T : NIL_PTR)	\
			:	(N_OP_eqq(tos_1, TEMPREG, error_addr))	\
			)						\
		)							\
)

#define N_OP_CLEQUAL_ILEQL(tos_1, tos, error_addr, op_function)		\
(	(((TOS_CACHE = tos) & 0xfffe0000) <= (S_POSITIVE & 0xfffe0000))	\
	? 	((TOS_CACHE == tos_1) ? ATOM_T : NIL_PTR)		\
	:(	(((TEMPREG = tos_1) & 0xfffe0000) <=			\
			 (S_POSITIVE & 0xfffe0000))			\
		?	((TOS_CACHE == TEMPREG) ? ATOM_T : NIL_PTR)	\
		:	op_function(TEMPREG, TOS_CACHE, error_addr)	\
	)								\
)

#define N_OP_EQUAL(tos_1, tos, error_addr)				\
(	(((TOS_CACHE = tos) & 0xffff0000) <= S_CHARACTER)		\
	? 	((TOS_CACHE == tos_1) ? ATOM_T : NIL_PTR)		\
	:(	(((TEMPREG = tos_1) & 0xffff0000) <= S_CHARACTER)	\
		?	((TOS_CACHE == TEMPREG) ? ATOM_T : NIL_PTR)	\
		:	N_OP_equal(TEMPREG, TOS_CACHE, error_addr)	\
	)								\
)


#define N_OP_DTEST(atom_index, exit_pc, opcode)				\
{	/* must have stack up to date */				\
	register struct dtd *dtd68k ;					\
 for(dtd68k=(struct dtd *) GetDTD(GetTypeNumber(TOS));			\
	atom_index != dtd68k->dtd_name ;				\
	    dtd68k=(struct dtd *) GetDTD(dtd68k->dtd_supertype))	\
	{								\
		if( dtd68k->dtd_supertype == 0)				\
		{							\
		 CALL_UFN(exit_pc, opcode);				\
		}							\
	}								\
}

#ifdef sun3
	/* these take advantage of the Shift Amount Register d5 */
#define NSMALLP_RANGE(x)	nop_nsmallp_range(x)
#define SMALLP_UNBOX(x)		nop_smallp_unbox(x)
#define SMALL_BOX(x)		nop_smallp_box(x)
#else
#define NSMALLP_RANGE(x)	(((int)((int)x << 15) >> 15) ^ x)
#define SMALLP_UNBOX(x)		( (int) (x << 15) >> 15)
#define SMALL_BOX(x)	(((unsigned int)(x << 15) >> 15) | S_POSITIVE)
#endif

#define NSMALLP(x)		(((x) >> 17) ^ 7)
#define MAKE_BOX(type, value)	Create_n_Set_Cell(type, value)
#define GET_BOX(type, laddr)	(* ((type *) (Addr68k_from_LADDR(laddr))))

#define FIXP_UNBOX(value)						\
(	NSMALLP((TEMPREG = value))					\
	?	GET_BOX(int, TEMPREG)					\
	:	SMALLP_UNBOX(TEMPREG)					\
)

#define FIXP_UNBOX_UFN(value, errorlabel)				\
(	NSMALLP((TEMPREG = value))					\
	?(	(GetTypeNumber(TEMPREG) == TYPE_FIXP)			\
		?GET_BOX(int, TEMPREG)					\
		:asmgoto(errorlabel)					\
	 )								\
	:	SMALLP_UNBOX(TEMPREG)					\
)


#define FLOATP_UNBOX(value)						\
	GET_BOX(floatvalue)	


#define FIXP_BOX(x)							\
(	NSMALLP_RANGE((TEMPREG = x))					\
	? MAKE_BOX(TYPE_FIXP, TEMPREG)					\
	: SMALL_BOX(TEMPREG)						\
)

#define FLOATP_BOX(x) 	Create_n_Set_Cell(TYPE_FLOATP, x)


/************************************************************************/
/* 	EXTERNAL ENTRY POINTS 						*/
/************************************************************************/

extern DLword *Atomspace; 		/* ATOMSPACE */
extern DLword *Stackspace;		/* STACKSPACE*/
extern DLword *Defspace;		/* DEFSPACE */
extern DLword *Valspace;		/* VALSPACE */
extern DLword *Lisp_world; 		/* Lisp Start BASE */
extern DLword *MDStypetbl;
extern DLword *DTDspace;		/* DTDSPACE */

extern DLword  *CurrentStackPTR;		/* rhS,S  */
extern LispPTR	TopOfStack ;		/*  TOSH(high 16),TOS (lo 16) */
extern LispPTR	Scratch_CSTK ;
extern ByteCode *PC;			/* Pointer to executing Byte Code   */
extern struct state MachineState;
extern struct fnhead *FuncObj;		/* Pointer to current ccode obj */
extern int EndSTKP;			/* End of Current Frame */

extern int *c_ret_to_dispatch;
extern int *ret_to_dispatch;
extern int *ret_to_unimpl;
extern int *ret_to_timer;
extern int *ret_to_fn0;
extern int *ret_to_fn1;
extern int *ret_to_fn2;
extern int *ret_to_fn3;
extern int *ret_to_fn4;
extern int *ret_to_fnx;
extern int *ret_to_apply;
extern int *ret_to_envcall;
extern int *ret_to_ufn;

extern int Irq_Stk_End;
extern int Irq_Stk_Check;
extern LispPTR native_closure_env;

