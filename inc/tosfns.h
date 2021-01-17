/* $Id: tosfns.h,v 1.2 1999/01/03 02:06:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1998 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/



/****************************************************************/
/******		 CURRENT Stack Overflow checks		 ********/
/****************************************************************/
/* JDS 22 May 96 this used to be just >, but were getting stack-overflow
   with last frame right against the edge of the stack, which caused
   lots of trouble w/ 0-length free blocks, etc. */
#if 0
#define  FN_STACK_CHECK			\
  if ((UNSIGNED)CSTKPTR >= (Irq_Stk_Check=(Irq_Stk_End-STK_MIN(LOCFNCELL))))	\
     goto check_interrupt;
#else
/* JDS 13 Feb 98 -- with Irq_Stk_Chk being unsigned, need to revise */
/* the test; if Irq_Stk_Chk == 0, can't just do the subtraction now */
/* or we get a HUGE unsigned, and the test doesn't work right.      */

#define FN_STACK_CHECK							\
  if ((Irq_Stk_End == 0) ||						\
      ((UNSIGNED)CSTKPTR >						\
	(Irq_Stk_Check=(Irq_Stk_End-STK_MIN(LOCFNCELL)))))		\
    goto check_interrupt;
#endif /* 0 */


/****************************************************************/
/******			 LOCAL MACROS			 ********/
/****************************************************************/

#ifdef BIGVM
#define SWAP_FNHEAD
#else
#undef SWAP_FNHEAD
#define SWAP_FNHEAD(x) SWAP_WORDS(x)
#endif /* BIGVM */



#ifdef GCC386
#define ASM_LABEL_OF_FN_COMMON asm("fn_common:");
#else
#define ASM_LABEL_OF_FN_COMMON
#endif /* GCC386 */



/************************************************************************/
/*									*/
/*		   A P P L Y _ P O P _ P U S H _ T E S T		*/
/*									*/
/*	Part of op_fn_common; decide what to do to the stack, depending	*/
/*	on whether we're FNcalling, APPLYing, or calling a UFN.  What	*/
/*	happens depends on the value of fn_apply, which is set by	*/
/*	the various opcode macros, as follows:				*/
/*									*/
/*	0 Normal function calls; do nothing additional.			*/
/*	1 APPLY:  POP the #ARGS and FN-NAME arguments.			*/
/*	2 UFN with 0 args from the opcode byte stream.  Do nothing.	*/
/*	3 UFN with 1 byte of arg from the code stream as a SMALLP	*/
/*	4 UFN with 2 bytes of arg from the code stream as a SMALLP	*/
/*	5 UFN with 3 bytes of arg from the code stream as a SMALLP	*/
/*	      or as a symbol (for big atoms, e.g.)			*/
/*	6 UFN with 4 bytes of arg from the code stream as a SMALLP	*/
/*	      or as a symbol (for big atoms, e.g.)			*/
/*									*/
/*	The latter 3 cases push the additional argument; THE 3-BYTE	*/
/*	CASE IS INCOMPLETE:  IT SHOULD BOX ANY NON-SMALLP VALUES!	*/
/*									*/
/************************************************************************/

#ifdef BIGATOMS
#define APPLY_POP_PUSH_TEST						\
  {									\
    switch (fn_apply)							\
      {									\
	case 0:	break; /* do nothing */					\
	case 1: POP; POP;  break; /* from apply */			\
	case 2: break; /* ufn 0 args */					\
	case 3: PUSH(S_POSITIVE | Get_BYTE_PCMAC1); break;		\
	case 4: PUSH(S_POSITIVE | Get_DLword_PCMAC1); break;		\
	case 6:	/* BIGVM possibility */			\
	case 5: {							\
		  unsigned int atm = Get_AtomNo_PCMAC1;			\
		  if (atm & SEGMASK) PUSH(atm) /* new atom */		\
       		  else PUSH(S_POSITIVE | atm);   /* old atom as SMALLP*/\
		}							\
		break;							\
	default: error("Storage error: invalid UFN entry");		\
      }									\
    if (needpush) PUSH(fn_atom_index);					\
  }
