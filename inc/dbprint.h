/* $Id: dbprint.h,v 1.2 1999/01/03 02:05:55 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include <stdio.h>

/*  ================================================================  */
/*  Debugprint usage:  DBPRINT( paren'ed arglist )

    e.g.  DBPRINT ( ("value of foo is %d\n", foo) );

    the double parens are needed because of cpp's limited macro
    capability (can only handle variable number of args if they
    are paren'ed.  The motivation for this macro is, its easier to
    read:
            DBPRINT ( ("value of foo is %d\n", foo) );
    than:
	#ifdef DEBUG
            printf("value of foo is %d\n", foo);
	#endif

    e.g.  TRACER(expr);

    executes the expression if TRACE is on.                           */
/*  ================================================================  */

	/* For debugging print statements */

#if defined(DEBUG) || defined(TRACE) || defined(OPTRACE) || defined(FNTRACE) || defined(FNSTKCHECK)
extern int flushing;
#endif

#ifdef DEBUG
#define DBPRINT(X) do {printf X ; if (flushing) fflush(stdout); } while(0)
#define DEBUGGER(X) X
#else
#define DBPRINT(X) do {} while(0)
#define DEBUGGER(X)
#endif


	/* For trace print statements */

#ifdef TRACE
#define TPRINT(X) do {  printf X; if (flushing) fflush(stdout); } while (0)
#define TRACER(X) X
#else /* TRACE */

#define TPRINT(X) do { } while (0)
#define TRACER(X)
#endif /* TRACE */



	/* For tracing individual opcode executions */

#ifdef OPTRACE
#define OPTPRINT(X) do { printf X; if (flushing) fflush(stdout); } while (0)
#define OPTRACER(X) X
#else
#define OPTPRINT(X) do { } while (0)
#define OPTRACER(X)
#endif


	/* For tracing function calls */

#ifdef FNTRACE
#define FNTPRINT(X) do { printf X; if (flushing) fflush(stdout); } while (0)
#define FNTRACER(X)  X
#else
#define FNTPRINT(X) do { } while (0)
#define FNTRACER(X)
#endif


	/* For function-call & return stack checking */

#ifdef FNSTKCHECK
#define FNCHKPRINT(X) do { printf X ; if (flushing) fflush(stdout); } while (0)
#define FNCHECKER(X) X
#else
#define FNCHKPRINT(X) do { } while (0)
#define FNCHECKER(X)
#endif

