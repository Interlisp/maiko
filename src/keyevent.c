/* $Id: keyevent.c,v 1.3 2001/12/24 01:09:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	This file contains the routines that interface Lisp to the
 *	Sun keyboard and mouse.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#if   SUNDISPLAY
#include <sunwindow/window_hs.h>
#include <sunwindow/win_ioctl.h>
#include <suntool/window.h>
#include <ctype.h>
#include <sundev/kbio.h>
#endif /* SUNDISPLAY */

#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "address.h"
#include "stack.h"
#include "keyboard.h"
#include "display.h"
#include "lsptypes.h"

#include "iopage.h"
#include "ifpage.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"

#include "keyeventdefs.h"
#include "osmsgdefs.h"
#include "xwinmandefs.h"

#ifdef MAIKO_ENABLE_ETHERNET
#include "etherdefs.h"
#endif /* MAIKO_ENABLE_ETHERNET */

#include "dbprint.h"
#if defined(XWINDOW)
#include "devif.h"
extern DspInterface currentdsp;
extern IOPAGE *IOPage68K;
#endif /* XWINDOW */

/* for contextsw */
#define AS_OPCODE 1
#define AS_CPROG 0

/*  EmMouseX68K are already swapped, no need for GETWORD */
#define MouseMove(x, y)           \
  {                               \
    *((DLword *)EmMouseX68K) = x; \
    *((DLword *)EmMouseY68K) = y; \
  }
#ifdef NEVER
#ifndef BYTESWAP
#define PUTBASEBIT68K(base68k, offset, bitvalue)               \
  {                                                            \
    if (bitvalue)                                              \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) |=   \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);    \
    else                                                       \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) &=   \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD)); \
  }
#else

/* convert to real 68 k address, then do arithmetic, and convert
   back to i386 address pointer */

#define PUTBASEBIT68K(base68k, offset, bitvalue)                       \
  {                                                                    \
    int real68kbase;                                                   \
    real68kbase = 2 ^ ((int)(base68k));                                \
    if (bitvalue)                                                      \
      GETWORD((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)) |= \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);            \
    else                                                               \
      GETWORD((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)) &= \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD));         \
  }
#endif
#endif /* NEVER */

extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K, *EmUtilin68K;
extern DLword *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K;
extern u_char *SUNLispKeyMap;
extern int LispWindowFd;
extern int RS232C_Fd, RS232C_remain_data, XLocked;
extern fd_set LispIOFds;
fd_set LispReadFds;
int XNeedSignal = 0; /* T if an X interrupt happened while XLOCK asserted */

extern int LogFileFd;

#ifdef MAIKO_ENABLE_ETHERNET
extern int ether_fd;
#endif /* MAIKO_ENABLE_ETHERNET */

extern DLword *DisplayRegion68k;

static struct timeval SelectTimeout = {0, 0};

#ifdef XWINDOW
extern volatile sig_atomic_t Event_Req;
#endif /* XWINDOW */

extern MISCSTATS *MiscStats;
LispPTR *LASTUSERACTION68k;
LispPTR *CLastUserActionCell68k;
LispPTR *CURSORDESTHEIGHT68k;
LispPTR *CURSORDESTWIDTH68k;

LispPTR *CURSORHOTSPOTX68k;
LispPTR *CURSORHOTSPOTY68k;
LispPTR *SOFTCURSORUPP68k;
LispPTR *SOFTCURSORWIDTH68k;
LispPTR *SOFTCURSORHEIGHT68k;
LispPTR *CURRENTCURSOR68k;

extern DLword *EmCursorX68K;
extern DLword *EmCursorY68K;

#ifndef BYTESWAP
typedef struct {
  unsigned nil : 8;
  unsigned type : 8;
  unsigned num : 16;
} SNum;
#else
typedef struct {
  unsigned num : 16;
  unsigned type : 8;
  unsigned nil : 8;
} SNum;

#endif /* BYTESWAP */

#define IDiff(x68k, y68k) (((SNum *)(x68k))->num - ((SNum *)(y68k))->num)

/*  EmXXXX68K are already swapped, no need for GETWORD */


#ifdef SUNDISPLAY
#ifdef OLD_CURSOR
#define TrackCursor(cx, cy)                          \
  {                                                  \
    *CLastUserActionCell68k = MiscStats->secondstmp; \
    *EmCursorX68K = cx;                              \
    *EmCursorY68K = cy;                              \
  }
