#ifndef KEYBOARD_H
#define KEYBOARD_H 1
/* $Id: keyboard.h,v 1.2 1999/01/03 02:06:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/** Header File for K/B MOUSE */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for DLword */

#define	MOUSE_LEFT	13
#define	MOUSE_MIDDLE	15
#define	MOUSE_RIGHT	14
#define	CAPSKEY		16
#define DLMOUSEUP 	0
#define DLMOUSEWAITING 	1
#define DLMOUSENORMAL 	2
#define MOUSE_ALLBITS	7
#define KB_ALLUP	0xffff
#define HARDCURSORHEIGHT 16

#ifndef BYTESWAP
typedef struct
  {
    DLword read;
    DLword write;
  } RING;
#else
typedef struct
  {
    DLword write;
    DLword read;
  } RING;
#endif /* BYTESWAP */


	/* macros for getting to the next-read and next-write ring buf ptrs */
#define RING_READ(head68k)	(((RING*)(head68k))->read)
#define RING_WRITE(head68k)	(((RING*)(head68k))->write)


/* for feature use */
#ifndef BYTESWAP
typedef struct
  {
    DLword 	mousex;
    DLword	mousey;
    DLword	utilin;
    DLword	kbdad0;
    DLword	kbdad1;
    DLword	kbdad2;
    DLword	kbdad3;
    DLword	kbdad4;
    DLword	kbdad5;
    DLword	nil;
  } IOState;

	/* Corresponds to the Lisp KEYBOARDEVENT structure */
typedef struct
  {
    DLword W0;
    DLword W1;
    DLword W2;
    DLword W3;
    DLword WU;
    DLword W4;
    DLword W5;
	/* int		time; */
    short  timehi;
    short	timelo;
    unsigned	mousestate	: 3;
    unsigned	shift1		: 1;
    unsigned	shift2		: 1;
    unsigned	lock		: 1;
    unsigned	ctrl		: 1;
    unsigned	meta		: 1;
    unsigned	font		: 1;
    unsigned	usermode1	: 1;
    unsigned	usermode2	: 1;
    unsigned	usermode3	: 1;
    unsigned	altgr :		  1;
    unsigned	deadkey		: 1;
    unsigned	nil		: 2;
    DLword	mousex;
    DLword	mousey;
	/* DLword  nil2; */
    LispPTR deadkeyalist;
  } KBEVENT;

#define RCLK(place) {  struct timeval time;\
					gettimeofday(&time,NULL);\
					(place)=(time.tv_sec * 1000000)+time.tv_usec;}

#else
typedef struct
  {
    DLword	mousey;
    DLword 	mousex;
    DLword	kbdad0;
    DLword	utilin;
    DLword	kbdad2;
    DLword	kbdad1;
    DLword	kbdad4;
    DLword	kbdad3;
    DLword	nil;
    DLword	kbdad5;
  } IOState;

	/* Corresponds to the Lisp KEYBOARDEVENT structure */
typedef struct 
  {
    DLword W1;
    DLword W0;
    DLword W3;
    DLword W2;
    DLword W4;
    DLword WU;
    DLword timehi;
    DLword W5;
    short timelo;

    DLword	mousex;
    unsigned	nil		: 4;
    unsigned	usermode3	: 1;
    unsigned	usermode2	: 1;
    unsigned	usermode1	: 1;
    unsigned	font		: 1;
    unsigned	meta		: 1;
    unsigned	ctrl		: 1;
    unsigned	lock		: 1;
    unsigned	shift2		: 1;
    unsigned	shift1		: 1;
    unsigned	mousestate	: 3;
/*		DLword  nil2; */
    DLword	mousey;
    LispPTR deadkeyalist;
  } KBEVENT;  /* CHANGED-BY-TAKE ***/

/*** OBSOLETE
**#define RCLK(hi,lo) \
  { \
    struct timeval time;\
    int timetemp; \
    gettimeofday(&time,NULL);\
    timetemp = (time.tv_sec * 1000000)+time.tv_usec; \
    (hi)=(DLword)(timetemp>>16); \
    (lo) = (DLword)(timetemp & 0xFFFF); \
}
******/
#define RCLK(place) {  struct timeval time;\
					gettimeofday(&time,NULL);\
					(place)=(time.tv_sec * 1000000)+time.tv_usec;}

#endif /* BYTESWAP */


	

	/* Size of a KEYBOARDEVENT structure, and	*/
	/* the size of the kbd-event ring buffer	*/
#define MINKEYEVENT	2	/* leave 2 words for read,write offsets */
#ifdef NOEUROKBD	/* set to disable new european kbd support */
#define KEYEVENTSIZE 12
#else
#define KEYEVENTSIZE ((sizeof(KBEVENT)+1)>>1)
#endif
	/* Offset of the end of the ring buffer */
#define MAXKEYEVENT	(MINKEYEVENT + (383*KEYEVENTSIZE))
#define NOEUROKEYEVENTSIZE 12
#define EUROKEYEVENTSIZE ((sizeof(KBEVENT) + 1) >> 1)
#define NUMBEROFKEYEVENTS 383


typedef union
{
  struct
   {
     RING vectorindex;	/* Index for the vector of DLwords in this structure */
     KBEVENT event[NUMBEROFKEYEVENTS + 1];
   } ring;
  /* The array of KBEVENTS (indexed by DLword) for euro */
  DLword euro[MINKEYEVENT + (NUMBEROFKEYEVENTS * EUROKEYEVENTSIZE)];
	/* The array of KBEVENTS (indexed by DLword) for noeuro */
  DLword noeuro[MINKEYEVENT + (NUMBEROFKEYEVENTS * NOEUROKEYEVENTSIZE)];
 } keybuffer;

#endif /* KEYBOARD_H */