#else  /* not big atoms */
#define APPLY_POP_PUSH_TEST						\
  {									\
    switch (fn_apply)							\
      {									\
	case 0:	break; /* do nothing */					\
	case 1: POP; POP;  break; /* from apply */			\
	case 2: break; /* ufn 0 args */					\
	case 3: PUSH(S_POSITIVE | Get_BYTE_PCMAC1); break;		\
	case 4: PUSH(S_POSITIVE | Get_DLword_PCMAC1); break;		\
	case 5: PUSH(S_POSITIVE | Get_AtomNo_PCMAC1); break;		\
	default: error("Storage error: invalid UFN entry");		\
      }									\
    if (needpush) PUSH(fn_atom_index);					\
  }

#endif /* BIGATOMS */



#define N_APPLY_POP_PUSH_TEST {						\
	APPLY_POP_PUSH_TEST;						\
	native_closure_env=closure_env;					\
	}

#define N_ENVCALL_POP_TEST {						\
	CSTKPTRL -=2;							\
	native_closure_env=closure_env;					\
	}



/****************************************************************/
/******			 OPAPPLY			 ********/
/****************************************************************/
#ifndef BIGATOMS
#define OPAPPLY	{							\
  if ( GET_TOS_1_HI  == SPOS_HI ) {					\
	fn_num_args = GET_TOS_1_LO;					\
	fn_opcode_size = 1;						\
	fn_apply = 1;							\
	fn_atom_index = TOPOFSTACK;					\
	FNTRACER(Trace_APPLY(fn_atom_index));				\
	FNCHECKER(if (quick_stack_check()) Trace_APPLY(fn_atom_index));	\
	if ( (SEGMASK & TOPOFSTACK) == 0)				\
	     {	fn_defcell = (DefCell *) GetDEFCELL68k(TOPOFSTACK);	\
		goto op_fn_common;					\
	     }								\
	else								\
	if (GetTypeNumber(TOPOFSTACK)==TYPE_COMPILED_CLOSURE)		\
	     {	TopOfStack=TOPOFSTACK;					\
		fn_defcell = (DefCell *) &TopOfStack;			\
		goto op_fn_common;					\
	     }								\
	else {	fn_defcell = (DefCell *) GetDEFCELL68k(NIL_PTR);	\
		goto op_fn_common;					\
	     }								\
	}								\
  goto op_ufn;								\
} /* OPAPPLY */
#else
#define OPAPPLY	{							\
  if ( GET_TOS_1_HI  == SPOS_HI ) {					\
	fn_num_args = GET_TOS_1_LO;					\
	fn_opcode_size = 1;						\
	fn_apply = 1;							\
	fn_atom_index = TOPOFSTACK;					\
	FNTRACER(Trace_APPLY(fn_atom_index));				\
	FNCHECKER(if (quick_stack_check()) Trace_APPLY(fn_atom_index));	\
	if ( (SEGMASK & TOPOFSTACK) == 0)				\
	     {	fn_defcell = (DefCell *) GetDEFCELLlitatom(TOPOFSTACK);	\
		goto op_fn_common;					\
	     }								\
	else switch (GetTypeNumber(TOPOFSTACK))				\
	  {								\
	    case TYPE_NEWATOM:						\
		fn_defcell = (DefCell *) GetDEFCELLnew(TOPOFSTACK);	\
		goto op_fn_common;					\
	    case TYPE_COMPILED_CLOSURE:					\
		TopOfStack=TOPOFSTACK;					\
		fn_defcell = (DefCell *) &TopOfStack; 			\
		goto op_fn_common;					\
	    default: fn_defcell = (DefCell *) GetDEFCELL68k(NIL_PTR);	\
		     goto op_fn_common;					\
	  } /* end of switch */						\
	}								\
  goto op_ufn;								\
} /* OPAPPLY */
#endif /* BIGATOMS */



