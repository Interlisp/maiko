#ifndef STACK_H
#define STACK_H 1
/* $Id: stack.h,v 1.2 1999/01/03 02:06:23 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "version.h"  /* for BIGVM, UNSIGNED */
#include "adr68k.h"   /* for LAddrFromNative, NativeAligned4FromLAddr */
#include "address.h"  /* for LOLOC */
#include "commondefs.h" /* for error */
#include "lispemul.h" /* for LispPTR, DLword, FRAMESIZE, DLWORDSPER_CELL */

/* ContextSW frame number */
#define CurrentFXP 0
#define ResetFXP 1
#define SubovFXP 2
#define KbdFXP 3
#define HardReturnFXP 4
#define GCFXP 5
#define FAULTFXP 6

#define STK_FSB_WORD 0xA000u
#define STK_GUARD_WORD 0xE000u
#define BF_MARK 0x8000u
#define BF_MARK32 0x80000000
#define FX_MARK 0xc000u

#define STK_GUARD 7
#define STK_FX 6
#define STK_FSB 5
#define STK_BF 4
#define STK_NOTFLG 0

#define STK_SAFE 32 /* added with stkmin */
#define MINEXTRASTACKWORDS 32
#define STACKAREA_SIZE 768

/* For Fvar operations */
#define FVSTACK 2
#define FVGLOBAL 6
#define FVUBNBOUND 3
#define FVIVAR 0x0
#define FVPVAR 0x80
#define FVFVAR 0xC0
#define ENDSTACKMARK 0xb

#define FXPTR(base) ((struct frameex1 *)WORDPTR(base))

#ifndef BYTESWAP

/*******************************************************/
/*      Normal definitions for structures on stack     */
/*******************************************************/
typedef struct fnhead {
  DLword stkmin; /* ?? */
  short na;      /* Numbers of arguments */
  short pv;      /* ?? */
  DLword startpc;
  /* head of ByteCodes, DLword offset from stkmin */
  unsigned nil4 : 1;        /* not used, prev: native translated? */
  unsigned byteswapped : 1; /* code was reswapped.	 */
  unsigned argtype : 2;     /* ?? */
#ifdef BIGVM
  unsigned framename : 28; /* index in AtomSpace */
#else
  unsigned nil2 : 2;       /* not used */
  unsigned nil3 : 2;       /* not used */
  unsigned framename : 24; /* index in AtomSpace */
#endif /* BIGVM */
  DLword ntsize;        /* size of NameTable */
  unsigned nlocals : 8; /* ?? */
  unsigned fvaroffset : 8;
  /* DLword offset from head of NameTable */
  /* NameTable of variable length is following with this structure. */
} FNHEAD;

typedef struct frameex1 {
  unsigned short flags : 3;
  unsigned short fast : 1;
  unsigned short nil2 : 1; /* not used, prev: This frame treats N-func */
  unsigned short incall : 1;
  unsigned short validnametable : 1;
  /* 0: look for FunctionHeader
     1: look for NameTable on this FrameEx */
  unsigned short nopush : 1;
  unsigned short usecount : 8;
  DLword alink; /* alink pointer (Low addr) */
#ifdef BIGVM
  LispPTR fnheader; /* pointer to FunctionHeader (Hi2 addr) */
#else
  DLword lofnheader;        /* pointer to FunctionHeader (Low addr) */
  unsigned short hi1fnheader : 8; /* pointer to FunctionHeader (Hi1 addr) */
  unsigned short hi2fnheader : 8; /* pointer to FunctionHeader (Hi2 addr) */
#endif /* BIGVM */
  DLword nextblock; /* pointer to FreeStackBlock */
  DLword pc;        /* Program counter */
#ifdef BIGVM
  LispPTR nametable; /* ptr to NameTable of this FrameEx (Hi2 addr) */
#else
  DLword lonametable;        /* ptr to NameTable of this FrameEx (Low addr) */
  unsigned short hi1nametable : 8; /* ptr to NameTable of this FrameEx (Hi1 addr) */
  unsigned short hi2nametable : 8; /* ptr to NameTable of this FrameEx (Hi2 addr) */
#endif /* BIGVM */
  DLword blink; /* blink pointer (Low addr) */
  DLword clink; /* clink pointer (Low addr) */
} FX;

