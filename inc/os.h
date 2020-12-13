/* $Id: os.h,v 1.2 1999/01/03 02:06:19 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
 *
 * os.h	- operating system definitions.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * $Log: os.h,v $
 * Revision 1.2  1999/01/03 02:06:19  sybalsky
 * Add ID comments / static to files for CVS use
 *
 * Revision 1.1.1.1  1998/12/17 05:03:18  sybalsky
 * Import of Medley 3.5 emulator
 *
 * Revision 4.5  1996/02/12 13:20:26  davy
 * Updated for Solaris 2.5
 *
 * Revision 4.4  1993/10/13 01:13:25  mogul
 * IRIX40 fix
 *
 * Revision 4.3  1993/10/01  14:56:38  mogul
 * Bugfix to compile on SunOS
 *
 * Revision 4.2  93/10/01  10:45:55  mogul
 * Revert to int32, u_int32 names
 * 
 * Revision 4.1  93/09/28  21:17:14  mogul
 * Added support for DECOSF
 * 
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 1.6  1993/01/16  19:12:54  davy
 * Moved cpp controls to left margin.
 *
 * Revision 1.5  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 1.4  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 1.3  1993/01/13  21:41:37  davy
 * Got rid of old IRIX versions.
 *
 * Revision 1.2  1993/01/13  21:24:54  davy
 * Added IRIX40.
 *
 * Revision 1.1  1993/01/13  20:18:17  davy
 * Initial revision
 *
 */
#ifdef OS4
#ifndef USE_NIT
#define USE_NIT	1
#endif
#define	U_INT32_DECLARED_IN_AUTH	1
#endif

#ifdef OS5
#ifndef SVR4
#define SVR4		1
#endif
#ifndef USE_DLPI
#define USE_DLPI	1
#endif
#define	U_INT32_DECLARED_IN_AUTH	1
#endif

#ifdef OS5
#ifndef SVR4
#define SVR4		1
#endif
#ifndef USE_DLPI
#define USE_DLPI	1
#endif
#define	U_INT32_DECLARED_IN_AUTH	1
#endif

#ifdef SUNOS55
#ifndef SUNOS54
#define SUNOS54		1
#endif
#undef U_INT32_DECLARED_IN_AUTH
#define index		strchr
#define rindex		strrchr
#define bzero(b,n)	memset(b,0,n)
#define bcmp(a,b,n)	memcmp(a,b,n)
#define bcopy(a,b,n)	memcpy(b,a,n)
#endif

#ifdef SVR4
#ifndef USE_DLPI
#define USE_DLPI	1
#endif
#define index		strchr
#define rindex		strrchr
#define signal		sigset
#define bzero(b,n)	memset(b,0,n)
#define bcmp(a,b,n)	memcmp(a,b,n)
#define bcopy(a,b,n)	memcpy(b,a,n)
#endif

#ifdef DECOSF
#ifndef USE_PFILT
#define USE_PFILT	1
#endif
#endif
