#ifndef CELL_H
#define CELL_H 1
/* $Id: cell.h,v 1.2 1999/01/03 02:05:55 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/**********************************************************************/
/*
                File Name :	cell.h

                Cell Manipulate Macros

                                Date :		December 16, 1986
                                Edited by :	Takeshi Shimizu

*/
/**********************************************************************/
#include "adr68k.h"   /* for NativeAligned4FromLPage, NativeAligned4FromLAddr */
#include "lispemul.h" /* for LispPTR, DLword */

/*  CONS CELL (LISTP) definitions moved to lispemulater.h */

/* This Macro may produce a pointer that points CAR cell */
/* On 68010,68000 This Macro does not effect */

#ifdef NEWCDRCODING
#define CARFIELD(x) ((int)(x) & 0x0fffffff)

/* CDR-Codes defs */
#define CDR_ONPAGE 8
#define CDR_NIL 8
#define CDR_INDIRECT 0
#define CDR_MAXINDIRECT 7
#define CONSPAGE_LAST 0xffffffff /* dummy "last cons page" */

#else

#define CARFIELD(x) ((int)x & 0x00ffffff)

/* CDR-Codes defs */
#define CDR_ONPAGE 128
#define CDR_NIL 128
#define CDR_INDIRECT 0
#define CDR_MAXINDIRECT 127
#define CONSPAGE_LAST 0xffff

#endif /* NEWCDRCODING */

/************************************************************************/
/*									*/
/*	CONSPAGE describes the free-space management fields at the	*/
/*	beginning of a cons page -- # of free cells on the page, etc.	*/
/*									*/
/************************************************************************/

#ifndef BYTESWAP
#ifdef NEWCDRCODING
struct conspage {
  /* used to be ifdef NEWNEW */
  LispPTR cell6;          /* cell 6 in shifted world */
  LispPTR cell4;          /* cell 4 in shifted world */
                          /* used to endif NEWNEW here */
  unsigned count : 8;     /* free cells on this page */
  unsigned next_cell : 8; /* next free cell in chain */
  unsigned nil : 16;
  LispPTR next_page; /* next cons page, or 0 if none */
};
#else
struct conspage {
  unsigned count : 8;
  unsigned next_cell : 8;
  DLword next_page;
};
#endif /* NEWCDRCODING */
#else
/* byte-swapped version */
#ifdef NEWCDRCODING
struct conspage {
  LispPTR cell6; /* cell 6 in shifted world */
  LispPTR cell4; /* cell 4 in shifted world */
  unsigned nil : 16;
  unsigned next_cell : 8;
  unsigned count : 8;
  LispPTR next_page;
};
#else
struct conspage {
  DLword next_page;
  unsigned next_cell : 8;
  unsigned count : 8;
};
#endif /* NEWCDRCODING */
#endif /* BYTESWAP */

/* Following MACROs for Conspage */

/* lisp_ptr is LISP pointer, returns 68k ptr points struct conspage obj */
#define Get_ConsPageBase(lisp_ptr) (struct conspage *)NativeAligned4FromLPage(POINTER_PAGEBASE(lisp_ptr))

#define GetNewCell_68k(conspage68k) \
  (ConsCell *)(((DLword *)(conspage68k)) + (unsigned)((conspage68k)->next_cell))

/* page : LISP page */
#define GetCONSCount(page) (((struct conspage *)NativeAligned4FromLPage(page))->count)

#ifndef BYTESWAP
/* For chaining together free cons cells on a page */
typedef struct freec {
  unsigned next_free : 8; /* next free cell on this page */
  unsigned nil : 24;
} freecons;

#else

typedef struct freec {
  unsigned nil : 24;
  unsigned next_free : 8;
} freecons;

#endif /* BYTESWAP */

#define FREECONS(page, offset) ((freecons *)((DLword *)(page) + (offset)))

/************************************************************************/
/*									*/
/*				S Y M B O L   D E F S			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifndef BYTESWAP
/************************************************/
/*  Definitions for normal byte-order machines	*/
/************************************************/

#ifdef BIGVM
/* Definition of a new-atom, where all the cells are in one object */
typedef struct new_atom {
  unsigned dfccodep : 1; /* DEFCELL FLAGS */
  unsigned dffastp : 1;
  unsigned dfargtype : 2;
  unsigned pncell : 28;  /* pointer to the pname itself */
  unsigned nil2 : 4;     /* top 4 bits of value cell */
  unsigned valcell : 28; /* pointer to the top-level value */
  unsigned nil3 : 4;     /* top 4 bits of definition cell */
  unsigned defcell : 28; /* pointer to function definition */
  unsigned nil4 : 4;     /* */
  unsigned plcell : 28;  /* pointer to property list */
  unsigned pnpkg : 8;    /* package */
  unsigned dfnil1 : 4;   /* DEFCELL FLAGS */
  unsigned dfpseudo : 1;
  unsigned dfswapped : 1; /* T for native-order fn opcodes */
  unsigned dfnil : 2;
  unsigned plunused : 1; /* PROPLIST FLAGS */
  unsigned plgensymp : 1;
  unsigned plfatpnamep : 1;
  unsigned plnil : 5;
  unsigned nil5 : 8; /* Fill out last byte of final cell */
} NewAtom;