/****************************************************************/
/******			 OPFN(x)			 ********/
/****************************************************************/

#if (defined(SUN3_OS3_OR_OS4_IL) &&  !(defined(NOASMFNCALL)) )

#define OPFN(x, num_args_fn, fn_xna_args, fn_native)			\
{    /* asm inlines for fn call (much care put into keeping optimizer	\
	from moving things around). */					\
	fn_section1();							\
	fn_section2();							\
	num_args_fn();							\
	fn_section3();							\
	fn_xna_args();							\
	fn_section4();							\
	fast1_dispatcher();		/* nextop0 don't work here */	\
	fn_section5();							\
			/* asm code jumps here when not ccodep */	\
	{ fn_atom_index = Get_AtomNo_PCMAC1;				\
	  fn_defcell = (DefCell *) GetDEFCELL68k(fn_atom_index);	\
	  fn_num_args = x;						\
	  fn_opcode_size = FN_OPCODE_SIZE;				\
	  fn_apply = 0;							\
	  goto op_fn_common;						\
	}								\
}

#define OPFNX								\
{    /* asm inlines for fn call (much care put into keeping optimizer	\
	from moving things around.	*/				\
	fnx_section1();							\
	fn_section2();							\
	fnx_args();							\
	fn_section3();							\
	fnx_xna();							\
	fn_section4();							\
	fast1_dispatcher();		/* nextop0 don't work here */	\
	fn_section5();							\
	fn_atom_index = Get_AtomNo_PCMAC2;				\
	fn_defcell = (DefCell *) GetDEFCELL68k(fn_atom_index);		\
	fn_num_args = Get_BYTE_PCMAC1;				\
	fn_opcode_size = FNX_OPCODE_SIZE;				\
	fn_apply = 0;							\
	goto op_fn_common;						\
		/* *** these carefully arranged to satisfy optimizer */ \
label1:	fast1_dispatcher();						\
									\
}

#else

#define OPFN(argcount, num_args_fn, fn_xna_args, fn_native)		\
{	 /* argcount is a number of the arguments on stack */		\
  register struct fnhead *LOCFNCELL;					\
  register int defcell_word;						\
  register int NEXTBLOCK;						\
  FNTRACER(Trace_FNCall(argcount, Get_AtomNo_PCMAC1, TOPOFSTACK, CSTKPTR-1));			\
  FNCHECKER(if (quick_stack_check()) Trace_FNCall(argcount, Get_AtomNo_PCMAC1, TOPOFSTACK,CSTKPTR-1));	\
  fn_defcell = (DefCell *)GetDEFCELL68k(fn_atom_index = Get_AtomNo_PCMAC1); \
  defcell_word = *(int *)fn_defcell;					\
  FNTPRINT((" def cell = 0x%x.\n", defcell_word));  \
  if(!(fn_defcell->ccodep))						\
	{ /* it's not a CCODEP  */					\
	  fn_num_args = argcount;					\
	  fn_opcode_size = FN_OPCODE_SIZE;				\
	  fn_apply = 0;							\
	  goto op_fn_common;						\
	}								\
  LOCFNCELL = (struct fnhead *)Addr68k_from_LADDR((defcell_word &= POINTERMASK));\
  BCE_CURRENTFX->pc = ((UNSIGNED)PCMAC - (UNSIGNED)FuncObj) + FN_OPCODE_SIZE;\
  FN_STACK_CHECK;							\
  {register UNSIGNED newivar;						\
	newivar = (UNSIGNED) (IVARL = (DLword *)(CSTKPTR-argcount+1));	\
	BCE_CURRENTFX->nextblock =					\
	NEXTBLOCK =							\
		StkOffset_from_68K(newivar);				\
  }									\
  HARD_PUSH(TOPOFSTACK);  /* save TOS */				\
  if( LOCFNCELL->na >= 0 )						\
  {register int RESTARGS;						\
	RESTARGS = argcount - LOCFNCELL->na;				\
	while(RESTARGS <0) {						\
	  HARD_PUSH(NIL_PTR);						\
	  RESTARGS++;							\
	}								\
	CSTKPTRL -= (RESTARGS);						\
  } /* if end */							\
 /* Set up BF */							\
 HARD_PUSH(BF_MARK32 | NEXTBLOCK);					\
 *((LispPTR *)CSTKPTR) = (FX_MARK << 16) | (StkOffset_from_68K(PVAR));	\
 ((struct frameex2 *)CSTKPTR)->fnheader = SWAP_FNHEAD(defcell_word);	\
  CSTKPTRL = (LispPTR *)(((DLword *)CSTKPTR) + FRAMESIZE);		\
  PVARL = (DLword *) CSTKPTR;						\
  {register int result;							\
	result = LOCFNCELL->pv;						\
	if (result >= 0)						\
	  {register LispPTR unboundval;					\
	   unboundval = (LispPTR) 0xffffffff;				\
	   HARD_PUSH(unboundval);					\
	   HARD_PUSH(unboundval);					\
	   if (result > 0)						\
	    {HARD_PUSH(unboundval);					\
	     HARD_PUSH(unboundval);					\
	     result-=1;							\
	     for (; --result >= 0;) {					\
	       HARD_PUSH(unboundval);					\
	       HARD_PUSH(unboundval);					\
	     }								\
	   }								\
	 }								\
 }									\
 CSTKPTRL += 1;								\
 PCMACL = (ByteCode *)LOCFNCELL + LOCFNCELL->startpc + 1;		\
 FuncObj = LOCFNCELL;							\
 nextop0;								\
} /* end OPFN */


