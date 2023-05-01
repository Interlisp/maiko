#ifndef LSPTYPES_H
#define LSPTYPES_H 1
/*  @(#) lsptypes.h Version 1.4 (12/29/94). copyright Venue   */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/**********************************************************************/
/*
		File Name :	lsptypes.h

		Define Constants for Datatype

*/
/**********************************************************************/
#include "version.h" /* for BIGVM, BIGATOMS */
#include <stdint.h> /* for int32_t */
#include "lispemul.h" /* for LispPTR, DLword */

/************************************************************************/
/*									*/
/*		T Y P E   T A B L E    M A S K    B I T S		*/
/*									*/
/*  These are used for quick type checks, and are ORed into the type	*/
/*  number in the DTD.							*/
/*									*/
/*  [TT_LISPREF renamed to TT_SYMBOLP 2/7/91 by JDS for 3-byte atoms]	*/
/*									*/
/************************************************************************/

#define TT_ATOM		0x0800	/* ATOM in CL sense			 */
#define TT_NUMBERP	0x1000	/* It's a number. 			 */
#define TT_FIXP		0x2000	/* It's an integer.			 */
#define TT_SYMBOLP	0x4000	/* It's a symbol (LITATOM or NEWATOM.	 */
#define TT_NOREF	0x8000	/* Don't Refcount these objects.	 */




/************************************************************************/
/*									*/
/*			T Y P E   N U M B E R S				*/
/*									*/
/*  These are the type numbers "known to the microcode".  This must     */
/*  match the list \BUILT-IN-SYSTEM-TYPES, in LLDATATYPE.  It's the     */
/*  list of types that MUST be known to microcode or emulator, and      */
/*  which can't rely on the luck of what's loaded when for assignment.  */
/*									*/
/*  Change History:	2/27/89 Sybalsky  Added BIGNUM-PATHNAME, to     */
/*					  support SXHASH opcode. 	*/
/*			3/1/89	Sybalsky  Added type #s for hunked	*/
/*					  storage, for use in GC, in	*/
/*					  place of absolute #s (!!)	*/
/*			7/25/90 Osamu     Added type# for NEWATOM.						*/
/************************************************************************/

#define TYPE_ARRAYBLOCK		0	/* Pseudo type # for array blocks */
#define TYPE_SMALLP		1
#define TYPE_FIXP		2
#define TYPE_FLOATP		3
#define TYPE_LITATOM		4
#define TYPE_LISTP		5
#define TYPE_ARRAYP		6
#define TYPE_STRINGP		7	/* old IL strings (obs) */
#define TYPE_STACKP		8
#define TYPE_CHARACTERP		9
#define TYPE_VMEMPAGEP		10
#define TYPE_STREAM		11

#define TYPE_BITMAP		12
#define TYPE_COMPILED_CLOSURE	13
#define TYPE_ONED_ARRAY		14	/* array, string */
#define TYPE_TWOD_ARRAY		15
#define TYPE_GENERAL_ARRAY	16

#define TYPE_BIGNUM		17	/* Bignums */
#define TYPE_RATIO		18	/* Commonlisp rationals */
#define TYPE_COMPLEX		19	/* Commonlisp complex's */
#define TYPE_PATHNAME		20	/* CL path-name structure */

#ifdef BIGATOMS
#define TYPE_NEWATOM		21	/* New Atom */

/* type number 22~30 reserved for future use */

#define TYPE_PTRHUNK1		31	/********************************/
#define TYPE_PTRHUNK2		32	/*  				*/
#define TYPE_PTRHUNK3		33	/*  Type numbers for "hunked"	*/
#define TYPE_PTRHUNK4		34	/*  Storage region, used for	*/
#define TYPE_PTRHUNK5		35	/*  small arrays.  This range	*/
#define TYPE_PTRHUNK6		36	/*  is for arrays of pointers.	*/
#define TYPE_PTRHUNK7		37	/*				*/
#define TYPE_PTRHUNK8		38	/********************************/
#define TYPE_PTRHUNK9		39
#define TYPE_PTRHUNK10		40
#define TYPE_PTRHUNK11		41
#define TYPE_PTRHUNK12		42
#define TYPE_PTRHUNK13		43