/* DEFs for DEFINITIONCELL */
typedef struct definition_cell {
  unsigned ccodep : 1;
  unsigned fastp : 1;
  unsigned argtype : 2;
  unsigned defpointer : 28;
  LispPTR nil_PL;      /* skip the proplist cell */
  unsigned nilpkg : 8; /* skip pkg byte */
  unsigned nil2 : 4;
  unsigned pseudocodep : 1;
  unsigned byteswapped : 1; /* T for native-order fn opcodes */
  unsigned nil_last : 18;

} DefCell;

typedef struct pname_cell {
  unsigned nil : 4;
  unsigned pnamebase : 28;
  LispPTR nil_val;
  LispPTR nil_def;
  LispPTR nil_plist;
  unsigned pkg_index : 8;
  unsigned nil2 : 24;
} PNCell;

typedef struct proplist_cell {
  unsigned nil : 4;
  unsigned propbase : 28;
  unsigned nilpkg : 8;
  unsigned nildef : 8;
  unsigned unused : 1;
  unsigned gensymp : 1;
  unsigned fatpnamep : 1;
  unsigned nil2 : 5;
  unsigned nil3 : 8;

} PLCell;

struct xpointer {
  unsigned flags : 4;
  unsigned addr : 28;
};

#else /* not BIGVM */

/* DEFs for DEFINITIONCELL */
typedef struct definition_cell {
  unsigned ccodep : 1;
  unsigned fastp : 1;
  unsigned argtype : 2;
  unsigned pseudocodep : 1;
  unsigned byteswapped : 1; /* T for native-order fn opcodes */
  unsigned nil : 2;
  unsigned defpointer : 24;

} DefCell;

typedef struct pname_cell {
  unsigned pkg_index : 8;
  unsigned pnamebase : 24;
} PNCell;

typedef struct proplist_cell {
  unsigned unused : 1;
  unsigned gensymp : 1;
  unsigned fatpnamep : 1;
  unsigned nil : 5;
  unsigned propbase : 24;
} PLCell;

struct xpointer {
  unsigned flags : 8;
  unsigned addr : 24;
};

#endif /* BIGVM */

#else
/************************************************/
/*  Definitions for byte-swapped machines	*/
/************************************************/
#ifdef BIGVM
/* Definition of a new-atom, where all the cells are in one object */
typedef struct new_atom {
  unsigned pncell : 28; /* pointer to the pname itself */
  unsigned dfargtype : 2;
  unsigned dffastp : 1;
  unsigned dfccodep : 1; /* DEFCELL FLAGS */
  unsigned valcell : 28; /* pointer to the top-level value */
  unsigned nil2 : 4;     /* top 4 bits of value cell */
  unsigned defcell : 28; /* pointer to function definition */
  unsigned nil3 : 4;     /* top 4 bits of definition cell */
  unsigned plcell : 28;  /* pointer to property list */
  unsigned nil4 : 4;     /* */
  unsigned nil5 : 8;     /* Fill out last byte of final cell */
  unsigned plnil : 5;
  unsigned plfatpnamep : 1;
  unsigned plgensymp : 1;
  unsigned plunused : 1; /* PROPLIST FLAGS */
  unsigned dfnil : 2;
  unsigned dfswapped : 1; /* T for native-order fn opcodes */
  unsigned dfpseudo : 1;
  unsigned dfnil1 : 4; /* DEFCELL FLAGS */
  unsigned pnpkg : 8;  /* package */
} NewAtom;

/* DEFs for DEFINITIONCELL */
typedef struct definition_cell {
  unsigned defpointer : 28;
  unsigned argtype : 2;
  unsigned fastp : 1;
  unsigned ccodep : 1;
  LispPTR nil_PL; /* skip the proplist cell */
  unsigned nil_last : 18;
  unsigned byteswapped : 1; /* T for native-order fn opcodes */
  unsigned pseudocodep : 1;
  unsigned nil2 : 4;
  unsigned nilpkg : 8; /* skip pkg byte */

} DefCell;

typedef struct pname_cell {
  unsigned pnamebase : 28;
  unsigned nil : 4;
  LispPTR nil_val;
  LispPTR nil_def;
  LispPTR nil_plist;
  unsigned nil2 : 24;
  unsigned pkg_index : 8;
} PNCell;

typedef struct proplist_cell {
  unsigned propbase : 28;
  unsigned nil : 4;
  unsigned nil3 : 8;
  unsigned nil2 : 5;
  unsigned fatpnamep : 1;
  unsigned gensymp : 1;
  unsigned unused : 1;
  unsigned nildef : 8;
  unsigned nilpkg : 8;

} PLCell;

