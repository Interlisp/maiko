/* $Id: vesainit.c,v 1.2 1999/01/03 02:07:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: vesainit.c,v 1.2 1999/01/03 02:07:44 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1992, 1993, 1994, 1995 Venue. All Rights Reserved.			*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*			    V E S A I N I T . C				*/
/*									*/
/*	Using VESA calls, initialize the (S)VGA for Medley's use.	*/
/*									*/
/************************************************************************/

#include <i32.h> /* #pragma interrupt & _get_stk_frame */
#include <errno.h>
#include <stk.h> /* _XSTACK struct definition          */
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <graph.h>

#include "dbprint.h"
#include "lispemul.h"
#include "devif.h"

#define VESA 0x4f
#define SUCESS 0x00
/* VESA functions */
#define _DESCRIBEMODE 0x1
#define _SETMODE 0x2
#define _GETCURRENTMODE 0x3

/* VESA modevector indexes */
#define _DISPLAYWIDTH 0x12
#define _DISPLAYHEIGHT 0x14
#define _COLORPLANES 0x18
#define _BITSPERPIXEL 0x19

#define VESA_MODE_SUPPORTED_P(vector) ((((short *)vector)[0] & 1) ? TRUE : FALSE)
#define VESA_OPT_INFO_P(vector) ((((short *)vector)[0] & 2) ? TRUE : FALSE)
#define VESA_COLOR_MODE_P(vector) ((((short *)vector)[0] & 4) ? TRUE : FALSE)
#define VESA_GRAPHICS_MODE_P(vector) ((((short *)vector)[0] & 8) ? TRUE : FALSE)
#define VESA_SWITCH_BANK(vector) ((PFV)(((long *)vector)[3]))
#define VESA_DSP_SEGSIZE(vector) ((long)(0xffff & ((short *)vector)[3]))
#define VESA_DSP_STARTSEG_A(vector) ((long)(0xffff & ((short *)vector)[4]))
#define VESA_DSP_WIDTH(vector) ((long)(((short *)vector)[9]))
#define VESA_DSP_HEIGHT(vector) ((long)(((short *)vector)[10]))
#define VESA_DSP_COLORS(vector) ((long)(1 << (((char *)vector)[0x19])))
#define VESA_DSP_BPP(vector) ((long)(((char *)vector)[0x19]))
#define VESA_DSP_NUM_OF_BANKS(vector) ((long)(((char *)vector)[0x1a]))
#define VESA_DSP_BANK_SIZE(vector) ((long)(((char *)vector)[0x1c]))

#define MAKE_SEG(low_flat) ((FP_OFF(low_flat) & 0xfffffff0) >> 4)
#define MAKE_OFF(low_flat) (FP_OFF(low_flat & 0x0000000f))

#pragma interrupt(VESA_Intrpt_Hndlr) /* int 0x10 intercept                 */

extern DLword *DisplayRegion68k;
extern DLword *DisplayRegion68k_end_addr;
extern DspInterface currentdsp;
extern void docopy();

extern PFUL GenericReturnT();
extern void GenericPanic();
extern unsigned long VGA_not_color();
extern void VGA_exit();
extern unsigned long Dosbbt1();
extern unsigned long Dosbbt2();
extern unsigned long Dosbbt3();
extern void Dosclearbanks();
extern long DOSCursorVisible();
extern long dos_cursor_invisible();
extern int dostaking_mouse_down();
extern int dostaking_mouse_up();

void VESA_Intrpt_Hndlr(void);
void *VESA_prev_hndlr; /* addr of previous 0x10 intercept      */

extern int dosdisplaymode;

/**************************************************************/
/*                      V E S A _ c a l l                     */
/* General utility function for doing a BIOS call to the VESA */
/* bios.                                                      */
/**************************************************************/
int VESA_call(int class, int subfunc)
{
  union REGS inregs, outregs;

  inregs.h.ah = VESA;
  inregs.h.al = class;
  inregs.x.bx = subfunc;
  int86(0x10, &inregs, &outregs);
  return ((outregs.h.al == VESA) && (outregs.h.ah == SUCESS));
}

unsigned long set_DAC_color(LispPTR args[])
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x1010;
  inregs.x.bx = (unsigned char)(args[0] & 0xff);
  inregs.h.dh = (unsigned char)(args[1] & 0xff);
  inregs.h.ch = (unsigned char)(args[2] & 0xff);
  inregs.h.cl = (unsigned char)(args[3] & 0xff);
  int86(0x10, &inregs, &outregs);
}

