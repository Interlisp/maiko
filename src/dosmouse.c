/* $Id: dosmouse.c,v 1.2 1999/01/03 02:06:56 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: dosmouse.c,v 1.2 1999/01/03 02:06:56 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

#include <i32.h> /* Defines "#pragma interrupt"  */
#include <stk.h> /* _XSTACK struct definition    */
#include <dos.h> /* Defines REGS & other structs */

#include "lispemul.h"
#include "display.h"
#include "bb.h"

#include "dbprint.h"
#include "devif.h"
#include "keyboard.h"
#include "ifpage.h"

extern int eurokbd;
extern IOPAGE *IOPage68K;
extern MISCSTATS *MiscStats;
extern IFPAGE *InterfacePage;
extern int KBDEventFlg;
extern DspInterface currentdsp;
extern MouseInterface currentmouse;
extern KbdInterface currentkbd;

extern keybuffer *CTopKeyevent;
extern DLword *DisplayRegion68k;
extern DLword *Lisp_world;

extern LispPTR *KEYBOARDEVENTQUEUE68k;
extern LispPTR *KEYBUFFERING68k;
extern LispPTR *LASTUSERACTION68k;

#define MOUSE_MV 0x01 /* Mouse movement occurred */
#define LB_PRESS 0x02 /* Left button pressed     */
#define LB_OFF 0x04   /* Left button released    */
#define RB_PRESS 0x08 /* Right button pressed    */
#define RB_OFF 0x10   /* Right button released   */
#define CB_PRESS 0x20 /* Center button released  */
#define CB_OFF 0x40   /* Center button pressed   */

#pragma interrupt(TwoButtonHandler)
#pragma interrupt(ThreeButtonHandler)
#pragma interrupt(ButtonTimer)
void ButtonTimer();
void MouseButtonSignal();

void EnterDosMouse(MouseInterface mouse, DspInterface dsp)
{
  union REGS regs;

  probemouse();
  _dpmi_lockregion((void *)&MouseButtonSignal, 4096);
  /* Set up the ringbuffer */
  if (eurokbd)
    mouse->keyeventsize = EUROKEYEVENTSIZE;
  else
    mouse->keyeventsize = NOEUROKEYEVENTSIZE;

  mouse->eurokbd = eurokbd;

  /* Offset of the end of the ring buffer */
  mouse->maxkeyevent = (MINKEYEVENT + (NUMBEROFKEYEVENTS * mouse->keyeventsize));

  /* Lock myself */
  _dpmi_lockregion((void *)mouse, sizeof(*mouse));
  _dpmi_lockregion((void *)&currentmouse, sizeof(currentmouse));

  /* Lock the handler routines */
  _dpmi_lockregion((void *)mouse->Handler, 4096);

  if (mouse->Button.TwoButtonP) { _dpmi_lockregion((void *)&ButtonTimer, 4096); }

  /* Lock the structures used, both pointers to 'em & the whole structure. */
  _dpmi_lockregion((void *)&IOPage68K, sizeof(IOPage68K));
  _dpmi_lockregion((void *)IOPage68K, sizeof(IOPAGE));
  _dpmi_lockregion((void *)&InterfacePage, sizeof(InterfacePage));
  _dpmi_lockregion((void *)InterfacePage, sizeof(IFPAGE));
  _dpmi_lockregion((void *)&MiscStats, sizeof(MiscStats));
  _dpmi_lockregion((void *)MiscStats, sizeof(MISCSTATS));

  /* Lock the flags */
  _dpmi_lockregion((void *)&KBDEventFlg, sizeof(KBDEventFlg));
  _dpmi_lockregion((void *)&MachineState, sizeof(MachineState));

  /* Lock the pointers into the sysout */
  _dpmi_lockregion((void *)&KEYBUFFERING68k, sizeof(KEYBUFFERING68k));

  /* Lock the regions of the sysout that the pointers points to */
  /* KEYBUFFERINF68k points to the value cell for a symbol */
  _dpmi_lockregion((void *)KEYBUFFERING68k, sizeof(LispPTR));

  /* CTopKeyevent points to the ring-buffer of keyboard events */
  _dpmi_lockregion((void *)&CTopKeyevent, sizeof(CTopKeyevent));
  _dpmi_lockregion((void *)CTopKeyevent, sizeof(*CTopKeyevent));

  /* Now Lock the lastuseraction machine. */
  /*  _dpmi_lockregion(mouse->timestamp, sizeof(LispPTR)); */ /* Lisp_world itself */

  regs.w.eax = 0x0001; /* Function 1 = turn on mouse cursor */
  int86(0x33, &regs, &regs);
  regs.w.eax = 0x0002; /* Function 2 = hide mouse cursor */
  int86(0x33, &regs, &regs);

  regs.w.eax = 0x0C; /* Function 0C = set user-defined mouse handler */
  regs.w.ecx = LB_PRESS | LB_OFF | CB_PRESS | CB_OFF | RB_PRESS | RB_OFF | MOUSE_MV;
  regs.w.edx = FP_OFF(*(mouse->Handler)); /* Address of our mouse handler routine */
  int86(0x33, &regs, &regs);              /* Install our handler to process events */

  if (regs.w.eax == 'MERR')
    VESA_errorexit("Unable to install mouse handler - not enough low memory.\n", -1);

  regs.x.ax = 0x7; /* Set mouse horizontal range */
  regs.x.cx = 0x0;
  regs.x.dx = (short)(dsp->Display.width - 1);
  int86(0x33, &regs, &regs);

  regs.x.ax = 0x8; /* Set mouse vertical range */
  regs.x.cx = 0x0;
  regs.x.dx = (short)(dsp->Display.height - 1);
  int86(0x33, &regs, &regs);

  /* See if turning this off fixes the "mouse granularity" problem */
  /* Nope. It didn't. */
  regs.x.ax = 0xf; /* Set mickey per pixel range */
  regs.x.cx = 0x8;
  regs.x.dx = 0x8;
  int86(0x33, &regs, &regs);

  mouse->Button.NextHandler = _dos_getvect(0x1c);
  _dos_setvect(0x1c, ButtonTimer);

  mouse->device.active = TRUE;
}