#else
#define TrackCursor(cx, cy)                          \
  {                                                  \
    *CLastUserActionCell68k = MiscStats->secondstmp; \
    taking_mouse_down();                             \
    taking_mouse_up(cx, cy);                         \
    *EmCursorX68K = cx;                              \
    *EmCursorY68K = cy;                              \
  }
#endif /* OLD_CURSOR */
#endif /* SUNDISPLAY */

/* commented out is some code that would also clobber
        Irq_Stk_Check & Irq_Stk_End to force
        a new interrupt as rapidly as possible; it causes odd behavior...
        needs some study and thought */

/* this is currently called EVERY time the timer expires. It checks for
   keyboard input */

LispPTR *MOUSECHORDTICKS68k;

/**NEW GLOBAL***-> will be moved***/
LispPTR *KEYBOARDEVENTQUEUE68k;
LispPTR *KEYBUFFERING68k;
int KBDEventFlg = NIL;
DLword *CTopKeyevent;

LispPTR DOBUFFEREDTRANSITION_index;
LispPTR INTERRUPTFRAME_index;
LispPTR *TIMER_INTERRUPT_PENDING68k;
LispPTR *PENDINGINTERRUPT68k;
LispPTR ATOM_STARTED;
LispPTR *PERIODIC_INTERRUPT68k;
LispPTR *PERIODIC_INTERRUPT_FREQUENCY68k;
LispPTR PERIODIC_INTERRUPTFRAME_index;
LispPTR DORECLAIM_index;

LispPTR *IOINTERRUPTFLAGS_word;

int URaid_req = NIL;
int ScreenLocked = NIL;

DLword MonoCursor_savebitmap[CURSORHEIGHT]; /* for mono cursor save-image */
#define COLOR_DEPTH 8
#define COLORPIXELS_IN_LONGWORD 4
#define COLORPIXELS_IN_DLWORD 2
DLword ColorCursor_savebitmap[CURSORWIDTH / COLORPIXELS_IN_DLWORD * CURSORHEIGHT];

/************************************************************************/
/*									*/
/*			G E T S I G N A L D A T A			*/
/*									*/
/*	Handler for the SIGIO interrupt, which happens			*/
/*		1. When a key transition happens			*/
/*		2. On mouse moves					*/
/*		3. When TCP input becomes available.			*/
/*		4. When a NIT ethernet packet becomes available.	*/
/*		5. When a console/log/stderr msg needs to be printed.	*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*	Statics:  LispReadFds	A 32-bit vector with a 1 for each	*/
/*				FD that can get SIGIO interrupts.	*/
/*				12/04/2020 - now an fd_set		*/
/*									*/
/*		  LispWindowFd	The keyboard/window FD, for keyboard	*/
/*				and mouse events.			*/
/*									*/
/*		  LispIOFds	A bit vector of TCP FDs, or other	*/
/*				FDs Lisp is doing async I/O on.		*/
/*				Activity on any of these will signal	*/
/*				the Lisp IOInterrupt interrupt.		*/
/*				12/04/2020 - now an fd_set		*/
/*									*/
/*		  ether_fd	The raw ethernet FD, for 10MB I/O	*/
/*									*/
/*		  EtherReadFds	A bit vector with the raw enet's	*/
/*				bit turned on.  To speed up processing.	*/
/*				12/04/2020 - now obsolete		*/
/*									*/
/*		  LogFileFd	A bit vector with the log-file's	*/
/*				bit on, for capturing console msgs.	*/
/*				12/04/2020 - now just the FD number	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void getsignaldata(int sig)
{
#ifdef SUNDISPLAY
  struct inputevent event;
#endif /* SUNDISPLAY */
  fd_set rfds, efds;
  u_int iflags;
  int i;

#ifdef XWINDOW
#if defined(sun)
  if (Event_Req) {
    if (!XLocked++)
      getXsignaldata(currentdsp);
    else
      XNeedSignal = 1;
    Event_Req = FALSE;
    XLocked--;
  }
#endif
#endif /* XWINDOW */

  /* #ifndef KBINT */
  /* FD_COPY would be preferred but uses deprecated bcopy() on macOS.  Why? */
  memcpy(&rfds, &LispReadFds, sizeof(rfds));
  memcpy(&efds, &LispReadFds, sizeof(efds));