/*************** OPFNX *************/
#define OPFNX	{							\
  register struct fnhead *LOCFNCELL;					\
  register DefCell *defcell;	/* this reg is not allocated */		\
  register int NEXTBLOCK;						\
  int num_args = Get_BYTE_PCMAC1;					\
  defcell = (DefCell *) GetDEFCELL68k(Get_AtomNo_PCMAC2);		\
  FNTRACER(Trace_FNCall(num_args, Get_AtomNo_PCMAC2, TOPOFSTACK, CSTKPTR-1));			\
  FNCHECKER(if (quick_stack_check()) Trace_FNCall(num_args, Get_AtomNo_PCMAC2, TOPOFSTACK, CSTKPTR-1));	\
  if( defcell->ccodep == 0 )						\
	{ fn_defcell = defcell;						\
	  fn_num_args = num_args;					\
	  fn_opcode_size = FNX_OPCODE_SIZE;				\
	  fn_atom_index = Get_AtomNo_PCMAC2;				\
	  fn_apply = 0;							\
	  goto op_fn_common;						\
	}								\
  LOCFNCELL = (struct fnhead *)Addr68k_from_LADDR(defcell->defpointer);	\
  BCE_CURRENTFX->pc = ((UNSIGNED)PCMAC - (UNSIGNED)FuncObj) + FNX_OPCODE_SIZE;\
  FN_STACK_CHECK;							\
  {register UNSIGNED newivar;						\
	newivar = (UNSIGNED)(IVARL = (DLword *)(CSTKPTR-num_args+1));	\
	BCE_CURRENTFX->nextblock =					\
	NEXTBLOCK =							\
		StkOffset_from_68K(newivar);				\
  }									\
  HARD_PUSH(TOPOFSTACK);  /* save TOS */				\
  if( LOCFNCELL->na >= 0 )						\
  {register int RESTARGS;						\
	RESTARGS = num_args - LOCFNCELL->na;				\
	while(RESTARGS <0) {						\
	  HARD_PUSH(NIL_PTR);						\
	  RESTARGS++;							\
	}								\
	CSTKPTRL -= (RESTARGS);						\
  } /* if end */							\
 /* Set up BF */							\
 HARD_PUSH(BF_MARK32 | NEXTBLOCK);					\
 *((LispPTR *)CSTKPTR) = (FX_MARK << 16) | (StkOffset_from_68K(PVAR));	\
 ((struct frameex2 *)CSTKPTR)->fnheader = SWAP_FNHEAD(defcell->defpointer);\
  CSTKPTRL = (LispPTR *) (((DLword *)CSTKPTR) + FRAMESIZE);		\
  PVARL = (DLword *) CSTKPTR;						\
  {register int result;							\
	result = LOCFNCELL->pv;						\
	if (result >= 0)						\
	  {register LispPTR unboundval;					\
	   unboundval = (LispPTR) 0xffffffff;				\
	   HARD_PUSH(unboundval);					\
	   HARD_PUSH(unboundval);					\
	   if (result > 0)						\
	    {HARD_PUSH(unboundval);					\
	     HARD_PUSH(unboundval);					\
	     result-=1;							\
	     for (; --result >= 0;) {					\
	       HARD_PUSH(unboundval);					\
	       HARD_PUSH(unboundval);					\
	     }								\
	   }								\
	 }								\
 }									\
 CSTKPTRL += 1;								\
 PCMACL = (ByteCode *)LOCFNCELL + LOCFNCELL->startpc + 1;		\
 FuncObj = LOCFNCELL;							\
} /* end OPFN */

