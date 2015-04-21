/* $Id: sunttycom.h,v 1.2 1999/01/03 02:06:26 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)ttycom.h 1.5 88/02/08 SMI	*/

#ifndef _sunTTYCOM_
#define _sunTTYCOM_

/*
 * Window/terminal size structure.
 * This information is stored by the kernel
 * in order to provide a consistent interface,
 * but is not used by the kernel.
 *
 */

#define	sunTIOCGWINSZ	_sunIOR(t, 104, struct winsize)	/* get window size */
#define	sunTIOCSWINSZ	_sunIOW(t, 103, struct winsize)	/* set window size */


#define	sunTIOCSSIZE	_sunIOW(t,37,struct ttysize)/* set tty size */
#define	sunTIOCGSIZE	_sunIOR(t,38,struct ttysize)/* get tty size */

/*
 * 4.3BSD and SunOS terminal "ioctl"s with no "termios" equivalents.
 * This file is included by <sys/termios.h> and indirectly by <sys/ioctl.h>
 * so that programs that include either one have these "ioctl"s defined.
 */
#define	sunTIOCGPGRP	_sunIOR(t, 119, int)	/* get pgrp of tty */
#define	sunTIOCSPGRP	_sunIOW(t, 118, int)	/* set pgrp of tty */
#define	sunTIOCOUTQ	_sunIOR(t, 115, int)	/* output queue size */
#define	sunTIOCSTI		_sunIOW(t, 114, char)	/* simulate terminal input */
#define	sunTIOCNOTTY	_sunIO(t, 113)		/* void tty association */
#define	sunTIOCPKT		_sunIOW(t, 112, int)	/* pty: set/clear packet mode */
#define		sunTIOCPKT_DATA		0x00	/* data packet */
#define		sunTIOCPKT_FLUSHREAD	0x01	/* flush data not yet written to controller */
#define		sunTIOCPKT_FLUSHWRITE	0x02	/* flush data read from controller but not yet processed */
#define		sunTIOCPKT_STOP		0x04	/* stop output */
#define		sunTIOCPKT_START		0x08	/* start output */
#define		sunTIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		sunTIOCPKT_DOSTOP		0x20	/* now do ^S, ^Q */
#define		sunTIOCPKT_IOCTL		0x40	/* "ioctl" packet */
#define	sunTIOCMSET	_sunIOW(t, 109, int)	/* set all modem bits */
#define	sunTIOCMBIS	_sunIOW(t, 108, int)	/* bis modem bits */
#define	sunTIOCMBIC	_sunIOW(t, 107, int)	/* bic modem bits */
#define	sunTIOCMGET	_sunIOR(t, 106, int)	/* get all modem bits */
#define		sunTIOCM_LE	0001		/* line enable */
#define		sunTIOCM_DTR	0002		/* data terminal ready */
#define		sunTIOCM_RTS	0004		/* request to send */
#define		sunTIOCM_ST	0010		/* secondary transmit */
#define		sunTIOCM_SR	0020		/* secondary receive */
#define		sunTIOCM_CTS	0040		/* clear to send */
#define		sunTIOCM_CAR	0100		/* carrier detect */
#define		sunTIOCM_CD	sunTIOCM_CAR
#define		sunTIOCM_RNG	0200		/* ring */
#define		sunTIOCM_RI	sunTIOCM_RNG
#define		sunTIOCM_DSR	0400		/* data set ready */

#define	sunTIOCREMOTE	_sunIOW(t, 105, int)	/* remote input editing */
#define	sunTIOCUCNTL	_sunIOW(t, 102, int)	/* pty: set/clr usr cntl mode */

/*
 * Sun-specific ioctls with no "termios" equivalents.
 */
#define sunTIOCTCNTL	_sunIOW(t, 32, int)	/* pty: set/clr intercept ioctl mode */
#define sunTIOCSIGNAL	_sunIOW(t, 33, int)	/* pty: send signal to slave */
#define	sunTIOCCONS	_sunIO(t, 36)		/* get console I/O */
#define	sunTIOCSSOFTCAR	_sunIOW(t, 101, int)	/* set soft carrier flag */
#define	sunTIOCGSOFTCAR	_sunIOR(t, 100, int)	/* get soft carrier flag */

#endif
