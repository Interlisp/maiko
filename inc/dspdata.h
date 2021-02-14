#ifndef DSPDATA_H
#define DSPDATA_H 1
/* $Id: dspdata.h,v 1.2 1999/01/03 02:05:58 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */
#include "version.h" /* for BIGVM */

#ifndef BYTESWAP
	/******************************************************/
	/* Normal-byte-order declarations, for, e.g., 68020's */
	/******************************************************/
typedef struct	displaydata{

     LispPTR	ddxposition ;
     LispPTR	ddyposition;
     LispPTR	ddxoffset;
     LispPTR	ddyoffset;
     LispPTR	dddestination;
     LispPTR	ddclippingregion;
     LispPTR	ddfont;
     LispPTR	ddslowprintingcase;
     LispPTR	ddwidthscache;
     LispPTR	ddoffsetscache;
     LispPTR	ddcolor;
     LispPTR	ddlinefeed;
     LispPTR	ddrightmargin;
     LispPTR	ddleftmargin;
     LispPTR	ddscroll;
     LispPTR	ddoperation;
     unsigned   ddheldflg : 1;
#ifdef BIGVM
     unsigned   nil1 : 3;
     unsigned	ddsourcetype : 28;
#else
     unsigned   nil1 : 7;
     unsigned	ddsourcetype : 24;
#endif /* BIGVM */
     DLword	ddclippingleft;
     DLword	ddclippingright;
     DLword	ddclippingbottom;
     DLword	ddclippingtop;
     DLword	nil2;
     DLword	ddcharsetascent; 
     LispPTR	xwindowhint	; 	/* xpointer  */
     LispPTR	ddpilotbbt;
     LispPTR	ddxscale;
     LispPTR	ddyscale;
     LispPTR	ddcharimagewidths;/* Lisp POINTER to DLword array (49,50)*/
     LispPTR	ddeolfn;
     LispPTR	ddpagefullfn;
     LispPTR	ddtexture;
     LispPTR	ddmicaxpos;
     LispPTR	ddmicaypos;
     LispPTR	ddmicarightmargin;
     LispPTR	ddcharset;
     DLword	ddcharsetdescent;
     DLword	ddspacewidth;  /* ??*/
     LispPTR	ddcharheightdelta; /* NUM PTR */
}DISPLAYDATA;

#else
	/*************************************************/
	/* Byte-swapped declarations, for, e.g., 80386's */
	/*************************************************/
typedef struct	displaydata{

     LispPTR	ddxposition ;
     LispPTR	ddyposition;
     LispPTR	ddxoffset;
     LispPTR	ddyoffset;
     LispPTR	dddestination;
     LispPTR	ddclippingregion;
     LispPTR	ddfont;
     LispPTR	ddslowprintingcase;
     LispPTR	ddwidthscache;
     LispPTR	ddoffsetscache;
     LispPTR	ddcolor;
     LispPTR	ddlinefeed;
     LispPTR	ddrightmargin;
     LispPTR	ddleftmargin;
     LispPTR	ddscroll;
     LispPTR	ddoperation;
#ifdef BIGVM
     unsigned	ddsourcetype : 28;
     unsigned   nil1 : 3;
#else
     unsigned	ddsourcetype : 24;
     unsigned   nil1 : 7;
#endif /* BIGVM */
     unsigned   ddheldflg : 1;

     DLword	ddclippingright;
     DLword	ddclippingleft;
     DLword	ddclippingtop;
     DLword	ddclippingbottom;
     DLword	ddcharsetascent; 
     DLword	nil2;
 
     LispPTR	xwindowhint	; 	/* xpointer  */
     LispPTR	ddpilotbbt;
     LispPTR	ddxscale;
     LispPTR	ddyscale;
     LispPTR	ddcharimagewidths;/* Lisp POINTER to DLword array (49,50)*/
     LispPTR	ddeolfn;
     LispPTR	ddpagefullfn;
     LispPTR	ddtexture;
     LispPTR	ddmicaxpos;
     LispPTR	ddmicaypos;
     LispPTR	ddmicarightmargin;
     LispPTR	ddcharset;
     DLword	ddspacewidth;  /* ??*/
     DLword	ddcharsetdescent;
     LispPTR	ddcharheightdelta; /* NUM PTR */
}DISPLAYDATA;

#endif /* BYTESWAP */

#endif /* DSPDATA_H */
