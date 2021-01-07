#ifndef DEVCONF_H
#define DEVCONF_H 1
/* $Id: devconf.h,v 1.2 1999/01/03 02:05:56 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/**********************************************************/
/*
	 devconf.h

	Device Configurations assignments
	for IFPAGE->devconfig

	LSB(0)~3 -> KBD
	4~7 -> DISPLAY,
	8~10 -> CPU

         [CPU] [DSP]   [KB]
  !!!!!! !!!   !!!!    !!!  LSB>

	By Takeshi
*/
/**********************************************************/



/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/




/* MAIKO(sun3,sun4)*/
/* KBD */
#define SUN_KEYTYPE_MASK 7
#define SUN_TYPE3_KBD	0x0
#define SUN_TYPE4_KBD	0x1

/* DISPLAY */
#define SUN_DISPTYPE_MASK 0x78
#define  SUN2BW		(2<<3)
#define  SUN2COLOR	(3<<3)
#define  SUN4COLOR	(8<<3)
#define	SUNMEMCOLOR	(7<<3)

/* CPUTYPE NOT IMPLEMENTED */

/* useful macros */
#define SUN_GETKEYTYPE		(InterfacePage->devconfig & SUN_KEYTYPE_MASK)
#define SUN_GETDISPTYPE		(InterfacePage->devconfig & SUN_DISPTYPE_MASK)
#endif /* DEVCONF_H */
