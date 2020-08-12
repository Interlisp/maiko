/* $Id: timeout.h,v 1.2 1999/01/03 02:06:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

extern jmp_buf jmpbuf;




/*** TIMEOUT_TIME is changeable by UNIX env var LDEFILETIMEOUT. 
#define	TIMEOUT_TIME	10 **/

extern	int	TIMEOUT_TIME;


#define	SETJMP(x)	\
  {				\
    if(setjmp(jmpbuf) != 0) return(x);		\
  }

#define	TIMEOUT(exp)			\
  {			\
    alarm(TIMEOUT_TIME);	\
    INTRSAFE(exp);			\
    alarm(0);		\
  }

#define	TIMEOUT0(exp)			\
  {			\
    alarm(TIMEOUT_TIME);	\
    INTRSAFE0(exp);			\
    alarm(0);		\
  }

#define	S_TOUT(exp)	\
		alarm(TIMEOUT_TIME),\
		(exp),			\
		alarm(0)

#define	ERRSETJMP(rval)					\
  {							\
    if(setjmp(jmpbuf) != 0)				\
      {							\
	*Lisp_errno = 100;				\
	return(rval);					\
      }							\
  }


/************************************************************************/
/*									*/
/*				INTRSAFE				*/
/*									*/
/*	Put a check for EINTR around a system call, and keep executing	*/
/*	the call until we don't get that error any more.		*/
/*									*/
/************************************************************************/

#define INTRSAFE(exp)				\
  do {} while ((int)(exp) == -1 && errno == EINTR)

#define INTRSAFE0(exp)				\
  do {} while ((int)(exp) == 0 && errno == EINTR)

#ifdef OS5
#define INTRCHECK(exp,var)				\
  while ((int)var == 0 && errno == EINTR) { exp; }
#else
#define INTRCHECK(exp,var)
#endif
