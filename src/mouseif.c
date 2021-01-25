/* $Id: mouseif.c,v 1.2 1999/01/03 02:07:26 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/* * * *   D O S   M O U S E   I N T E R F A C E   * * * */

#include "version.h"

#include "lispemul.h"
#include "dbprint.h"
#include "devif.h"

MouseInterfaceRec _curmouse;
MouseInterface currentmouse = &_curmouse;

#ifdef DOS
#include <dos.h>

int nomouseflag = FALSE;
extern DLword *Lisp_world;
extern LispPTR *LASTUSERACTION68k;
extern int twobuttonflag;

extern void EnterDosMouse();
extern void ExitDosMouse();
extern void DosMouseAfterRaid();
extern void DosMouseBeforeRaid();
extern unsigned long GenericReturnT();
extern void ThreeButtonHandler();
extern void TwoButtonHandler();
#endif /* DOS */

#ifdef DOS
/*****************************************************************/
/*                       p r o b e m o u s e                     */
/*                                                               */
/* Probe for mouse and return the number of buttons available.   */
/*****************************************************************/
int probemouse() {
  union REGS regs;
  char c;
  /***************************************************************************
   * Reset mouse driver, exit if no mouse driver present
   ***************************************************************************/
  /* int 33h, case 0000, ax = drive installed, bx = # of buttons. */
  if (nomouseflag) {
    return (666); /* return something, why not 666? */
  } else {
    regs.w.eax = 0; /* Func 0 = Reset mouse, ret. button info */
    int86(0x33, &regs, &regs);

    if (regs.x.ax == 0x0000) VESA_errorexit("No mouse driver found.", -1);
    return (regs.x.bx);
  }
}
#endif

void make_mouse_instance(MouseInterface mouse)
{
#ifdef DOS

  int NumberOfButtons;
  if (nomouseflag) {
    mouse->device.enter = &GenericReturnT;
    mouse->device.exit = &GenericReturnT;
    mouse->device.before_raid = &GenericReturnT;
    mouse->device.after_raid = &GenericReturnT;
    mouse->device.active = FALSE;
    NumberOfButtons = 3;
  } else {
    mouse->device.enter = &EnterDosMouse;
    mouse->device.exit = &ExitDosMouse;
    mouse->device.before_raid = &DosMouseBeforeRaid;
    mouse->device.after_raid = &DosMouseAfterRaid;
    mouse->device.active = FALSE;
    NumberOfButtons = probemouse();
  }
  mouse->Button.StartTime = 2;

  mouse->Cursor.Last.width = 16;
  mouse->Cursor.Last.height = 16;

  if (nomouseflag == FALSE) {
    if (twobuttonflag) { /* We force two button handling. */
      mouse->Handler = &TwoButtonHandler;
      mouse->Button.TwoButtonP = TRUE;
    } else /* Determine how many buttons we have. */
      switch (NumberOfButtons) {
        case 0x0000: /* Other than 2 buttons, assume three */
          mouse->Button.TwoButtonP = FALSE;
          mouse->Handler = &ThreeButtonHandler;
          break;
        case 0x0002: /* Two buttons. */
          mouse->Button.TwoButtonP = TRUE;
          mouse->Handler = &TwoButtonHandler;
          break;
        case 0x0003: /* Three buttons. */
          mouse->Button.TwoButtonP = FALSE;
          mouse->Handler = &ThreeButtonHandler;
          break;
        case 0xffff: /* Two buttons. */
          mouse->Button.TwoButtonP = TRUE;
          mouse->Handler = &TwoButtonHandler;
          break;
        default: /* Strange case, assume three. */
          mouse->Button.TwoButtonP = FALSE;
          mouse->Handler = &ThreeButtonHandler;
          break;
      }
  }
/* mouse->timestamp = ((*LASTUSERACTION68k& 0xffffff) + Lisp_world); */
#elif XWINDOW
#endif /* DOS or XWINDOW */
}
