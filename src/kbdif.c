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

#ifdef DOS
extern void Kbd_event(void);
extern void EnterDosKbd(void);
extern void ExitDosKbd(void);
extern unsigned long GenericReturnT(void);
#endif /* DOS */

void make_kbd_instance(KbdInterface kbd) {
#ifdef DOS
  kbd->device_event = &Kbd_event; /*  */
  kbd->device.enter = &EnterDosKbd;
  kbd->device.exit = &ExitDosKbd;
  kbd->device.before_raid = &ExitDosKbd;
  kbd->device.after_raid = &EnterDosKbd;
  kbd->device.active = FALSE;
#elif XWINDOW
#endif /* DOS or XWINDOW */
}
