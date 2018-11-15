/* $Id: inlineC.h,v 1.3 1999/01/03 02:06:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

/* 	
	These are the Macros Used to generate inline c code.
	These are the goto ni definitions of the opcodes.
*/

/************************************************************************/
/*									*/
/*	    Macros for fetching bytes & words PC-relative		*/
/*									*/
/*	These are this way so that they can be redefined for the 386i,	*/
/*	where it makes a difference in speed if you know in advance.	*/
/*									*/
/************************************************************************/

#define Get_BYTE_PCMAC0 Get_BYTE(PCMAC)
#define Get_BYTE_PCMAC1 Get_BYTE(PCMAC+1)
#define Get_BYTE_PCMAC2 Get_BYTE(PCMAC+2)
#define Get_BYTE_PCMAC3 Get_BYTE(PCMAC+3)

#define Get_DLword_PCMAC0 Get_DLword(PCMAC)
#define Get_DLword_PCMAC1 Get_DLword(PCMAC+1)
#define Get_DLword_PCMAC2 Get_DLword(PCMAC+2)
#define Get_DLword_PCMAC3 Get_DLword(PCMAC+3)

#define Get_Pointer_PCMAC0
#define Get_Pointer_PCMAC1 Get_Pointer(PCMAC + 1)
#define Get_Pointer_PCMAC2 Get_Pointer(PCMAC + 2)

	/* For getting a signed byte */
#ifndef BYTESWAP
/* in the unswapped case, the type of the argument carries through to the result */
#define Get_SBYTE_PCMAC0 GETBYTE((s_char *)PCMAC)
#define Get_SBYTE_PCMAC1 GETBYTE((s_char *)PCMAC+1)
#else
/* cf. GETBYTE in lsptypes.h */
#define Get_SBYTE_PCMAC0 (* (s_char *) (3^(UNSIGNED)(PCMAC)))
#define Get_SBYTE_PCMAC1 (* (s_char *) (3^(UNSIGNED)(PCMAC+1)))
#endif

	/* for getting an atom number, e.g., for FNx or DTEST */
#ifdef BIGATOMS
#define Get_AtomNo_PCMAC1 Get_Pointer_PCMAC1
#define Get_AtomNo_PCMAC2 Get_Pointer_PCMAC2
#ifdef BIGVM
#define nextop_atom nextop5
#define nextop_ptr nextop5
#else
#define nextop_atom nextop4
#define nextop_ptr nextop4
#endif /* BIGVM */
#else
#define Get_AtomNo_PCMAC1 Get_DLword_PCMAC1
#define Get_AtomNo_PCMAC2 Get_DLword_PCMAC2
#define nextop_atom nextop3
#define nextop_ptr nextop4
#endif /* BIGATOMS */




#define CHECK_INTERRUPT {if((UNSIGNED)CSTKPTR > Irq_Stk_Check) goto check_interrupt;} 
#define SWAP_WORDS(x) (((unsigned int)x << 16) | (((unsigned int)x >> 16) & 0xFFFF))


#define NATIVECHECK							\
	{if (BCE_CURRENTFX->native) {goto gonative ;} nextop0; }

#define nextop0 {goto nextopcode; }
#define nextop1 {PCMACL += 1; nextop0; }
#define nextop2 {PCMACL += 2; nextop0; }
#define nextop3 {PCMACL += 3; nextop0; }
#define nextop4 {PCMACL += 4; nextop0; }
#define nextop5 {PCMACL += 5; nextop0; }

#define OPCAR \
 if (Listp(TOPOFSTACK))\
 {\
 register ConsCell *DATUM68K = (ConsCell *)(Addr68k_from_LADDR(TOPOFSTACK));\
	if (DATUM68K->cdr_code == CDR_INDIRECT)\
	{\
		TOPOFSTACK = ((LispPTR)((ConsCell *)Addr68k_from_LADDR(DATUM68K->car_field))->car_field);\
		nextop1; \
	}\
	else \
	{ \
		TOPOFSTACK = ((LispPTR)DATUM68K->car_field);\
		nextop1; \
  } \
  }\
 else	if (TOPOFSTACK == NIL_PTR)\
		{ nextop1; } \
	else if ( TOPOFSTACK == ATOM_T)\
		{ nextop1; } \
	else \
	  {\
		goto op_ufn; \
	  } /* end of OPCAR */

#ifdef NEWCDRCODING
#define OPCDR  \
 if (Listp(TOPOFSTACK))\
 {\
register ConsCell *DATUM68K = (ConsCell *)(Addr68k_from_LADDR(TOPOFSTACK));\
register int CDRCODEX = DATUM68K->cdr_code;\
	if (CDRCODEX == CDR_NIL) {\
		/* cdr-nil */\
		TOPOFSTACK = (NIL_PTR);\
		nextop1; \
		} \
	else if (CDRCODEX> CDR_ONPAGE) {\
		/* cdr-samepage */\
		TOPOFSTACK = ((TOPOFSTACK) + ((CDRCODEX & 7) << 1));\
		nextop1; \
		} \
	else if (CDRCODEX == CDR_INDIRECT) {	/* CDRCODEX < CDR_ONPAGE */\
		/* cdr-indirect */\
		TOPOFSTACK = (cdr ((LispPTR)(DATUM68K->car_field)));\
		nextop1; \
		} \
	else\
	{\
		/* cdr-differentpage */\
		TOPOFSTACK = ((ConsCell *)(Addr68k_from_LADDR \
		((TOPOFSTACK) + (CDRCODEX << 1)))\
											)->car_field;\
		nextop1; \
	}\
  }\
 else	if (TOPOFSTACK == NIL_PTR)\
		{ nextop1; } \
	else\
	 {\
		goto op_ufn; \
	 } /* end of OPCDR */