struct xpointer {
  unsigned addr : 28;
  unsigned flags : 4;
};
#else  /* BIGVM */
/* DEFs for DEFINITIONCELL */
typedef struct definition_cell {
  unsigned defpointer : 24;
  unsigned nil : 2;
  unsigned byteswapped : 1; /* T if opcodes are native-order */
  unsigned pseudocodep : 1;
  unsigned argtype : 2;
  unsigned fastp : 1;
  unsigned ccodep : 1;

} DefCell;

typedef struct pname_cell {
  unsigned pnamebase : 24;
  unsigned pkg_index : 8;
} PNCell;

typedef struct proplist_cell {
  unsigned propbase : 24;
  unsigned nil : 5;
  unsigned fatpnamep : 1;
  unsigned gensymp : 1;
  unsigned unused : 1;
} PLCell;

struct xpointer {
  unsigned addr : 24;
  unsigned flags : 8;
};
#endif /* BIGVM */

#endif /* BYTESWAP */

struct cadr_cell {
  LispPTR car_cell; /* Lisp address (word addressing) */
  LispPTR cdr_cell; /* Lisp address (word addressing) */
};

/************************************************************************/
/*									*/
/*	Access to the parts of a SYMBOL: Pname, Definition, Value, 	*/
/*	and property list.						*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifndef BIGATOMS
#define GetDEFCELL68k(index) ((LispPTR *)Defspace + (index))
#define GetVALCELL68k(index) ((LispPTR *)Valspace + (index))
#define GetPnameCell(index) ((LispPTR *)Pnamespace + (index))
#define GetPropCell(index) ((LispPTR *)Plistspace + (index))

/* Good only for old-style LITATOMS */
#define GetDEFCELLlitatom(index) ((LispPTR *)Defspace + (index))
#define GetVALCELLlitatom(index) ((LispPTR *)Valspace + (index))
#define GetPnameCelllitatom(index) ((LispPTR *)Pnamespace + (index))
#define GetPropCelllitatom(index) ((LispPTR *)Plistspace + (index))

#else
/* Good for old LITATOMS and new NEW-ATOMs */
#define GetDEFCELL68k(index)                                                                 \
  ((((index) & SEGMASK) != 0) ? GetDEFCELLnew(index) : GetDEFCELLlitatom(index))

#define GetVALCELL68k(index)                                                                  \
  ((((index) & SEGMASK) != 0) ? GetVALCELLnew(index) : GetVALCELLlitatom(index))

#define GetPnameCell(index)                                                                   \
  ((((index) & SEGMASK) != 0) ? GetPnameCellnew(index) : GetPnameCelllitatom(index))

#define GetPropCell(index)                                                                    \
  ((((index) & SEGMASK) != 0) ? GetPropCellnew(index) : GetPropCelllitatom(index))

/* Good only for old-style LITATOMS */
#ifdef BIGVM
#define GetDEFCELLlitatom(index) ((LispPTR *)AtomSpace + (5 * (index)) + NEWATOM_DEFN_PTROFF)
#define GetVALCELLlitatom(index) ((LispPTR *)AtomSpace + (5 * (index)) + NEWATOM_VALUE_PTROFF)
#define GetPnameCelllitatom(index) ((LispPTR *)AtomSpace + (5 * (index)) + NEWATOM_PNAME_PTROFF)
#define GetPropCelllitatom(index) ((LispPTR *)AtomSpace + (5 * (index)) + NEWATOM_PLIST_PTROFF)
#else /* BIGVM not set, so use old name-space format */
#define GetDEFCELLlitatom(index) ((LispPTR *)Defspace + (index))
#define GetVALCELLlitatom(index) ((LispPTR *)Valspace + (index))
#define GetPnameCelllitatom(index) ((LispPTR *)Pnamespace + (index))
#define GetPropCelllitatom(index) ((LispPTR *)Plistspace + (index))
#endif

/* Good only for new-style NEW-ATOMs */
/* Note: the _OFFSET values are in units of DLword so need to be adjusted before doing pointer
 *       arithmetic since we now have native pointers to cells not DLwords
 */
#define GetDEFCELLnew(index) (NativeAligned4FromLAddr(index) + (NEWATOM_DEFN_OFFSET / DLWORDSPER_CELL))
#define GetVALCELLnew(index) (NativeAligned4FromLAddr(index) + (NEWATOM_VALUE_OFFSET / DLWORDSPER_CELL))
#define GetPnameCellnew(index) (NativeAligned4FromLAddr(index) + (NEWATOM_PNAME_OFFSET / DLWORDSPER_CELL))
#define GetPropCellnew(index) (NativeAligned4FromLAddr(index) + (NEWATOM_PLIST_OFFSET / DLWORDSPER_CELL))

#endif /* BIGATOMS */

/* When cadr() function is called, type check should be done. */

#define S_N_CHECKANDCADR(sour, dest, tos)    \
  do {                                        \
    LispPTR parm = sour;            \
    if (GetTypeNumber(parm) != TYPE_LISTP) { \
      ERROR_EXIT(tos);                       \
    } else                                   \
      (dest) = cadr(parm);                   \
  } while (0)
#endif