/**************************************************************/
/*		V E S A _ c u r r e n t m o d e               */
/* Returns the current VESA mode for the display.             */
/* Modes defined to date are:                                 */
/* 0x100	640x400 & 256 colors  (Not tested yet)        */
/* 0x101	640x480 & 256 colors                          */

/* 0x102	800x600 & 16 colors                           */
/* 0x103	800x600 & 256 colors  (Not tested yet)        */

/* 0x104       1024x768 & 16 colors                           */
/* 0x105       1024x768 & 256 colors  (Not tested yet)        */

/* 0x106      1280x1024 & 16 colors   (Not tested yet)        */
/* 0x107      1280x1024 & 256 colors  (Not tested yet)        */
/*                                                            */
/**************************************************************/
int VESA_currentmode() {
  union REGS inregs, outregs;

  inregs.h.ah = VESA;
  inregs.h.al = _GETCURRENTMODE;
  inregs.x.bx = 0;
  int86(0x10, &inregs, &outregs);
  if ((outregs.h.al == VESA) && (outregs.h.ah == SUCESS)) {
    return (outregs.x.bx);
  } else {
    return (FALSE);
  }
}

int VESA_setmode(int mode, int clearscreen)
{
  union REGS inregs, outregs;

  inregs.h.ah = VESA;
  inregs.h.al = _SETMODE | ((clearscreen) ? 0x8000 : 0);
  inregs.x.bx = mode;
  int86(0x10, &inregs, &outregs);
  return ((outregs.h.al == VESA) && (outregs.h.ah == SUCESS));
}

#define VESABUFLEN 256
char VESAmodevector[VESABUFLEN];
char *VESAmodebytes = VESAmodevector;

/* VESA_describemode */
int VESA_describemode(int mode)
{
  union REGS inregs, outregs;
  int i;
  unsigned buf;

  TPRINT(("Enter VESA_describemode\n"));
  VESA_prev_hndlr = _dos_getvect(0x10);  /* get current 10 intercept, if any   */
  _dos_setvect(0x10, VESA_Intrpt_Hndlr); /* install our int 10 intercept       */

  if (_dos_allocmem(VESABUFLEN, &buf) != 0) return (-1);

  inregs.w.eax = 0x4f01;
  inregs.w.ebx = buf;
  inregs.w.ecx = mode;
  int86(0x10, &inregs, &outregs);

  if (outregs.x.ax == VESA) {
    for (i = 0; i < VESABUFLEN; i++) { VESAmodebytes[i] = ((char *)buf)[i]; }
  } else {
    return (-1);
  }

  _dos_freemem(buf);                   /* release the buffer */
  _dos_setvect(0x10, VESA_prev_hndlr); /* re-install previous 10 intercept   */
  TPRINT(("Exit VESA_describemode\n"));
  return (0);
}

void VESA_Intrpt_Hndlr(void) {
  int inbuffer, func;
  _XSTACK *stk_ptr;

  /* get ptr to _XSTACK - regs, etc  */
  stk_ptr = (_XSTACK *)_get_stk_frame();

  func = (unsigned short)stk_ptr->eax; /* get function */

  if (((stk_ptr->flg & _FLAG_VM) == 0) /* process only if NOT virtual mode*/
      && (func == 0x4f01)) {           /* AND only if func 4f             */
    inbuffer = stk_ptr->ebx;           /* get ebx as passed (flat addr)   */
    stk_ptr->edi = MAKE_OFF(inbuffer); /* convert to seg:off form         */
    stk_ptr->es = MAKE_SEG(inbuffer);  /* service requires it in es:di    */

    ((PFV)VESA_prev_hndlr)(); /* call VESA getmode */
  } else {
    _chain_intr(VESA_prev_hndlr); /* always best to chain to prev int*/
  }
}

void VESA_beforeraid(DspInterface dsp)
{
  TPRINT(("Enter VESA_beforeraid\n"));
  _setvideomode(_DEFAULTMODE);
  _clearscreen(_GCLEARSCREEN);
  TPRINT(("Exit VESA_beforeraid\n"));
}

void VESA_afterraid(DspInterface dsp)
{
  TPRINT(("Enter VESA_afterraid\n"));
  VESA_setmode(dsp->graphicsmode, TRUE);
  TPRINT(("Exit VESA_afterraid\n"));
}