void ExitDosMouse(MouseInterface mouse)
{
  if (mouse->device.active) {
    /* Unlock myself */
    _dpmi_unlockregion((void *)mouse, sizeof(*mouse));
    _dpmi_unlockregion((void *)&currentmouse, sizeof(currentmouse));

    /* Unlock the handler routines */
    _dpmi_unlockregion((void *)mouse->Handler, 4096);
    _dpmi_unlockregion((void *)&ButtonTimer, 4096);
    _dpmi_unlockregion((void *)&MouseButtonSignal, 4096);

    /* Unlock the structures used. */
    _dpmi_unlockregion((void *)&IOPage68K, sizeof(IOPage68K));
    _dpmi_unlockregion((void *)&InterfacePage, sizeof(InterfacePage));
    _dpmi_unlockregion((void *)&MiscStats, sizeof(MiscStats));

    /* Unlock the flags */
    _dpmi_unlockregion((void *)&KBDEventFlg, sizeof(KBDEventFlg));
    _dpmi_unlockregion((void *)&MachineState, sizeof(MachineState));

    /* Unlock the pointers into the sysout */
    _dpmi_unlockregion((void *)&KEYBUFFERING68k, sizeof(KEYBUFFERING68k));
    /* _dpmi_unlockregion((void *)&KEYBOARDEVENTQUEUE68k, sizeof(KEYBOARDEVENTQUEUE68k)); */

    /* Unlock the regions of the sysout that the pointers points to */
    _dpmi_unlockregion((void *)KEYBUFFERING68k, sizeof(LispPTR));

    _dpmi_unlockregion((void *)&CTopKeyevent, sizeof(CTopKeyevent));
    _dpmi_unlockregion((void *)CTopKeyevent, sizeof(*CTopKeyevent));

    /*     _dpmi_unlockregion(mouse->timestamp, sizeof(LispPTR)); */

    _dos_setvect(0x1c, mouse->Button.NextHandler);
    mouse->device.active = FALSE;
  }
}

void DosMouseAfterRaid(MouseInterface mouse, DspInterface dsp)
{
  union REGS regs;

  /* Screen mode changed. We have to reinit the ranges. */
  regs.x.ax = 0x7; /* Set mouse horizontal range */
  regs.x.cx = 0x0;
  regs.x.dx = (short)(dsp->Display.width - 1);
  int86(0x33, &regs, &regs);

  regs.x.ax = 0x8; /* Set mouse vertical range */
  regs.x.cx = 0x0;
  regs.x.dx = (short)(dsp->Display.height - 1);
  int86(0x33, &regs, &regs);

  mouse->device.active = TRUE;
}