#else
#define OPCDR  \
 if (Listp(TOPOFSTACK))\
 {\
register ConsCell *DATUM68K = (ConsCell *)(Addr68k_from_LADDR(TOPOFSTACK));\
register int CDRCODEX = DATUM68K->cdr_code;\
	if (CDRCODEX == CDR_NIL) {\
		/* cdr-nil */\
		TOPOFSTACK = (NIL_PTR);\
		nextop1; \
		} \
	else if (CDRCODEX> CDR_ONPAGE) {\
		/* cdr-samepage */\
		TOPOFSTACK = (POINTER_PAGEBASE(TOPOFSTACK) + \
		((CDRCODEX & 127) << 1));\
		nextop1; \
		} \
	else if (CDRCODEX == CDR_INDIRECT) {	/* CDRCODEX < CDR_ONPAGE */\
		/* cdr-indirect */\
		TOPOFSTACK = (cdr ((LispPTR)(DATUM68K->car_field)));\
		nextop1; \
		} \
	else\
	{\
		/* cdr-differentpage */\
		TOPOFSTACK = ((ConsCell *)(Addr68k_from_LADDR \
		(POINTER_PAGEBASE(TOPOFSTACK) + (CDRCODEX << 1)))\
											)->car_field;\
		nextop1; \
	}\
  }\
 else	if (TOPOFSTACK == NIL_PTR)\
		{ nextop1; } \
	else\
	 {\
		goto op_ufn; \
	 } /* end of OPCDR */
#endif /* NEWCDRCODING */

#define	IVARMACRO(x)	{PUSH(IVAR[x]); nextop1;}
#define	PVARMACRO(x)	{PUSH(PVAR[x]); nextop1;}
#define	PVARSETMACRO(x)	{PVAR[x] = TOPOFSTACK; nextop1;}
#define	PVARSETPOPMACRO(x) {PVAR[x] = TOPOFSTACK; POP; nextop1;}
#define	PUSHATOM(x)	{PUSH(x); nextop1;}


#define	JUMPMACRO(x)	{CHECK_INTERRUPT; PCMACL += x; nextop0;}

#define	FJUMPMACRO(x)	{if(TOPOFSTACK != 0) {goto PopNextop1 ; }	\
			{CHECK_INTERRUPT; POP; PCMACL += x; nextop0;}	\
			}
#define	TJUMPMACRO(x)	{if(TOPOFSTACK == 0) {goto PopNextop1 ; }	\
			{CHECK_INTERRUPT; POP; PCMACL += x; nextop0;}	\
			}

#define GETBASE_N(N)	{ 						\
		TOPOFSTACK = 						\
			(S_POSITIVE | GETWORD((DLword *)			\
			Addr68k_from_LADDR((POINTERMASK & TOPOFSTACK) + N)));\
		nextop2;						\
		}

#define GETBASEPTR_N(N) {						\
		TOPOFSTACK = 						\
			( POINTERMASK & *((LispPTR *)			\
			Addr68k_from_LADDR((POINTERMASK & TOPOFSTACK) + N)));\
		nextop2;						\
		}
#define PUTBASEBYTE							\
	{ register int byteoffset;					\
	  register char	*p_data;					\
	 if(((SEGMASK & TOPOFSTACK) != S_POSITIVE) ||		\
	    ((unsigned short)TOPOFSTACK >= 256))			\
		goto op_ufn;						\
	 byteoffset = GET_TOS_1;					\
	 switch( (SEGMASK & byteoffset) ){				\
	 case S_POSITIVE:						\
	 	byteoffset &=  0x0000FFFF;				\
	 	break;							\
	 case S_NEGATIVE:						\
		byteoffset |=  0xFFFF0000;				\
		break;							\
	 default:							\
		goto op_ufn;						\
	/***	if( GetTypeNumber(byteoffset) == TYPE_FIXP )		\
			byteoffset = *((int *)Addr68k_from_LADDR(byteoffset));	\
		else							\
			goto op_ufn; ***/				\
	 }								\
	 --CSTKPTRL;							\
	 p_data = (char*)Addr68k_from_LADDR(POINTERMASK & (POP_TOS_1)) + byteoffset;	\
	 GETBYTE(p_data) = 0xFF & TOPOFSTACK;				\
	 nextop1;							\
	}

#define GETBASEBYTE							\
	{switch( (SEGMASK & TOPOFSTACK) ){				\
	 case S_POSITIVE:						\
		TOPOFSTACK &=  0x0000FFFF;				\
		break;							\
	 case S_NEGATIVE:						\
		TOPOFSTACK |=  0xFFFF0000;				\
		break;							\
	 default:							\
		if( GetTypeNumber(TOPOFSTACK) == TYPE_FIXP )		\
		 TOPOFSTACK = *((int *)Addr68k_from_LADDR(TOPOFSTACK));	\
		else							\
			goto op_ufn;					\
	 }								\
	 TOPOFSTACK = (0xFF & (GETBYTE((char*)Addr68k_from_LADDR((POINTERMASK & (POP_TOS_1))) + TOPOFSTACK))) | S_POSITIVE;	\
	 nextop1;							\
	}


#define PUTBASEPTR_N(n)							\
	{ register int base;						\
	  base = POINTERMASK & POP_TOS_1;					\
	  *((LispPTR *)Addr68k_from_LADDR(base + n)) = TOPOFSTACK;      \
	  TOPOFSTACK = base;						\
	  nextop2;							\
	}

#define PUTBASE_N(n)							\
	{ register int base;						\
	  if (GetHiWord(TOPOFSTACK) != (S_POSITIVE >> 16))		\
		goto op_ufn;						\
	  base = POINTERMASK & POP_TOS_1;					\
	  GETWORD((DLword *)Addr68k_from_LADDR(base + n)) = GetLoWord(TOPOFSTACK);\
	  TOPOFSTACK = base;						\
	  nextop2;							\
	}


