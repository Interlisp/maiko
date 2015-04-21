/* $Id: arith4.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: arith4.c,v 1.3 1999/05/31 23:35:21 sybalsky Exp $ Copyright (C) Venue";



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


#include "version.h"




/***********************************************************************/
/*
		File Name :	arith4.c

		Including :	OP_times2 326Q(OP_itimes2 332Q)

				OP_quot 327Q(OP_iquot 333Q)
				OP_reminder 334Q

*/
/**********************************************************************/

#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "cell.h"
#include "medleyfp.h"
#include "arith.h"



/**********************************************************************/
/*

		Func name :	N_OP_times2(itimes2)

*/
/**********************************************************************/
int N_OP_times2(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;



	N_GETNUMBER( tosm1, arg1, doufn );
	N_GETNUMBER( tos, arg2, doufn );

#ifdef SUN3_OS3_OR_OS4_IL

	result = mpy32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	mpy_err_label();

#else

	result = arg1 * arg2;
	if ((arg2 !=0) &&((result / arg2) != arg1) ) goto doufn2;
	N_ARITH_SWITCH(result);

#endif

doufn2:	ERROR_EXIT(tos);
doufn:	return(N_OP_ftimes2(tosm1, tos));


} /* end N_OP_times2 */

int N_OP_itimes2(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;


	N_IGETNUMBER( tosm1, arg1, doufn );
	N_IGETNUMBER( tos, arg2, doufn );

#ifdef SUN3_OS3_OR_OS4_IL

	result = impy32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	impy_err_label();

#else

	result = arg1 * arg2;
	if ((arg2 !=0) &&( (result / arg2) != arg1) ) {goto doufn;}
	N_ARITH_SWITCH(result);

#endif

doufn:	ERROR_EXIT(tos);


} /* end N_OP_itimes2 */



/**********************************************************************/
/*

		Func name :	N_OP_quot(iquot)

*/
/**********************************************************************/
int N_OP_quot(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;


	N_GETNUMBER( tosm1, arg1, doufn );
	N_GETNUMBER( tos, arg2, doufn );
	if (arg2 == 0) goto doufn2;

#ifdef SUN3_OS3_OR_OS4_IL

	result = quot32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	quot_err_label();

#else

	result = arg1/arg2;	/* lmm: note: no error case!! */
	N_ARITH_SWITCH(result);
#endif
doufn2:	ERROR_EXIT(tos);
doufn:	return(N_OP_fquotient(tosm1, tos));

} /* end N_OP_quot */

int N_OP_iquot(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;


	N_IGETNUMBER( tosm1, arg1, doufn );
	N_IGETNUMBER( tos, arg2, doufn );
	if (arg2 == 0) goto doufn;

#ifdef SUN3_OS3_OR_OS4_IL

	result = iquot32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	iquot_err_label();

#else

	result = arg1/arg2;
	N_ARITH_SWITCH(result);

#endif

doufn:	ERROR_EXIT(tos);

} /* end N_OP_quot */


/**********************************************************************/
/*

		Func name :	N_OP_iremainder

*/
/**********************************************************************/

int N_OP_iremainder(int tosm1, int tos)
{
register int	arg1,arg2;
register int	result;


	N_IGETNUMBER( tosm1, arg1, doufn );
	N_IGETNUMBER( tos, arg2, doufn );
	if (arg2 == 0) goto doufn;

#ifdef SUN3_OS3_OR_OS4_IL

	result = irem32(arg1, arg2);
	N_ARITH_SWITCH(result);
dummy:	irem_err_label();

#else

	result = arg1 % arg2;
	N_ARITH_SWITCH(result);

#endif

doufn:	ERROR_EXIT(tos);

} /* end N_OP_iremainder */