/* label and ifs not needed if only keyboard on SIGIO */
getmore:
  if (select(32, &rfds, NULL, &efds, &SelectTimeout) >= 0)
  {
      /* need to print out fd sets...
    DBPRINT(("SIGIO: fd mask(r/e) = 0x%x/0x%x.\n", rfds, efds));
      */

#ifdef SUNDISPLAY
    if (LispWindowFd >= 0 && FD_ISSET(LispWindowFd, &rfds)) {
      /* #endif */
      while (input_readevent(LispWindowFd, &event) >= 0) {
        /*if(!kb_event( &event )) {goto getmore;};*/
        if ((KBDEventFlg += kb_event(&event)) > 0) {
          /* immediately request for IRQ check */
          Irq_Stk_End = Irq_Stk_Check = 0;
        }
      }
      /* #ifndef KBINT */
    }
#endif /* SUNDISPLAY */

#ifdef XWINDOW
    if (FD_ISSET(ConnectionNumber(currentdsp->display_id), &rfds)) {
      if (!XLocked)
        getXsignaldata(currentdsp);
      else
        XNeedSignal = 1;
    }

#endif /* XWINDOW */

#ifdef MAIKO_ENABLE_ETHERNET
    if (ether_fd >= 0 && FD_ISSET(ether_fd, &rfds)) { /* Raw ethernet (NIT) I/O happened, so handle it. */
      DBPRINT(("Handling enet interrupt.\n\n"));
      check_ether();
    }
#endif /* MAIKO_ENABLE_ETHERNET */

#ifdef RS232
    if (RS232C_Fd >= 0 && (FD_ISSET(RS232C_Fd, &rfds) || (RS232C_remain_data && rs232c_lisp_is_ready())))
      rs232c_read();
#endif /* RS232 */

#if defined(MAIKO_HANDLE_CONSOLE_MESSAGES) && defined(LOGINT)
    if (LogFileFd >= 0 && FD_ISSET(LogFileFd, &rfds)) { /* There's info in the log file.  Tell Lisp to print it. */
      flush_pty();          /* move the msg(s) to the log file */

      ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->LogFileIO = 1;

      *PENDINGINTERRUPT68k = ATOM_T;
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
#endif
    iflags = 0;
    for (i = 0; i < 32; i++)
        if (FD_ISSET(i, &rfds) & FD_ISSET(i, &LispIOFds)) iflags |= 1 << i;
    if (iflags) { /* There's activity on a Lisp-opened FD.  Tell Lisp. */
      u_int *flags;
      flags = (u_int *)Addr68k_from_LADDR(*IOINTERRUPTFLAGS_word);
      *flags = iflags;

      ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->IOInterrupt = 1;

      *PENDINGINTERRUPT68k = ATOM_T;
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
  }
/* #endif */
} /* end getsignaldata */

#ifdef SUNDISPLAY
/************************************************************************/
/*									*/
/*			    k b _ e v e n t				*/
/*									*/
/*	Given an event from the kbd code, return 1 if a key transition	*/
/*	occurred, 0 if one didn't occur.				*/
/*									*/
/************************************************************************/
extern int for_makeinit;

int kb_event(struct inputevent *event);
{
  register u_int upflg;
  int kn;
  DLword w, r;
  KBEVENT *kbevent;

#ifdef INIT

  /* generate some code to check if we are running as an INIT.  Don't
     needlessly generate this code, and don't return if we aren't
     running with the -INIT flag turned on.  --was 2/7/89 */

  if (for_makeinit) { return (0); };

#endif

  upflg = event_is_up(event);

#ifdef SHOWKEYSTROKES
  printf("Key # %d, upflg %d.\n", (unsigned short)event->ie_code, upflg);
#endif

  switch (((unsigned short)event->ie_code)) {
    case LOC_MOVE:
#ifndef OLD_CURSOR
      if (!ScreenLocked)
#endif
      {
        ScreenLocked = T;
        MouseMove(event->ie_locx, event->ie_locy);
        TrackCursor(event->ie_locx, event->ie_locy);
        ScreenLocked = NIL;
      }
      return (0);

    case MS_LEFT: /*mouse_button( MOUSE_LEFT, upflg );*/
      PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, upflg);
      break;

    case MS_MIDDLE: /*mouse_button( MOUSE_MIDDLE, upflg );*/
      PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, upflg);
      break;

    case MS_RIGHT: /*mouse_button( MOUSE_RIGHT, upflg );*/
      PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, upflg);
      break;

    default: /* keystroke */
      if ((kn = SUNLispKeyMap[((unsigned short)event->ie_code)]) < 255)
        kb_trans(kn, upflg);
      else
        printf("kb_event: unknown key number=%d\n", event->ie_code);

      break;
  };
  {
  do_ring:
    /* Emxxx do not use GETWORD */
    if (((*EmKbdAd268K) & 2113) == 0) { /*Ctrl-shift-NEXT*/
      error("******  EMERGENCY Interrupt ******");
      (*EmKbdAd268K) = KB_ALLUP;        /*reset*/
      ((RING *)CTopKeyevent)->read = 0; /* reset queue */
      ((RING *)CTopKeyevent)->write = MINKEYEVENT;
      /*return(0);*/
    } else if (((*EmKbdAd268K) & 2114) == 0) { /* Ctrl-Shift-DEL */
      (*EmKbdAd268K) = KB_ALLUP;               /*reset*/
      URaid_req = T;
      ((RING *)CTopKeyevent)->read = 0; /* reset queue */
      ((RING *)CTopKeyevent)->write = MINKEYEVENT;

      /*return(0);*/
    }

#ifdef OS4_TYPE4BUG
    else if (((*EmKbdAd268K) & 2120) == 0) { /* Ctrl-Shift-Return */
      error("******  EMERGENCY Interrupt ******");
      (*EmKbdAd268K) = KB_ALLUP;        /*reset*/
      ((RING *)CTopKeyevent)->read = 0; /* reset queue */
      ((RING *)CTopKeyevent)->write = MINKEYEVENT;
      /*return(0);*/
    }
#endif

    r = RING_READ(CTopKeyevent);
    w = RING_WRITE(CTopKeyevent);

    if (r == w) goto KBnext; /* event queue FULL */

    kbevent = (KBEVENT *)(CTopKeyevent + w);

    /*	RCLK(kbevent->time); */

    kbevent->W0 = (*EmKbdAd068K); /* Emxxxx do not use GETWORD */
    kbevent->W1 = (*EmKbdAd168K);
    kbevent->W2 = (*EmKbdAd268K);
    kbevent->W3 = (*EmKbdAd368K);
    kbevent->W4 = (*EmKbdAd468K);
    kbevent->W5 = (*EmKbdAd568K);
    kbevent->WU = (*EmRealUtilin68K);

    if (r == 0) /* Queue was empty */
      ((RING *)CTopKeyevent)->read = w;
    if (w >= MAXKEYEVENT)
      ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    else
      ((RING *)CTopKeyevent)->write = w + KEYEVENTSIZE;

  KBnext:
    if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;

  } /* if *EmRealUtilin68K end */
  return (1);
}
#endif /* SUNDISPLAY */

