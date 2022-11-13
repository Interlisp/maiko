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
#ifndef DOS
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#else
#include <time.h>
#endif /* DOS */
#ifdef DOS

#include <i32.h> /* Defines "#pragma interrupt"  */
#include <dos.h> /* Defines REGS & other structs */
#include <stk.h> /* _XSTACK struct definition    */
#pragma interrupt(Mouse_hndlr)

void Mouse_hndlr(void); /* Fields mouse events from driver        */
                        /*  (during servicing of mouse interrupt) */

#endif /* DOS */

#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "address.h"
#include "stack.h"
#include "initdspdefs.h"
#include "keyboard.h"
#include "display.h"
#include "lsptypes.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"

#include "keyeventdefs.h"
#include "osmsgdefs.h"
#ifdef XWINDOW
#include "xwinmandefs.h"
#endif

#if defined(MAIKO_ENABLE_ETHERNET) || defined(MAIKO_ENABLE_NETHUB)
#include "etherdefs.h"
#endif /* MAIKO_ENABLE_ETHERNET or MAIKO_ENABLE_NETHUB */

#include "dbprint.h"
#if (defined(DOS) || defined(XWINDOW))
#include "devif.h"
extern DspInterface currentdsp;
#endif /* DOS */

/* for contextsw */
#define AS_OPCODE 1
#define AS_CPROG 0

/*  EmMouseX68K are already swapped, no need for GETWORD */
#define MouseMove(x, y)           \
  do {                               \
    *((DLword *)EmMouseX68K) = x; \
    *((DLword *)EmMouseY68K) = y; \
  } while (0)
#ifdef NEVER
#ifndef BYTESWAP
#define PUTBASEBIT68K(base68k, offset, bitvalue)               \
  do {                                                            \
    if (bitvalue)                                              \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) |=   \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);    \
    else                                                       \
      *((DLword *)(base68k) + (((u_short)(offset)) >> 4)) &=   \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD)); \
  } while (0)
#else

/* convert to real 68 k address, then do arithmetic, and convert
   back to i386 address pointer */

#define PUTBASEBIT68K(base68k, offset, bitvalue)                       \
  do {                                                                    \
    int real68kbase;                                                   \
    real68kbase = 2 ^ ((int)(base68k));                                \
    if (bitvalue)                                                      \
      GETWORD((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)) |= \
          1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD);            \
    else                                                               \
      GETWORD((DLword *)(real68kbase) + (((u_short)(offset)) >> 4)) &= \
          ~(1 << (15 - ((u_short)(offset)) % BITSPER_DLWORD));         \
  } while (0)
#endif
#endif /* NEVER */

extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K, *EmUtilin68K;
extern DLword *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K;
extern u_char *SUNLispKeyMap;
extern int RS232C_Fd, RS232C_remain_data;
extern fd_set LispIOFds;
fd_set LispReadFds;
extern volatile sig_atomic_t XLocked;
extern volatile sig_atomic_t XNeedSignal;

extern int LogFileFd;

#if defined(MAIKO_ENABLE_ETHERNET) || defined(MAIKO_ENABLE_NETHUB)
extern int ether_fd;
#endif /* MAIKO_ENABLE_ETHERNET or MAIKO_ENABLE_NETHUB */

extern DLword *DisplayRegion68k;

#ifndef DOS
static struct timeval SelectTimeout = {0, 0};
#endif /* DOS */

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

/*  EmXXXX68K are already swapped, no need for GETWORD */



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
int KBDEventFlg = 0;
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
/*			p r o c e s s _ i o _ e v e n t s		*/
/*									*/
/*	Periodically, or After a SIGIO interrupt which happens		*/
/*		1. When TCP input becomes available.			*/
/*		2. When a NIT ethernet packet becomes available.	*/
/*		3. When a console/log/stderr msg needs to be printed.	*/
/*									*/
/*									*/
/*	Statics:  LispReadFds	A 32-bit vector with a 1 for each	*/
/*				FD that can get SIGIO interrupts.	*/
/*				12/04/2020 - now an fd_set		*/
/*									*/
/*		  LispWindowFd	The keyboard/window FD, for keyboard	*/
/*				and mouse events.			*/
/*				01/07/2023 - unused, obsolete		*/
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

