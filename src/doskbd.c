/* $Id: doskbd.c,v 1.2 1999/01/03 02:06:55 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
/************************************************************************/
/*                                                                      */
/*                D O S   K E Y B O A R D   H A N D L E R               */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <dos.h> /* defines REGS & other structs       */
#include <i32.h> /* "#pragma interrupt" & '_chain_intr'*/
#include <stk.h>
#include <conio.h>

#include "lispemul.h"
#include "keyboard.h"
#include "keysym.h"
#include "devif.h"

int nokbdflag = FALSE;
extern int eurokbd;
extern KbdInterface currentkbd;
extern MouseInterface currentmouse;
extern IOPAGE *IOPage68K;
extern IFPAGE *InterfacePage;
extern int KBDEventFlg;

extern keybuffer *CTopKeyevent;

extern LispPTR *LASTUSERACTION68k;
extern LispPTR *KEYBUFFERING68k;

/************************************************/
/*  Keyboard-Interface Registers, Status Codes  */
/************************************************/
#define KBD_COMMAND_PORT 0x64 /* I/O port commands go out on */
#define KBD_ENABLE 0xAE
#define KBD_DISABLE 0xAD
#define KBD_RESET 0xF6

#define PORT_8042 0x60 /* Port scan codes come in on */
#define KBD_SCAN_CODE_PORT 0x60
#define KBD_resend 0xFE        /* KBD asked for resend */
#define KBD_ack 0xFA           /* KBD ack's our command */
#define KBD_echo_response 0xEE /* KBD responds to echo req */
#define KBD_failure 0xFD       /* Failure code */
#define KBD_prefix 0xE0        /* Prefix for extended chars */
#define KBD_pause_prefix 0xE1  /* Pause prefix?? */
#define KBD_overflow 0x00      /* Overflow of some kind */
#define KBD_overrun 0xFF       /* KBD buffer overrun */

#define KBD_STATUS_PORT 0x64 /* Port KBD status comes in on */
#define KBD_INP_FULL 0x02    /* input buffer full */

#define INTA00 0x20 /* The 8259 port, to reset irq */
#define ENDOFINTERRUPT 0x20

#define PRTSC_KEY 0x37 /* Keyboard codes for extended chars */
#define HOME_KEY 0x47
#define UPARROW_KEY 0x48
#define PGUP_KEY 0x49
#define LEFTARROW_KEY 0x4b
#define RIGHTARROW_KEY 0x4d
#define END_KEY 0x4f
#define DOWNARROW_KEY 0x50
#define PGDOWN_KEY 0x51
#define INS_KEY 0x52
#define DEL_KEY 0x53

/******************************************************/
/*  Tell the Interrupt-dispatch IC we're done, and    */
/*  Tell the keyboard itself that we're ready again.  */
/*                                                    */
/*  (This process is critical to the proper function  */
/*  of the handler, so let's do it once, correctly.)  */
/******************************************************/
#define ENABLE_KBD                                               \
  do {                                                              \
    outp(INTA00, ENDOFINTERRUPT);                                \
    outp(KBD_COMMAND_PORT, KBD_ENABLE); /* Turn kbd on again. */ \
  } while (0)

/************************************************************************/
/*                                                                      */
/*                     K B D _ E V E N T                                */
/*                                                                      */
/*      Keyboard interrupt handler routine                              */
/************************************************************************/
extern DLword *DisplayRegion68k;

#pragma interrupt(Kbd_event)