#define TYPE_UNBOXEDHUNK1	44	/********************************/
#define TYPE_UNBOXEDHUNK2	45	/*  				*/
#define TYPE_UNBOXEDHUNK3	46	/*  This range is for arrays	*/
#define TYPE_UNBOXEDHUNK4	47	/*  of unboxed items -- the	*/
#define TYPE_UNBOXEDHUNK5	48	/*  contents are not GC'd	*/
#define TYPE_UNBOXEDHUNK6	49	/*				*/
#define TYPE_UNBOXEDHUNK7	50	/********************************/
#define TYPE_UNBOXEDHUNK8	51
#define TYPE_UNBOXEDHUNK9	52
#define TYPE_UNBOXEDHUNK10	53
#define TYPE_UNBOXEDHUNK11	54
#define TYPE_UNBOXEDHUNK12	55
#define TYPE_UNBOXEDHUNK13	56
#define TYPE_UNBOXEDHUNK14	57
#define TYPE_UNBOXEDHUNK15	58
#define TYPE_UNBOXEDHUNK16	59
#define TYPE_UNBOXEDHUNK17	60
#define TYPE_UNBOXEDHUNK18	61
#define TYPE_UNBOXEDHUNK19	62
#define TYPE_UNBOXEDHUNK20	63

#define TYPE_CODEHUNK1		64	/********************************/
#define TYPE_CODEHUNK2		65	/*  				*/
#define TYPE_CODEHUNK3		66	/*  This range is for small	*/
#define TYPE_CODEHUNK4		67	/*  CODE blocks -- the code 	*/
#define TYPE_CODEHUNK5		68	/*  for small functions.  It	*/
#define TYPE_CODEHUNK6		69	/*  gets reclaimed using the	*/
#define TYPE_CODEHUNK7		70	/*  function reclaimcodeblock.	*/
#define TYPE_CODEHUNK8		71	/*				*/
#define TYPE_CODEHUNK9		72	/********************************/
#define TYPE_CODEHUNK10		73


#define INIT_TYPENUM		74

#else
#define TYPE_PTRHUNK1		21	/********************************/
#define TYPE_PTRHUNK2		22	/*  				*/
#define TYPE_PTRHUNK3		23	/*  Type numbers for "hunked"	*/
#define TYPE_PTRHUNK4		24	/*  Storage region, used for	*/
#define TYPE_PTRHUNK5		25	/*  small arrays.  This range	*/
#define TYPE_PTRHUNK6		26	/*  is for arrays of pointers.	*/
#define TYPE_PTRHUNK7		27	/*				*/
#define TYPE_PTRHUNK8		28	/********************************/
#define TYPE_PTRHUNK9		29
#define TYPE_PTRHUNK10		30
#define TYPE_PTRHUNK11		31
#define TYPE_PTRHUNK12		32
#define TYPE_PTRHUNK13		33

#define TYPE_UNBOXEDHUNK1	34	/********************************/
#define TYPE_UNBOXEDHUNK2	35	/*  				*/
#define TYPE_UNBOXEDHUNK3	36	/*  This range is for arrays	*/
#define TYPE_UNBOXEDHUNK4	37	/*  of unboxed items -- the	*/
#define TYPE_UNBOXEDHUNK5	38	/*  contents are not GC'd	*/
#define TYPE_UNBOXEDHUNK6	39	/*				*/
#define TYPE_UNBOXEDHUNK7	40	/********************************/
#define TYPE_UNBOXEDHUNK8	41
#define TYPE_UNBOXEDHUNK9	42
#define TYPE_UNBOXEDHUNK10	43
#define TYPE_UNBOXEDHUNK11	44
#define TYPE_UNBOXEDHUNK12	45
#define TYPE_UNBOXEDHUNK13	46
#define TYPE_UNBOXEDHUNK14	47
#define TYPE_UNBOXEDHUNK15	48
#define TYPE_UNBOXEDHUNK16	49
#define TYPE_UNBOXEDHUNK17	50
#define TYPE_UNBOXEDHUNK18	51
#define TYPE_UNBOXEDHUNK19	52
#define TYPE_UNBOXEDHUNK20	53

