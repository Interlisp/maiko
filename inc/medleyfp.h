/* $Id: medleyfp.h,v 1.2 1999/01/03 02:06:16 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-94 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef DOS
#include <i32.h>
#endif /* DOS */

#ifdef RISCOS
/*#include <ieeefp.h> */
/*#define isnan isnand */
#define isnan(x) 0
#endif /* RISCOS */


/*  --------------------------------------------------
    FPCLEAR         - clear status as necessary
    FPTEST(result)  - check result or status

    Sun 4 compiler w. -O2 moves too much code around
    to use FLTINT.
    --------------------------------------------------  */

#ifdef FLTINT
volatile extern int  FP_error;


/*  Note that a compiler may very likely move code around the arithmetic
    operation, causing this test (set by an interrupt handler) to be
    incorrect.  For example, the Sun SPARC compiler with -O2 makes
    this test incorrect.
 */

#define FPCLEAR         FP_error = 0;
#define FPTEST(result)  FP_error

#else

/*  fpstatus_ is a FORTRAN library routine (in libc) which
    can be called to determine floating point status results.
    Documented in the Sun manual, "Floating Point Programmer's Guide",
    (Rev. A 19-Sep-86), pg. 34, it does *not* exist in libc for the
    SPARC.

    For sparc, should also check for isnan?  Don't know what isnormal
    & issubnormal do (these are sunos4.0 only)
 */
#if defined(OS5)
#define FPCLEAR
#define FPTEST(result) (!finite(result))

#elif defined(sparc) || defined(I386)
#define FPCLEAR
#define FPTEST(result) (isinf(result) || isnan(result))

#elif defined(OSF1)
#include <fp.h>
#define FPCLEAR
#define FPTEST(result) (!FINITE(result))

#elif defined(AIX)
#define FPCLEAR
#define FPTEST(result) ((!finite(result)) || isnan(result))

#elif defined(MACOSX) || defined(FREEBSD)
#include <math.h>
#define FPCLEAR
#define FPTEST(result) (!isfinite(result))

#elif defined(DOS)
#define FPCLEAR
#define FPTEST(result) (_getrealerror() & ( I87_ZERO_DIVIDE | I87_OVERFLOW | I87_UNDERFLOW))

#else
static int constant0 = 0;
unsigned int fpstatus_();

#define FPCLEAR         fpstatus_(&constant0);
#define FPTEST(result) (fpstatus_(&constant0) & 0xF0)

#endif
#endif /* FLTINT */



