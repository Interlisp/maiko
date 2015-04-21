/* $Id: sunioccom.h,v 1.2 1999/01/03 02:06:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)ioccom.h 1.3 88/02/08 SMI; from UCB ioctl.h 7.1 6/4/86	*/

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef _sunIO

/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 255 bytes.
 */
#define	sunIOCPARM_MASK	0xff		/* parameters must be < 256 bytes */
#define	sunIOC_VOID	0x20000000	/* no parameters */
#define	sunIOC_OUT		0x40000000	/* copy out parameters */
#define	sunIOC_IN		0x80000000	/* copy in parameters */
#define	sunIOC_INOUT	(sunIOC_IN|sunIOC_OUT)
/* the 0x20000000 is so we can distinguish new ioctl's from old */
#define	_sunIO(x,y)	(sunIOC_VOID|('x'<<8)|y)
#define	_sunIOR(x,y,t)	(sunIOC_OUT|((sizeof(t)&sunIOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_sunIORN(x,y,t)	(sunIOC_OUT|(((t)&sunIOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_sunIOW(x,y,t)	(sunIOC_IN|((sizeof(t)&sunIOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_sunIOWN(x,y,t)	(sunIOC_IN|(((t)&sunIOCPARM_MASK)<<16)|('x'<<8)|y)
/* this should be _sunIORW, but stdio got there first */
#define	_sunIOWR(x,y,t)	(sunIOC_INOUT|((sizeof(t)&sunIOCPARM_MASK)<<16)|('x'<<8)|y)

#endif