#define TYPE_CODEHUNK1		54	/********************************/
#define TYPE_CODEHUNK2		55	/*  				*/
#define TYPE_CODEHUNK3		56	/*  This range is for small	*/
#define TYPE_CODEHUNK4		57	/*  CODE blocks -- the code 	*/
#define TYPE_CODEHUNK5		58	/*  for small functions.  It	*/
#define TYPE_CODEHUNK6		59	/*  gets reclaimed using the	*/
#define TYPE_CODEHUNK7		60	/*  function reclaimcodeblock.	*/
#define TYPE_CODEHUNK8		61	/*				*/
#define TYPE_CODEHUNK9		62	/********************************/
#define TYPE_CODEHUNK10		63


#define INIT_TYPENUM		64

#endif /* BIGATOMS */



#ifndef BYTESWAP
	/* Normal byte-order versions of declarations */
#ifdef BIGVM
struct dtd {
	unsigned unuse	:2 ;
	unsigned dtd_obsolate :1 ;
	unsigned dtd_finalizable :1 ;
	unsigned dtd_name: 28 ;	/* type name */
	DLword dtd_cnt0 ;
	DLword dtd_size ;
	LispPTR dtd_free ;		/* really a FULLXPOINTER */
	unsigned dtd_lockedp : 1 ;
	unsigned dtd_hunkp : 1 ;
	unsigned dtd_gctype :2 ;
	unsigned dtd_descrs: 28;
	LispPTR dtd_typespecs;
	LispPTR dtd_ptrs ;
	int  dtd_oldcnt;
	int dtd_nextpage ;
	DLword dtd_typeentry ;
	DLword dtd_supertype ;
};

typedef  struct stringp {

	unsigned origin			: 1 ;
	unsigned substringed		: 1 ;
	unsigned readonly		: 1 ;
	unsigned nil			: 1 ;

	unsigned base			: 28 ;
	unsigned type			: 4 ;
	unsigned   length: 28 ;
	LispPTR   offset ;
 } STRINGP ;

typedef struct oned_array {
		unsigned int nil1 : 4 ;
		unsigned int base : 28  ;
		unsigned int readonlyp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int bitp :1 ;
		unsigned int stringp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int displacedp : 1;
		unsigned int fillpointerp :1;
		unsigned int extendablep : 1;
		unsigned int typenumber : 8 ;
		DLword offset;
		int32_t fillpointer ;
		int32_t totalsize ; } OneDArray;

typedef struct oned_array NEWSTRINGP;

typedef struct general_array {
		unsigned int nil1 : 4 ;
		unsigned int base : 28  ;
		unsigned int readonlyp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int bitp :1 ;
		unsigned int stringp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int displacedp : 1;
		unsigned int fillpointerp :1;
		unsigned int extendablep : 1;
		unsigned int typenumber : 8 ;
		unsigned int nil2:16;
		int32_t Dim0;
		int32_t totalsize;
		int32_t Dim1 ;
		int32_t Dim2 ; } LispArray;

typedef struct compiled_closure {
		unsigned int nil1	: 4 ;
		unsigned int def_ptr	: 28; /* function */
		unsigned int nil2	: 4 ;
		unsigned int env_ptr	: 28; /* environment */
  } CClosure ;
#else
/* Structure for DTD */
struct dtd {
	DLword dtd_namelo ;
	DLword dtd_size ;
	LispPTR dtd_free ;		/* really a FULLXPOINTER */
	unsigned unuse	:2 ;
	unsigned dtd_obsolate :1 ;
	unsigned dtd_finalizable :1 ;
	unsigned dtd_lockedp : 1 ;
	unsigned dtd_hunkp : 1 ;
	unsigned dtd_gctype :2 ;
	unsigned dtd_descrs : 24;
	unsigned dtd_namehi : 8;
	unsigned dtd_typespecs : 24 ;
	LispPTR dtd_ptrs ;
	int  dtd_oldcnt;
	DLword dtd_cnt0 ;
	DLword dtd_nextpage ;
	DLword dtd_typeentry ;
	DLword dtd_supertype ;
};

typedef  struct stringp {
	unsigned origin			: 1 ;
	unsigned substringed		: 1 ;
	unsigned readonly		: 1 ;
	unsigned nil			: 1 ;
	unsigned type			: 4 ;

	unsigned base			: 24 ;
	DLword   length ;
	DLword   offset ;
 } STRINGP ;

