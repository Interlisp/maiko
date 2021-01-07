#ifndef INITATMS_H
#define INITATMS_H 1
/* $Id: initatms.h,v 1.2 1999/01/03 02:06:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


/*
 *
 *	Author	:	Takeshi Shimizu
 *			Hiroshi Hayata
 */
/*********************************************************/
/*
		File :	initatms.h
		System ATOMs

			last changed :	5-Mar-87  (take)
					12-Aug-87 take
	** MERGED AT AIS

*/
/*********************************************************/

#define ATOM_EVALFORM		248
#define ATOM_GCHANDLEOVERFLOW	249
#define ATOM_INTERPRETER	256

#define ATOM_SMALLP		257
#define ATOM_FIXP		258
#define ATOM_FLOATP		259
#define ATOM_LITATOM		260
#define ATOM_LISTP		261
#define ATOM_ARRAYP		262
#define ATOM_STRINGP		263
#define ATOM_STACKP		264
#define ATOM_CHARACTER		265
#define ATOM_VMEMPAGEP		266
#endif /* INITATMS_H */