#define PVARX(x)	{ PUSH(GetLongWord((DLword *)PVAR + x)); nextop2; }
#define PVARX_(x)	{ *((LispPTR *)((DLword *)PVAR+x))=TOPOFSTACK; nextop2;}
#define IVARX(x)	{ PUSH(GetLongWord((DLword *)IVAR + x)); nextop2; }
#define IVARX_(x)	{ *((LispPTR *)((DLword *)IVAR+x))=TOPOFSTACK; nextop2;}

#ifndef BIGATOMS
#define GVAR(x)		{ PUSH(GetLongWord(Valspace + ((x)<<1))); nextop_atom; }
#elif defined(BIGVM)
#define GVAR(x)								\
  { register int tx = x;									\
    if (tx & SEGMASK)							\
      {									\
	PUSH(GetLongWord(						\
	      Addr68k_from_LADDR((tx)+NEWATOM_VALUE_OFFSET)));		\
      }									\
    else PUSH(GetLongWord((LispPTR *)AtomSpace + (tx*5) + NEWATOM_VALUE_PTROFF));			\
									\
    nextop_atom;							\
  }
#else
#define GVAR(x)								\
  { register int tx = x;									\
    if (tx & SEGMASK)							\
      {									\
	PUSH(GetLongWord(						\
	      Addr68k_from_LADDR((tx)+NEWATOM_VALUE_OFFSET)));		\
      }									\
    else PUSH(GetLongWord(Valspace + ((tx)<<1)));			\
									\
    nextop_atom;							\
  }
#endif /* BIGATOMS */



#define COPY		{ HARD_PUSH(TOPOFSTACK); nextop1; }

#define SWAP		{ register LispPTR temp;			\
			 temp = GET_TOS_1;				\
			 GET_TOS_1 = TOPOFSTACK;			\
		 	 TOPOFSTACK = temp;				\
			 nextop1;					\
			}

   /*********************************************/
   /* Note: No matter how smart it seems, don't */
   /* AND in POINTERMASK in VAG2, because there */
   /* is code that depends on VAG2 building     */
   /* full, 32-bit pointers from 16-bit ints.   */
   /*********************************************/
#define	N_OP_VAG2	{ TOPOFSTACK = ((GET_TOS_1 << 16)		\
			  | (0xFFFF & TOPOFSTACK)); CSTKPTRL--; nextop1; }

#define FN0		{ OPFN(0, fn0_args, fn0_xna, fn0_native); }
#define FN1		{ OPFN(1, fn1_args, fn1_xna, fn1_native); }
#define FN2		{ OPFN(2, fn2_args, fn2_xna, fn2_native); }
#define FN3		{ OPFN(3, fn3_args, fn3_xna, fn3_native); }
#define FN4		{ OPFN(4, fn4_args, fn4_xna, fn4_native); }
#define FNX		{ OPFNX; nextop0; }
#define ENVCALL		{ OP_ENVCALL; nextop0; }
#define RETURN		{ OPRETURN; nextop0; }
#define APPLY		{ OPAPPLY;}
#define CHECKAPPLY	{ OPCHECKAPPLY; nextop1; }

#define BIN								\
{									\
register Stream *stream68k; /* stream instance on TOS */		\
register  char *buff68k;     /* pointer to BUFF */			\
									\
  if ( GetTypeNumber(TOPOFSTACK) == TYPE_STREAM ) {			\
	stream68k=(Stream *) Addr68k_from_LADDR(TOPOFSTACK);		\
	if( ( !stream68k->BINABLE ) ||					\
	    (  stream68k->COFFSET >=					\
	       stream68k->CBUFSIZE   ) ) goto op_ufn;			\
									\
	/* get BUFFER instance */					\
	buff68k =(char *)Addr68k_from_LADDR(stream68k->CBUFPTR);	\
									\
	/* get BYTE data and set it to TOS */				\
	TOPOFSTACK = (S_POSITIVE |					\
		      (Get_BYTE(buff68k + (stream68k->COFFSET)++)) );	\
	nextop1;							\
	}								\
  else	goto op_ufn;							\
}

#ifdef RECLAIMINC
#define RECLAIMCELL	{ TOPOFSTACK = gcreclaimcell(TOPOFSTACK); nextop1; }
#else
#define RECLAIMCELL	{ goto op_ufn; }
#endif

#define GCSCAN1		{ TOPOFSTACK=gcscan1(TOPOFSTACK & 0xffff);	\
			 if (TOPOFSTACK) {TOPOFSTACK |= S_POSITIVE; };nextop1;}

#define GCSCAN2		{ TOPOFSTACK=gcscan2(TOPOFSTACK & 0xffff);	\
			  if (TOPOFSTACK) {TOPOFSTACK |=S_POSITIVE; };nextop1;}


#define CONTEXTSWITCH	{ EXT; OP_contextsw(); RET; 			\
			  /*CHECK_INTERRUPT;*/ CLR_IRQ;			\
			  NATIVE_NEXTOP0; }

#define	NOP		{ nextop1; }
#define RESLIST(n)	{ goto op_ufn; }

#define FINDKEY(x)							\
		{							\
		 TOPOFSTACK = N_OP_findkey(TOPOFSTACK, x);		\
		 nextop2;						\
		}

#define RPLPTR(n)							\
		{							\
		 TOPOFSTACK = N_OP_rplptr(POP_TOS_1, TOPOFSTACK, n);	\
		 nextop2;						\
		}

#define GVAR_(atom_index)						\
		{							\
		 N_OP_gvar_(TOPOFSTACK, atom_index);			\
		 nextop_atom;						\
		}