#endif /* NOASMFNCALL */





/****************************************************************/
/******			 OPCHECKAPPLY			 ********/
/****************************************************************/
#ifdef BIGATOMS
#define OPCHECKAPPLY {							\
  register DefCell *defcell;						\
  defcell = (DefCell *) GetDEFCELL68k(TOPOFSTACK & POINTERMASK);		\
  if  (!(  defcell->ccodep  && (((TOPOFSTACK & SEGMASK) == 0) || (GetTypeNumber(TOPOFSTACK) == TYPE_NEWATOM))	&&	\
	( ( defcell->argtype == 0 ) || ( defcell->argtype == 2 ) ) ) )	\
	goto op_ufn;							\
}
#else
#define OPCHECKAPPLY {							\
  register DefCell *defcell;						\
  defcell = (DefCell *) GetDEFCELL68k(TOPOFSTACK & POINTERMASK);		\
  if  (!(  defcell->ccodep  && ((TOPOFSTACK & SEGMASK) == 0) )	&&	\
	( ( defcell->argtype == 0 ) || ( defcell->argtype == 2 )  ) )	\
	goto op_ufn;							\
}
#endif /* BIGATOMS */


/****************************************************************/
/*		UFN_COMMON at op_ufn				*/
/****************************************************************/
#define GetUFNEntry(num)	(((UFN *)UFNTable) + (num))

#define	UFN_COMMON							\
op_ufn:		 use code in XC.c					\
{ register UFN *entry68k;						\
   entry68k = (UFN *)GetUFNEntry(Get_BYTE_PCMAC0);			\
   fn_num_args = entry68k->arg_num;					\
   fn_opcode_size = entry68k->byte_num+1;				\
   fn_atom_index = entry68k->atom_name;					\
   fn_defcell = (DefCell *) GetDEFCELL68k(fn_atom_index);		\
   fn_apply = 0;							\
   goto op_fn_common;							\
  };



