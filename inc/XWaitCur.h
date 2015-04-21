/* $Id: XWaitCur.h,v 1.2 1999/01/03 02:05:49 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
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

char waitcursor_bitmap[]=
  {
	  0xFF, 0xFE, 0xC0, 0x06, 0x60, 0x1C, 0x3D, 0x78
	, 0x1F, 0xF0, 0x0F, 0xE0, 0x06, 0xC0, 0x03, 0x80
	, 0x02, 0x80, 0x03, 0xC0, 0x0D, 0x60, 0x19, 0x30
	, 0x37, 0xD8, 0x67, 0xEC, 0xFF, 0xFE, 0xFF, 0xFE
  };

LISP_CURSOR wait_cursor =
  {
     1
   , waitcursor_bitmap
   , waitcursor_bitmap
   , 7
   , 8
   , NULL
  };
	
