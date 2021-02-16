/* $Id: mouseif.c,v 1.2 1999/01/03 02:07:26 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/* * * *   D O S   M O U S E   I N T E R F A C E   * * * */

#include "version.h"

#include "lispemul.h"
#include "dbprint.h"
#include "devif.h"

MouseInterfaceRec curmouse;
MouseInterface currentmouse = &curmouse;

void make_mouse_instance(MouseInterface mouse)
{
#if   XWINDOW
#endif /* DOS or XWINDOW */
}