void Kbd_event() {
  _XSTACK *ebp; /* Real-mode handler stack frame */
  DLword w, r;
  KBEVENT *kbevent;
  unsigned char keycode, tmpkey;

  ebp = (_XSTACK *)_get_stk_frame(); /* Get stack frame address */
  ebp->opts |= _STK_NOINT;           /* Bypass real-mode handler */

  /*************************************************/
  /*  First, get the scan code from the keyboard.  */
  /*  Handle exceptional conditions & errors, and  */
  /*  the extended-character prefix, 0xE0          */
  /*  generated for, e.g., the INSERT key.         */
  /*************************************************/

  _disable();                          /* prevent further interrupts from killing us */
  outp(KBD_COMMAND_PORT, KBD_DISABLE); /* Turn the kbd off. */
  do { tmpkey = inp(KBD_STATUS_PORT); } while (tmpkey & KBD_INP_FULL);

  /* Finite state machine that either returns or goes to label handle: */
  switch (tmpkey = inp(KBD_SCAN_CODE_PORT)) {
    case KBD_overflow: /* Ignore these. */
    case KBD_pause_prefix:
    case KBD_echo_response:
    case KBD_ack:
    case KBD_failure:
    case KBD_resend:
    case KBD_overrun:
      ENABLE_KBD;
      return;
      break;

    case KBD_prefix: /* It's a prefix, so really use next char. */
      /* Remember that we saw the prefix: */
      currentkbd->lastbyte = tmpkey;
      ENABLE_KBD;
      return;
      break;

    default:
      tmpkey = inp(KBD_SCAN_CODE_PORT);
      if (currentkbd->lastbyte == KBD_prefix)
        switch (tmpkey) /* deal with prefixed characters */
        {
          case 0x2A: /* by ignoring some (what are they??) */
          case 0xAA:
          case 0xB6:
          case 0x36:
            ENABLE_KBD;
            return;
            break;

          default:                         /* and passing the rest thru as-is */
            currentkbd->lastbyte = tmpkey; /*  Set the state. */
            goto handle;
            break;
        }
      else {
        currentkbd->lastbyte = tmpkey; /*  Set the state. */
        goto handle;
      }
  }

  return; /* Don't have anything to handle yet, so just return */

/*****************************************************/
/*  Second, translate the scan code into a LISP key  */
/*  transition, add it to the ring buffer, and set   */
/*  the interrupt-request flags so lisp sees it.     */
/*****************************************************/
handle:
  /* The upflag is the eight bit in the char ie. upflag = currentkbd->lastbyte >> 7 */
  /* The event is the lower seven bits of the byte */

  keycode = currentkbd->KeyMap[currentkbd->lastbyte & 0x7f];

  if (keycode != 0xff) {
    if (keycode < 64) {
      PUTBASEBIT68K(&(IOPage68K->dlkbdad0), keycode, (currentkbd->lastbyte >> 7) & 1);
    } else if (keycode >= 80) {
      PUTBASEBIT68K(&(IOPage68K->dlkbdad0), keycode - 16, (currentkbd->lastbyte >> 7) & 1);
    } else {
      PUTBASEBIT68K(&(IOPage68K->dlutilin), (keycode & 0xf), (currentkbd->lastbyte >> 7) & 1);
      PUTBASEBIT68K(&(InterfacePage->fakemousebits), (keycode & 0xf),
                    (currentkbd->lastbyte >> 7) & 1);
    }
  }

  /* In DOS we can't enter uraid inside an exception handler. */
  /* Uraid may touch a swapped out address and that dumps Medley */
  if (((IOPage68K->dlkbdad2 & 2113) == 0) || /* Ctrl-shift-NEXT */
      ((IOPage68K->dlkbdad2 & 2114) == 0)) { /* Ctrl-shift-DEL */
    currentkbd->URaid = TRUE;                /* Tell the dispatch loop about it. */
    return;
  }

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
    if (w >= currentkbd->maxkeyevent)
      CTopKeyevent->ring.vectorindex.write = MINKEYEVENT;
    else
      CTopKeyevent->ring.vectorindex.write += currentkbd->keyeventsize;
  }
  if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;

  KBDEventFlg++;
  Irq_Stk_End = 0;
  Irq_Stk_Check = 0;

  ENABLE_KBD;
  return;
}

extern u_char DOSLispKeyMap_101[];

