#ifndef STREAM_H
#define STREAM_H 1
/* $Id: stream.h,v 1.2 1999/01/03 02:06:23 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "version.h" /* for BIGVM */
#include "lispemul.h" /* for LispPTR, DLword, DLbyte */

#ifndef BYTESWAP
	/********************************/
	/* Normal byte-order definition */
	/********************************/
typedef struct	stream{
	DLword	COFFSET;
     DLword	CBUFSIZE;
#ifndef BIGVM
     unsigned	BINABLE	:1;
     unsigned	BOUTABLE	:1;
     unsigned	EXTENDABLE	:1;
     unsigned	CBUFDIRTY	:1;
     unsigned	PEEKEDCHARP	:1;
	unsigned  ACCESS	:3;
     unsigned	CBUFPTR		:24;
#else
     unsigned	PEEKEDCHARP	:1;
	unsigned  ACCESS	:3;
     unsigned	CBUFPTR		:28;
#endif /* BIGVM */
      DLbyte	BYTESIZE;
     DLbyte	CHARSET;
     DLword	PEEKEDCHAR;
     DLword	CHARPOSITION;
     DLword	CBUFMAXSIZE;
     unsigned	NONDEFAULTDATEFLG	:1;
     unsigned	REVALIDATEFLG	:1;
     unsigned	MULTIBUFFERHINT	:1;
     unsigned	USERCLOSEABLE	:1;
#ifndef BIGVM
     unsigned	USERVISIBLE	:1;
     unsigned	EOLCONVENTION	:2;
     unsigned	NIL1	:1;
     unsigned	FULLFILENAME	:24;
#else
     unsigned	FULLFILENAME	:28;
#endif /* BIGVM */
#ifdef BIGVM
     unsigned	BINABLE	:1;
     unsigned	BOUTABLE	:1;
     unsigned	EXTENDABLE	:1;
     unsigned	CBUFDIRTY	:1;
     unsigned	DEVICE: 28;
#else
     LispPTR	DEVICE;
#endif /* BIGVM */
#ifdef BIGVM
     unsigned	USERVISIBLE	:1;
     unsigned	EOLCONVENTION	:2;
     unsigned	NIL1	:1;
     unsigned	VALIDATION: 28;
#else
     LispPTR	VALIDATION;
#endif /* BIGVM */
     LispPTR	CPAGE;
     LispPTR	EPAGE;
     DLword	EOFFSET;
     DLword	LINELENGTH;
     LispPTR	F1;
     LispPTR	F2;
     LispPTR	F3;
     LispPTR	F4;
     LispPTR	F5;
     DLword	FW6;
     DLword	FW7;
     DLword	FW8;
     DLword	FW9;
     LispPTR	F10;
     LispPTR	STRMBINFN;
     LispPTR	STRMBOUTFN;
     LispPTR	OUTCHARFN;
     LispPTR	ENDOFSTREAMOP;
     LispPTR	OTHERPROPS;
     LispPTR	IMAGEOPS;
     LispPTR	IMAGEDATA;
     LispPTR	BUFFS;
     DLword	MAXBUFFERS;
     DLword	NIL2;
     LispPTR	EXTRASTREAMOP;
}Stream;

#else

	/***************************/
	/* Byte-swapped definition */
	/***************************/
typedef struct	stream
  {
     DLword	CBUFSIZE;
	DLword	COFFSET;
#ifndef BIGVM
     unsigned	CBUFPTR		:24;
	unsigned  ACCESS	:3;
     unsigned	PEEKEDCHARP	:1;
     unsigned	CBUFDIRTY	:1;
     unsigned	EXTENDABLE	:1;
     unsigned	BOUTABLE	:1;
     unsigned	BINABLE	:1;
#else
     unsigned	CBUFPTR		:28;
     unsigned   ACCESS	:3;
     unsigned	PEEKEDCHARP	:1;
#endif /* BIGVM */
     DLword	PEEKEDCHAR;
     DLbyte	CHARSET;
     DLbyte	BYTESIZE;
     DLword	CBUFMAXSIZE;
     DLword	CHARPOSITION;
#ifdef BIGVM
     unsigned	FULLFILENAME	:28;
#else
     unsigned	FULLFILENAME	:24;
     unsigned	NIL1	:1;
     unsigned	EOLCONVENTION	:2;
     unsigned	USERVISIBLE	:1;
#endif /* BIGVM */
     unsigned	USERCLOSEABLE	:1;
     unsigned	MULTIBUFFERHINT	:1;
     unsigned	REVALIDATEFLG	:1;
     unsigned	NONDEFAULTDATEFLG	:1;
#ifndef BIGVM
     LispPTR	DEVICE;
#else
     unsigned	DEVICE: 28;
     unsigned	CBUFDIRTY	:1;
     unsigned	EXTENDABLE	:1;
     unsigned	BOUTABLE	:1;
     unsigned	BINABLE	:1;
#endif /* BIGVM */
#ifndef BIGVM
     LispPTR	VALIDATION;
#else
     unsigned	VALIDATION: 28;
     unsigned	NIL1	:1;
     unsigned	EOLCONVENTION	:2;
     unsigned	USERVISIBLE	:1;
#endif /* BIGVM */
     LispPTR	CPAGE;
     LispPTR	EPAGE;
     DLword	LINELENGTH;
     DLword	EOFFSET;
     LispPTR	F1;
     LispPTR	F2;
     LispPTR	F3;
     LispPTR	F4;
     LispPTR	F5;
     DLword	FW7;
     DLword	FW6;
     DLword	FW9;
     DLword	FW8;
     LispPTR	F10;
     LispPTR	STRMBINFN;
     LispPTR	STRMBOUTFN;
     LispPTR	OUTCHARFN;
     LispPTR	ENDOFSTREAMOP;
     LispPTR	OTHERPROPS;
     LispPTR	IMAGEOPS;
     LispPTR	IMAGEDATA;
     LispPTR	BUFFS;
     DLword	NIL2;
     DLword	MAXBUFFERS;
     LispPTR	EXTRASTREAMOP;
}Stream;

#endif /* BYTESWAP */

#endif /* STREAM_H */