#define BIND	{register int byte = Get_BYTE_PCMAC1;			\
		 register int n1;					\
		register int n2;					\
		register LispPTR *ppvar;				\
		register int i;						\
		n1 = byte >> 4;				\
		n2 = byte & 0xf;				\
		ppvar = (LispPTR *)PVAR + 1 + Get_BYTE_PCMAC2;	\
		for(i=n1; --i >= 0;){ *--ppvar = NIL_PTR; }		\
		if(n2 == 0){						\
			*CSTKPTRL++ = TOPOFSTACK;			\
		}else{							\
			*--ppvar = TOPOFSTACK;				\
			for(i=1; i<n2; i++) { *--ppvar = *(--CSTKPTRL); }	\
		}							\
		i = ~(n1 + n2);						\
		TOPOFSTACK = (i<<16) | (Get_BYTE_PCMAC2<<1);		\
		nextop3;						\
		}

#define UNBIND	{register int num;					\
		register LispPTR *ppvar;				\
		register int	i;					\
		register LispPTR value;					\
		for(; ( ((int)*--CSTKPTRL) >= 0 ););			\
		value = *CSTKPTR;					\
		num = (~value)>>16;					\
		ppvar = (LispPTR *)((DLword *)PVAR + 2 + GetLoWord(value));\
		for(i=num; --i >= 0;){*--ppvar = 0xffffffff;}		\
		nextop1;						\
		}

#define DUNBIND	{register int num;					\
		register LispPTR *ppvar;				\
		register int	i;					\
		register LispPTR value;					\
		if((int)TOPOFSTACK < 0){ 				\
		  num =(~TOPOFSTACK)>>16;				\
		  if(num != 0){						\
		    ppvar = (LispPTR *)((DLword *)PVAR + 2 + GetLoWord(TOPOFSTACK)); \
		    for(i=num; --i >= 0;) {  \
					    *--ppvar = 0xffffffff; }	\
		  }							\
		}else{							\
		for(; ( ((int)*--CSTKPTRL) >= 0 ););			\
		  value = *CSTKPTR;					\
		  num = (~value)>>16;					\
		  ppvar = (LispPTR *)((DLword *)PVAR + 2 + GetLoWord(value));\
		  for(i=num; --i >= 0;) {  \
					  *--ppvar = 0xffffffff; }	\
		}							\
		POP;							\
		nextop1;						\
		}

#define N_OP_HILOC							\
		{							\
		 TOPOFSTACK = GetHiWord(TOPOFSTACK) | S_POSITIVE;	\
		 nextop1;						\
		}
#define N_OP_LOLOC							\
		{							\
		 TOPOFSTACK = GetLoWord(TOPOFSTACK) | S_POSITIVE;	\
		 nextop1;						\
		}

#define GETBITS_N_M(a, b)						\
		{register int temp, bb = b;					\
		temp = 0xF & bb;						\
		TOPOFSTACK = S_POSITIVE | 				\
		 (( (GETWORD(Addr68k_from_LADDR(POINTERMASK & (TOPOFSTACK+a))))	\
			>> (16 - ( (0xF & (bb >> 4)) + temp + 1)) )	\
			& n_mask_array[temp] );				\
		 nextop3;						\
		}

#define PUTBITS_N_M(a, b)						\
	{ int	base;							\
	  register int bb = b;					\
	  register DLword	*pword;					\
	  register int shift_size, field_size, fmask;			\
	 if( (SEGMASK & TOPOFSTACK) != S_POSITIVE ){ goto op_ufn; };	\
	 base = POINTERMASK & POP_TOS_1;					\
	 pword = (DLword*)Addr68k_from_LADDR( base + a );		\
	 field_size = 0xF & bb;						\
	 shift_size = 15 - (0xF & (bb >> 4)) - field_size;		\
	 fmask = n_mask_array[field_size] << shift_size;		\
	 GETWORD(pword) = ( (TOPOFSTACK << shift_size) & fmask) |	\
			    (GETWORD(pword) & (~fmask));		\
	 TOPOFSTACK = base;						\
	 nextop3;							\
	}


#define CONS								\
	{ TOPOFSTACK = N_OP_cons(POP_TOS_1, TOPOFSTACK);		\
	  nextop1;							\
	}

#define MYALINK								\
	{								\
	 PUSH(((( CURRENTFX->alink) & 0xfffe)-FRAMESIZE) | S_POSITIVE);	\
	 nextop1;							\
	}

#define MYARGCOUNT							\
	{ register UNSIGNED arg_num;						\
	  if (( CURRENTFX->alink & 1) == 0)				\
	    arg_num = (UNSIGNED)((LispPTR *)(CURRENTFX) - 1);				\
 	  else								\
	    arg_num = (UNSIGNED)(Stackspace + CURRENTFX->blink);		\
	 PUSH( (DLword)((arg_num - (UNSIGNED)IVar) >> 2) | S_POSITIVE);	\
	 nextop1;							\
	}

#define RCLK								\
	{								\
	 TOPOFSTACK = N_OP_rclk(TOPOFSTACK);				\
	 nextop1;							\
	}

#define LISTP	{							\
		 if((DLword)GetTypeNumber(TOPOFSTACK) != TYPE_LISTP)\
    			TOPOFSTACK = NIL_PTR;				\
		 nextop1;						\
		}

#define NTYPEX								\
		{							\
		 TOPOFSTACK = S_POSITIVE | (DLword)(GetTypeNumber(TOPOFSTACK));\
		nextop1;						\
		}

#define TYPEP(n) 							\
		{							\
		  if((DLword)GetTypeNumber(TOPOFSTACK) != n)		\
    			TOPOFSTACK = NIL_PTR;				\
		 nextop2;						\
		}

#define TYPEMASK(n) 							\
		{							\
  		 if( ( ((DLword)GetTypeEntry(TOPOFSTACK))  & 		\
		     ( (DLword)n << 8)) == 0)				\
    			TOPOFSTACK = NIL_PTR;				\
		 nextop2;						\
		}

#define INSTANCEP(atom_index)						\
		{							\
		 TOPOFSTACK = N_OP_instancep(TOPOFSTACK,atom_index);	\
		 nextop_atom;						\
		}

