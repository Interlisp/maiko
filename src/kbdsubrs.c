/* $Id: kbdsubrs.c,v 1.2 1999/01/03 02:07:10 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <stdio.h>
#ifdef DOS
#include <time.h>
#include <conio.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/select.h>
#endif /* DOS */


#include "lispemul.h"

#include "kbdsubrsdefs.h"
#include "commondefs.h"
#ifdef XWINDOW
#include "lisp2cdefs.h"
#include "xwinmandefs.h"
#endif

#ifdef DOS
#define PORT_A 0x60
#include "devif.h"
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
#elif XWINDOW
#include "devif.h"
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
#endif /* DOS */

/****************************************************
 *
 *	KB_enable() entry of SUBRCALL 82 1
 *			called from (\KB_enable X)
 *
 ****************************************************/


#ifdef XWINDOW
#include <X11/Xlib.h>
#endif /* XWINDOW */

extern fd_set LispReadFds;

void KB_enable(LispPTR *args) /* args[0] :	ON/OFF flag
                                     *		T -- ON
                                     *		NIL -- OFF
                                     */
{
  if (args[0] == ATOM_T) {
#if   XWINDOW
    enable_Xkeyboard(currentdsp);
#elif DOS
    (currentkbd->device.enter)(currentkbd);
#endif /* DOS */
  } else if (args[0] == NIL) {
#if   XWINDOW
    disable_Xkeyboard(currentdsp);
#elif DOS
    (currentkbd->device.exit)(currentkbd);
#endif /* DOS */
  } else {
    error("KB_enable: illegal arg \n");
    printf("KB_enable: arg = %d\n", args[0]);
  }
}

/****************************************************
 *
 *	KB_beep() entry of SUBRCALL 80 2
 *			called from (\KB_beep SW FREQ)
 *
 ****************************************************/
/*
struct timeval belltime ={
        0,100
};
*/
extern int LispKbdFd;

#ifdef DOS
int bell_status_word;
#endif /* DOS */

void KB_beep(LispPTR *args) /* args[0] :	ON/OFF flag
                                   *		T -- ON
                                   *		NIL -- OFF
                                   * args[1] :	frequency
                                   */
{
#if   XWINDOW
  if (args[0] == ATOM_T) beep_Xkeyboard(currentdsp);
#elif DOS
  if (args[0] == ATOM_T) {
    bell_status_word = inp(0x61);
    outp(0x61, bell_status_word | 0x3); /* Turn on the speaker */
    /* Prepare timer by sending 10111100 to port 43. */
    outp(0x43, 0xb6);

    /* Divide input frequency by timer ticks per second and
     * write (byte by byte) to timer. */
    outp(0x42, (char)(1193180L / (LispIntToCInt(args[1]))));
    outp(0x42, (char)(1193180L / (LispIntToCInt(args[1])) >> 8));
  } else {
    outp(0x61, bell_status_word & ~0x3); /* Turn off the speaker (with */
    /* bits 0 and 1). */
  }
#endif /* SUNDISPLAY, XWINDOW, DOS */
}

/****************************************************
 *
 *	KB_setmp() entry of SUBRCALL 81 1
 *
 ****************************************************/

void KB_setmp(LispPTR *args) /* args[0] :	MPCODE	*/
{
#ifdef DEBUG
  printf("MP: %d\n", args[0] & 0xffff);
#endif
}

/****************************************************
 *
 *	KB_setled()
 * Set the status LED's on the kbd.
 * arg[0] Caps lock LED.
 * arg[1] Num lock LED.
 * arg[2] Scroll lock LED.
 * NIL -> LED off
 * not NIL -> LED on.
 *
 ****************************************************/

void KB_setled(LispPTR *args)
{
#ifdef DOS
  outp(PORT_A, (unsigned char)0xED);
  outp(PORT_A,
       (unsigned char)(((args[0] != NIL) << 2) | ((args[1] != NIL) << 1) | (args[2] != NIL)));
#endif /* DOS */
}
