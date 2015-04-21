/* $Id: inlnMIPS.h,v 1.2 1999/01/03 02:06:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	    I N L I N E   F U N C T I O N S   F O R   M I P S   	*/
/*									*/
/*	These are GCC-style inline asm functions for use when 		*/
/*	compiling Medley for MIPS RISCstations.				*/
/*									*/
/*	JDS 28 Aug 91 Created; no valid defns yet.			*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*	(C) Copyright 1991 Venue. All Rights Reserved.			*/
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
/*		   R E G I S T E R   C O N V E N T I O N S		*/
/*									*/
/*									*/
/************************************************************************/

/* Copyright Venue 1991 All rights reserved. */

/* inline defs for MIPS */



extern inline const unsigned int swapx (unsigned int word)
 {
    asm("roll	$16,%0" : "=g" (word) : "0" (word));
    return(word);
 }



extern inline const unsigned int word_swap_longword (unsigned int word)
 {
    asm("roll	$16,%0" : "=g" (word) : "0" (word));

    return(word);
  }



extern inline const unsigned short byte_swap_word (unsigned short word)
 {
    asm("rolw	$8,%0" : "=g" (word) : "0" (word));

    return(word);
  }



extern inline const void word_swap_page(unsigned short * page, int count)
  {
    asm("\
	pushl %ebp					\n\
	movl %esp,%ebp					\n\
	subl $4,%esp					\n\
	pushl %edi					\n\
	pushl %esi					\n\
	pushl %ebx					\n\
	cld						\n\
	movl 8(%ebp),%esi	// word pointer.	\n\
	movl %esi,%edi					\n\
	movl 12(%ebp),%ecx	// count		\n\
							\n\
$0:	lodsl						\n\
	rolw	$8,%ax					\n\
	roll	$16,%eax				\n\
	rolw	$8,%ax					\n\
	stosl						\n\
	loop	$0					\n\
							\n\
	// epilogue.					\n\
	leal -16(%ebp),%esp				\n\
	popl %ebx					\n\
	popl %esi					\n\
	popl %edi					\n\
	leave						\n\
	ret");

  }