#define STOREN(n)							\
		{ *(CSTKPTR  - ((n+2) >> 1)) = TOPOFSTACK;		\
		 nextop2;						\
		}

#define COPYN(n)							\
		{ PUSH(*(CSTKPTR  - ((n+2) >> 1)));			\
		 nextop2;						\
		}

#define POPN(n)								\
		{TOPOFSTACK = *(CSTKPTRL -= ((n)+1));			\
		 nextop2;						\
		}

#define CLARITHEQUAL	{						\
register int arg2;							\
  SV; arg2 = POP_TOS_1;							\
  if ((TOPOFSTACK & SEGMASK) == S_POSITIVE)				\
  {									\
  if (arg2 == TOPOFSTACK) {TOPOFSTACK = ATOM_T; nextop1;}		\
  if ((arg2 & SEGMASK) == S_POSITIVE) {TOPOFSTACK = NIL; nextop1;}	\
  }									\
  N_OP_POPPED_CALL_2(N_OP_eqq, arg2);					\
}

#define S_CHARACTER 0x70000

#define AREF1	{							\
LispPTR arrayarg;							\
register LispPTR baseL;							\
register int index;							\
register OneDArray *arrayblk;						\
DLword	*createcell68k();						\
    SV; arrayarg = POP_TOS_1;						\
    if (GetTypeNumber(arrayarg) != TYPE_ONED_ARRAY) goto aref_ufn;	\
    arrayblk = (OneDArray *)Addr68k_from_LADDR(arrayarg);		\
    if ((TOPOFSTACK & SEGMASK) != S_POSITIVE) goto aref_ufn;		\
    index = TOPOFSTACK & 0xFFFF;					\
    if (index >= arrayblk->totalsize) goto aref_ufn;		\
    index += arrayblk->offset;				\
    baseL = arrayblk->base;					\
    switch (arrayblk->typenumber) {			\
      case 38: /* pointer : 32 bits */					\
         TOPOFSTACK = *(((int *)Addr68k_from_LADDR(baseL)) + index);	\
         nextop1;							\
      case 20: /* signed : 16 bits */					\
         TOPOFSTACK = (GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF;									\
         if (TOPOFSTACK & 0x8000) TOPOFSTACK |= S_NEGATIVE;		\
         else TOPOFSTACK |= S_POSITIVE;					\
         nextop1;							\
      case 67: /* Character :  8 bits */				\
         TOPOFSTACK = S_CHARACTER | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF);								\
         nextop1;							\
      case 22: /* signed : 32 bits */					\
         TOPOFSTACK = *(((int *)Addr68k_from_LADDR(baseL)) + index);	\
         switch(TOPOFSTACK & 0xFFFF0000){				\
           case 0:							\
             TOPOFSTACK |= S_POSITIVE;					\
             break;							\
           case (unsigned)0xFFFF0000:					\
             TOPOFSTACK &= S_NEGATIVE;					\
             break;							\
           default:{register DLword *wordp;				\
             wordp = createcell68k(TYPE_FIXP);				\
             *((int *)wordp) = TOPOFSTACK;				\
             TOPOFSTACK = (LispPTR)LADDR_from_68k(wordp);		\
             }								\
         }								\
         nextop1;							\
      case 0: /* unsigned : 1 bit per element */			\
         TOPOFSTACK = S_POSITIVE | (((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + (index >> 3))) >> (7 - (index & 7))) & 1);				\
         nextop1;							\
      case 3: /* unsigned : 8 bits per element */			\
         TOPOFSTACK = S_POSITIVE | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF);								\
         nextop1;							\
      case 4: /* unsigned : 16 bits per element */			\
         TOPOFSTACK = S_POSITIVE | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF);							\
         nextop1;							\
      case 54: /* Float : 32 bits */{register DLword *wordp;		\
         wordp = createcell68k(TYPE_FLOATP);				\
         *((int *)wordp) = *(((int *)Addr68k_from_LADDR(baseL)) + index);\
         TOPOFSTACK = (LispPTR)LADDR_from_68k(wordp);			\
         }								\
         nextop1;							\
      case 68: /* Character :  16 bits */				\
         TOPOFSTACK = S_CHARACTER | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF);							\
         nextop1;							\
      case 86: /* XPointer : 32 bits */					\
         TOPOFSTACK = *(((int *)Addr68k_from_LADDR(baseL)) + index);	\
         nextop1;							\
      default: /* Illegal or Unimplemented */				\
        goto aref_ufn;							\
    }/* end switch typenumber */					\
aref_ufn:								\
	N_OP_POPPED_CALL_2(N_OP_aref1, arrayarg); 			\
}

#ifdef BIGVM
#define DTEST(n)							\
{									\
	register int atom_index;					\
	register struct dtd *dtd68k ;					\
	atom_index = n;							\
 for(dtd68k=(struct dtd *) GetDTD(GetTypeNumber(TOPOFSTACK));		\
	atom_index != dtd68k->dtd_name ;	\
	    dtd68k=(struct dtd *) GetDTD(dtd68k->dtd_supertype))	\
	{								\
		if( dtd68k->dtd_supertype == 0)				\
		{							\
		 goto op_ufn;						\
		}							\
	}								\
nextop_atom;								\
}
#else /* BIGVM */
#define DTEST(n)							\
{									\
	register int atom_index;					\
	register struct dtd *dtd68k ;					\
	atom_index = n;							\
 for(dtd68k=(struct dtd *) GetDTD(GetTypeNumber(TOPOFSTACK));		\
	atom_index != dtd68k->dtd_namelo +((int)(dtd68k->dtd_namehi)<<16) ;	\
	    dtd68k=(struct dtd *) GetDTD(dtd68k->dtd_supertype))	\
	{								\
		if( dtd68k->dtd_supertype == 0)				\
		{							\
		 goto op_ufn;						\
		}							\
	}								\
nextop_atom;								\
}
#endif /* BIGVM */

