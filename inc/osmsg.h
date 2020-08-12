/* $Id: osmsg.h,v 1.2 1999/01/03 02:06:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/*************************************************
	This is OSMESSAGE stuf.

	Print a console message.
*************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#define	OSMESSAGE_PRINT(print_exp)		\
  {						\
    flush_pty();				\
    print_exp;					\
  }

