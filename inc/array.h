#ifndef ARRAY_H
#define ARRAY_H 1
/* $Id: array.h,v 1.2 1999/01/03 02:05:53 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "lispemul.h" /* for LispPTR, DLword */
#include "version.h" /* for BIGVM */

#ifndef BYTESWAP
	/********************************/
	/*  Normal byte-order version   */
	/********************************/
#ifdef BIGVM
typedef struct sequencedescriptor {
	unsigned	orig		:1;
	unsigned	nil		:1;
	unsigned	readonly	:1;
	unsigned 	nil2		:1;
	unsigned	base		:28;
	unsigned	typ		:4;
	unsigned	length		:28;
	int32_t		offst;
} Arrayp;

struct arrayheader {
        unsigned        nil             :4;
	unsigned        base            :28;
	unsigned        readonlyp       :1;
	unsigned        indirectp       :1;
	unsigned        bitp            :1;
	unsigned        stringp         :1;
	unsigned        adjustablep     :1;
	unsigned        displacedp      :1;
	unsigned        fillpointerp    :1;
	unsigned        extendablep     :1;
	unsigned        typenumber      :8;
	DLword          offset;
	int32_t         fillpointer;
	int32_t         totalsize;
      };
#else
typedef struct sequencedescriptor {
	unsigned	orig		:1;
	unsigned	nil		:1;
	unsigned	readonly	:1;
	unsigned 	nil2		:1;
	unsigned	typ		:4;
	unsigned	base		:24;
	DLword		length;
	DLword		offst;
} Arrayp;

struct arrayheader {
        unsigned        nil             :8;
	unsigned        base            :24;
	unsigned        readonlyp       :1;
	unsigned        indirectp       :1;
	unsigned        bitp            :1;
	unsigned        stringp         :1;
	unsigned        adjustablep     :1;
	unsigned        displacedp      :1;
	unsigned        fillpointerp    :1;
	unsigned        extendablep     :1;
	unsigned        typenumber      :8;
	DLword          offset;
	DLword          fillpointer;
	DLword          totalsize;
      };
#endif /* BIGVM */

struct arrayblock {
	unsigned	password	:13;
	unsigned	gctype		:2;
	unsigned	inuse		:1;
	DLword		arlen;
	LispPTR		fwd;
	LispPTR		bkwd;
};

struct abdum {
	DLword		abflags;
};


#else
	/********************************/
	/*  Byte-swapped version, for   */
	/*  e.g., 80386's		*/
	/********************************/
#ifdef BIGVM
typedef struct sequencedescriptor {
	unsigned	base		:28;
	unsigned 	nil2		:1;
	unsigned	readonly	:1;
	unsigned	nil		:1;
	unsigned	orig		:1;
	unsigned	length		:28;
	unsigned	typ		:4;
	int32_t		offst;
} Arrayp;

struct arrayheader {
	unsigned        base            :28;
        unsigned        nil             :4;
	DLword          offset;
	unsigned        typenumber      :8;
	unsigned        extendablep     :1;
	unsigned        fillpointerp    :1;
	unsigned        displacedp      :1;
	unsigned        adjustablep     :1;
	unsigned        stringp         :1;
	unsigned        bitp            :1;
	unsigned        indirectp       :1;
	unsigned        readonlyp       :1;
	int32_t         totalsize;
	int32_t         fillpointer;
      };
#else
typedef struct sequencedescriptor {
	unsigned	base		:24;
	unsigned	typ		:4;
	unsigned 	nil2		:1;
	unsigned	readonly	:1;
	unsigned	nil		:1;
	unsigned	orig		:1;
	DLword		offst;
	DLword		length;
} Arrayp;

struct arrayheader {
	unsigned        base            :24;
        unsigned        nil             :8;
	DLword          offset;
	unsigned        typenumber      :8;
	unsigned        extendablep     :1;
	unsigned        fillpointerp    :1;
	unsigned        displacedp      :1;
	unsigned        adjustablep     :1;
	unsigned        stringp         :1;
	unsigned        bitp            :1;
	unsigned        indirectp       :1;
	unsigned        readonlyp       :1;
	DLword          totalsize;
	DLword          fillpointer;
      };
#endif /* BIGVM */
struct arrayblock {
	DLword		arlen;
	unsigned	inuse		:1;
	unsigned	gctype		:2;
	unsigned	password	:13;
	LispPTR		fwd;
	LispPTR		bkwd;
};

struct abdum
  {
	DLword		abflags;
};


#endif /* BYTESWAP */




/****************************************************************************/
/*                                                                          */
/*        The following data are the constant values for array itself.      */
/*                                                                          */
/****************************************************************************/

/* #define ST_BYTE			0 in lsptypes.h */
/* #define ST_POS16			1 in lsptypes.h */
#define ST_INT32		2
/*#define ST_CODE			4 in load.h      */
#define ST_PTR			6
#define ST_FLOAT		7
#define ST_BIT			8
#define ST_PTR2			11

/****************************************************************************/
/*                                                                          */
/*        The following data are the constant values for array reclaimer.   */
/*                                                                          */
/****************************************************************************/

#define MAXARRAYBLOCKSIZE	65535
#define MAXARRAYLEN		65535
#define ARRAYBLOCKHEADERCELLS	1
#define ARRAYBLOCKTRAILERCELLS	1
#define ARRAYBLOCKOVERHEADCELLS	(ARRAYBLOCKHEADERCELLS+ARRAYBLOCKTRAILERCELLS)
#define MAXARRAYNCELLS		(MAXARRAYBLOCKSIZE-ARRAYBLOCKOVERHEADCELLS)
#define ARRAYBLOCKHEADERWORDS	2
#define ARRAYBLOCKTRAILERWORDS	2
#define ARRAYBLOCKOVERHEADWORDS	(ARRAYBLOCKHEADERWORDS+ARRAYBLOCKTRAILERWORDS)
#define ARRAYBLOCKLINKINGCELLS	2
#define MINARRAYBLOCKSIZE	(ARRAYBLOCKOVERHEADCELLS+ARRAYBLOCKLINKINGCELLS)
#define MAXBUCKETINDEX		30
#define UNBOXEDBLOCK_GCT	0
#define PTRBLOCK_GCT		1
#define CODEBLOCK_GCT		2
#define ABPASSWORDSHIFT		3
#define ARRAYBLOCKPASSWORD	(43690 >> ABPASSWORDSHIFT)
				/* = 1010101010101010 >> 3 = 5461 */
#define FREEARRAYFLAGWORD	((ARRAYBLOCKPASSWORD << ABPASSWORDSHIFT) | (UNBOXEDBLOCK_GCT << 1))
				/* = 43688 */
#define USEDARRAYFLAGWORD	((ARRAYBLOCKPASSWORD << ABPASSWORDSHIFT) | 1)
#define CODEARRAYFLAGWORD	((ARRAYBLOCKPASSWORD << ABPASSWORDSHIFT) | ((CODEBLOCK_GCT << 1) | 1))
#define FIRSTARRAYSEGMENT	19
#define MAXCELLSPERHUNK		64

/****************************************************************************/
/*                                                                          */
/*                  End of Definitions                                      */
/*                                                                          */
/****************************************************************************/
#endif /* ARRAY_H */
