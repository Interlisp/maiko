#ifndef OSMSG_H
#define OSMSG_H 1
/* $Id: osmsg.h,v 1.2 1999/01/03 02:06:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/*************************************************
	This is OSMESSAGE stuff.

	Print a console message.
*************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "osmsgdefs.h"   // for flush_pty

#define	OSMESSAGE_PRINT(print_exp)		\
  do {						\
    flush_pty();				\
    print_exp;					\
  } while (0)

#endif /* OSMSG_H */