void DosMouseBeforeRaid(MouseInterface mouse, DspInterface dsp)
{ mouse->device.active = FALSE; }

/***************************************************************/
/*            d o s _ c u r s o r _ i n v i s s i b l e        */
/* Since we only blit the cursor to the VESA/VGA displaybuffer */
/* and not to the emulator displaybuffer we can make the cursor*/
/* invissible just by updateing the area under the cursor!     */
/***************************************************************/
void dos_cursor_invissible(DspInterface dsp, IOPAGE *iop)

{ (dsp->bitblt_to_screen)(dsp, DisplayRegion68k, iop->dlcursorx, iop->dlcursory, 16, 16); }

/***************************************************************/
/*              d o s _ c u r s o r _ v i s s i b l e          */
/* blit the mouse to the display ...                           */
/* The cursor should be blitted according to the following:    */
/* ((backgroundbm AND maskbm) OR ((NOT mask) OR cursorbm))     */
/* ie. bltAND the mask to the background then bltOR the rest   */
/*                                                             */
/* Hah!! this crappy machine doesn't have a mask!! /jarl       */
/* ie. use the inverted bitmap as a mask!!!!                   */
/*                                                             */
/* More to the point. The mask is the image inverted. (sigh..) */
/*                                                             */
/***************************************************************/

set_DOSmouseposition(DspInterface dsp, int x, int y)

{
  union REGS regs;

  dsp->device.locked++;
  currentmouse->device.active++;

#ifdef NEVER
  /* int 33h, case 0004, cx=col, dx = row */
  regs.w.eax = 4; /* Function 4 = move cursor */
  regs.w.ecx = x;
  regs.w.edx = y;
  int86(0x33, &regs, &regs);
#endif /* NEVER */

  /* Actually move the cursor image */
  IOPage68K->dlmousex = x;
  IOPage68K->dlmousey = y;

  /*  *(currentmouse->timestamp) = MiscStats->secondstmp; */

  (currentdsp->mouse_invissible)(currentdsp, IOPage68K);
  currentmouse->Cursor.New.x = IOPage68K->dlcursorx = x;
  currentmouse->Cursor.New.y = IOPage68K->dlcursory = y;
  (currentdsp->mouse_vissible)(x, y);

  dsp->device.locked--;
  currentmouse->device.active--;
}

void docopy(int newx, int newy)
{
  register DLword *srcbase, *dstbase;
  static int sx, dx, w = 16, h = 16, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

  srcbase = IOPage68K->dlcursorbitmap;
  dstbase = DisplayRegion68k + (newy * currentdsp->Display.width / 16);
  sx = 0;
  dx = newx;
  w = currentmouse->Cursor.Last.width;
  h = currentmouse->Cursor.Last.height;
  srcbpl = 16;
  dstbpl = currentdsp->Display.width;
  op = 2; /* OR-in */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, 2, 0, 0, 0);
#else
  new_bitblt_code;
#endif /* NEWBITBLT */
}

dostaking_mouse_up(int newx, int newy)
{
  /* save hidden bitmap */

  register DLword *srcbase, *dstbase;
  static int sx, dx, w = 16, h = 16, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

  /* newx and newy are hotspot coordinates. */
  /* newx -= currentmouse->Cursor.Hotspot.x; */
  /* newy -= ( 15 - currentmouse->Cursor.Hotspot.y); */

  /* save image */
  srcbase = DisplayRegion68k + (newy * currentdsp->Display.width / 16);
  dstbase = currentmouse->Cursor.Savebitmap;
  sx = newx;
  dx = 0;

  if (currentdsp->Display.width < (newx + 16)) {
    currentmouse->Cursor.Last.width = w = currentdsp->Display.width - newx;
  } else {
    currentmouse->Cursor.Last.width = w = 16;
  };

  if (currentdsp->Display.height < (newy + 16)) {
    currentmouse->Cursor.Last.height = h = currentdsp->Display.height - newy;
  } else {
    currentmouse->Cursor.Last.height = h = 16;
  };

  srcbpl = currentdsp->Display.width;
  dstbpl = 16;
  op = 0; /* replace */

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, 0, gray, num_gray,
         curr_gray_line);
