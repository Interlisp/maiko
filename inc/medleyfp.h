#ifndef MEDLEYFP_H
#define MEDLEYFP_H 1
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

/*  --------------------------------------------------
    FPCLEAR         - clear status as necessary
    FPTEST(result)  - check result or status

    Sun 4 compiler w. -O2 moves too much code around
    to use FLTINT.
    --------------------------------------------------  */

#ifdef FLTINT
#include <signal.h>
extern volatile sig_atomic_t FP_error;

/*  Note that a compiler may very likely move code around the arithmetic
    operation, causing this test (set by an interrupt handler) to be
    incorrect.  For example, the Sun SPARC compiler with -O2 makes
    this test incorrect.
 */

#define FPCLEAR         FP_error = 0;
#define FPTEST(result)  FP_error

#else
#include <math.h>
#define FPCLEAR
#define FPTEST(result) (!isfinite(result))

#endif /* FLTINT */
#endif /* MEDLEYFP_H */
