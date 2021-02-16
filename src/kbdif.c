/* $Id: kbdif.c,v 1.3 1999/05/31 23:35:35 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/* * K Y E B O A R D   I N T E R F A C E * */

#include "lispemul.h"
#include "dbprint.h"
#include "devif.h"

KbdInterfaceRec curkbd;
KbdInterface currentkbd = &curkbd;

void make_kbd_instance(KbdInterface kbd) {
#if   XWINDOW
#endif /* DOS or XWINDOW */
}