#else
  new_bitblt_code;
#endif /* NEWBITBLT */

  /* Copy Cursor Image */
  docopy(newx, newy);

  currentmouse->Cursor.Last.x = newx;
  currentmouse->Cursor.Last.y = newy;

  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, currentmouse->Cursor.Last.x,
                                 currentmouse->Cursor.Last.y, w, h);
}

dostaking_mouse_down(DspInterface dsp, IOPAGE *iop)
{
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

  /* restore saved image */
  srcbase = currentmouse->Cursor.Savebitmap;
  dstbase =
      DisplayRegion68k + ((currentmouse->Cursor.Last.y) * (dsp->Display.width / 16)); /* old y */
  sx = 0;
  dx = currentmouse->Cursor.Last.x; /* old x */
  w = currentmouse->Cursor.Last.width;
  h = currentmouse->Cursor.Last.height;
  srcbpl = 16;
  dstbpl = dsp->Display.width;
  op = 0;

#ifdef NEWBITBLT
  bitblt(srcbase, dstbase, sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, 0, 0, 0, 0);
#else
  new_bitblt_code;
#endif /* NEWBITBLT */

  (dsp->bitblt_to_screen)(dsp, DisplayRegion68k, currentmouse->Cursor.Last.x,
                          currentmouse->Cursor.Last.y, w, h);
}

/************************************************************************/
/*                                                                      */
/*                  M o u s e B u t t o n S i g n a l                   */
/*                                                                      */
/*  Tell LISP about a mouse event by putting an entry on the ring buf-  */
/*  fer of mouse/kbd events, with the new mouse-button state in it.     */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void MouseButtonSignal(MouseInterface mouse)
{
  DLword w, r;
  KBEVENT *kbevent;

  /* In the mouse device TRUE means button pressed */
  /* In the IOPage 0 means button pressed          */
  /* Hence the ! in the lines below.               */
  PUTBASEBIT68K(&(IOPage68K->dlutilin), MOUSE_LEFT, !mouse->Button.Left);
  PUTBASEBIT68K(&(IOPage68K->dlutilin), MOUSE_MIDDLE, !mouse->Button.Middle);
  PUTBASEBIT68K(&(IOPage68K->dlutilin), MOUSE_RIGHT, !mouse->Button.Right);

  r = CTopKeyevent->ring.vectorindex.read;
  w = CTopKeyevent->ring.vectorindex.write;

  if (r != w) {
    kbevent = (KBEVENT *)((DLword *)CTopKeyevent + w);

    /* Copy the Hardware bits. */
    kbevent->W0 = IOPage68K->dlkbdad0;
    kbevent->W1 = IOPage68K->dlkbdad1;
    kbevent->W2 = IOPage68K->dlkbdad2;
    kbevent->W3 = IOPage68K->dlkbdad3;
    kbevent->W4 = IOPage68K->dlkbdad4;
    kbevent->W5 = IOPage68K->dlkbdad5;
    kbevent->WU = IOPage68K->dlutilin;

    /* If queue was empty, update the read pointer */
    if (r == 0) CTopKeyevent->ring.vectorindex.read = w;

    /* Update the write pointer */
    if (w >= mouse->maxkeyevent)
      CTopKeyevent->ring.vectorindex.write = MINKEYEVENT;
    else
      CTopKeyevent->ring.vectorindex.write += mouse->keyeventsize;
  }

  if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;

  KBDEventFlg++; /* Signal the emulator to tell Lisp */
  Irq_Stk_Check = 0;
  Irq_Stk_End = 0;
}