/****************************************************************/
/******			 OP_FN_COMMON			 ********/
/* vars:							*/
/*	fn_atom_index						*/
/*	fn_num_args						*/
/*	fn_opcode_size						*/
/*	fn_defcell						*/
/*	fn_apply						*/
/*								*/
/* All Closure Calls go through here				*/
/****************************************************************/
#define needpush NEXTBLOCK
#define OP_FN_COMMON							\
op_fn_common:								\
	ASM_LABEL_OF_FN_COMMON;						\
{ register struct fnhead *LOCFNCELL;					\
  register DefCell *defcell;	/* this reg is not allocated */		\
  CClosure *closure;							\
  LispPTR closure_env = (LispPTR) 0xffffffff;				\
 {register int NEXTBLOCK = NIL;						\
  defcell = fn_defcell;							\
  if(defcell->ccodep == 0) {						\
    if(GetTypeNumber(defcell->defpointer)==TYPE_COMPILED_CLOSURE)	\
	 { /* setup closure */						\
		closure=(CClosure *)Addr68k_from_LADDR(defcell->defpointer);\
		defcell=(DefCell *)closure;				\
		/* not  a closure if closure's env is NIL */		\
		if(closure->env_ptr )					\
			{closure_env =  (LispPTR) (closure->env_ptr);	\
			}						\
	 } /* if end */							\
	else {								\
	/* NOT compiled object . We must use Interpreter*/		\
	defcell = (DefCell *)GetDEFCELL68k(ATOM_INTERPRETER);		\
	needpush = 1;							\
	 } /*else end */						\
  }                                                                     \
  LOCFNCELL = (struct fnhead *)Addr68k_from_LADDR(defcell->defpointer);	\
  BCE_CURRENTFX->pc = ((UNSIGNED)PCMAC 				\
			- (UNSIGNED)FuncObj) + fn_opcode_size;	\
  FNTPRINT(("Saving PC = 0%o (0x%x).\n", 				\
	    BCE_CURRENTFX->pc, PCMAC+fn_opcode_size)); 			\
  FN_STACK_CHECK;							\
  APPLY_POP_PUSH_TEST;							\
 {register UNSIGNED newivar;							\
	newivar = (UNSIGNED)(IVARL = (DLword *) (CSTKPTR+(1-fn_num_args-needpush)));	\
	BCE_CURRENTFX->nextblock =					\
	NEXTBLOCK =							\
		StkOffset_from_68K(newivar);				\
  }									\
  HARD_PUSH(TOPOFSTACK);  /* save TOS */				\
  if( LOCFNCELL->na >= 0 )						\
  {register int RESTARGS;						\
	RESTARGS = fn_num_args - LOCFNCELL->na;				\
	while(RESTARGS <0) {						\
	  HARD_PUSH(NIL_PTR);						\
	  RESTARGS++;							\
	}								\
	CSTKPTRL -= (RESTARGS);						\
  } /* if end */							\
 /* Set up BF */							\
 HARD_PUSH(BF_MARK32 | NEXTBLOCK);					\
 } /* NEXTBLOCK BLOCK */						\
 *((LispPTR *)CSTKPTR) = (FX_MARK << 16) | (StkOffset_from_68K(PVAR));	\
 ((struct frameex2 *)CSTKPTR)->fnheader = SWAP_FNHEAD(defcell->defpointer);\
  CSTKPTRL = (LispPTR *) (((DLword *)CSTKPTR) + FRAMESIZE);		\
  PVARL = (DLword *) CSTKPTR;						\
  {register int result;							\
   register LispPTR unboundval;						\
   unboundval = (LispPTR) 0xffffffff;					\
   result = LOCFNCELL->pv;						\
   HARD_PUSH(closure_env);						\
   HARD_PUSH(unboundval);						\
   for (; --result >= 0;) {						\
       HARD_PUSH(unboundval);						\
       HARD_PUSH(unboundval);						\
       }								\
 }	/* result, unboundval block */					\
 CSTKPTRL += 1;								\
 PCMACL = (ByteCode *)LOCFNCELL + LOCFNCELL->startpc + 1;		\
 FuncObj = LOCFNCELL;							\
 SWAPPED_FN_CHECK;    /* see if callee needs swapping */      \
 CHECK_INTERRUPT;							\
 nextop0;								\
} /* end OP_FN_COMMON */



/************************************************************************/
/*									*/
/*			O P _ E N V C A L L				*/
/*									*/
/*	Environment call on a code object.  Takes an arg count on	*/
/*	the stack, along with a pointer to an environment.  If non-	*/
/*	NIL, the environment is stuffed into the PVAR0 slot of the	*/
/*	frame. [This NIL check is in the UFN, and seems to be meant	*/
/*	to allow closures to be called without an environment, without	*/
/*	the compiler having to emit special code.]			*/
/*									*/
/************************************************************************/

