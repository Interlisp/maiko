#ifndef PICTURE_H
#define PICTURE_H 1
/* $Id: picture.h,v 1.2 1999/01/03 02:06:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */

typedef struct _picture{
	DLword	width;
	DLword	height;
	DLword	bitsperpixel;
	DLword	nil;
	unsigned int storage;
	LispPTR	userdata;
} LispPicture;
#endif /* PICTURE_H */
