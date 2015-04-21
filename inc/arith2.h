/* $Id: arith2.h,v 1.2 1999/01/03 02:05:52 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



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


/************************************************************************/
/*									*/
/*	Take care of results for inlined arithmetic cases.  */
/*	xxx_RESULT does overflow checking and boxing.								*/
/*									*/
/*									*/
/************************************************************************/
#ifdef ARITHINLINE
#ifdef GCC386
/* Inline defines for arith on GCC386 machines */


extern inline const int plus32 (int arg1, int arg2)
{
	asm(" addl %2,%0					\n\
	jo plus_err": "=r" (arg2): "0" (arg2), "r" (arg1));
	return arg2;
}
#define PLUS_RESULT							\
	INLINE_ARITH_SWITCH(result,"plus_ret");	\
	asm("plus_err:");		\
	INLINE_ERROR_EXIT(tos,"plus_ret")



extern inline const int iplus32 (int arg1, int arg2)
{
	asm(" addl %2,%0					\n\
	jo iplus_err":  "=r" (arg2): "0" (arg2), "r" (arg1));
	return arg2;
}
#define IPLUS_RESULT		\
	INLINE_ARITH_SWITCH(result,"iplus_ret");	\
	asm("iplus_err:");							\
	INLINE_ERROR_EXIT(tos,"iplus_ret")



extern inline const int sub32 (int arg1, int arg2)
{
	asm("subl %2,%0						\n\
	jo diff_err": "=r" (arg1): "0" (arg1), "r" (arg2));
	return arg1;
}
#define DIFF_RESULT 						\
	INLINE_ARITH_SWITCH(result,"diff_ret");		\
	asm("diff_err:");							\
	INLINE_ERROR_EXIT(tos,"diff_ret")



extern inline const int isub32 (int arg1, int arg2)
{
	asm(" subl %2,%0						\n\
	jo idiff_err": "=r" (arg1): "0" (arg1), "r" (arg2));
	return arg1;
}
#define IDIFF_RESULT 						\
	INLINE_ARITH_SWITCH(result,"idiff_ret");		\
	asm("idiff_err:");							\
	INLINE_ERROR_EXIT(tos,"idiff_ret")



extern inline const int iplus32n(int arg1, int arg2)
{
	asm("addl %2,%0						\n\
	jo iplusn_err": "=r" (arg2): "0" (arg2), "r" (arg1));
	return arg2;
}
#define IPLUSN_RESULT						\
	INLINE_ARITH_SWITCH(result,"iplusn_ret");	\
	asm("iplusn_err:");							\
	INLINE_ERROR_EXIT(tos,"iplusn_ret")



extern inline const int sub32n (int arg1, int arg2)
{
	asm(" subl %2,%0						\n\
	jo idiffn_err": "=r" (arg1): "0" (arg1), "r" (arg2));
	return arg1;
}
#define IDIFFN_RESULT							\
	INLINE_ARITH_SWITCH(result,"idiffn_ret");	\
	asm("idiffn_err:");							\
	INLINE_ERROR_EXIT(tos,"idiffn_ret")


#else	/* Any other ARITHINLINE case */

#define PLUS_RESULT							\
	N_ARITH_SWITCH(result);					\
doufn2:	plus_err_label();					\
	ERROR_EXIT(tos);

	
#define IPLUS_RESULT						\
	N_ARITH_SWITCH(result);					\
dummy:	iplus_err_label();


#define DIFF_RESULT							\
	N_ARITH_SWITCH(result);					\
doufn2:	diff_err_label();					\
	ERROR_EXIT(tos);
	
	
#define IDIFF_RESULT						\	
	N_ARITH_SWITCH(result);					\
dummy:	idiff_err_label();


#define IPLUSN_RESULT						\
	N_ARITH_SWITCH(result);					\
dummy:	iplusn_err_label();


#define IDIFFN_RESULT						\
	N_ARITH_SWITCH(result);					\
dummy:	idiffn_err_label();

#endif /* GCC386 */
#endif /* ARITHINLINE */