#define OP_ENVCALL	{						\
  register struct fnhead *LOCFNCELL;					\
  register int NEXTBLOCK;						\
  register LispPTR closure_env = TOPOFSTACK;				\
  register int num_args;						\
  register LispPTR Fn_DefCell=  GET_TOS_1;				\
  LOCFNCELL = (struct fnhead *)Addr68k_from_LADDR(Fn_DefCell);		\
  FNTPRINT(("ENVCall.\n"));						\
  FNCHECKER(if (quick_stack_check()) printf("In ENVCALL.\n"));	\
  N_GETNUMBER(GET_TOS_2, num_args, op_ufn);				\
  BCE_CURRENTFX->pc = ((UNSIGNED)PCMAC - (UNSIGNED)FuncObj) + 1;\
  FN_STACK_CHECK;							\
  CSTKPTRL -= 2;							\
  {register UNSIGNED newivar;						\
	newivar = (UNSIGNED) (IVARL = (DLword *) (CSTKPTR-num_args));			\
	BCE_CURRENTFX->nextblock =					\
	NEXTBLOCK =							\
		StkOffset_from_68K(newivar);				\
  }									\
  if( LOCFNCELL->na >= 0 )						\
  {register int RESTARGS;						\
	RESTARGS = num_args - LOCFNCELL->na;				\
	while(RESTARGS <0) {						\
	  HARD_PUSH(NIL_PTR);						\
	  RESTARGS++;							\
	}								\
	CSTKPTRL -= (RESTARGS);						\
  } /* if end */							\
 /* Set up BF */							\
 HARD_PUSH(BF_MARK32 | NEXTBLOCK);					\
 *((LispPTR *)CSTKPTR) = (FX_MARK << 16) | (StkOffset_from_68K(PVAR));	\
 ((struct frameex2 *)CSTKPTR)->fnheader = SWAP_FNHEAD(Fn_DefCell);	\
  CSTKPTRL = (LispPTR *)(((DLword *)CSTKPTR) + FRAMESIZE);		\
  PVARL = (DLword *) CSTKPTR;						\
  {register int result;							\
	result = LOCFNCELL->pv;						\
	if (result >= 0)						\
	  {register LispPTR unboundval;					\
	   unboundval = (LispPTR) 0xffffffff;				\
	   if (closure_env == NIL_PTR) HARD_PUSH(unboundval);		\
	     else HARD_PUSH(closure_env);				\
	   HARD_PUSH(unboundval);					\
	   if (result > 0)						\
	    {HARD_PUSH(unboundval);					\
	     HARD_PUSH(unboundval);					\
	     result-=1;							\
	     for (; --result >= 0;) {					\
	       HARD_PUSH(unboundval);					\
	       HARD_PUSH(unboundval);					\
	     }								\
	   }								\
	 }								\
 }									\
 CSTKPTRL += 1;								\
 PCMACL = (ByteCode *)LOCFNCELL + LOCFNCELL->startpc + 1;		\
 FuncObj = LOCFNCELL;							\
 SWAPPED_FN_CHECK;                            \
} /* end OP_ENVCALL */


	/***************************/
	/*								*/
	/*  Check a code block to make sure, on a byte-swapped	*/
	/*	machine, that the code stream has been put back		*/
	/*	in "natural order" for faster fetching.				*/
	/*	(Only in on ISC, now.								*/
    /********************************************************/
#ifdef RESWAPPEDCODESTREAM
#define SWAPPED_FN_CHECK \
  if (!FuncObj->byteswapped) { byte_swap_code_block(FuncObj); FuncObj->byteswapped = 1;}
#else
#define SWAPPED_FN_CHECK
#endif /* RESWAPPEDCODESTREAM */




