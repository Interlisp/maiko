#ifndef TOS1DEFS_H
#define TOS1DEFS_H 1
/* $Id: tos1defs.h,v 1.2 1999/01/03 02:06:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*		T O P - O F - S T A C K   D E F I N I T I O N S		*/
/*									*/
/*	TOPOFSTACK	cached top of stack value.			*/
/*	CSTKPTR		points to where TOPOFSTACK should be stored.	*/
/*									*/
/************************************************************************/

#ifndef BYTESWAP
/********************************************************/
/*   Normal byte-order definitions, for e.g., 68020s	*/
/********************************************************/

/* These are the TOS manipulation Macros */

#define HARD_PUSH(x) *(CSTKPTRL++) = x
#define PUSH(x)            \
  do {                     \
    HARD_PUSH(TOPOFSTACK); \
    TOPOFSTACK = x;        \
  } while (0)
#define POP TOPOFSTACK = *(--CSTKPTRL)
#define GET_TOS_1 *(CSTKPTR - 1)
#define GET_TOS_2 *(CSTKPTR - 2)
#define GET_POPPED *CSTKPTR
#define POP_TOS_1 *(--CSTKPTRL)
#define TOPOFSTACK tscache
#define GET_TOS_1_HI *((DLword *)(CSTKPTR - 1))
#define GET_TOS_1_LO *((DLword *)(CSTKPTR - 1) + 1)

#else

/********************************************************/
/*	Byte-swapped definitions, for e.g., 80386s	*/
/********************************************************/

/* These are the TOS manipulation Macros */

#define HARD_PUSH(x) *(CSTKPTRL++) = x
#define PUSH(x)            \
  do {                     \
    HARD_PUSH(TOPOFSTACK); \
    TOPOFSTACK = x;        \
  } while (0)
#define POP TOPOFSTACK = *(--CSTKPTRL)
#define GET_TOS_1 *(CSTKPTR - 1)
#define GET_TOS_2 *(CSTKPTR - 2)
#define GET_POPPED *CSTKPTR
#define POP_TOS_1 *(--CSTKPTRL)
#define TOPOFSTACK tscache
#define GET_TOS_1_HI GETWORD((DLword *)(CSTKPTR - 1))
#define GET_TOS_1_LO GETWORD((DLword *)(CSTKPTR - 1) + 1)

#endif /* BYTESWAP */

/* OPCODE interface routines */

#define StackPtrSave \
  do { CurrentStackPTR = (void *)(CSTKPTR - 1); /* CSTKPTR in cells */ } while (0)
#define StackPtrRestore \
  do { CSTKPTRL = (void *)(CurrentStackPTR + 2); /* CurrentStackPTR in DLwords */ } while (0)

#define EXT                  \
  do {                       \
    PC = pccache - 1;        \
    TopOfStack = TOPOFSTACK; \
    StackPtrSave;            \
  } while (0)

#define RET                  \
  do {                       \
    pccache = PC + 1;        \
    StackPtrRestore;         \
    TOPOFSTACK = TopOfStack; \
  } while (0)

#define NRET \
  do {       \
    RET;     \
    nextop0; \
  } while (0)

#endif /* TOS1DEFS_H */
