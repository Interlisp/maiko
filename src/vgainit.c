/* $Id: vgainit.c,v 1.2 1999/01/03 02:07:45 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: vgainit.c,v 1.2 1999/01/03 02:07:45 sybalsky Exp $ Copyright (C) Venue";

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

#include <stdio.h>
#include <graph.h>
#include "lispemul.h"
#include "devif.h"

extern unsigned long Dosbbt1();
extern unsigned long Dosbbt2();
extern int dostaking_mouse_down();
extern int dostaking_mouse_up();

extern DLword *DisplayRegion68k;
extern DLword *DisplayRegion68k_end_addr;
extern DspInterface currentdsp;
extern void docopy();

extern void GenericPanic();

void VGA_setmax(dsp) DspInterface dsp;
{
  struct videoconfig vc;

  if (!_setvideomode(_MAXRESMODE)) {
    fprintf(stderr, "Can't set graphics mode.\n");
    exit(1);
  }
  _getvideoconfig(&vc);
  dsp->Display.width = vc.numxpixels;
  dsp->Display.height = vc.numypixels;
  dsp->bitsperpixel = vc.bitsperpixel;
  dsp->colors = vc.numcolors;

  /* These are needed in the interrupt processing. Lock'em to prevent pagefaults. */
  _dpmi_lockregion(&currentdsp, sizeof(currentdsp));
  _dpmi_lockregion(&DisplayRegion68k, sizeof(DisplayRegion68k));
  _dpmi_lockregion(DisplayRegion68k, 1600 * 1208 / 8);

  /* These are needed in the interrupt processing. Lock'em to prevent pagefaults. */
  _dpmi_lockregion(dsp, sizeof(*dsp));
  _dpmi_lockregion(dsp->bitblt_to_screen, 0x2000);
  _dpmi_lockregion(dsp->mouse_invissible, 0x2000);
  _dpmi_lockregion(dsp->mouse_vissible, 0x2000);
  _dpmi_lockregion((void *)&docopy, 0x2000);
}

void VGA_exit(dsp) DspInterface dsp;
{
  /* Unlock the following to avoid waste of mainmem. */
  _dpmi_unlockregion(&currentdsp, sizeof(currentdsp));
  _dpmi_unlockregion(&DisplayRegion68k, sizeof(DisplayRegion68k));
  _dpmi_unlockregion(DisplayRegion68k, 1600 * 1208 / 8);

  /* These are needed in the interrupt processing. Lock'em to prevent pagefaults. */
  _dpmi_unlockregion(dsp, sizeof(*dsp));
  _dpmi_unlockregion(dsp->bitblt_to_screen, 0x2000);
  _dpmi_unlockregion(dsp->mouse_invissible, 0x2000);
  _dpmi_unlockregion(dsp->mouse_vissible, 0x2000);
  _dpmi_unlockregion((void *)&docopy, 0x2000);

  _setvideomode(_DEFAULTMODE);
  _clearscreen(_GCLEARSCREEN);
}

unsigned long VGA_not_color(dsp) DspInterface dsp;
{
  printf("Colormode not set!\n");
  fflush(stdout);
  return (0);
}

unsigned long VGA_colornum(dsp) DspInterface dsp;
{ /* Return the number of available colors */
  return (1 << dsp->bitsperpixel);
}

unsigned long VGA_possiblecolors(dsp) DspInterface dsp;
{ return (dsp->colors); }

void VGA_mono_drawline(dsp, startX, startY, width, height, function, color, thickness, butt, clipX,
                       clipY, clipWidth, clipHeight, dashing) DspInterface dsp;
unsigned long startX, startY, width, height;
int function;
unsigned long color, thickness;
int butt;
unsigned long clipX, clipY, clipWidth, clipHeight;
LispPTR *dashing;

{
  _moveto_w(startX, startY);
  _lineto_w(width, height);
}

void VGA_color_drawline(dsp, startX, startY, width, height, function, color, thickness, butt, clipX,
                        clipY, clipWidth, clipHeight, dashing) DspInterface dsp;
unsigned long startX, startY, width, height;
int function;
unsigned long color, thickness;
int butt;
unsigned long clipX, clipY, clipWidth, clipHeight;
LispPTR *dashing;

{}

void VGA_cleardisplay(dsp) DspInterface dsp;
{ _clearscreen(_GCLEARSCREEN); }

VGA_init(dsp, lispbitmap, width_hint, height_hint, depth_hint) DspInterface dsp;
char *lispbitmap;
int width_hint, height_hint, depth_hint;
{
  struct videoconfig vc;

  dsp->device.enter = (PFV)&VGA_setmax;
  dsp->device.exit = (PFV)&VGA_exit;

  dsp->device.before_raid = (PFV)&VGA_exit;
  dsp->device.after_raid = (PFV)&VGA_setmax;

  dsp->drawline = (PFV)&VGA_mono_drawline;

  dsp->cleardisplay = (PFV)&VGA_cleardisplay;

  dsp->get_color_map_entry = (PFUL)&VGA_not_color;
  dsp->set_color_map_entry = (PFUL)&VGA_not_color;
  dsp->available_colors = (PFUL)&VGA_not_color;
  dsp->possible_colors = (PFUL)&VGA_not_color;

  dsp->medley_to_native_bm = (PFUL)&GenericPanic;
  dsp->native_to_medley_bm = (PFUL)&GenericPanic;

  dsp->bitblt_to_screen = (PFUL)&Dosbbt1;
  dsp->bitblt_from_screen = (PFUL)&GenericPanic;
  dsp->scroll_region = (PFUL)&GenericPanic;

  dsp->DisplayStartAddr = 0xa0000;
  dsp->DisplaySegSize = 0x10000;   /* 64K segments */
  dsp->DisplaySegMagnitude = 0x10; /* How many bits in the addr. */
  dsp->BytesPerLine = 80;
  dsp->LinesPerBank = 512;

  dsp->mouse_invissible = (PFV)&dostaking_mouse_down;
  dsp->mouse_vissible = (PFV)&dostaking_mouse_up;

  dsp->device.locked = FALSE;
  dsp->device.active = FALSE;
}