typedef struct frameex2 {
  unsigned short flags : 3;
  unsigned short fast : 1;
  unsigned short nil2 : 1; /* not used, prev: This frame treats N-func */
  unsigned short incall : 1;
  unsigned short validnametable : 1;
  /* 0: look for FunctionHeader
     1: look for NameTable on this FrameEx */
  unsigned short nopush : 1;
  unsigned short usecount : 8;
  DLword alink;      /* alink pointer (Low addr) */
  LispPTR fnheader;  /* pointer to FunctionHeader */
  DLword nextblock;  /* pointer to FreeStackBlock */
  DLword pc;         /* Program counter */
  LispPTR nametable; /* address of NameTable */
  DLword blink;      /* blink pointer (Low addr) */
  DLword clink;      /* clink pointer (Low addr) */
} FX2;

typedef struct fxblock {
  unsigned flagbyte : 8;
  unsigned nil : 23;
  unsigned slowp : 1;
} FXBLOCK;

typedef struct basic_frame {
  unsigned short flags : 3;
  unsigned short nil : 3;
  unsigned short residual : 1;
  unsigned short padding : 1;
  unsigned short usecnt : 8;
  DLword ivar; /* stk offset of IVARs for this frame ?? */

} Bframe;

typedef struct stkword {
  unsigned short flags : 3;
  unsigned short nil : 5;
  unsigned short usecount : 8;
} StackWord;

typedef struct stack_block {
  DLword flagword;
  DLword size;
} STKBLK;

/* Lisp DATATYPE STACKP */
typedef struct stackp {
  DLword stackp0;
  DLword edfxp;
} STACKP;

/*************************************************************/
/*  Pointer-dereferencing macros for one-word structure ptrs */
/*************************************************************/
#define BFRAMEPTR(ptr) ((Bframe *)(void *)(ptr))
#define STKWORDPTR(ptr) ((StackWord *)(ptr))

#else

/*******************************************************/
/*    Byte-swapped/Word-swapped definitions of stack   */
/*******************************************************/
typedef struct fnhead {
  short na;      /* Numbers of arguments */
  DLword stkmin; /* ?? */
  DLword startpc;
  /* head of ByteCodes, DLword offset from stkmin */
  short pv; /* ?? */
#ifdef BIGVM
  unsigned framename : 28; /* index in AtomSpace */
#else
  unsigned framename : 24; /* index in AtomSpace */
  unsigned nil3 : 2;       /* not used */
  unsigned nil2 : 2;       /* not used */
#endif /* BIGVM */
  unsigned short argtype : 2;     /* ?? */
  unsigned short byteswapped : 1; /* code was reswapped.	*/
  unsigned short nil4 : 1;        /* not used, prev: native translated? */
  unsigned short fvaroffset : 8;
  /* DLword offset from head of NameTable */
  unsigned short nlocals : 8; /* ?? */
  DLword ntsize;        /* size of NameTable */
                        /* NameTable of variable length is following with this structure. */
} FNHEAD;

typedef struct frameex1 {
  DLword alink; /* alink pointer (Low addr) */
  unsigned short usecount : 8;
  unsigned short nopush : 1;
  unsigned short validnametable : 1;
  /* 0: look for FunctionHeader
     1: look for NameTable on this FrameEx */
  unsigned short incall : 1;
  unsigned short nil2 : 1; /* not used, prev: This frame treats N-func */
  unsigned short fast : 1;
  unsigned short flags : 3; /* hi word */

#ifdef BIGVM
  LispPTR fnheader; /* pointer to FunctionHeader (Hi2 addr) */
#else
  unsigned short hi2fnheader : 8; /* pointer to FunctionHeader (Hi2 addr) */
  unsigned short hi1fnheader : 8; /* pointer to FunctionHeader (Hi1 addr) */
  DLword lofnheader;        /* pointer to FunctionHeader (Low addr) */
#endif /* BIGVM */

  DLword pc;        /* Program counter */
  DLword nextblock; /* pointer to FreeStackBlock */

#ifdef BIGVM
  LispPTR nametable; /* pointer to NameTable of this FX (Hi2 addr) */
#else
  unsigned short hi2nametable : 8; /* pointer to NameTable of this FX (Hi2 addr) */
  unsigned short hi1nametable : 8; /* pointer to NameTable of this FX (Hi1 addr) */
  DLword lonametable;        /* pointer to NameTable of this FX (Low addr) */
#endif /* BIGVM */

  DLword clink; /* clink pointer (Low addr) */
  DLword blink; /* blink pointer (Low addr) */
} FX;

