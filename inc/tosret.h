#ifndef TOSRET_H
#define TOSRET_H 1
/* $Id: tosret.h,v 1.2 1999/01/03 02:06:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1998 Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "kprintdefs.h" // for prindatum

/************************************************************************/
/*									*/
/*			t o s r e t m a c r o . h			*/
/*									*/
/*	Implements RETURN for the inner evaluation loop.		*/
/*									*/
/************************************************************************/

#define OPRETURN	{						\
 struct frameex2 *returnFX ;					\
 int alink;							\
 FNCHECKER(struct frameex2 *old_bce_fx = (struct frameex2 *) BCE_CURRENTFX); \
 alink = ((struct frameex2 *) BCE_CURRENTFX)->alink;			\
 FNTPRINT(("RETURN = 0x%x,  ", TOPOFSTACK));						\
 FNTRACER(prindatum(TOPOFSTACK); printf("\n"); fflush(stdout);)  \
 if (alink & 1) { EXT; if(slowreturn()) goto stackoverflow_help; RET;	\
	Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,FuncObj);		\
	if (((UNSIGNED)(CSTKPTR) >= Irq_Stk_Check) || (Irq_Stk_End <= 0))	\
			{ goto check_interrupt;	}			\
	Irq_Stk_End = (UNSIGNED) EndSTKP;					\
	goto retxit;							\
	};								\
 CSTKPTRL = (LispPTR *) IVAR;						\
 returnFX = (struct  frameex2 *)					\
	((DLword *)							\
	    (PVARL = (DLword *) NativeAligned2FromStackOffset(alink))	\
	    - FRAMESIZE);						\
 IVARL = (DLword *)							\
	    NativeAligned2FromStackOffset(GETWORD((DLword *)returnFX -1));\
	/* Get PC from Returnee's pc slot in FX */			\
 PCMACL = returnFX->pc  + (ByteCode *)					\
	(FuncObj = (struct fnhead *)					\
	NativeAligned4FromLAddr(SWAP_FNHEAD(returnFX->fnheader) & POINTERMASK)) + 1;\
 Irq_Stk_Check = STK_END_COMPUTE(EndSTKP,FuncObj);			\
  FNCHECKER(if (quick_stack_check()) printf("In RETURN.\n"));	\
 if (((UNSIGNED)(CSTKPTR) >= Irq_Stk_Check) || (Irq_Stk_End <= 0))		\
		{ goto check_interrupt;	}				\
 Irq_Stk_End = (UNSIGNED) EndSTKP;						\
retxit:	 {}								\
} /* OPRETURN end */

#endif /* TOSRET_H */