void VESA_enter(DspInterface dsp)
{
  union REGS regs;

  TPRINT(("Enter VESA_enter\n"));
  VESA_setmode(dsp->graphicsmode, TRUE);
  if (!((VESA_describemode(dsp->graphicsmode) == 0))) {
    _setvideomode(_DEFAULTMODE);
    _clearscreen(_GCLEARSCREEN);
    fprintf(stderr, "Can't set VESA mode %o.\n", dsp->graphicsmode);
    exit(0);
  }
  /* Get the segaddr. An addr. is a seg shifted 4 bits! */
  dsp->DisplayStartAddr = VESA_DSP_STARTSEG_A(VESAmodebytes) << 4;
  /* Get the size. */
  dsp->DisplaySegSize = (VESA_DSP_SEGSIZE(VESAmodebytes) << 10);

  /* How many bits in the segment addr. */
  switch (VESA_DSP_SEGSIZE(VESAmodebytes)) {
    case 64: dsp->DisplaySegMagnitude = 0x10; break;
    case 128: dsp->DisplaySegMagnitude = 0x11; break;
    case 256: dsp->DisplaySegMagnitude = 0x12; break;
  }

  dsp->SwitchBank = NULL;

  if (VESA_OPT_INFO_P(VESAmodebytes)) {
    dsp->Display.width = VESA_DSP_WIDTH(VESAmodebytes);
    dsp->Display.height = VESA_DSP_HEIGHT(VESAmodebytes);
  } else { /* Drat! No optional info. Hardcode. */
    switch (dsp->graphicsmode) {
      case 0x100:
        dsp->Display.width = 640;
        dsp->Display.height = 400;
        break;
      case 0x101:
        dsp->Display.width = 640;
        dsp->Display.height = 480;
        break;
      case 0x102:
      case 0x103:
        dsp->Display.width = 800;
        dsp->Display.height = 600;
        break;
      case 0x104:
      case 0x105:
        dsp->Display.width = 1024;
        dsp->Display.height = 768;
        break;
      case 0x106:
      case 0x107:
        dsp->Display.width = 1280;
        dsp->Display.height = 1024;
        break;
    }
  }
  dsp->colors = VESA_DSP_COLORS(VESAmodebytes);
  dsp->bitsperpixel = VESA_DSP_BPP(VESAmodebytes);
  dsp->numberofbanks = VESA_DSP_NUM_OF_BANKS(VESAmodebytes);

  /* Assumption: 8 bits per pixel */
  dsp->LinesPerBank = 8 * (dsp->DisplaySegSize / dsp->Display.width);

  _dpmi_lockregion(&currentdsp, sizeof(currentdsp));
  _dpmi_lockregion(&DisplayRegion68k, sizeof(DisplayRegion68k));
  _dpmi_lockregion(DisplayRegion68k, 1600 * 1208 / 8);

  /* These are needed in the interrupt processing. Lock'em to prevent pagefaults. */
  _dpmi_lockregion(dsp, sizeof(*dsp));
  _dpmi_lockregion(dsp->bitblt_to_screen, 0x2000);
  _dpmi_lockregion(dsp->mouse_invisible, 0x2000);
  _dpmi_lockregion(dsp->mouse_visible, 0x2000);
  _dpmi_lockregion((void *)&docopy, 0x2000);

  TPRINT(("Exit VESA_enter\n"));
}

void VESA_exit(DspInterface dsp)
{
  TPRINT(("Enter VESA_exit\n"));
  /* Unlock the following to avoid waste of mainmem. */
  _dpmi_unlockregion(&currentdsp, sizeof(currentdsp));
  _dpmi_unlockregion(&DisplayRegion68k, sizeof(DisplayRegion68k));
  _dpmi_unlockregion(DisplayRegion68k, 1600 * 1208 / 8);

  _dpmi_unlockregion(dsp->bitblt_to_screen, 0x2000);
  _dpmi_unlockregion(dsp->mouse_invisible, 0x2000);
  _dpmi_unlockregion(dsp->mouse_visible, 0x2000);
  _dpmi_unlockregion((void *)&docopy, 0x2000);
  _dpmi_unlockregion(dsp, sizeof(*dsp));

  _setvideomode(_DEFAULTMODE);
  _clearscreen(_GCLEARSCREEN);
  TPRINT(("Exit VESA_exit\n"));
}

VESA_errorexit(char *s, int errno)
{
  _setvideomode(_DEFAULTMODE);
  _clearscreen(_GCLEARSCREEN);
  fprintf(stderr, s);
  fflush(stderr);
  exit(errno);
}

void tmpclearbanks(DspInterface dsp)
{
  TPRINT(("Enter tmpclearbanks\n"));
  /*  Dosclearbanks(dsp); */
  TPRINT(("Exit tmpclearbanks\n"));
}