typedef struct frameex2 {
  DLword alink; /* alink pointer (Low addr) */
  unsigned short usecount : 8;
  unsigned short nopush : 1;
  unsigned short validnametable : 1;
  /* 0: look for FunctionHeader
     1: look for NameTable on this FrameEx */
  unsigned short incall : 1;
  unsigned short nil2 : 1; /* not used, prev: This frame treats N-func */
  unsigned short fast : 1;
  unsigned short flags : 3;

  LispPTR fnheader; /* pointer to FunctionHeader (swapped) */

  DLword pc;        /* Program counter */
  DLword nextblock; /* pointer to FreeStackBlock */

  LispPTR nametable; /* address of NameTable (swapped) */

  DLword clink; /* clink pointer (Low addr) */
  DLword blink; /* blink pointer (Low addr) */
} FX2;

typedef struct fxblock {
  unsigned slowp : 1;
  unsigned nil : 23;
  unsigned flagbyte : 8;
} FXBLOCK;

typedef struct basic_frame {
  DLword ivar;
  unsigned short usecnt : 8;
  unsigned short padding : 1;
  unsigned short residual : 1;
  unsigned short nil : 3;
  unsigned short flags : 3;

} Bframe;

typedef struct stkword {
  USHORT usecount : 8;
  USHORT nil : 5;
  USHORT flags : 3;
} StackWord;

typedef struct stack_block {
  DLword size;
  DLword flagword;
} STKBLK;

/* Lisp DATATYPE STACKP */
typedef struct stackp {
  DLword edfxp;
  DLword stackp0;
} STACKP;

/*************************************************************/
/*  Pointer-dereferencing macros for one-word structure ptrs */
/*************************************************************/
#define BFRAMEPTR(ptr) ((Bframe *)(void *)(ptr))
#define STKWORDPTR(ptr) ((StackWord *)(2 ^ (UNSIGNED)(ptr)))

#endif

#define STKWORD(stkptr) ((StackWord *)WORDPTR(stkptr))

#define FX_INVALIDP(fx68k) (((fx68k) == 0) || ((DLword *)(fx68k) == Stackspace))
#define FX_size(fx68k) (((FX *)(void *)(fx68k))->nextblock - LOLOC(LAddrFromNative(fx68k)))
#define FSBP(ptr68k) (((STKBLK *)(void *)(ptr68k))->flagword == STK_FSB_WORD)
#define FSB_size(ptr68k) (((STKBLK *)(void *)(ptr68k))->size)
/** Following suff assumes fx is 68kptr and val is LISP_LO_OFFSET **/
#define DUMMYBF(fx) (((DLword *)(fx)) - DLWORDSPER_CELL)
#define SLOWP(fx) (((FXBLOCK *)(void *)(fx))->slowp)
#define FASTP(fx) (!SLOWP(fx))
#define SET_FASTP_NIL(fx68k)                                       \
  do {                                                             \
    if (FASTP(fx68k)) {                                            \
      ((FX *)(fx68k))->blink = StackOffsetFromNative(DUMMYBF(fx68k)); \
      ((FX *)(fx68k))->clink = ((FX *)(fx68k))->alink;             \
      SLOWP(fx68k) = T;                                            \
    }                                                              \
  } while (0)

#define GETALINK(fx) ((((fx)->alink) & 0xfffe) - FRAMESIZE)
#define SETALINK(fx, val)                                \
  do {                                                   \
    if (FASTP(fx)) {                                     \
      ((FX *)(fx))->blink = LAddrFromNative(DUMMYBF(fx)); \
      ((FX *)(fx))->clink = ((FX *)(fx))->alink;         \
    }                                                    \
    ((FX *)(fx))->alink = (val) + FRAMESIZE + 1;         \
  } while (0)

#define GETBLINK(fx) (SLOWP(fx) ? ((FX *)(fx))->blink : LOLOC(LAddrFromNative(DUMMYBF(fx))))
#define SETBLINK(fx, val)                        \
  do {                                           \
    ((FX *)(fx))->blink = (val);                 \
    if (FASTP(fx)) {                             \
      ((FX *)(fx))->clink = ((FX *)(fx))->alink; \
      SLOWP(fx) = 1;                             \
    }                                            \
  } while (0)

#define GETCLINK(fx) \
  (SLOWP(fx) ? (((FX *)(fx))->clink - FRAMESIZE) : (((FX *)(fx))->alink - FRAMESIZE))
