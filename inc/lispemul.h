#ifndef LISPEMUL_H
#define LISPEMUL_H 1
/* $Id: lispemul.h,v 1.4 2001/12/24 01:08:57 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "version.h"

#include "lispmap.h"     // for S_POSITIVE

#ifndef BYTESWAP
/*** Normal byte-order type decls */
typedef struct {
  unsigned char code;
} BYTECODE;
typedef char ByteCode;
typedef unsigned short DLword;
typedef char DLbyte;
typedef unsigned int LispPTR;
/* 32 bit Cell Chang. 14 Jan 87 take */
typedef DLword mds_page; /* Top word of the MDS */

#ifdef BIGVM
typedef struct consstr {
  unsigned cdr_code : 4;
  unsigned car_field : 28;
} ConsCell;

typedef struct ufn_entry {
  DLword atom_name;      /* UFN's atomindex */
  unsigned byte_num : 8; /* num of byte code */
  unsigned arg_num : 8;  /* num of arguments */
} UFN;

typedef struct closure_type {
  unsigned nil1 : 4;
  unsigned def_ptr : 28; /* LispPTR to definition cell */
  unsigned nil2 : 4;
  unsigned env_ptr : 28; /* LispPTR to environment */
} Closure;

#else  /* not BIGVM */
typedef struct consstr {
  unsigned cdr_code : 8;
  unsigned car_field : 24;
} ConsCell;

typedef struct ufn_entry {
  DLword atom_name;      /* UFN's atomindex */
  unsigned byte_num : 8; /* num of byte code */
  unsigned arg_num : 8;  /* num of arguments */
} UFN;

typedef struct closure_type {
  unsigned nil1 : 8;
  unsigned def_ptr : 24; /* LispPTR to definition cell */
  unsigned nil2 : 8;
  unsigned env_ptr : 24; /* LispPTR to environment */
} Closure;
#endif /* BIGVM */

typedef struct interrupt_state { /* Interrupt-request mask to communicate with INTERRUPTED */
  unsigned LogFileIO : 1;        /* console msg arrived to print */
  unsigned ETHERInterrupt : 1;   /* 10MB activity happened */
  unsigned IOInterrupt : 1;      /* I/O happened (not used yet) */
  unsigned gcdisabled : 1;
  unsigned vmemfull : 1;
  unsigned stackoverflow : 1;
  unsigned storagefull : 1;
  unsigned waitinginterrupt : 1;
  unsigned nil : 8; /* mask of ints being processed */
  DLword intcharcode;
} INTSTAT;

typedef struct interrupt_state_2 { /* alternate view of the interrupt state */
  unsigned pendingmask : 8;
  unsigned handledmask : 8;
  DLword nil;
} INTSTAT2;

struct state {
  DLword *ivar;
  DLword *pvar;
  DLword *csp;
  ByteCode *currentpc;
  struct fnhead *currentfunc;
  DLword *endofstack;
  UNSIGNED irqcheck;
  UNSIGNED irqend;
  LispPTR tosvalue;
  LispPTR scratch_cstk;
  int errorexit;
};

/***** Get_DLword(ptr) ptr is char* ***/
#ifndef UNALIGNED_FETCH_OK
#define Get_DLword(ptr) ((Get_BYTE(ptr) << 8) | Get_BYTE((ptr) + 1))
#else
#define Get_DLword(ptr) *(((DLword *)WORDPTR(ptr)))
#endif

#ifdef BIGVM
#define Get_Pointer(ptr) \
  ((Get_BYTE(ptr) << 24) | (Get_BYTE((ptr) + 1) << 16) | (Get_BYTE((ptr) + 2) << 8) | Get_BYTE((ptr) + 3))
#else
#define Get_Pointer(ptr) ((Get_BYTE(ptr) << 16) | (Get_BYTE((ptr) + 1) << 8) | Get_BYTE((ptr) + 2))
#endif /* BIGVM */

#define Get_code_BYTE Get_BYTE
#define Get_code_DLword Get_DLword
#define Get_code_AtomNo Get_AtomNo
#define Get_code_Pointer Get_Pointer

#ifdef BIGATOMS
#define Get_AtomNo(ptr) Get_Pointer(ptr)
#else
#define Get_AtomNo(ptr) Get_DLword(ptr)
#endif /* BIGATOMS */