typedef struct oned_array {
		unsigned int nil1 : 8 ;
		unsigned int base : 24  ;
		unsigned int readonlyp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int bitp :1 ;
		unsigned int stringp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int displacedp : 1;
		unsigned int fillpointerp :1;
		unsigned int extendablep : 1;
		unsigned int typenumber : 8 ;
		DLword offset;
		DLword fillpointer ;
		DLword totalsize ; } OneDArray;

typedef struct oned_array NEWSTRINGP;

typedef struct general_array {
		unsigned int nil1 : 8 ;
		unsigned int base : 24  ;
		unsigned int readonlyp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int bitp :1 ;
		unsigned int stringp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int displacedp : 1;
		unsigned int fillpointerp :1;
		unsigned int extendablep : 1;
		unsigned int typenumber : 8 ;
		DLword Dim0;
		DLword totalsize;
		DLword Dim1 ;
		DLword Dim2 ; } LispArray;

typedef struct compiled_closure {
		unsigned int nil1	: 8 ;
		unsigned int def_ptr	: 24; /* function */
		unsigned int nil2	: 8 ;
		unsigned int env_ptr	: 24; /* environment */
  } CClosure ;
#endif /* BIGVM */

/* Structure for initialdtdcontents */
 struct system_dtd_contents {
	char   *dtd_name ;  /* type name string >> changed 4-feb-87 */
	DLword  name_len ;  /* type name length in BYTE 27-Mar-87 take */
	DLword  dtd_size ;
};

typedef struct{
	LispPTR	bmbase;
	DLword	bmrasterwidth;
	DLword	bmheight;
	DLword	bmwidth;
	DLword	bmbitperpixel;
}BITMAP;


	/****************************************************************/
	/*		Byte- and Word-array access macros 		*/
	/* 								*/
	/* Use these macros instead of dereferencing a char pointer	*/
	/* so we can encapsulate byte-ordering effects on different	*/
	/* hardware!							*/
	/* 								*/
	/****************************************************************/

#define GETBYTE(base) (* (base))
/* GETBASEWORD only works if base points to a 32-bit boundary */
#define GETBASEWORD(base, offset) (* ((base)+(offset)))
  /* GETWORDBASEWORD works right with base on a 16-bit boundary. */
#define GETWORDBASEWORD(base, offset) (* (((DLword *)(base))+(offset)))
#define GETWORD(base) (* (base))
#define WORDPTR(base) (base)
#define BYTEPTR(base) (base)

#else
	/********************************************************/
	/*							*/
	/*  Byte-swapped structure definitions, for 80386 &c	*/
	/*							*/
	/********************************************************/

#ifdef BIGVM
struct dtd {
	unsigned dtd_name: 28 ;	/* type name */
	unsigned dtd_finalizable :1 ;
	unsigned dtd_obsolate :1 ;
	unsigned unuse	:2 ;
	DLword dtd_size ;
	DLword dtd_cnt0 ;
	LispPTR dtd_free ;		/* really a FULLXPOINTER */
	unsigned dtd_descrs: 28;
	unsigned dtd_gctype :2 ;
	unsigned dtd_hunkp : 1 ;
	unsigned dtd_lockedp : 1 ;
	LispPTR dtd_typespecs;
	LispPTR dtd_ptrs ;
	int  dtd_oldcnt;
	LispPTR dtd_nextpage ;
	DLword dtd_supertype ;
	DLword dtd_typeentry ;
};

typedef  struct stringp {

	unsigned base			: 28 ;
	unsigned nil			: 1 ;
	unsigned readonly		: 1 ;
	unsigned substringed		: 1 ;
	unsigned origin			: 1 ;

	unsigned   length: 28 ;
	unsigned type			: 4 ;
	LispPTR   offset ;
 } STRINGP ;


typedef struct oned_array {
		unsigned int base : 28  ;
		unsigned int nil1 : 4 ;
		DLword offset;
		unsigned int typenumber : 8 ;
		unsigned int extendablep : 1;
		unsigned int fillpointerp :1;
		unsigned int displacedp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int stringp : 1;
		unsigned int bitp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int readonlyp :1 ;
		int32_t fillpointer ;
		int32_t totalsize ; } OneDArray;


typedef struct oned_array NEWSTRINGP;