/************************************************************************/
/*									*/
/*			   k b _ t r a n s				*/
/*									*/
/*	Return the transition code??					*/
/*									*/
/************************************************************************/

void kb_trans(u_short keycode, u_short upflg)
{
  extern IFPAGE *InterfacePage;
  if (keycode < 64) /* DLKBDAD0 ~ 3	*/
  {
    PUTBASEBIT68K(EmKbdAd068K, keycode, upflg);
  } else if (keycode >= 80) /* DLKBDAD4, 5	*/
  {
    PUTBASEBIT68K(EmKbdAd068K, keycode - 16, upflg);
  } else if (keycode >= 64 && keycode < 80) /* DLUTILIN	*/
  {
    PUTBASEBIT68K(EmRealUtilin68K, (keycode & 15), upflg);
    PUTBASEBIT68K(EmUtilin68K, (keycode & 15), upflg);
  }
}

/**********************************************************/
/*
        MOUSE tracking
*/
/**********************************************************/

typedef struct {
  LispPTR CUIMAGE;
  LispPTR CUMASK;
  LispPTR CUHOTSPOTX;
  LispPTR CUHOTSPOTY;
  LispPTR CUDATA;
} CURSOR;

#define CursorClippingX(posx, width)                         \
  {                                                          \
    if (displaywidth < ((posx) + HARD_CURSORWIDTH)) {        \
      LastCursorClippingX = (width) = displaywidth - (posx); \
    } else {                                                 \
      LastCursorClippingX = (width) = HARD_CURSORWIDTH;      \
    }                                                        \
  }