/* For bit test */
typedef struct wbits {
  unsigned xMSB : 1;
  unsigned B1 : 1;
  unsigned B2 : 1;
  unsigned B3 : 1;
  unsigned B4 : 1;
  unsigned B5 : 1;
  unsigned B6 : 1;
  unsigned B7 : 1;
  unsigned B8 : 1;
  unsigned B9 : 1;
  unsigned B10 : 1;
  unsigned B11 : 1;
  unsigned B12 : 1;
  unsigned B13 : 1;
  unsigned B14 : 1;
  unsigned LSB : 1;
} WBITS;

#define PUTBASEBIT68K(base68k, offset, bitvalue)               \
  do {                                                         \
    if (bitvalue)                                              \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) |=   \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);    \
    else                                                       \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) &=   \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD)); \
  } while (0)

#else
/*** Byte-swapped structure declarations, for 80386 ***/
typedef struct {
  unsigned char code;
} BYTECODE;
typedef char ByteCode;
typedef unsigned short DLword;
typedef char DLbyte;
typedef unsigned int LispPTR;
/* 32 bit Cell Chang. 14 Jan 87 take */
typedef DLword mds_page; /* Top word of the MDS */

#ifdef BIGVM
typedef struct consstr {
  unsigned car_field : 28;
  unsigned cdr_code : 4;
} ConsCell;

typedef struct ufn_entry {
  unsigned arg_num : 8;  /* num of arguments */
  unsigned byte_num : 8; /* num of byte code */
  DLword atom_name;      /* UFN's atomindex */
} UFN;

typedef struct closure_type {
  unsigned def_ptr : 28; /* LispPTR to definition cell */
  unsigned nil1 : 4;
  unsigned env_ptr : 28; /* LispPTR to environment */
  unsigned nil2 : 4;
} Closure;
#else  /* BIGVM */
typedef struct consstr {
  unsigned car_field : 24;
  unsigned cdr_code : 8;
} ConsCell;

typedef struct ufn_entry {
  unsigned arg_num : 8;  /* num of arguments */
  unsigned byte_num : 8; /* num of byte code */
  DLword atom_name;      /* UFN's atomindex */
} UFN;

typedef struct closure_type {
  unsigned def_ptr : 24; /* LispPTR to definition cell */
  unsigned nil1 : 8;
  unsigned env_ptr : 24; /* LispPTR to environment */
  unsigned nil2 : 8;
} Closure;
#endif /* BIGVM */

typedef struct interrupt_state { /* Interrupt-request mask to communicate with INTERRUPTED */
  DLword intcharcode;
  unsigned nil : 8;
  unsigned waitinginterrupt : 1;
  unsigned storagefull : 1;
  unsigned stackoverflow : 1;
  unsigned vmemfull : 1;
  unsigned gcdisabled : 1;
  unsigned IOInterrupt : 1;    /* I/O happened (not used yet) */
  unsigned ETHERInterrupt : 1; /* 10MB activity happened */
  unsigned LogFileIO : 1;      /* console msg arrived to print */
} INTSTAT;

typedef struct interrupt_state_2 { /* alternate view of the interrupt state */
  DLword nil;
  unsigned handledmask : 8;
  unsigned pendingmask : 8;
} INTSTAT2;

struct state {
  DLword *ivar;
  DLword *pvar;
  DLword *csp;
  ByteCode *currentpc;
  struct fnhead *currentfunc;
  DLword *endofstack;
  UNSIGNED irqcheck;
  UNSIGNED irqend;
  LispPTR tosvalue;
  LispPTR scratch_cstk;
  int errorexit;
};

/* Fetching 2 bytes to make a word -- always do it the hard way */
/* if we're byte-swapped:  You can't rely on byte ordering!!    */
#define Get_DLword(ptr) ((Get_BYTE(ptr) << 8) | Get_BYTE((ptr) + 1))

#ifdef BIGVM
#define Get_Pointer(ptr) \
  ((Get_BYTE(ptr) << 24) | (Get_BYTE((ptr) + 1) << 16) | (Get_BYTE((ptr) + 2) << 8) | Get_BYTE((ptr) + 3))