#define FVAR(n)	{							\
register LispPTR *chain;						\
chain = (LispPTR *) (PVar + n);						\
if(WBITSPTR(chain)->LSB){						\
	PUSH(GetLongWord(Addr68k_from_LADDR(				\
		POINTERMASK & SWAP_WORDS(native_newframe(n >> 1)))));	\
	nextop1;							\
    }/* if(((WBITS */							\
PUSH(GetLongWord(Addr68k_from_LADDR(POINTERMASK & SWAP_WORDS(*chain))));	\
nextop1;								\
}

#define FVARX(n)	{						\
register int nn = n;							\
register LispPTR *chain;						\
chain = (LispPTR *) (PVar + nn);						\
if(WBITSPTR(chain)->LSB){						\
	PUSH(GetLongWord(Addr68k_from_LADDR(				\
		POINTERMASK & SWAP_WORDS(native_newframe(nn >> 1)))));	\
	nextop2;							\
    }/* if(((WBITS */							\
PUSH(GetLongWord(Addr68k_from_LADDR(POINTERMASK & SWAP_WORDS(*chain))));	\
nextop2;								\
}


/* ********************************************************************	*/
/* THE FOLLOWING WAS IN n_op_inlinedefsC.h */
/* ********************************************************************	*/

#define GCREF(n) {							\
	GCLOOKUPV(TOPOFSTACK, n, TOPOFSTACK);				\
	nextop2;}

#ifndef BIGATOMS
#define ATOMCELL_N(n)							\
	{if ((unsigned int)TOPOFSTACK >> 16) {goto op_ufn;}		\
	TOPOFSTACK = (n << 16) + (TOPOFSTACK << 1) ;			\
	nextop2;							\
	}