#define CursorClippingY(posy, height)                          \
  {                                                            \
    if (displayheight < ((posy) + HARD_CURSORHEIGHT)) {        \
      LastCursorClippingY = (height) = displayheight - (posy); \
    } else {                                                   \
      LastCursorClippingY = (height) = HARD_CURSORHEIGHT;      \
    }                                                          \
  }

extern int displaywidth, displayheight;
extern int DisplayInitialized;
extern int MonoOrColor; /* MONO_SCREEN or COLOR_SCREEN */
int LastCursorClippingX = HARD_CURSORWIDTH;
int LastCursorClippingY = HARD_CURSORHEIGHT;
int LastCursorX = 0;
int LastCursorY = 0;

void cursor_hidden_bitmap(int, int);

#ifndef COLOR
/* FOR MONO ONLY */
void taking_mouse_down() {
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

  if (!DisplayInitialized) return;

  /* restore saved image */
  srcbase = MonoCursor_savebitmap;
  dstbase = DisplayRegion68k + ((LastCursorY)*DLWORD_PERLINE); /* old y */
  sx = 0;
  dx = LastCursorX;        /* old x */
  w = LastCursorClippingX; /* Old clipping */
  h = LastCursorClippingY;
  srcbpl = HARD_CURSORWIDTH;
  dstbpl = displaywidth;
  op = 0;
  new_bitblt_code;
#ifdef DISPLAYBUFFER
  flush_display_region(dx, (LastCursorY), w, h);
#endif /* DISPLAYBUFFER */
}
#else

/* For COLOR & MONO */
extern DLword *ColorDisplayRegion68k;
/* It assumes that MONO screen size and COLOR screen size are identical */
void taking_mouse_down() {
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

  if (!DisplayInitialized) return;
  /* restore saved image */
  sx = 0;

  if (MonoOrColor == MONO_SCREEN) {
    dx = LastCursorX; /* old x */
    srcbase = MonoCursor_savebitmap;
    dstbase = DisplayRegion68k + ((LastCursorY)*DLWORD_PERLINE); /* old y */
    w = LastCursorClippingX;                                     /* Old clipping */
    h = LastCursorClippingY;
    srcbpl = HARD_CURSORWIDTH;
    dstbpl = displaywidth;
  } else {
    dx = LastCursorX * COLOR_BITSPER_PIXEL; /* old x */
    srcbase = ColorCursor_savebitmap;
    dstbase =
        ColorDisplayRegion68k + ((LastCursorY)*DLWORD_PERLINE * COLOR_BITSPER_PIXEL); /* old y */
    w = LastCursorClippingX * COLOR_BITSPER_PIXEL; /* Old clipping */
    h = LastCursorClippingY;
    srcbpl = HARD_CURSORWIDTH * COLOR_BITSPER_PIXEL;
    dstbpl = displaywidth * COLOR_BITSPER_PIXEL;
  }
  op = 0;
  new_bitblt_code;
#ifdef DISPLAYBUFFER
  if (MonoOrColor == MONO_SCREEN) flush_display_region(dx, LastCursorY, w, h);
#endif
}
#endif /* COLOR */

/* LastCursorClippingX must be set before calling
 To avoid duplicate calculation */
#ifndef COLOR
/* FOR MONO ONLY */
void copy_cursor(int newx, int newy)
{
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;
  extern DLword *EmCursorBitMap68K;
  /* copy cursor image */
  srcbase = EmCursorBitMap68K;
  dstbase = DisplayRegion68k + (newy * DLWORD_PERLINE);
  sx = 0;
  dx = newx;
  w = LastCursorClippingX;
  h = LastCursorClippingY;
  ;
  srcbpl = HARD_CURSORWIDTH;
  dstbpl = displaywidth;
  op = 2; /* OR-in */
  new_bitblt_code;
#ifdef DISPLAYBUFFER
  flush_display_region(dx, newy, w, h);
#endif
}