#else
#define Get_Pointer(ptr) ((Get_BYTE(ptr) << 16) | (Get_BYTE(ptr + 1) << 8) | Get_BYTE(ptr + 2))
#endif /* BIGVM */

#ifndef RESWAPPEDCODESTREAM
#define Get_code_BYTE(ptr) Get_BYTE(ptr)
#define Get_code_AtomNo Get_AtomNo
#define Get_code_DLword Get_DLword
#else
#define Get_code_BYTE(ptr) (((BYTECODE *)(ptr))->code)

#define Get_code_Pointer(ptr) \
  ((Get_code_BYTE(ptr) << 16) | (Get_code_BYTE(ptr + 1) << 8) | Get_code_BYTE(ptr + 2))
#define Get_code_DLword(ptr) ((Get_code_BYTE(ptr) << 8) | Get_code_BYTE(ptr + 1))
#define Get_code_AtomNo Get_code_Pointer
#endif /* RESWAPPEDCODESTREAM */

#ifdef BIGATOMS
#define Get_AtomNo(ptr) Get_Pointer(ptr)
#else
#define Get_AtomNo(ptr) Get_DLword(ptr)
#endif /* BIGATOMS */

/* For bit test */
typedef struct wbits {
  USHORT LSB : 1;
  USHORT B14 : 1;
  USHORT B13 : 1;
  USHORT B12 : 1;
  USHORT B11 : 1;
  USHORT B10 : 1;
  USHORT B9 : 1;
  USHORT B8 : 1;
  USHORT B7 : 1;
  USHORT B6 : 1;
  USHORT B5 : 1;
  USHORT B4 : 1;
  USHORT B3 : 1;
  USHORT B2 : 1;
  USHORT B1 : 1;
  USHORT xMSB : 1;
} WBITS;

#define PUTBASEBIT68K(base68k, offset, bitvalue)                                             \
  do {                                                                                       \
    UNSIGNED real68kbase;                                                                    \
    real68kbase = 2 ^ ((UNSIGNED)(base68k));                                                 \
    if (bitvalue)                                                                            \
      (*(DLword *)(2 ^ (UNSIGNED)((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)))) |= \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);                                  \
    else                                                                                     \
      (*(DLword *)(2 ^ (UNSIGNED)((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)))) &= \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD));                               \
  } while (0)

#endif /* BYTESWAP */

/* Because a WBITS is only 1 word long, need byte-swapped */
/* access to it.  Use WBITSPTR(x) instead of ((WBITS *) x) */

#define WBITSPTR(ptr) ((WBITS *)WORDPTR(ptr))

extern struct state MachineState;

#define CURRENTFX ((struct frameex1 *)(void *)(((DLword *)PVar) - FRAMESIZE))
#define IVar (MachineState.ivar)
#define PVar (MachineState.pvar)
#define CurrentStackPTR (MachineState.csp)
#define TopOfStack (MachineState.tosvalue)
#define PC (MachineState.currentpc)
#define FuncObj (MachineState.currentfunc)
#define EndSTKP (MachineState.endofstack)
#define Irq_Stk_Check (MachineState.irqcheck)
#define Irq_Stk_End (MachineState.irqend)
#define Scratch_CSTK (MachineState.scratch_cstk)
#define Error_Exit (MachineState.errorexit)

/****************************************************
 MakeAddr:
        base:	DLword*
        offset:	word offset from base
        return:	DLword*
****************************************************/
#define MakeAddr(base, offset) ((DLword *)((base) + (int)(offset)))

/****************************************************
GetHiWord:
*****************************************************/
#define GetHiWord(x) ((DLword)((x) >> 16))

/****************************************************
GetLoWord:
*****************************************************/
#define GetLoWord(x) ((DLword)(x))

/****************************************************
GetLongWord:
        address:	DLword*
        return:		int
*****************************************************/
#define GetLongWord(address) (*((LispPTR *)(address)))

/* The stack is maintained as a DLword* pointer, carefully incremented
 * and decremented by 2 (= 4 bytes), and mostly accessed as a LispPTR*
 * to fetch 4-byte cells. There are a few places where items are
 * accessed as 2 separate DLwords, so don't be tempted to blindly replace
 * the DLword* declaration by a LispPTR*.  The (void *) cast discourages
 * the compiler from complaining about a required upgrade in the
 * alignment when we know(!) that it will always point to an
 * appropriate boundary.
 */

