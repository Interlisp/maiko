/* $Id: sunfilio.h,v 1.2 1999/01/03 02:06:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)filio.h 1.3 88/02/08 SMI; from UCB ioctl.h 7.1 6/4/86	*/

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * General file ioctl definitions.
 */
#ifndef _filio_h
#define _filio_h

#include <sunioccom.h>

#define	sunFIOCLEX		_sunIO(f, 1)		/* set exclusive use on fd */
#define	sunFIONCLEX	_sunIO(f, 2)		/* remove exclusive use */
/* another local */
#define	sunFIONREAD	_sunIOR(f, 127, int)	/* get # bytes to read */
#define	sunFIONBIO		_sunIOW(f, 126, int)	/* set/clear non-blocking i/o */
#define	sunFIOASYNC	_sunIOW(f, 125, int)	/* set/clear async i/o */
#define	sunFIOSETOWN	_sunIOW(f, 124, int)	/* set owner */
#define	sunFIOGETOWN	_sunIOR(f, 123, int)	/* get owner */

#endif
