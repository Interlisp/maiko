/* $Id: vmemsave.h,v 1.2 1999/01/03 02:06:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


/*
	File Name : vmemsave.h
	DEfinition for vmemsave
*/

#define	FP_IFPAGE  512			/* IFPAGE address in sysoutfile by Byte */
#define	DOMINOPAGES  301		/* skip dominopages  in  fptovp */
#define	SKIPPAGES  301			/* save first filepage  */
#define	SKIP_DOMINOPAGES  153600	/* Byte size in sysoutfile for dominocode */
#define	SAVE_IFPAGE	223		/* Virtual address for IFPAGES's buffer page. This value is \EMUSWAPBUFFERS in lisp. */
