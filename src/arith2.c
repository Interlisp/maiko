/* $Id: arith2.c,v 1.4 2001/12/24 01:08:58 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: arith2.c,v 1.4 2001/12/24 01:08:58 sybalsky Exp $ Copyright (C) Venue";



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "medleyfp.h"
#include "arith.h"

DLword	*createcell68k(unsigned int type);
LispPTR N_OP_flpus2();
LispPTR N_OP_fdifference(LispPTR parg1, LispPTR parg2);
LispPTR N_OP_fgreaterp(LispPTR parg1, LispPTR parg2);

/************************************************************
N_OP_plus2
	entry		PLUS2		OPCODE[0324]
	entry		IPLUS2		OPCODE[0330]
	return(tos + b)
************************************************************/

LispPTR N_OP_plus2(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;

	N_GETNUMBER( tos, arg1, doufn );
	N_GETNUMBER( tosm1, arg2, doufn );

#ifdef USE_INLINE_ARITH

	result = plus32(arg1, arg2);
	N_ARITH_SWITCH(result);

doufn2:	plus_err_label();
	ERROR_EXIT(tos);

#else
	result = arg1 + arg2;
	if ( ((arg1>=0) == (arg2>=0)) && ((result>=0) != (arg1>=0)) )
		{ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif /* USE_INLINE_ARITH */

doufn:	return(N_OP_fplus2(tosm1, tos));

}



/************************************************************************/
/*									*/
/*			N _ O P _ i p l u s 2				*/
/*									*/
/*	Implements the IPLUS2 opcode--add the two arguments, which	*/
/*	must be SMALLP or FIXP						*/
/*									*/
/************************************************************************/

LispPTR N_OP_iplus2(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;

	N_IGETNUMBER( tos, arg1, doufn );
	N_IGETNUMBER( tosm1, arg2, doufn );

#ifdef USE_INLINE_ARITH

	result = iplus32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	iplus_err_label();

#else

	result = arg1 + arg2;
	if ( ((arg1>=0) == (arg2>=0)) && ((result>=0) != (arg1>=0)) )
		{ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif /* USE_INLINE_ARITH */

doufn:		ERROR_EXIT(tos);

}


/************************************************************
N_OP_difference
	entry		DIFFERENCE		OPCODE[0325]
	entry		IDIFFERENCE		OPCODE[0331]
	return(a - tos)
************************************************************/

LispPTR N_OP_difference(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;

	N_GETNUMBER( tosm1, arg1, doufn );
	N_GETNUMBER( tos, arg2, doufn );

#ifdef USE_INLINE_ARITH

	result = sub32(arg1, arg2);
	N_ARITH_SWITCH(result);

doufn2:	diff_err_label();
	ERROR_EXIT(tos);

#else

	result = arg1 - arg2;
	if ( ((arg1>=0) == (arg2<0)) && ((result>=0) != (arg1>=0)) )
		{ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif

doufn:	return(N_OP_fdifference(tosm1, tos));

}


LispPTR N_OP_idifference(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;

	N_IGETNUMBER( tosm1, arg1, doufn );
	N_IGETNUMBER( tos, arg2, doufn );

#ifdef USE_INLINE_ARITH

	result = isub32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	idiff_err_label();

#else

	result = arg1 - arg2;
	if ( ((arg1>=0) == (arg2<0)) && ((result>=0) != (arg1>=0)) )
		{ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif
doufn:	ERROR_EXIT(tos);

}



/************************************************************
N_OP_logxor
	entry		LOGXOR2		OPCODE[0346]
	return(tosm1 ^ tos)
************************************************************/

LispPTR N_OP_logxor(int tosm1, int tos)
{
	N_IARITH_BODY_2(tosm1, tos, ^);
}



/************************************************************
N_OP_logand
	entry		LOGAND2		OPCODE[0345]
	return(tosm1 & tos)
************************************************************/
LispPTR N_OP_logand(int tosm1, int tos)
{
	N_IARITH_BODY_2(tosm1, tos, &);
}



/************************************************************
N_OP_logor
	entry		LOGOR2		OPCODE[0344]
	return(tosm1 | tos)
************************************************************/
LispPTR N_OP_logor(int tosm1, int tos)
{
	N_IARITH_BODY_2(tosm1, tos, |);
}

/************************************************************
N_OP_greaterp
	entry		GREATERP		OPCODE[0363]
	entry		IGREATERP		OPCODE[0361]
	return(tosm1 > tos)

************************************************************/
LispPTR N_OP_greaterp(int tosm1, int tos)
{
register int arg1,arg2;

	N_GETNUMBER( tosm1, arg1, do_ufn);
	N_GETNUMBER( tos, arg2, do_ufn);

	if(arg1 > arg2)
		return(ATOM_T);
	else
		return(NIL_PTR);

do_ufn:	 return(N_OP_fgreaterp(tosm1, tos));
}


LispPTR N_OP_igreaterp(int tosm1, int tos)
{
register int arg1,arg2;

	N_IGETNUMBER( tosm1, arg1, do_ufn);
	N_IGETNUMBER( tos, arg2, do_ufn);

	if(arg1 > arg2)
		return(ATOM_T);
	else
		return(NIL_PTR);

do_ufn:	ERROR_EXIT(tos);
}



/************************************************************
N_OP_iplusn
	entry		IPLUS.N		OPCODE[0335]
	return(tos + n)
************************************************************/
LispPTR N_OP_iplusn(int tos, int n)
{

register int	arg1;
register int	result;

	N_IGETNUMBER( tos, arg1, do_ufn );

#ifdef USE_INLINE_ARITH

	result = iplus32n(arg1, n);
	N_ARITH_SWITCH(result);
dummy:	iplusn_err_label();

#else

	result = arg1 + n;
	if ((result < 0) && (arg1 >= 0)) {ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif

do_ufn:	ERROR_EXIT(tos);

}



/************************************************************
N_OP_idifferencen
	entry		IDIFFERENCE.N		OPCODE[0336]
	return(tos - n)
************************************************************/
LispPTR N_OP_idifferencen(int tos, int n)
{
register int	arg1;
register int	result;

	N_IGETNUMBER( tos, arg1, do_ufn );

#ifdef USE_INLINE_ARITH

	result = sub32n(arg1, n);
	N_ARITH_SWITCH(result);
dummy:	idiffn_err_label();

#else

	result = arg1 - n;
	if ((result >= 0) && (arg1 < 0)) {ERROR_EXIT(tos);}
	N_ARITH_SWITCH(result);

#endif

do_ufn:	ERROR_EXIT(tos);

}