/* store bitmap image inside rect. which specified by x,y */
void cursor_hidden_bitmap(int x, int y)
{
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;
  /* save image */
  srcbase = DisplayRegion68k + (y * DLWORD_PERLINE);
  dstbase = MonoCursor_savebitmap;
  sx = x;
  dx = 0;
  CursorClippingX(x, w); /* w and LastCursorClippingX rest */
  CursorClippingY(y, h); /* h and LastCursorClippingY reset */
  srcbpl = displaywidth;
  dstbpl = HARD_CURSORWIDTH;
  op = 0; /* replace */
  new_bitblt_code;
}

#else
/* For COLOR & MONO */
#define IMIN(x, y) (((x) > (y)) ? (y) : (x))
void copy_cursor(int newx, int newy)
{
  register DLword *srcbase, *dstbase;
  register int offsetx, offsety;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;
  CURSOR *cursor68k;
  BITMAP *bitmap68k;
  extern DLword *EmCursorBitMap68K;
  /* copy cursor image */
  if (MonoOrColor == MONO_SCREEN) {
    srcbase = EmCursorBitMap68K;
    dstbase = DisplayRegion68k + (newy * DLWORD_PERLINE);
    sx = 0;
    dx = newx;
    w = LastCursorClippingX;
    h = LastCursorClippingY;
    ;
    srcbpl = HARD_CURSORWIDTH;
    dstbpl = displaywidth;
  } else {
    cursor68k = (CURSOR *)Addr68k_from_LADDR(*CURRENTCURSOR68k);
    bitmap68k = (BITMAP *)Addr68k_from_LADDR(cursor68k->CUIMAGE);
    srcbase = (DLword *)Addr68k_from_LADDR(bitmap68k->bmbase);
    dstbase = ColorDisplayRegion68k + (newy * DLWORD_PERLINE * COLOR_BITSPER_PIXEL);
    sx = 0;
    dx = newx * COLOR_BITSPER_PIXEL;
    w = IMIN(LastCursorClippingX, LOLOC(bitmap68k->bmwidth)) * COLOR_BITSPER_PIXEL;
    h = IMIN(LastCursorClippingY, LOLOC(bitmap68k->bmheight));
    /* srcbpl=HARD_CURSORWIDTH * COLOR_BITSPER_PIXEL;*/
    srcbpl = bitmap68k->bmwidth * COLOR_BITSPER_PIXEL;
    dstbpl = displaywidth * COLOR_BITSPER_PIXEL;
  }
  op = 2; /* OR-in */
  new_bitblt_code;
#ifdef DISPLAYBUFFER
  if (MonoOrColor == MONO_SCREEN) flush_display_region(dx, newy, w, h);
#endif
}

/* I'll make it MACRO */
void taking_mouse_up(int newx, int newy)
{
  if (!DisplayInitialized) return;
  /* save hidden bitmap */
  cursor_hidden_bitmap(newx, newy);
/* Copy Cursor Image */
#ifndef INIT
  copy_cursor(newx, newy);
#endif
  LastCursorX = newx;
  LastCursorY = newy;
}

/* store bitmap image inside rect. which specified by x,y */
void cursor_hidden_bitmap(int x, int y)
{
  register DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;
  /* save image */
  if (MonoOrColor == MONO_SCREEN) {
    srcbase = DisplayRegion68k + (y * DLWORD_PERLINE);
    dstbase = MonoCursor_savebitmap;
    sx = x;
    dx = 0;
    CursorClippingX(x, w); /* w and LastCursorClippingX rest */
    CursorClippingY(y, h); /* h and LastCursorClippingY reset */
    srcbpl = displaywidth;
    dstbpl = HARD_CURSORWIDTH;
  } else {
    srcbase = ColorDisplayRegion68k + (y * DLWORD_PERLINE * COLOR_BITSPER_PIXEL);
    dstbase = ColorCursor_savebitmap;
    sx = x * COLOR_BITSPER_PIXEL;
    dx = 0;
    CursorClippingX(x, w); /* w and LastCursorClippingX rest */
    CursorClippingY(y, h); /* h and LastCursorClippingY reset */
    w = w * COLOR_BITSPER_PIXEL;
    srcbpl = displaywidth * COLOR_BITSPER_PIXEL;
    dstbpl = HARD_CURSORWIDTH * COLOR_BITSPER_PIXEL;
  }
  op = 0; /* replace */
  new_bitblt_code;
}
#endif /* COLOR */
