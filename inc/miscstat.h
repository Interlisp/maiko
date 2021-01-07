#ifndef MISCSTAT_H
#define MISCSTAT_H 1
/* $Id: miscstat.h,v 1.2 1999/01/03 02:06:17 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/*
 *	Copyright (C) 1987 by Fuji Xerox Co., Ltd. All rights reserved.
 *
 *	by :	Yasuhiko Kiuchi	
 */


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


typedef struct  misc 
  {
    int	starttime;
    int	totaltime;
    int	swapwaittime;
    int	pagefaults;
    int	swapwrites;
    int	diskiotime;
    int	diskops;
    int	keyboardwaittime;
    int	gctime;
    int	netiotime;
    int	netioops;
    int	swaptemp0;
    int	swaptemp1;
    unsigned int	rclksecond;
    unsigned int	secondsclock;
    unsigned int	millisecondsclock;
    unsigned int	baseclock;
    unsigned int	rclktemp0;
    unsigned int	secondstmp;
    unsigned int	millisecondstmp;
    unsigned int	basetmp;
    int	excesstimetmp;
    int	clocktemp0;
    int	disktemp0;
    int	disktemp1;
    int	teleraidtemp1;
    int	teleraidtemp2;
    int	teleraidtemp3;
    int	lastuseraction;
    int	dlmousetimer;
    int	dlmousetemp;
  } MISCSTATS;
#endif /* MISCSTAT_H */