/****************************************************
PopCStack:
#define PopCStack	{TopOfStack = *((LispPTR *)(--CurrentStackPTR)); --CurrentStackPTR;}
*****************************************************/
#define PopCStack                                 \
  do {                                            \
    TopOfStack = *((LispPTR *)(void *)(CurrentStackPTR));	\
    CurrentStackPTR -= 2;                         \
  } while (0)

/****************************************************
PopStackTo:  CSTK -> Place
#define PopStackTo(Place)	{Place= *((LispPTR *)(void *)(--CurrentStackPTR)); CurrentStackPTR--; }
*****************************************************/
#define PopStackTo(Place)                      \
  do {                                         \
    (Place) = *((LispPTR *)(void *)(CurrentStackPTR));	\
    CurrentStackPTR -= 2;                      \
  } while (0)

/****************************************************
PushCStack:
#define PushCStack	{*((int *)(++CurrentStackPTR)) = TopOfStack; ++CurrentStackPTR;}
*****************************************************/
#define PushCStack                                \
  do {                                            \
    CurrentStackPTR += 2;                         \
    *((LispPTR *)(void *)(CurrentStackPTR)) = TopOfStack;	\
  } while (0)

/****************************************************
PushStack:
#define PushStack(x)	{*((LispPTR *)(++CurrentStackPTR))=x;CurrentStackPTR++;}
*****************************************************/
#define PushStack(x)                     \
  do {                                   \
    CurrentStackPTR += 2;                \
    *((LispPTR *)(void *)(CurrentStackPTR)) = x;	\
  } while (0)

/****************************************************
SmashStack:
#define SmashStack(x)	(*((LispPTR *)(CurrentStackPTR-1))=x)
*****************************************************/
#define SmashStack(x) (*((LispPTR *)(void *)(CurrentStackPTR)) = (x))

/*********************************************************
Get_BYTE(byteptr)	byteptr: pointer to  8 bit data
**********************************************************/
/***** OLD definition ************* 13 Nov 1987 takeshi ***
#define Get_BYTE(byteptr)	(((unsigned)(*(byteptr))) & 0xff)
**********************************************/
#define Get_BYTE(byteptr) (((BYTECODE *)BYTEPTR(byteptr))->code)

/**********************************************************
DOSTACKOVERFLOW(argnum,bytenum) if it needs hardreturn-cleanup
        then punt to contextsw and immediately return
**********************************************************/
#define DOSTACKOVERFLOW(argnum, bytenum) \
  do {                                   \
    if (do_stackoverflow(T)) {           \
      PushStack(S_POSITIVE | (argnum));  \
      contextsw(SubovFXP, bytenum, 1);   \
      return;                            \
    }                                    \
  } while (0)

/************************************************************************/
/*									*/
/*			E X T E R N A L   F U N C T I O N S		*/
/*									*/
/*	Declare all functions that will default incorrectly under	*/
/*	normal C inference rules.  These functions are the ones		*/
/*	that return pointers, because on DEC Alpha, a pointer is 8	*/
/*	bytes--but the default fn decl is int, a 4-byte return.		*/
/*									*/
/************************************************************************/
/*  All external functions defined in xxx.c now declared in xxxdefs.h   */

/************************************************************************/
/*									*/
/*		E R R O R   &   T I M E R   P U N T   C A S E S		*/
/*									*/
/*	Set up the top-of-stack so we can continue gracefully after	*/
/*	handling the timer interrupt (e.g., FMEMB, which walks down	*/
/*	the list being searched, pushes its current state on TOS	*/
/*	so that it picks up where it left off after the interrupt.	*/
/*									*/
/*	Call Interface where neg number indicates an error return	*/
/*	but the function returns a LispPTR and casts back to int	*/
/*									*/
/************************************************************************/

#define ERROR_EXIT(tos) \
  do {                  \
    TopOfStack = (LispPTR)tos; \
    Error_Exit = 1;     \
    return ((LispPTR)-1); \
  } while (0)
#define TIMER_EXIT(tos) \
  do {                  \
    TopOfStack = (LispPTR)tos; \
    Error_Exit = 1;     \
    return ((LispPTR)-2); \
  } while (0)

#define WARN(message, operation) \
  do {                           \
    warn(message);               \
    operation;                   \
  } while (0)
