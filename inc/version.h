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
	/*  Architecture-specific flags:  Set flags			*/
	/*  based on thing we know about the architecture		*/
	/*  or idiosyncrasies of the machine we're compiling for.	*/
	/*								*/
	/*  Defaults:	Unaligned fetches OK	UNALIGNED_FETCH_OK	*/
	/*		type char is signed	SIGNED_CHARS		*/
	/*		fp values used with				*/
	/*		FPTEST can be in regs.	REGISTER		*/
	/*		CC supports "void"	VOID			*/
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
#undef UNSAFE
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
#define UNALIGNED_FETCH_OK
#define REGISTER register
#define HAS_GETHOSTID
#undef USE_UTIME
#define UNSIGNED unsigned long
#define INT long
#define VOID void



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
#ifdef DEC3100
#undef UNALIGNED_FETCH_OK
#ifdef OSF1
typedef signed char s_char;
#undef UNSIGNED
#undef INT
#define UNSIGNED unsigned long long
#define INT long long
#undef REGISTER
#define REGISTER
#endif /* OSF1 */
#endif /* DEC3100 */




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef AIXPS2
typedef signed char s_char;
#define GCC386
#endif



	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef RS6000
typedef signed char s_char;
#endif /* RS6000 */



	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef RISCOS
typedef signed char s_char;
#undef UNALIGNED_FETCH_OK
#define USE_UTIME
#define LOCK_X_UPDATES 1
#endif



	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef ISC
typedef signed char s_char;
#undef UNALIGNED_FETCH_OK
#define MAXPATHLEN MAXNAMLEN
#define EWOULDBLOCK EAGAIN
 /* we compile on a 386 with GCC, so can use optimizations. */
#define GCC386
#define RESWAPPEDCODESTREAM
#undef HAS_GETHOSTID
#define LOCK_X_UPDATES 1
#endif /* ISC */



	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef OS5
		/* Solaris, sort of SYSV-ish, but not really */
#undef HAS_GETHOSTID
#define BSD_COMP 1
#define SYSVSIGNALS 1
#define WAITINT 1
#define L_SET SEEK_SET
#define NOFORN
#define LOCK_X_UPDATES 1
#endif /* OS5 */




	/********************************************************/
	/*							*/
	/********************************************************/
#ifdef LINUX
		/* LINUX, the free POSIX-compliant Unix */
#define NOETHER 1
#define XWINDOWS 1
/* JDS trial 12/22/01 #define USETIMEFN 1 */

#undef REGISTER
#define REGISTER

typedef signed char s_char;

#undef UNALIGNED_FETCH_OK

/* #define sigvec sigaction */
/* #define sigmask __sigmask */
/* #define sv_handler sa_handler */
/* #define sv_mask sa_mask */
/* #define sv_flags sa_flags */
#endif /* LINUX */




/********************************************************/
/*							*/
/********************************************************/
#if defined(MACOSX) || defined(FREEBSD)
/* MacOS X, FreeBSD - mostly POSIX-compliant Unix */
#define NOETHER 1
#define XWINDOWS 1
#define WAITINT 1

#undef REGISTER
#define REGISTER

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
	/* DOS doesn't have the BSD bzero &c functions */
#define bzero(place,len) memset(place, 0, len)
#undef UNALIGNED_FETCH_OK
#undef HAS_GETHOSTID
#undef REGISTER
#define REGISTER
#define SYSVONLY 1
#define SYSVSIGNALS 1
#define NOETHER 1
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
	/* 	    --Start of system-specific flags (e.g. SYSVONLY)-- 	*/
	/*								*/
	/****************************************************************/

#ifdef SYSVONLY

#if defined(OS5) || defined(MACOSX) || defined(FREEBSD) || defined(LINUX)
#else
#define seteuid(x) setresuid(-1, (x), -1)
#endif /* OS5, MACOSX, FREEBSD, LINUX do have seteuid */

#if !defined(LINUX) && !defined(MACOSX) && !defined(FREEBSD)
/* these are in the POSIX standard */
#define getrusage(x, y) 
#define getpagesize() 4096
#endif /* LINUX , MACOSX, FREEBSD  */

#endif /* SYSVONLY */

	/****************************************************************/
	/* 	    End of system-specific flag settings		*/
	/****************************************************************/

