#ifndef HDW_CONF_H
#define HDW_CONF_H 1
/* $Id: hdw_conf.h,v 1.2 1999/01/03 02:06:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/




/**************************************************************/
/*
	File name :	machineconfig.h

	Created	:	Aug-19-87 take

	Desc. :		Define for Machine dependent param

*/
/**************************************************************/
/* Machine Type */

#define KATANA		3

/* Lastvmempage(32Mbyte) */
#ifdef BIGVM
#define LASTVMEMPAGE 0x1FFFF
#else
#define LASTVMEMPAGE	(65535)
#endif

#endif /* HDW_CONF_H */