typedef struct general_array {
		unsigned int base : 28  ;
		unsigned int nil1 : 4 ;
		unsigned int nil2:16;
		unsigned int typenumber : 8 ;
		unsigned int extendablep : 1;
		unsigned int fillpointerp :1;
		unsigned int displacedp : 1;
		unsigned int ajustablep : 1; /* as used arrayheader */
		unsigned int stringp : 1;
		unsigned int bitp :1 ;
		unsigned int indirectp : 1; /* as used arrayheader */
		unsigned int readonlyp :1 ;
		int32_t Dim0;
		int32_t totalsize;
		int32_t Dim1 ;
		int32_t Dim2 ; } LispArray;

typedef struct compiled_closure {
		unsigned int def_ptr	: 28; /* function */
		unsigned int nil1	: 4 ;
		unsigned int env_ptr	: 28; /* environment */
		unsigned int nil2	: 4 ;
  } CClosure ;
#else
/* Structure for DTD */
struct dtd {
	DLword dtd_size ;
	DLword dtd_namelo ;
	LispPTR dtd_free ;		/* really a FULLXPOINTER */
	unsigned dtd_descrs : 24;
	unsigned dtd_gctype :2 ;
	unsigned dtd_hunkp : 1 ;
	unsigned dtd_lockedp : 1 ;
	unsigned dtd_finalizable :1 ;
	unsigned dtd_obsolate :1 ;
	unsigned unuse	:2 ;
	unsigned dtd_typespecs : 24 ;
	unsigned dtd_namehi : 8;
	LispPTR dtd_ptrs ;
	int  dtd_oldcnt;
	DLword dtd_nextpage ;
	DLword dtd_cnt0 ;
	DLword dtd_supertype ;
	DLword dtd_typeentry ;
};

typedef  struct stringp
  {
	unsigned base			: 24 ;
	unsigned type			: 4 ;
	unsigned nil			: 1 ;
	unsigned readonly		: 1 ;
	unsigned substringed		: 1 ;
	unsigned origin			: 1 ;
	DLword   offset ;
	DLword   length ;
  } STRINGP ;

typedef struct oned_array
  {
	unsigned int base : 24  ;
	unsigned int nil1 : 8 ;
	DLword offset;
	unsigned int typenumber : 8 ;
	unsigned int extendablep : 1;
	unsigned int fillpointerp :1;
	unsigned int displacedp : 1;
	unsigned int ajustablep : 1; /* as used arrayheader */
	unsigned int stringp : 1;
	unsigned int bitp :1 ;
	unsigned int indirectp : 1; /* as used arrayheader */
	unsigned int readonlyp :1 ;
	DLword totalsize ;
	DLword fillpointer ;
  } OneDArray;

typedef struct oned_array NEWSTRINGP;

typedef struct general_array
  {
	unsigned int base : 24  ;
	unsigned int nil1 : 8 ;
	DLword Dim0;
	unsigned int typenumber : 8 ;
	unsigned int extendablep : 1;
	unsigned int fillpointerp :1;
	unsigned int displacedp : 1;
	unsigned int ajustablep : 1; /* as used arrayheader */
	unsigned int stringp : 1;
	unsigned int bitp :1 ;
	unsigned int indirectp : 1; /* as used arrayheader */
	unsigned int readonlyp :1 ;
	DLword Dim2 ;
	DLword Dim1 ;
  } LispArray;

typedef struct compiled_closure
  {
    unsigned int def_ptr : 24; /* function */
    unsigned int nil1 : 8 ;
    unsigned int env_ptr: 24; /* environment */
    unsigned int nil2 : 8 ;
  } CClosure ;
#endif /* BIGVM */
/* Structure for initialdtdcontents */

 struct system_dtd_contents
  {
    char   *dtd_name ;  /* type name string >> changed 4-feb-87 */
    DLword  dtd_size ;
    DLword  name_len ;  /* type name length in BYTE 27-Mar-87 take */
  };

typedef struct
  {
    LispPTR	bmbase;
    DLword	bmheight;
    DLword	bmrasterwidth;
    DLword	bmbitperpixel;
    DLword	bmwidth;
  }BITMAP;


	/****************************************************************/
	/*		Byte- and Word-array access macros 		*/
	/* 								*/
	/* Use these macros instead of dereferencing a char pointer	*/
	/* so we can encapsulate byte-ordering effects on different	*/
	/* hardware!							*/
	/* 								*/
	/****************************************************************/

