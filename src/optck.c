/* $Id: optck.c,v 1.3 1999/05/31 23:35:40 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: optck.c,v 1.3 1999/05/31 23:35:40 sybalsky Exp $ Copyright (C) Venue";
/* optck.c
 *
 * This example is almost same as one shown in
 *	SunOS 4.1 Release Manual: 4.2. Known Problems With SunOS Release 4.1 Products.
 *        
 * Compiling this file with -O or -O2 level and  executing, if assembler optimizes
 * incorrectly, "wrong" message is printed out to stdout.
 *        
 */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


int	boothowto = 1;

int
main(void)
{
	int	unit;

	if (boothowto & 1) {
	      retry:
		unit = -1;
		while (unit == -1) {
			if (unit != -1) {
				printf("wrong");
				exit(1);
			}
			unit = 0;
			foo(&unit);
		}
	} else {
		unit = 0;
		goto retry;
	}
}

foo(int *unitp)
{
}