/************************************************************************/
/*                                                                      */
/*                      E X I T   D O S   K B D                         */
/*                                                                      */
/*      Turn off the DOS keyboard handler, and reinstall the            */
/*      normal DOS handler.                                             */
/************************************************************************/

void ExitDosKbd(KbdInterface kbd)
{
  if (kbd->device.active == TRUE) {
    kbd->device.active = FALSE;

    _dpmi_unlockregion((void *)&currentkbd, sizeof(currentkbd));
    _dpmi_unlockregion((void *)kbd, sizeof(*kbd));
    _dpmi_unlockregion((void *)&InterfacePage, sizeof(InterfacePage));
    _dpmi_unlockregion((void *)InterfacePage, sizeof(IFPAGE));
    _dpmi_unlockregion((void *)&IOPage68K, sizeof(IOPage68K));
    _dpmi_unlockregion((void *)IOPage68K, sizeof(IOPAGE));

    _dpmi_unlockregion((void *)&CTopKeyevent, sizeof(CTopKeyevent));
    _dpmi_unlockregion((void *)CTopKeyevent, sizeof(*CTopKeyevent));

    _dpmi_unlockregion((void *)&MachineState, sizeof(MachineState));
    _dpmi_unlockregion((void *)&KEYBUFFERING68k, sizeof(KEYBUFFERING68k));
    _dpmi_unlockregion((void *)&ExitDosKbd, 4096);
    _dpmi_unlockregion((void *)&Kbd_event, 4096);

    _dos_setvect(0x09, kbd->prev_handler); /* unhook our handlr, install previous*/
  }
}

/************************************************************************/
/*                                                                      */
/*                      E N T E R D O S K B D                           */
/*                                                                      */
/*      Turn on the DOS keyboard device.                                */
/*                                                                      */
/************************************************************************/

void EnterDosKbd(KbdInterface kbd)
{
  int i;

  if (kbd->device.active == FALSE) {
    kbd->device.active = TRUE;
    for (i = 0; i < 0x80; i++) kbd->KeyMap[i] = DOSLispKeyMap_101[i];

    if (eurokbd)
      kbd->keyeventsize = EUROKEYEVENTSIZE;
    else
      kbd->keyeventsize = NOEUROKEYEVENTSIZE;
    kbd->eurokbd = eurokbd;

    /* Offset of the end of the ring buffer */
    kbd->maxkeyevent = (MINKEYEVENT + (NUMBEROFKEYEVENTS * kbd->keyeventsize));

    _dpmi_lockregion((void *)&currentkbd, sizeof(currentkbd));
    _dpmi_lockregion((void *)kbd, sizeof(*kbd));
    _dpmi_lockregion((void *)&InterfacePage, sizeof(InterfacePage));
    _dpmi_lockregion((void *)InterfacePage, sizeof(IFPAGE));
    _dpmi_lockregion((void *)&IOPage68K, sizeof(IOPage68K));
    _dpmi_lockregion((void *)IOPage68K, sizeof(IOPAGE));
    _dpmi_lockregion((void *)&MachineState, sizeof(MachineState));

    _dpmi_lockregion((void *)&CTopKeyevent, sizeof(CTopKeyevent));
    _dpmi_lockregion((void *)CTopKeyevent, sizeof(*CTopKeyevent));

    _dpmi_lockregion((void *)&KEYBUFFERING68k, sizeof(KEYBUFFERING68k));
    _dpmi_lockregion((void *)&ExitDosKbd, 4096);
    _dpmi_lockregion((void *)&Kbd_event, 4096);

    /* Don't hook in our handler if the user flagged he wants to run */
    /* without a kbd. */
    if (!nokbdflag) {
      kbd->prev_handler = _dos_getvect(0x09); /* get addr of current 09 hndlr */
      _dos_setvect(0x09, kbd->device_event);  /* hook our int handler to interrupt  */
    }
  }
}
