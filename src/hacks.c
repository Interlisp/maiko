/* $Id: hacks.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */
static char *id = "$Id: hacks.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "hacksdefs.h"

/* These functions are created so that you can split a float into */
/* four integers. The general idea behind these functions is to */
/* act as a caster between different entitys on the stack */

int pickapart1(int i1, int i2, int i3, int i4) { return (i1); }

int pickapart2(int i1, int i2, int i3, int i4) { return (i2); }

int pickapart3(int i1, int i2, int i3, int i4) { return (i3); }

int pickapart4(int i1, int i2, int i3, int i4) { return (i4); }