/***************************************************************/
/*                      B u t t o n T i m e r                  */
/* This function is used with the mouse chording machinery.    */
/* This function is the timer interrupt handler. When a button */
/* event happens we will wait to report it until the next      */
/* timeout happens. We will thus obtain the ``rubbery feeling''*/
/* that proponents of chording so desire.                      */
/***************************************************************/
void ButtonTimer() {
  if (currentmouse->Button.RunTimer)
    if (currentmouse->Button.tick-- <= 0) {
      currentmouse->Button.RunTimer = FALSE; /* Turn the timer off. */
      currentmouse->Button.Left |= currentmouse->Button.StateLeft;
      currentmouse->Button.Right |= currentmouse->Button.StateRight;

      /* Mouse chording code. If at the end of the timeout
         the left and right buttons are down we signal middle
         button and bring the others up. */
      /* Are L & R down? */

      if (currentmouse->Button.StateLeft && currentmouse->Button.StateRight) {
        currentmouse->Button.Left = FALSE;
        currentmouse->Button.Right = FALSE;
        currentmouse->Button.Middle = TRUE;
      }

      currentmouse->Button.StateLeft = FALSE;
      currentmouse->Button.StateRight = FALSE;

      /* Did L & R go up after a simulated M */
      /*          if((currentmouse->Button.Middle &&
                            !(currentmouse->Button.StateLeft ||
                          currentmouse->Button.StateRight)))
                    {
                      currentmouse->Button.Left = FALSE;
                      currentmouse->Button.Right = FALSE;
                      currentmouse->Button.Middle = FALSE;
                    }
      */
      MouseButtonSignal(currentmouse);
    }
  _chain_intr(currentmouse->Button.NextHandler);
}

/***************************************************************/
/*                 T w o B u t t o n H a n d l e r             */
/* This function is ther interrupt handler for the mouse.      */
/* This function sets the state of the mouse structure and     */
/* signals the dispatch loop to care of the matter. This       */
/* akward solution is due to the severe braindamage in DOS.    */
/***************************************************************/
void TwoButtonHandler(void) {
  _XSTACK *stk_ptr;

  /* First save the stack frame. */
  stk_ptr = (_XSTACK *)_get_stk_frame(); /* Get ptr to V86 _XSTACK frame */
  stk_ptr->opts |= _STK_NOINT;           /* Bypass real-mode handler */

  if (!currentmouse->device.active) return;

  if (stk_ptr->eax & LB_PRESS)
    if (currentmouse->Button.RunTimer) /* Prior right-down seen... */
    {
      currentmouse->Button.RunTimer = FALSE;
      currentmouse->Button.FakeMiddle = TRUE;
      currentmouse->Button.Middle = TRUE;
      currentmouse->Button.StateLeft = TRUE;
      MouseButtonSignal(currentmouse);
    } else if (currentmouse->Button.Right) {
      currentmouse->Button.Left = TRUE;
      currentmouse->Button.StateLeft = TRUE;
      MouseButtonSignal(currentmouse);
    } else /* No other button down... */
    {
      currentmouse->Button.StateLeft = TRUE;
      currentmouse->Button.tick = currentmouse->Button.StartTime;
      currentmouse->Button.RunTimer = TRUE;
    }
  if (stk_ptr->eax & LB_OFF)           /* Left button released, and */
    if (currentmouse->Button.RunTimer) /* Timer had been running */
    {
      currentmouse->Button.RunTimer = FALSE;
      currentmouse->Button.Left = TRUE;
      MouseButtonSignal(currentmouse);
      currentmouse->Button.StateLeft = FALSE;
      /*  currentmouse->Button.Left = FALSE;
        MouseButtonSignal(currentmouse); */
    } else /* timer wasn't running */
    {
      currentmouse->Button.StateLeft = FALSE;
      currentmouse->Button.Left = FALSE;
      currentmouse->Button.FakeMiddle = FALSE;
      currentmouse->Button.Middle = currentmouse->Button.StateMiddle;
      MouseButtonSignal(currentmouse);
    }

  if ((stk_ptr->eax & CB_PRESS) || (stk_ptr->eax & CB_OFF)) {
    currentmouse->Button.Middle =
        ((stk_ptr->eax & CB_PRESS) && TRUE) || currentmouse->Button.FakeMiddle;
    currentmouse->Button.StateMiddle = (stk_ptr->eax & CB_PRESS) && TRUE;
    currentmouse->Button.RunTimer = FALSE;
    MouseButtonSignal(currentmouse);
  }

  if (stk_ptr->eax & RB_PRESS)         /* Right button pressed, and */
    if (currentmouse->Button.RunTimer) /* Timer was running... */
    {
      currentmouse->Button.RunTimer = FALSE;
      currentmouse->Button.FakeMiddle = TRUE;
      currentmouse->Button.Middle = TRUE;
      currentmouse->Button.StateRight = TRUE;
      MouseButtonSignal(currentmouse);
    } else if (currentmouse->Button.Left) {
      currentmouse->Button.Right = TRUE;
      currentmouse->Button.StateRight = TRUE;
      MouseButtonSignal(currentmouse);
    } else {
      currentmouse->Button.StateRight = TRUE;
      currentmouse->Button.tick = currentmouse->Button.StartTime;
      currentmouse->Button.RunTimer = TRUE;
    }
  if (stk_ptr->eax & RB_OFF)           /* Right button released */
    if (currentmouse->Button.RunTimer) /* Timer had been running */
    {
      currentmouse->Button.RunTimer = FALSE;
      currentmouse->Button.Right = TRUE;
      MouseButtonSignal(currentmouse);
      currentmouse->Button.StateRight = FALSE;
      /*  currentmouse->Button.Right = FALSE;
        MouseButtonSignal(currentmouse); */
    } else {
      currentmouse->Button.StateRight = FALSE;
      currentmouse->Button.Right = FALSE;
      currentmouse->Button.FakeMiddle = FALSE;
      currentmouse->Button.Middle = currentmouse->Button.StateMiddle;
      MouseButtonSignal(currentmouse);
    }

  /* The dude moved the mouse. Set the chordstate NOW. */
  /* And turn the timer off. */
  if ((stk_ptr->eax & MOUSE_MV) && (!currentdsp->device.locked)) {
    currentmouse->Button.RunTimer = FALSE;

    /* Are L & R down? */
    /*      if(currentmouse->Button.StateLeft && currentmouse->Button.StateRight)
              {
                currentmouse->Button.Left = FALSE;
                currentmouse->Button.Right = FALSE;
                currentmouse->Button.Middle = TRUE;
              }
    */
    /*   currentmouse->Cursor.New.x = (DLword)stk_ptr->ecx & 0xFFFF;
       currentmouse->Cursor.New.y = (DLword)stk_ptr->edx & 0xFFFF; */
    currentmouse->Cursor.Moved = TRUE;

    if (currentmouse->Button.StateLeft && currentmouse->Button.StateRight) {
      currentmouse->Button.Left = FALSE;
      currentmouse->Button.Right = FALSE;
      currentmouse->Button.Middle = TRUE;
    } else {
      currentmouse->Button.Left |= currentmouse->Button.StateLeft;
      currentmouse->Button.Right |= currentmouse->Button.StateRight;
    }

    currentmouse->Button.StateLeft = currentmouse->Button.StateRight = FALSE;
    MouseButtonSignal(currentmouse);
  }
}