#define NO_WOP \
  {}

#define NIL 0 /* added 29-jan */
#define T 1
#define ATOM_T 0114 /* T's AtomIndex Number 114Q */

#define NIL_PTR 0 /* from cell.h 24-mar-87 take */
#define NOBIND_PTR 1

#define STKLIM 0x1FFFF
#define FRAMESIZE 10 /* size of frameex1: 10 words */
#define FNHEADSIZE 8 /* size of fnhead: 8 words */
#define BFSIZE 2     /* size of basic frame pointer: 2 words */

#define BITSPER_DLWORD 16
#define BITSPER_CELL 32
#define BYTESPER_DLWORD 2
#define BYTESPER_CELL 4
#define BYTESPER_QUAD 8
#define BYTESPER_PAGE 512
#define CELLSPER_QUAD 2
#define CELLSPER_PAGE 128
#define CELLSPER_SEGMENT 32768
#define DLWORDSPER_CELL 2
#define DLWORDSPER_QUAD 4
#define DLWORDSPER_PAGE 256
#define DLWORDSPER_SEGMENT 65536
#define PAGESPER_SEGMENT 256
#define PAGESPER_MDSUNIT 2
#define MDSINCREMENT 512

#define GUARDSTORAGEFULL 128
#define GUARD1STORAGEFULL 64

#define SFS_NOTSWITCHABLE 1
#define SFS_SWITCHABLE 2
#define SFS_ARRAYSWITCHED 3
#define SFS_FULLYSWITCHED 4

#define AtomHTSIZE (256 * DLWORDSPER_PAGE)

#define MAXPNCHARS 255 /* Maximum length of PnChars */

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

typedef unsigned int boolean;

/************************************************************************/
/*	Define sizes of FN and FNX opcodes; depends on atom size	*/
/************************************************************************/

#ifdef BIGVM
#define FN_OPCODE_SIZE 5
#define FNX_OPCODE_SIZE 6
#elif defined(BIGATOMS)
#define FN_OPCODE_SIZE 4
#define FNX_OPCODE_SIZE 5
#else
#define FN_OPCODE_SIZE 3
#define FNX_OPCODE_SIZE 4
#endif /* BIGATOMS */

/************************************************************************/
/*									*/
/*			Definitions for "NEW" Symbols			*/
/*									*/
/*	Offsets within the "New	 symbols that go with 3-byte atoms.	*/
/*									*/
/************************************************************************/
#ifdef BIGATOMS

typedef struct newatom {
  LispPTR na_pname; /* Pointer to the print name */
  LispPTR na_value; /* The value cell */
  LispPTR na_defn;  /* The definition cell */
  LispPTR na_plist; /* The property list */
  LispPTR na_flags; /* flags from other cells, to make BIGVM work ok */
} NEWATOM;

/* Offsets, in WORDS, from the start of the NEWATOM structure */
#define NEWATOM_PNAME_OFFSET 0
#define NEWATOM_VALUE_OFFSET 2
#define NEWATOM_DEFN_OFFSET 4
#define NEWATOM_PLIST_OFFSET 6

/* Offsets, in cells from start of the NEWATOM structure */
#define NEWATOM_PNAME_PTROFF 0
#define NEWATOM_VALUE_PTROFF 1
#define NEWATOM_DEFN_PTROFF 2
#define NEWATOM_PLIST_PTROFF 3
#endif

/************************************************************************/
/*									*/
/*	Mask to mask off relevant bits in a pointer.			*/
/*									*/
/************************************************************************/
#ifdef BIGVM
#define POINTERMASK 0xfffffff
#define SEGMASK 0xfff0000
#define mPAGEMASK 0xfffff00
#else
#define POINTERMASK 0xffffff
#define SEGMASK 0xff0000
#define mPAGEMASK 0xffff00
#endif /* BIGVM */

/************************************************************************/
/*									*/
/*			F P t o V P   M a n i p u l a t i o n			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef BIGVM
#define GETFPTOVP(b, o) b[o]
#define GETPAGEOK(b, o) ((b)[o] >> 16)
#else
#define GETFPTOVP GETWORDBASEWORD
#define GETPAGEOK GETWORDBASEWORD
#endif
#endif /* LISPEMUL_H */
