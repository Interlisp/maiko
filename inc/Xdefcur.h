/* $Id: Xdefcur.h,v 1.2 1999/01/03 02:05:50 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
*
*
* Copyright (C) 1988 by Fuji Xerox co.,Ltd. All rights reserved.
*
*		Author: Mitsunori Matsuda
*		Date  : August 30, 1988
*/


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/



char defaultcursor_bitmap[]=
  {
	  0x80, 0, 0xc0, 0, 0xe0, 0, 0xf0, 0 
	, 0xf8, 0, 0xfc, 0, 0xfe, 0, 0xf0, 0
	, 0xd8, 0, 0x98, 0, 0x0c, 0, 0x0c, 0
	, 0x06, 0, 0x06, 0, 0x03, 0, 0x03, 0
  };
	
LISP_CURSOR default_cursor =
  { 
     1
   , defaultcursor_bitmap
   , defaultcursor_bitmap
   , 0
   , 15
   , NULL 
  };