/***************************************************************/
/*              T h r e e B u t t o n H a n d l e r            */
/* This function is ther interrupt handler for the mouse.      */
/* This function sets the state of the mouse structure and     */
/* signals the dispatch loop to care of the matter. This       */
/* akward solution is due to the severe braindamage in DOS.    */
/***************************************************************/
void ThreeButtonHandler()

{
  _XSTACK *stk_ptr;
  unsigned long mouse_flags;

  /* First save the stack frame. */
  stk_ptr = (_XSTACK *)_get_stk_frame(); /* Get ptr to V86 _XSTACK frame */
  stk_ptr->opts |= _STK_NOINT;           /* Bypass real-mode handler */

  if (currentmouse->device.active) {
    mouse_flags = stk_ptr->eax; /* Save event flags from mouse driver */

    /* Decode the transition bits. */
    if (mouse_flags & LB_PRESS) currentmouse->Button.Left = TRUE;
    if (mouse_flags & LB_OFF) currentmouse->Button.Left = FALSE;

    if (mouse_flags & CB_PRESS) currentmouse->Button.Middle = TRUE;
    if (mouse_flags & CB_OFF) currentmouse->Button.Middle = FALSE;

    if (mouse_flags & RB_PRESS) currentmouse->Button.Right = TRUE;
    if (mouse_flags & RB_OFF) currentmouse->Button.Right = FALSE;

    if ((!currentdsp->device.locked) && (mouse_flags & MOUSE_MV)) {
      currentmouse->Cursor.Moved = TRUE;
      Irq_Stk_Check = 0;
      Irq_Stk_End = 0;
    }

    if (mouse_flags & (LB_PRESS | LB_OFF | CB_PRESS | CB_OFF | RB_PRESS | RB_OFF))
      MouseButtonSignal(currentmouse);
  }
}