unsigned long tmpbbt(DspInterface dsp, char *buffer, long left, long top, long swidth, long height)
{
  TPRINT(("Enter tmpbbt\n"));
  TPRINT(("Mode display is: %d\n", dsp->graphicsmode));
  TPRINT(("dsp->Display.width = %d\n", dsp->Display.width));
  TPRINT(("dsp->Display.height = %d\n", dsp->Display.height));
  TPRINT(("left = %d, top = %d, swidth = %d, height = %d\n", left, top, swidth, height));
  (Dosbbt3)(dsp, buffer, left, top, swidth, height);
  TPRINT(("Exit tmpbbt\n"));
}

void VESA_init(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint, int depth_hint)
{
  TPRINT(("Enter VESA_init\n"));
  /* Kludge. going away soon. */
  dsp->graphicsmode = dosdisplaymode;

  /* A graphics mode is requested. Test its validity */
  if (dsp->graphicsmode == 0) {
    /* No mode requested. Find a suitable mode from hints. */
    switch (depth_hint) {
      case 0:
      case 1:
        if ((VESA_describemode(0x104) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector)))
          dsp->graphicsmode = 0x104;
        else if ((VESA_describemode(0x102) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector)))
          dsp->graphicsmode = 0x102;
        else
          VESA_errorexit("Displaymode not supported", -1);
        break;
      case 8:
        if ((VESA_describemode(0x105) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector))) {
          dsp->graphicsmode = 0x105;
        } else if ((VESA_describemode(0x103) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector))) {
          dsp->graphicsmode = 0x103;
        } else if ((VESA_describemode(0x101) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector))) {
          dsp->graphicsmode = 0x101;
        } else if ((VESA_describemode(0x100) == 0) && (VESA_MODE_SUPPORTED_P(VESAmodevector))) {
          dsp->graphicsmode = 0x100;
        } else
          VESA_errorexit("Displaymode not supported", -1);
        break;
      default: VESA_errorexit("Requested display depth not supported", -1);
    }
  } else if (!((VESA_describemode(dsp->graphicsmode) == 0) &&
               (VESA_MODE_SUPPORTED_P(VESAmodevector)))) {
    VESA_errorexit("Can't find suitable VESA mode.\n", -1);
  }

  switch (dsp->graphicsmode) {
    case 0x100:
    case 0x101:
      dsp->BytesPerLine = 640;
      dsp->bitblt_to_screen = &tmpbbt;
      break;
    case 0x102:
      dsp->BytesPerLine = 100;
      dsp->bitblt_to_screen = &Dosbbt1;
      break;
    case 0x103:
      dsp->BytesPerLine = 800;
      dsp->bitblt_to_screen = &tmpbbt;
      break;
    case 0x104:
      dsp->BytesPerLine = 128;
      dsp->bitblt_to_screen = &Dosbbt2;
      break;
    case 0x105:
      dsp->BytesPerLine = 1024;
      dsp->bitblt_to_screen = &tmpbbt;
      break;
    case 0x106:
      dsp->BytesPerLine = 160;
      dsp->bitblt_to_screen = &Dosbbt2;
      break;
    case 0x107:
      dsp->BytesPerLine = 1280;
      dsp->bitblt_to_screen = &tmpbbt;
      break;
    default: VESA_errorexit("Displaymode not supported", -1);
  }

  TPRINT(("Mode display is: %d\n", dsp->graphicsmode));

  dsp->device.locked = FALSE;
  dsp->device.active = FALSE;

  dsp->device.enter = &VESA_enter;
  dsp->device.exit = &VESA_exit;

  dsp->device.before_raid = &VESA_beforeraid;
  dsp->device.after_raid = &VESA_afterraid;

  dsp->drawline = &GenericPanic;

  dsp->cleardisplay = &tmpclearbanks;

  dsp->get_color_map_entry = &VGA_not_color;
  dsp->set_color_map_entry = &set_DAC_color;
  dsp->available_colors = &VGA_not_color;
  dsp->possible_colors = &VGA_not_color;

  dsp->medley_to_native_bm = (PFUL)&GenericPanic;
  dsp->native_to_medley_bm = (PFUL)&GenericPanic;

  dsp->bitblt_from_screen = (PFUL)&GenericPanic;
  dsp->scroll_region = (PFUL)&GenericPanic;

  dsp->mouse_invisible = (PFV)&dostaking_mouse_down;
  dsp->mouse_visible = (PFV)&dostaking_mouse_up;

  TPRINT(("Exit VESA_init\n"));
}
