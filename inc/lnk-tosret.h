/* $Id: lnk-tosret.h,v 1.2 1999/01/03 02:06:12 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1992 Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*			t o s r e t m a c r o . h			*/
/*									*/
/*	Implements RETURN for the inner evaluation loop.  There are	*/
/*	two versions--one for when hand optimization has been done,	*/
/*	and one for the naive case.  To use the hand-optimization	*/
/*	version, you'll need to define an inline function or macro	*/
/*	called opreturn().  It must fall thru if alink is odd, but	*/
/*	must handle all other cases.  You can rely on check_interrupt	*/
/*	being a defined label.						*/
/*									*/
/************************************************************************/

#ifdef NATIVETRAN
#define RETD6 SaveD6 = 0x100
#define RET_CHECK_NATIVE(x) if(x ->native) { RET_TO_NATIVE; }
#else
#define RETD6
#define RET_CHECK_NATIVE(x)
#endif


#if ((defined(ISC) || defined(SUN3_OS3_OR_OS4_IL)) &&  !(defined(NOASMFNCALL)) )

#define OPRETURN							\
{	opreturn();							\
	EXT; if(slowreturn()) goto stackoverflow_help; RET;		\
	Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,FuncObj);		\
	if (((int)(CSTKPTR) > Irq_Stk_Check) || (Irq_Stk_End <= 0))	\
			{ RETD6; goto check_interrupt;	}		\
	Irq_Stk_End = (int) EndSTKP;					\
	RET_CHECK_NATIVE(BCE_CURRENTFX);				\
 }

#else

#define OPRETURN	{						\
 register struct frameex2 *returnFX ;					\
 register int alink;							\
 alink = ((struct frameex2 *) BCE_CURRENTFX)->alink;			\
 FNTPRINT(("RETURN = 0x%x,  ", TOPOFSTACK));						\
 FNTRACER(prindatum(TOPOFSTACK); printf("\n"); fflush(stdout);)  \
 if (alink & 1) { EXT; if(slowreturn()) goto stackoverflow_help; RET;	\
	Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,FuncObj);		\
	if (((int)(CSTKPTR) > Irq_Stk_Check) || (Irq_Stk_End <= 0))	\
			{ RETD6; goto check_interrupt;	}		\
	Irq_Stk_End = (int) EndSTKP;					\
	RET_CHECK_NATIVE(BCE_CURRENTFX);				\
	goto retxit;							\
	};								\
 CSTKPTRL = (LispPTR *) IVAR;						\
 returnFX = (struct  frameex2 *)					\
	((DLword *)							\
	    (PVARL = (DLword *) Addr68k_from_StkOffset(alink))		\
	    - FRAMESIZE);						\
 IVARL = (DLword *)							\
	    Addr68k_from_StkOffset(GETWORD((DLword *)returnFX -1));	\
	/* Get PC from Retunee's pc slot in FX */			\
 PCMACL = returnFX->pc  + (ByteCode *)					\
	(FuncObj = (struct fnhead *)					\
	Addr68k_from_LADDR(SWAP_FNHEAD(returnFX->fnheader) & POINTERMASK)) + 1;\
 Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,FuncObj);			\
  FNCHECKER(if (quick_stack_check()) printf("In RETURN.\n"));	\
 if (((int)(CSTKPTR) > Irq_Stk_Check) || (Irq_Stk_End <= 0))		\
		{ RETD6; goto check_interrupt;	}		\
 Irq_Stk_End = (int) EndSTKP;						\
 RET_CHECK_NATIVE(returnFX);						\
retxit:	 {}								\
} /* OPRETURN end */

#endif