/****************************************************************/
/******			 EVAL				 ********/
/****************************************************************/
#ifndef BIGATOMS
#define EVAL								\
  {									\
    LispPTR scratch;							\
    register LispPTR work;						\
    register LispPTR lookuped; /* keep looked up value */		\
									\
    switch(TOPOFSTACK & SEGMASK)					\
      {									\
	case S_POSITIVE:						\
	case S_NEGATIVE:  nextop1;					\
									\
	case ATOM_OFFSET: if( (TOPOFSTACK==NIL_PTR)			\
			     ||(TOPOFSTACK==ATOM_T))			\
				goto Hack_Label;			\
			    nnewframe(CURRENTFX,&scratch,		\
				      TOPOFSTACK & 0xffff);		\
			    work = ((scratch & 0xffff0000)>> 16) |	\
				   ((scratch & 0x00ff) <<16);		\
			    lookuped = *((LispPTR *)			\
					(Addr68k_from_LADDR(work)));	\
			    if(lookuped==NOBIND_PTR) goto op_ufn;	\
			    TOPOFSTACK = lookuped;			\
		Hack_Label: nextop1;					\
									\
	default:  switch(GetTypeNumber(TOPOFSTACK))			\
		    {							\
		      case TYPE_FIXP :					\
		      case TYPE_FLOATP :				\
		      case TYPE_STRINGP :				\
		      case TYPE_ONED_ARRAY :				\
		      case TYPE_GENERAL_ARRAY :	nextop1;		\
									\
		      case TYPE_LISTP :					\
			fn_atom_index = ATOM_EVALFORM;			\
			fn_num_args = 1;				\
			fn_opcode_size = 1;				\
			fn_defcell = (DefCell *)			\
				GetDEFCELL68k(ATOM_EVALFORM);		\
			fn_apply = 0;					\
			goto op_fn_common;				\
									\
		      default :	 goto op_ufn;				\
				}					\
									\
  } /* end switch */							\
									\
}/* EVAL end */

#else

#define EVAL								\
  {									\
    LispPTR scratch;							\
    register LispPTR work;						\
    register LispPTR lookuped; /* keep looked up value */		\
									\
    switch(TOPOFSTACK & SEGMASK)					\
      {									\
	case S_POSITIVE:						\
	case S_NEGATIVE:  nextop1;					\
									\
	case ATOM_OFFSET: if( (TOPOFSTACK==NIL_PTR)			\
			     ||(TOPOFSTACK==ATOM_T))			\
				goto Hack_Label;			\
			    nnewframe(CURRENTFX,&scratch,		\
				      TOPOFSTACK & 0xffff);		\
			    work = ((scratch & 0xffff0000)>> 16) |	\
				   ((scratch & 0x0fff) <<16);		\
			    lookuped = *((LispPTR *)			\
					(Addr68k_from_LADDR(work)));	\
			    if(lookuped==NOBIND_PTR) goto op_ufn;	\
			    TOPOFSTACK = lookuped;			\
		Hack_Label: nextop1;					\
									\
	default:  switch(GetTypeNumber(TOPOFSTACK))			\
		    {							\
		      case TYPE_FIXP :					\
		      case TYPE_FLOATP :				\
		      case TYPE_STRINGP :				\
		      case TYPE_ONED_ARRAY :				\
		      case TYPE_GENERAL_ARRAY :	nextop1;		\
									\
		      case TYPE_LISTP :					\
			fn_atom_index = ATOM_EVALFORM;			\
			fn_num_args = 1;				\
			fn_opcode_size = 1;				\
			fn_defcell = (DefCell *)			\
				GetDEFCELL68k(ATOM_EVALFORM);		\
			fn_apply = 0;					\
			goto op_fn_common;				\
									\
		      case TYPE_NEWATOM:				\
			nnewframe(CURRENTFX, &scratch, TOPOFSTACK);	\
			work =	POINTERMASK & SWAP_WORDS(scratch);		\
			lookuped = *((LispPTR *)			\
				   (Addr68k_from_LADDR(work)));		\
			if(lookuped==NOBIND_PTR) goto op_ufn;		\
			TOPOFSTACK = lookuped;				\
			nextop1;					\
		      default :	 goto op_ufn;				\
				}					\
									\
  } /* end switch */							\
									\
}/* EVAL end */
#endif /* BIGATOMS */