void process_io_events(void)
{
#ifndef DOS
  fd_set rfds;
  u_int iflags;
  int i;

  memcpy(&rfds, &LispReadFds, sizeof(rfds));

  if (select(32, &rfds, NULL, NULL, &SelectTimeout) > 0) {

#ifdef MAIKO_ENABLE_ETHERNET
    if (ether_fd >= 0 && FD_ISSET(ether_fd, &rfds)) { /* Raw ethernet (NIT) I/O happened, so handle it. */
      DBPRINT(("Handling enet interrupt.\n\n"));
      check_ether();
    }
#endif /* MAIKO_ENABLE_ETHERNET */

#ifdef MAIKO_ENABLE_NETHUB
    check_ether();
#endif /* MAIKO_ENABLE_NETHUB */

#ifdef RS232
    if (RS232C_Fd >= 0 && (FD_ISSET(RS232C_Fd, &rfds) || (RS232C_remain_data && rs232c_lisp_is_ready())))
      rs232c_read();
#endif /* RS232 */

#if defined(MAIKO_HANDLE_CONSOLE_MESSAGES) && defined(LOGINT)
    if (LogFileFd >= 0 && FD_ISSET(LogFileFd, &rfds)) { /* There's info in the log file.  Tell Lisp to print it. */
      flush_pty();          /* move the msg(s) to the log file */

      ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->LogFileIO = 1;

      *PENDINGINTERRUPT68k = ATOM_T;
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
#endif
    iflags = 0;
    for (i = 0; i < 32; i++)
        if (FD_ISSET(i, &rfds) & FD_ISSET(i, &LispIOFds)) iflags |= 1 << i;
    if (iflags) { /* There's activity on a Lisp-opened FD.  Tell Lisp. */
      u_int *flags;
      flags = (u_int *)NativeAligned4FromLAddr(*IOINTERRUPTFLAGS_word);
      *flags = iflags;

      ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->IOInterrupt = 1;

      *PENDINGINTERRUPT68k = ATOM_T;
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
  }
/* #endif */
#endif /* DOS */
} /* end process_io_events */


/************************************************************************/
/*									*/
/*			   k b _ t r a n s				*/
/*									*/
/*	Return the transition code??					*/
/*									*/
/************************************************************************/

void kb_trans(u_short keycode, u_short upflg)
{
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
  do {                                                       \
    if (displaywidth < ((posx) + HARD_CURSORWIDTH)) {        \
      LastCursorClippingX = (width) = displaywidth - (posx); \
    } else {                                                 \
      LastCursorClippingX = (width) = HARD_CURSORWIDTH;      \
    }                                                        \
  } while (0)

#define CursorClippingY(posy, height)                          \
  do {                                                         \
    if (displayheight < ((posy) + HARD_CURSORHEIGHT)) {        \
      LastCursorClippingY = (height) = displayheight - (posy); \
    } else {                                                   \
      LastCursorClippingY = (height) = HARD_CURSORHEIGHT;      \
    }                                                          \
  } while (0)

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
void taking_mouse_down(void) {
  DLword *srcbase, *dstbase;
  static int sx, dx, w, h, srcbpl, dstbpl, backwardflg = 0;
  static int src_comp = 0, op = 0, gray = 0, num_gray = 0, curr_gray_line = 0;

#ifdef DOS
  (currentdsp->mouse_invisible)(currentdsp, IOPage);
#else
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
#endif /* DOS */
}
#else

/* For COLOR & MONO */
extern DLword *ColorDisplayRegion68k;
/* It assumes that MONO screen size and COLOR screen size are identical */
void taking_mouse_down(void) {
  DLword *srcbase, *dstbase;
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
  DLword *srcbase, *dstbase;
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
  DLword *srcbase, *dstbase;
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
  DLword *srcbase, *dstbase;
  int offsetx, offsety;
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
    cursor68k = (CURSOR *)NativeAligned4FromLAddr(*CURRENTCURSOR68k);
    bitmap68k = (BITMAP *)NativeAligned4FromLAddr(cursor68k->CUIMAGE);
    srcbase = (DLword *)NativeAligned2FromLAddr(bitmap68k->bmbase);
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
#ifdef DOS
  (currentdsp->mouse_visible)(newx, newy);
#else
  if (!DisplayInitialized) return;
  /* save hidden bitmap */
  cursor_hidden_bitmap(newx, newy);
/* Copy Cursor Image */
#ifndef INIT
  copy_cursor(newx, newy);
#endif
  LastCursorX = newx;
  LastCursorY = newy;
#endif
}

/* store bitmap image inside rect. which specified by x,y */
void cursor_hidden_bitmap(int x, int y)
{
  DLword *srcbase, *dstbase;
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