#define GETBYTE(base) (* (unsigned char *) (3^(UNSIGNED)(base)))
  /* GETBASEWORD only works if base points to a 32-bit boundary */
#define GETBASEWORD(base, offset) GETWORDBASEWORD((base),(offset))
#define GETWORDBASEWORD(base, offset) (* (DLword *) (2^(UNSIGNED)((base)+(offset))))
#define GETWORD(base) (* (DLword *) (2^(UNSIGNED)(base)))
#define WORDPTR(base) ((DLword *)(2^(UNSIGNED)(base)))
#define BYTEPTR(base) ((char *) (3^(UNSIGNED)(base)))

#endif /* BYTESWAP */



#define ST_POS16	1
#define ST_BYTE		0
#define THIN_CHAR_TYPENUMBER	67
#define FAT_CHAR_TYPENUMBER	68


/************************************************************************/
/*									*/
/*		T Y P E   E N T R Y   A C C E S S O R S			*/
/*									*/
/*	GetDTD		Gets the (C-native) address for the DTD		*/
/*			(Data Type Descriptor) for the type whose	*/
/*			number you hand it (LESS the mask bits!).	*/
/*									*/
/*	GetTypeEntry	Gets the entire 16-bit "type number" field	*/
/*			from the DTD, including type-mask bits.		*/
/*			Use this one when you want the mask bits for	*/
/*			a quick category check.				*/
/*									*/
/*	GetTypeNumber	Gets only the Type Number (without mask bits)	*/
/*			from the DTD for the object you give it.	*/
/*			Use this one for doing type checks and 		*/
/*			dispatching.					*/
/*									*/
/*	Listp		If 'address' is the address of a cons cell,	*/
/*			returns TRUE.  Equivalent to the LISTP pred.	*/
/*									*/
/************************************************************************/

/* Get DTD pointer(68k) from typenum */
#ifdef BIGVM
#define GetDTD(typnum)	(void *)(DTDspace + ((typnum)<<4)+((typnum)<<1))
#else
#define GetDTD(typnum)	(void *)(DTDspace + ((typnum)<<4))
#endif /* BIGVM */

/* Get all type entry */
#define GetTypeEntry(address)      ( GETWORD(MDStypetbl+((address)>>9)) )

/* the type number is in the low 11 bits */
#define GetTypeNumber(address)     (GetTypeEntry(address) & 0x7ff)

/* This MACRO is similar to LISTP */
#define Listp(address)	(GetTypeNumber(address) == TYPE_LISTP)

#define Numberp(address) (GetTypeEntry(address) & TT_NUMBERP)

	/******************************************/
	/*        Lisp's PATHNAME datatype        */
	/*                                        */
	/*   This MUST change whenever the Lisp   */
	/*   definition of the PATHNAME structure */
	/*   changes.                             */
	/*                                        */
	/******************************************/
typedef
  struct
    {
      LispPTR host;
      LispPTR device;
      LispPTR directory;
      LispPTR name;
      LispPTR type;
      LispPTR version;
    } PATHNAME;
  
	/******************************************/
	/*         Lisp's COMPLEX datatype        */
	/*                                        */
	/*   This MUST change whenever the Lisp   */
	/*   definition of the COMPLEX structure  */
	/*   changes.                             */
	/*                                        */
	/******************************************/
typedef
  struct
    {
      LispPTR real;
      LispPTR imaginary;
    } COMPLEX;


	/******************************************/
	/*          Lisp's RATIO datatype         */
	/*                                        */
	/*   This MUST change whenever the Lisp   */
	/*   definition of the RATIO structure    */
	/*   changes.                             */
	/*                                        */
	/******************************************/
typedef
  struct
    {
      LispPTR numerator;
      LispPTR denominator;
    } RATIO;


	/******************************************/
	/*         Lisp's BIGNUM datatype         */
	/*                                        */
	/*   This MUST change whenever the Lisp   */
	/*   definition of the BIGNUM structure   */
	/*   changes.                             */
	/*                                        */
	/******************************************/
typedef
  struct
    {
      LispPTR contents;	/* a list of 12-bit segments of value, */
			/* low-order 12 bits first.            */
    } BIGNUM;

#endif /* LSPTYPES_H */
