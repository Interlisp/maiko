#ifndef VERSION_H
#define VERSION_H 1
/* $Id: version.h,v 1.5 2001/12/26 22:17:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

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
#define NOFORN
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
	/*		Use asm inline arith	USE_INLINE_ARITH	*/
	/*		pointer-wide unsigned	UNSIGNED		*/
	/*		pointer-wide int	INT	 		*/
	/*								*/
	/*								*/
	/*								*/
	/****************************************************************/




#ifdef NOASM
#undef SUN3_OS3_IL
#undef SUN3_OS4_IL
#undef SUN4_OS4_IL
#undef OPDISP
#undef NATIVETRAN
#undef PROFILE
#define NOASMFNCALL	1
#endif

#ifdef SUN3_OS3_IL
#define SUN3_OS3_OR_OS4_IL	1
#define USE_INLINE_ARITH	1
#endif

#ifdef SUN3_OS4_IL
#define SUN3_OS3_OR_OS4_IL	1
#define USE_INLINE_ARITH	1
#endif





	/* Set up defaults */
#define NOETHER 1
#define UNALIGNED_FETCH_OK
#define HAS_GETHOSTID
#define UNSIGNED unsigned long
#define INT long



	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef sun
typedef signed char s_char;
#endif /* sun */




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef sparc
	/* SPARCs and MIPSs can't do unaligned word-loads */
#undef UNALIGNED_FETCH_OK
#endif /* SPARC */




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef OS5
		/* Solaris, sort of SYSV-ish, but not really */
#undef NOETHER
#define SYSVSIGNALS 1
#define NOFORN
#define LOCK_X_UPDATES 1
#endif /* OS5 */




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef LINUX
		/* LINUX, the free POSIX-compliant Unix */
typedef signed char s_char;

#undef UNALIGNED_FETCH_OK
#endif /* LINUX */




/********************************************************/
/*							*/
/********************************************************/
#if defined(MACOSX) || defined(FREEBSD)
/* MacOS X, FreeBSD - mostly POSIX-compliant Unix */
typedef signed char s_char;
#endif /* MACOSX || FREEBSD */


	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef I386
#define USE_INLINE_ARITH	1
#endif /* I386 */

	/********************************************************/
	/*							*/
	/********************************************************/

#ifdef DOS
typedef unsigned char u_char;
typedef unsigned long u_int;
typedef signed char s_char;
typedef unsigned short u_short;
#undef UNALIGNED_FETCH_OK
#undef HAS_GETHOSTID
#define SYSVSIGNALS 1
#define USHORT unsigned
#else
#define USHORT unsigned short
#endif /* DOS */

	/********************************************************/
	/*							*/
	/********************************************************/

#ifdef OS4
#define __inline__
#endif

	/****************************************************************/
	/* 	    End of architecture-specific flag settings		*/
	/* 	    --Start of system-specific flags		 	*/
	/*								*/
	/****************************************************************/

	/****************************************************************/
	/* 	    End of system-specific flag settings		*/
	/****************************************************************/

#endif /* VERSION_H */