#define SETCLINK(fx, val)                                \
  do {                                                   \
    ((FX *)(fx))->clink = (val) + FRAMESIZE;             \
    if (FASTP((fx))) {                                   \
      ((FX *)(fx))->blink = LAddrFromNative(DUMMYBF(fx)); \
      SLOWP(fx) = 1;                                     \
    }                                                    \
  } while (0)

#define SETACLINK(fx, val)                                                \
  do {                                                                    \
    if (FASTP(fx)) { ((FX *)(fx))->blink = LAddrFromNative(DUMMYBF(fx)); } \
    ((FX *)(fx))->clink = (val) + FRAMESIZE;                              \
    ((FX *)(fx))->alink = ((FX *)(fx))->clink + 1;                        \
  } while (0)

#ifdef BIGVM
#define SWAP_FNHEAD
#else
#define SWAP_FNHEAD(x) swapx(x)
#endif /* BIGVM */

#define GETNAMETABLE(fx)                                                                          \
  ((struct fnhead *)NativeAligned4FromLAddr(                                                      \
      SWAP_FNHEAD(                                                                                \
        ((((FX2 *)(fx))->validnametable) ? ((FX2 *)(fx))->nametable : ((FX2 *)(fx))->fnheader)) & \
      POINTERMASK))

#define MAKEFREEBLOCK(ptr68k, size)                                   \
  do {                                                                \
    if ((size) <= 0) error("creating 0 length free stack block");      \
    if ((size) & 1) error("creating odd length free stack block");	\
    *((LispPTR *)(void *)(ptr68k)) = (STK_FSB_WORD << 16) | ((DLword)(size)); \
  } while (0)

#define SETUPGUARDBLOCK(ptr68k, size)                                     \
  do {                                                                    \
    if ((size) <= 0) error("creating 0 length stack guard block");        \
    if ((size) & 1) error("creating odd sized stack guard block");        \
    (*((LispPTR *)(void *)(ptr68k)) = (STK_GUARD_WORD << 16) | ((DLword)(size))); \
  } while (0)

/************************************************************************/
/*									*/
/*			   Stack-checking macros			*/
/*									*/
/*	Enabled when STACKCHECK compile flag is set.			*/
/*									*/
/************************************************************************/

#ifdef STACKCHECK

#include <stdio.h>
#include "testtooldefs.h"

#define S_CHECK(condition, msg)                          \
  do {                                                   \
    if (!(condition)) {                                  \
      printf("\n\nStack check failed:  %s.\n\n", (msg)); \
      error("S_Check..");                                \
    }                                                    \
  } while (0)

#define S_WARN(condition, msg, scanptr)                                                       \
  do {                                                                                           \
    if (!(condition)) { printf("\n\nStack check failed at %p:  %s.\n\n", (scanptr), (msg)); } \
  } while (0)

#define CHECK_BF(bf68k) check_BF(bf68k)

#define CHECK_FX(fx68k) check_FX(fx68k)

#define PreMoveFrameCheck(fx68k)                                                \
  do {                                                                          \
    LispPTR *tos_on_stack;                                                      \
    if (check_stack_rooms(fx68k) > 1000) {                                      \
      warn("moveframe:there is more than 100 words SPACE for FX");              \
      printf("# When calling ");                                                \
      tos_on_stack = (LispPTR *)NativeAligned4FromStackOffset((fx68k)->nextblock - 2); \
      print_atomname(*tos_on_stack);                                            \
      printf("\n");                                                             \
      stack_check(0);                                                           \
    }                                                                           \
    } while (0)

#else /* STACKCHECK */

#define S_CHECK(condition, msg) \
  do {} while (0)
#define S_WARN(condition, msg, scanptr) \
  do {} while (0)
#define PreMoveFrameCheck(fx68k) \
  do {} while (0)
#define CHECK_BF(bf68k) \
  do {} while (0)
#define CHECK_FX(fx68k) \
  do {} while (0)

#endif /* STACKCHECK */

#define STK_MIN(fnobj) (((fnobj)->stkmin /* NOT NEEDED in stkmin +STK_SAFE */) << 1)

#define STK_END_COMPUTE(stk_end, fnobj) ((UNSIGNED)(stk_end)-STK_MIN(fnobj))

#define CLR_IRQ	\
  Irq_Stk_Check = STK_END_COMPUTE((Irq_Stk_End = (UNSIGNED) EndSTKP), FuncObj)
#endif
