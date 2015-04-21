/* $Id: picture.h,v 1.2 1999/01/03 02:06:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

typedef struct _picture{
	DLword	width;
	DLword	height;
	DLword	bitsperpixel;
	DLword	nil;
	unsigned int storage;
	LispPTR	userdata;
} LispPicture;
