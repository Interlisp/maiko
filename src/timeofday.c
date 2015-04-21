/* $Id: timeofday.c,v 1.2 1999/01/03 02:07:37 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: timeofday.c,v 1.2 1999/01/03 02:07:37 sybalsky Exp $ Copyright (C) Venue";


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


#include <sys/time.h>
#include <sys/resource.h>

gettimeofday(time, ptr)
    struct timeval *time;
    int ptr;
{
	struct rusage stats;
	getrusage(RUSAGE_SELF, &stats);
	time->tv_sec = stats.ru_utime.tv_sec;
	time->tv_usec = stats.ru_utime.tv_usec;
}