#elif defined(BIGVM)
#define ATOMCELL_N(n)							\
      { register int nn = n;					\
		if (0==((unsigned int)(TOPOFSTACK&= POINTERMASK) & SEGMASK))			\
	    { /* old-symbol case; just add cell-number arg */		\
	    switch (nn)							\
	      {								\
		case PLIS_HI:	/* PLIST entry for symbol */		\
		  TOPOFSTACK = (ATOMS_HI << 16) + (10*(unsigned int)TOPOFSTACK) + NEWATOM_PLIST_OFFSET;	\
		  break;						\
		case PNP_HI:	/* PNAME entry for symbol */		\
		  TOPOFSTACK = (ATOMS_HI << 16) + (10*(unsigned int)TOPOFSTACK) + NEWATOM_PNAME_OFFSET;	\
		  break;						\
		case VALS_HI:	/* VALUE cell for symbol */		\
		  TOPOFSTACK = (ATOMS_HI << 16) + (10*(unsigned int)TOPOFSTACK) + NEWATOM_VALUE_OFFSET;	\
		  break;						\
		case DEFS_HI:	/* DEFINITION for symbol */		\
		  TOPOFSTACK = (ATOMS_HI << 16) + (10*(unsigned int)TOPOFSTACK) + NEWATOM_DEFN_OFFSET;	\
		  break;						\
		default:  goto op_ufn;					\
	      }								\
	    nextop2;							\
	    }								\
	else if (TYPE_NEWATOM == GetTypeNumber(TOPOFSTACK))		\
	  { /* NEW-symbol case; it's an offset from the main ptr */	\
	    switch (nn)							\
	      {								\
		case PLIS_HI:	/* PLIST entry for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_PLIST_OFFSET;	\
		  break;						\
		case PNP_HI:	/* PNAME entry for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_PNAME_OFFSET;	\
		  break;						\
		case VALS_HI:	/* VALUE cell for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_VALUE_OFFSET;	\
		  break;						\
		case DEFS_HI:	/* DEFINITION for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_DEFN_OFFSET;	\
		  break;						\
		default:  goto op_ufn;					\
	      }								\
	    nextop2;							\
	  }								\
	else goto op_ufn;						\
	}
#else /* 	 */

#define ATOMCELL_N(n)							\
      { register int nn = n;					\
		if (0==((unsigned int)TOPOFSTACK & SEGMASK))			\
	    { /* old-symbol case; just add cell-number arg */		\
	    TOPOFSTACK = (nn << 16) + (TOPOFSTACK << 1) ;		\
	    nextop2;							\
	    }								\
	else if (TYPE_NEWATOM == GetTypeNumber(TOPOFSTACK))		\
	  { /* NEW-symbol case; it's an offset from the main ptr */	\
	    switch (nn)							\
	      {								\
		case PLIS_HI:	/* PLIST entry for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_PLIST_OFFSET;	\
		  break;						\
		case PNP_HI:	/* PNAME entry for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_PNAME_OFFSET;	\
		  break;						\
		case VALS_HI:	/* VALUE cell for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_VALUE_OFFSET;	\
		  break;						\
		case DEFS_HI:	/* DEFINITION for symbol */		\
		  TOPOFSTACK = TOPOFSTACK + NEWATOM_DEFN_OFFSET;	\
		  break;						\
		default:  goto op_ufn;					\
	      }								\
	    nextop2;							\
	  }								\
	else goto op_ufn;						\
	}
#endif /* BIGATOMS */


#define DIFFERENCE	{N_OP_CALL_2(N_OP_difference);}
#define LOGOR		{N_OP_CALL_2(N_OP_logor);}
#define LOGAND 		{N_OP_CALL_2(N_OP_logand);}
#define LOGXOR 		{N_OP_CALL_2(N_OP_logxor);}
#define PLUS2		{N_OP_CALL_2(N_OP_plus2);}
#define QUOTIENT	{N_OP_CALL_2(N_OP_quot);}
#define TIMES2		{N_OP_CALL_2(N_OP_times2);}
#define	GREATERP	{N_OP_CALL_2(N_OP_greaterp);}
#define IDIFFERENCE	{N_OP_CALL_2(N_OP_idifference);}
#define IPLUS2		{N_OP_CALL_2(N_OP_iplus2);}
#define IQUOTIENT	{N_OP_CALL_2(N_OP_iquot);}
#define ITIMES2		{N_OP_CALL_2(N_OP_itimes2);}
#define	IGREATERP	{N_OP_CALL_2(N_OP_igreaterp);}
#define IREMAINDER	{N_OP_CALL_2(N_OP_iremainder);}
#define IPLUS_N(n)	{N_OP_CALL_1d(N_OP_iplusn, n)}
#define IDIFFERENCE_N(n)	{N_OP_CALL_1d(N_OP_idifferencen, n);}
#define BOXIPLUS	{N_OP_CALL_2(N_OP_boxiplus);}
#define BOXIDIFFERENCE	{N_OP_CALL_2(N_OP_boxidiff);}
#define FPLUS2		{N_OP_CALL_2(N_OP_fplus2);}
#define FDIFFERENCE	{N_OP_CALL_2(N_OP_fdifference);}
#define FTIMES2		{N_OP_CALL_2(N_OP_ftimes2);}
#define FQUOTIENT	{N_OP_CALL_2(N_OP_fquotient);}
#define FGREATERP	{N_OP_CALL_2(N_OP_fgreaterp);}
#define UBFLOAT1(n)	{N_OP_UNBOXED_CALL_1d(N_OP_ubfloat1, n);}
#define UBFLOAT2(n)	{N_OP_UNBOXED_CALL_2d(N_OP_ubfloat2, n);}
#define UBFLOAT3(n)	{N_OP_UNBOXED_CALL_3d(N_OP_ubfloat3, n);}
#define LRSH1		{N_OP_CALL_1(N_OP_lrsh1);}
#define LRSH8		{N_OP_CALL_1(N_OP_lrsh8);}
#define LLSH1		{N_OP_CALL_1(N_OP_llsh1);}
#define LLSH8		{N_OP_CALL_1(N_OP_llsh8);}
#define LSH		{N_OP_CALL_2(N_OP_lsh);}
#define RPLACA		{N_OP_CALL_2(N_OP_rplaca);}
#define RPLACD		{N_OP_CALL_2(N_OP_rplacd);}
#define RPLCONS		{N_OP_CALL_2(N_OP_rplcons);}
#define MAKENUMBER 	{N_OP_CALL_2(N_OP_makenumber);}
#define EQLOP		{N_OP_CALL_2(N_OP_eqlop);}
#define CLEQUAL		{N_OP_CALL_2(N_OP_clequal);}
#define ILEQUAL		{N_OP_CALL_2(N_OP_equal);}
#define CLFMEMB		{N_OP_CALL_exception_2(N_OP_clfmemb);}
#define CLASSOC		{N_OP_CALL_exception_2(N_OP_classoc);}
#define FMEMB		{N_OP_CALL_exception_2(N_OP_fmemb);}
#define ASSOC		{N_OP_CALL_exception_2(N_OP_assoc);}
#define ARG0		{N_OP_CALL_1(N_OP_arg0);}
#define LISTGET		{N_OP_CALL_exception_2C(N_OP_listget);}
#define DRAWLINE	{N_OP_CALL_9(N_OP_drawline);}
#define N_OP_ADDBASE	{N_OP_CALL_2(N_OP_addbase);}

#define UNWIND(n, m)							\
		{	 						\
		 if ((INT)(CSTKPTRL = (LispPTR *)			\
		      N_OP_unwind(CSTKPTR, TOPOFSTACK, n, m)) < 0) 	\
			goto unwind_err;				\
		 POP;							\
		 nextop3;						\
		}

#define STKSCAN								\
		{TOPOFSTACK = N_OP_stkscan(TOPOFSTACK);			\
		 nextop1;						\
		}

#define FVARX_(n)							\
		{TOPOFSTACK = N_OP_fvar_(TOPOFSTACK, n);		\
		 nextop2;						\
		}

#define BLT		{N_OP_CALL_3(N_OP_blt);}

#define PILOTBITBLT							\
		{TOPOFSTACK = N_OP_pilotbitblt(POP_TOS_1, TOPOFSTACK);	\
		 nextop1;						\
		}

#define CREATECELL	{N_OP_CALL_1(N_OP_createcell);}

#define RESTLIST(n)	{TOPOFSTACK = N_OP_restlist(POP_TOS_1, TOPOFSTACK, n);\
			nextop2;}


#define ASET1		{N_OP_CALL_3(N_OP_aset1);}
#define ASET2		{N_OP_CALL_4(N_OP_aset2);}
#define MISC3(n)	{N_OP_CALL_3d(N_OP_misc3, n);}
#define MISC4(n)	{N_OP_CALL_4d(N_OP_misc4, n);}
#define MISC7(n)	{N_OP_CALL_7d(N_OP_misc7, n);}
#define AREF2		{N_OP_CALL_3(N_OP_aref2);}
#define MISCN(index, args)						\
{	EXT;								\
	if (OP_miscn(index,args))  {					\
		RET;							\
	/*	PUSH(S_POSITIVE | (index << 8) | args);		*/	\
		goto op_ufn;						\
		}							\
	RET;								\
	nextop0;							\
}


/* ******************************************************************** */
/*	Call Interface where -1 indicates an error return		*/
/* ******************************************************************** */

/* SV need do no work */
#define SV

/* UFN_CALLS are inserted in xc.c. Note that only ufn_2 calls have decremented the stack at the time the UFN is called */

/* ufn_x	there are x args from the Lisp stack
   ufn_xd	there are x args from the Lisp stack & 
		some from the code stream.
*/
#define	UFN_CALLS							\
									\
unwind_err:	 							\
	CSTKPTRL = (LispPTR *) CurrentStackPTR;				\
	Error_Exit = 0;							\
	goto op_ufn;							\
ufn_2d:	CSTKPTRL += 1;							\
	goto fix_tos_ufn;						\
ufn_2d2:CSTKPTRL += 1;							\
	goto fix_tos_ufn;						\
ufn_2:	CSTKPTRL += 1;							\
	goto fix_tos_ufn;						\
exception_2 : 								\
	Error_Exit = 0;							\
	CSTKPTRL += 1;							\
	TOPOFSTACK = TopOfStack;					\
	if(!Irq_Stk_End){						\
		 goto check_interrupt;					\
	}								\
	else goto op_ufn;						\
exception_2C : 								\
	Error_Exit = 0;							\
	TOPOFSTACK = TopOfStack;					\
	*CSTKPTRL = Scratch_CSTK;					\
	CSTKPTRL += 1;							\
	if(!Irq_Stk_End){						\
		 goto check_interrupt;					\
	}								\
	else {								\
		goto op_ufn;						\
	}								\
fix_tos_ufn:								\
	TOPOFSTACK = TopOfStack;					\
	Error_Exit = 0;							\
	goto op_ufn;

#define N_OP_CALL_1(op_name)						\
if ((int)(TOPOFSTACK = (LispPTR)op_name(TOPOFSTACK)) < 0) goto fix_tos_ufn;\
nextop1;


#define N_OP_CALL_1d(op_name, n)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(TOPOFSTACK, n)) < 0) goto fix_tos_ufn;\
nextop2;

#define N_OP_UNBOXED_CALL_1d(op_name, n)				\
TOPOFSTACK = op_name(TOPOFSTACK, n);					\
if (Error_Exit) goto fix_tos_ufn;					\
nextop2;


#define N_OP_CALL_2(op_name)						\
if ((int)(TOPOFSTACK = (LispPTR)op_name(POP_TOS_1, TOPOFSTACK)) < 0)	\
	goto ufn_2;							\
nextop1;

#define N_OP_POPPED_CALL_2(op_name, popped_arg)				\
if ((int)(TOPOFSTACK = (LispPTR)op_name(popped_arg, TOPOFSTACK)) < 0)	\
	goto ufn_2;							\
nextop1;

#define N_OP_CALL_2d(op_name, n)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(POP_TOS_1, TOPOFSTACK, n)) < 0)	\
	goto ufn_2d;							\
nextop2;

#define N_OP_UNBOXED_CALL_2d(op_name, n)				\
TOPOFSTACK = op_name(POP_TOS_1, TOPOFSTACK, n);				\
if (Error_Exit) goto ufn_2d;						\
nextop2;

#define N_OP_CALL_2d2(op_name, a, b)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(POP_TOS_1, TOPOFSTACK, a, b)) < 0) \
	goto ufn_2d2;							\
nextop3;

#define N_OP_CALL_exception_2(op_name)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(POP_TOS_1, TOPOFSTACK)) < 0)	\
	goto exception_2;						\
nextop1;

#define N_OP_CALL_exception_2C(op_name)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(POP_TOS_1, TOPOFSTACK)) < 0)	\
	goto exception_2C;						\
nextop1;

#define N_OP_CALL_3(op_name)						\
if ((int)(TOPOFSTACK = (LispPTR)op_name(					\
		 *(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK)) < 0)		\
	goto fix_tos_ufn;						\
CSTKPTRL -= 2;								\
nextop1;

#define N_OP_CALL_3d(op_name, n)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(				\
		 *(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK, n)) < 0)	\
	goto fix_tos_ufn;						\
CSTKPTRL -= 2;								\
nextop2;

#define N_OP_UNBOXED_CALL_3d(op_name, n)				\
TOPOFSTACK = op_name(*(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK, n);	\
if (Error_Exit) goto fix_tos_ufn;					\
CSTKPTRL -= 2;								\
nextop2;


#define N_OP_CALL_4(op_name)						\
if ((int)(TOPOFSTACK = (LispPTR)op_name(				\
	*(CSTKPTR-3), *(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK)) < 0)	\
	goto fix_tos_ufn;						\
CSTKPTRL -= 3;								\
nextop1;


#define N_OP_CALL_4d(op_name, n)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(					\
	*(CSTKPTR-3), *(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK, n)) < 0)	\
	goto fix_tos_ufn;						\
CSTKPTRL -= 3;								\
nextop2;

#define N_OP_CALL_7d(op_name, n)					\
if ((int)(TOPOFSTACK = (LispPTR)op_name(					\
	*(CSTKPTR-6), *(CSTKPTR-5), *(CSTKPTR-4),			\
	*(CSTKPTR-3), *(CSTKPTR-2), *(CSTKPTR-1), TOPOFSTACK, n)) < 0)	\
	goto fix_tos_ufn;						\
CSTKPTRL -= 6;								\
nextop2;



#define N_OP_CALL_9(op_name)						\
if ((int)(TOPOFSTACK = (LispPTR)op_name(				\
		 *(CSTKPTR-8), *(CSTKPTR-7), *(CSTKPTR-6),		\
		 *(CSTKPTR-5), *(CSTKPTR-4), *(CSTKPTR-3), *(CSTKPTR-2),\
		 *(CSTKPTR-1), TOPOFSTACK /*, fix_tos_ufn*/)) < 0)	\
	goto fix_tos_ufn;						\
CSTKPTRL -= 8;								\
nextop1;


#ifdef SUN3_OS3_OR_OS4_IL

/* need jump point for inline asm code, e.g., for IPLUS */

#define OPCODEFAIL							\
									\
fixtos1:								\
	fixtos1_label();						\
	FIXTOS1;							\
	goto op_ufn;

#else

/* no opcode fail point necessary */

#define OPCODEFAIL

#endif
