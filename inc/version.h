#ifndef VERSION_H
#define VERSION_H 1
/* $Id: version.h,v 1.5 2001/12/26 22:17:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "maiko/platform.h"

/************************************************************************/
/*									*/
/*			V E R S I O N . H			*/
/*									*/
/*  Version control:  Set the two values that keep sysouts and emul-    */
/*  ators in sync:  The LVERSION and MINBVERSION fields in the IFPAGE   */
/*									*/
/*  MINBVERSION is the current emulator version, incremented with each  */
/*  modification.  This must be >= a sysout's ifpage.minbversion.       */
/*									*/
/*  LVERSION is the minimum lisp version that will run with this emu-   */
/*  lator.  This must be <= a sysouts's ifpage.lversion.                */
/*									*/
/*  The loadup process sets both of these values in the sysout.         */
/*									*/
/*									*/
/*									*/
/*	C O N F I G U R A T I O N / O P T I O N   C O N T R O L		*/
/*								*/
/*	Given a release specification, set flags for the features	*/
/*	that release has.  This lets us set one flag in the make-	*/
/*	file, rather than remembering all the options that must change.	*/
/*									*/
/*	-DRELEASE=115   Medley 1.15, small atoms			*/
/*	-DRELEASE=200   Medley 2.0 as released				*/
/*	-DRELEASE=201   Medley with DOS & European kbd support		*/
/*	-DRELEASE=210   Medley with big VM				*/
/*	-DRELEASE=300   Medley bigvm as released.			*/
/*	-DRELEASE=350   Medley with 256MB vm.				*/
/*	-DRELEASE=351   Medley with 256MB vm and cursor fix		*/
/*									*/
/*									*/
/************************************************************************/


  /* The current values */

#define LVERSION 21000
#define MINBVERSION 21001


  /* But remember old values, if we can figure them out from ifdef's */

#if (RELEASE == 115)

#undef LVERSION
#undef MINBVERSION
#define LVERSION 15000
#define MINBVERSION 15000
#undef BIGATOMS
#define NOEUROKBD
#define NOVERSION

#elif (RELEASE == 200)

  /* Medley 2.0 as released */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 20000
#define MINBVERSION 20000

#define BIGATOMS
#define NOEUROKBD
#define NOVERSION

#elif (RELEASE == 201 )


  /* Medley 2.0 with EUROKBD modification */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 20100
#define MINBVERSION 20100

#define BIGATOMS
#undef NOEUROKBD
#define NOVERSION

#elif (RELEASE == 210)

  /* Medley 2.1, big-vm Medley while in beta-test */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 21000
#define MINBVERSION 21000

#define BIGATOMS
#define BIGVM 1
#define NEWCDRCODING


#	elif (RELEASE == 300 )

  /* Medley 301, big-vm Medley in release?? */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 30000
#define MINBVERSION 30000

#define BIGATOMS
#define BIGVM
#define NEWCDRCODING


#elif (RELEASE == 350)

 /* Medley 3.5, 256Mb version */

#undef LVERSION
#undef MINBVERSION
#define LVERSION 35000
#define MINBVERSION 35000

#define BIGATOMS
#define BIGVM
#define BIGBIGVM
#define NEWCDRCODING


#elif (RELEASE == 351)

 /* Medley 3.5, 256Mb version, X cursor hotspot fix 1/00 */

#undef LVERSION
#undef MINBVERSION
#define LVERSION 35010
#define MINBVERSION 35010

#define BIGATOMS
#define BIGVM
#define BIGBIGVM
#define NEWCDRCODING
#define NEWXCURSOR


#else
error Must specify RELEASE to build Medley.
#endif



	/****************************************************************/
	/*								*/
	/*  There used to be a define NEW_STORAGE, but this wasn't tied */
	/*  clearly to any RELEASE values. There are comments related   */
	/*  to this in LLPARAMS on the Lisp side.			*/
	/*								*/
	/****************************************************************/

	/****************************************************************/
	/*								*/
	/*  Architecture-specific flags:  Set flags			*/
	/*  based on thing we know about the architecture		*/
	/*  or idiosyncrasies of the machine we're compiling for.	*/
	/*								*/
	/*  Defaults:	Unaligned fetches OK	UNALIGNED_FETCH_OK	*/
	/*		fp values used with				*/
	/*		pointer-wide unsigned	UNSIGNED		*/
	/*		pointer-wide int	INT	 		*/
	/*								*/
	/*								*/
	/*								*/
	/****************************************************************/




#ifdef NOASM
#undef OPDISP
#undef PROFILE
#endif

	/* Set up defaults */
#define UNALIGNED_FETCH_OK
#define UNSIGNED unsigned long
#define INT long



/* Not all platforms want to do unaligned reads, so
 * we will disable those here. */
#if defined(MAIKO_ARCH_SPARC) || defined(MAIKO_ARCH_ARM)
#undef UNALIGNED_FETCH_OK
#endif




	/********************************************************/
	/*							*/
	/********************************************************/
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_sadd_overflow) && \
  __has_builtin(__builtin_ssub_overflow) && \
  __has_builtin(__builtin_smul_overflow)
#define USE_OVERFLOW_BUILTINS
#endif




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef OS5
		/* Solaris, sort of SYSV-ish, but not really */
#define MAIKO_ENABLE_ETHERNET
#define LOCK_X_UPDATES 1
#endif /* OS5 */




	/********************************************************/
	/*							*/
	/********************************************************/

#define USHORT unsigned short

	/****************************************************************/
	/* 	    End of architecture-specific flag settings		*/
	/* 	    --Start of system-specific flags		 	*/
	/*								*/
	/****************************************************************/

	/****************************************************************/
	/* 	    End of system-specific flag settings		*/
	/****************************************************************/

#endif /* VERSION_H */
