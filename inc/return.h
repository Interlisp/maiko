/* $Id: return.h,v 1.2 1999/01/03 02:06:22 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/**************************************************************/
/*

	File Name : 	return.h
	Desc. :		Macros for return,contextsw

	Written by :	Takeshi Shimizu
			11-May-88

*/
/**************************************************************/


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


#ifdef BIGVM
#define FX_FNHEADER CURRENTFX->fnheader
#else
#define FX_FNHEADER (CURRENTFX->hi2fnheader << 16) | CURRENTFX->lofnheader
#endif /* BIGVM */



/* FAST case return use */
#ifndef ISC
#define FastRetCALL							\
  {									\
    /* Get IVar from Retunee's IVAR offset slot(BF) */ 			\
    IVar = Addr68k_from_LADDR(STK_OFFSET | GETWORD((DLword *)CURRENTFX -1)); \
    /* Get FuncObj from Retunee's FNHEAD slot in FX */ 			\
    FuncObj = (struct fnhead *)						\
		Addr68k_from_LADDR(FX_FNHEADER);	\
    /* Get PC from Retunee's pc slot in FX */ 				\
    PC = (ByteCode *)FuncObj + CURRENTFX->pc ; 				\
  }
#else
#define FastRetCALL							\
  {									\
    /* Get IVar from Retunee's IVAR offset slot(BF) */ 			\
    IVar = Addr68k_from_LADDR(STK_OFFSET | GETWORD((DLword *)CURRENTFX -1)); \
    /* Get FuncObj from Retunee's FNHEAD slot in FX */ 			\
    FuncObj = (struct fnhead *)						\
		Addr68k_from_LADDR(FX_FNHEADER);	\
    /* Get PC from Retunee's pc slot in FX */ 				\
    PC = (ByteCode *)FuncObj + CURRENTFX->pc ; 				\
    if (!(FuncObj->byteswapped))					\
      {								\
	byte_swap_code_block(FuncObj);				\
	FuncObj->byteswapped = 1;				\
      }								\
  }
#endif /* ISC */



/** in CONTEXTSW , for exchanging context **/

#define Midpunt(fxnum) 							\
  { register DLword midpunt; 					\
    midpunt = LOLOC(LADDR_from_68k(CURRENTFX));			\
    PVar=(DLword *)							\
	    Addr68k_from_StkOffset(					\
			(GETWORD(((DLword *)InterfacePage) +fxnum)))	\
		+ FRAMESIZE; 						\
    GETWORD(((DLword *)InterfacePage) +fxnum) = midpunt ;		\
  }


#define CHECKFX							\
  if (((UNSIGNED)PVar -(UNSIGNED)CURRENTFX) != 20)			\
    { printf("Invalid FX(0x%x) and PV(0x%x) \n",		\
	     LADDR_from_68k(CURRENTFX),LADDR_from_68k(PVar));	\
    }



/**** Calls when invoke the function is assumed
	that it is called by CONTEXTSW in orginal LISP code **/

#define BEFORE_CONTEXTSW						\
  { CurrentStackPTR += 2; 						\
    CURRENTFX->nextblock=StkOffset_from_68K(CurrentStackPTR); 		\
    GETWORD(CurrentStackPTR)=STK_FSB_WORD; 				\
    GETWORD(CurrentStackPTR+1)= (((UNSIGNED)EndSTKP-(UNSIGNED)(CurrentStackPTR))>>1); \
    if (GETWORD(CurrentStackPTR+1) == 0) error("0-long free block."); \
  }


#define AFTER_CONTEXTSW							\
  { register DLword *ptr68k,*freeptr;					\
    ptr68k = (DLword*)Addr68k_from_StkOffset(CURRENTFX->nextblock);	\
    if(GETWORD(ptr68k) != STK_FSB_WORD) error("pre_moveframe: MP9316");	\
    CHECK_FX(CURRENTFX);						\
    freeptr=ptr68k;							\
    while(GETWORD(freeptr) == STK_FSB_WORD)  				\
      EndSTKP=freeptr=freeptr+  GETWORD(freeptr+1);			\
    S_CHECK(CURRENTFX->incall== NIL, "CONTEXTSW during fn call");	\
    /*S_CHECK(CURRENTFX->nopush== NIL, "CONTEXTSW, NOPUSH is set");	** JDS 4/9/96 this seems not to matter, so I removed it. */\
    CurrentStackPTR = ptr68k- 2 ;					\
    CHECK_FX(CURRENTFX);						\
    S_CHECK( EndSTKP > CurrentStackPTR, 				\
		"End of stack isn't beyond current stk pointer."); 	\
  }

