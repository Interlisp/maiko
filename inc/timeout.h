#ifndef TIMEOUT_H
#define TIMEOUT_H 1
/* $Id: timeout.h,v 1.2 1999/01/03 02:06:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include <setjmp.h> /* for jmp_buf */
#include <unistd.h> /* for alarm */

extern jmp_buf jmpbuf;

/*** TIMEOUT_TIME is changeable by UNIX env var LDEFILETIMEOUT. 
#define	TIMEOUT_TIME	10 **/

extern	unsigned int TIMEOUT_TIME;

#define	SETJMP(x)	\
  do {				\
    if(setjmp(jmpbuf) != 0) return(x);		\
  } while (0)

#define	TIMEOUT(exp)			\
  do {			\
    alarm(TIMEOUT_TIME);	\
    INTRSAFE(exp);			\
    alarm(0);		\
  } while (0)

#define	TIMEOUT0(exp)			\
  do {			\
    alarm(TIMEOUT_TIME);	\
    INTRSAFE0(exp);			\
    alarm(0);		\
  } while (0)

#define	S_TOUT(exp)	\
  alarm(TIMEOUT_TIME),                  \
    (exp),                              \
    alarm(0)

#define	ERRSETJMP(rval)					\
  do {							\
    if(setjmp(jmpbuf) != 0)				\
      {							\
	*Lisp_errno = 100;				\
	return(rval);					\
      }							\
  } while (0)


/************************************************************************/
/*									*/
/*				INTRSAFE				*/
/*									*/
/*	Put a check for EINTR around a system call, and keep executing	*/
/*	the call until we don't get that error any more.		*/
/*									*/
/************************************************************************/

#define INTRSAFE(exp)				\
  do {errno = 0; } while ((exp) == -1 && errno == EINTR)

#define INTRSAFE0(exp)				\
  do {errno = 0; } while ((exp) == NULL && errno == EINTR)
#endif /* TIMEOUT_H */
