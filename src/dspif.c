/* $Id: dspif.c,v 1.4 2001/12/24 01:09:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */
/* This is the display interface  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994,         */
/*                    1995, 1999 Venue.                                 */
/*	    All Rights Reserved.		                        */
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include "lispemul.h"
#include "dbprint.h"
#include "devif.h"

#include "dspifdefs.h"
#include "xinitdefs.h"

DspInterfaceRec _curdsp;

DspInterface currentdsp = &_curdsp;

#ifdef XWINDOW
extern int LispDisplayRequestedWidth;
extern int LispDisplayRequestedHeight;

extern DspInterface X_init(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint,
                           int depth_hint);
#endif /* XWINDOW */

#ifdef DOS
extern int dosdisplaymode;
#endif /* DOS */

void make_dsp_instance(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint,
                       int depth_hint) {
#ifdef DOS

  TPRINT(("Enter make_dsp_instance, dosdisplaymode is: %d\n", dosdisplaymode));

  if (depth_hint == 0) depth_hint = 1;

  switch (dosdisplaymode) {
    case 1: VGA_init(dsp, 0, 0, 0, depth_hint); break;
    case 0x102:
    case 0x104: VESA_init(dsp, 0, 0, 0, depth_hint); break;
    default:
      if (VESA_p()) {
        VESA_init(dsp, 0, 0, 0, depth_hint);
      } else if (VGA_p()) {
        VGA_init(dsp, 0, 0, 0, depth_hint);
      } else { /* Can't set *ANY* video mode! */
        (void)fprintf(stderr, "No portable graphics mode supported by this host.\n");
        (void)fprintf(stderr, "\n-Expected VESA or VGA.\n");
        exit(1);
      }
      break;
  }

#elif XWINDOW
  /* lispbitmap is 0 when we call X_init the first time. */
  if (X_init(dsp, 0, LispDisplayRequestedWidth, LispDisplayRequestedHeight, depth_hint) == NULL) {
    fprintf(stderr, "Can't open display.");
    exit(-1);
  }
#endif /* DOS | XWINDOW */
} /* Now we know the maximum capabilities of the hardware. */

#ifdef DOS
VESA_p() {
  /* Magic. Do a vesa call to determine the current mode. */
  return (VESA_call(3, 0));
}

VGA_p() { return (TRUE); }
#endif /* DOS */

/*********************************************************************/
/*                                                                   */
/*		     G e n e r i c R e t u r n T                     */
/*                                                                   */
/* Utility function that just returns T                              */
/*                                                                   */
/*********************************************************************/
unsigned long GenericReturnT(void) { return (T); }

void GenericPanic(DspInterface dsp) {
  TPRINT(("Enter GenericPanic\n"));
  fprintf(stderr, "Panic! Call to uninitialized display slot!");
  exit(0);
}

void describedsp(DspInterface dsp) {
  if (dsp == 0) {
    printf("describedsp: Not a dsp!\n");
    return;
  }

  printf("\n");
  printf("width= %d\n", dsp->Display.width);
  printf("height= %d\n", dsp->Display.height);
  printf("bitsperpixel= %d\n", dsp->bitsperpixel);
  printf("colors= %lu\n", dsp->colors);
  printf("graphicsmode= %lu\n", dsp->graphicsmode);
  printf("numberofbanks= %lu\n", dsp->numberofbanks);
#ifdef DOS
  printf("BytesPerLine= %d\n", dsp->BytesPerLine);
  printf("DisplayStartAddr= %d\n", dsp->DisplayStartAddr);
#endif /* DOS */
  printf("bitblt_to_screen= %p\n", dsp->bitblt_to_screen);
  printf("cleardisplay= %p\n", dsp->cleardisplay);
#ifdef DOS
  printf("mouse_visible= %d\n", dsp->mouse_visible);
  printf("mouse_invisible= %d\n", dsp->mouse_invisible);
  printf("\n");
#endif /* DOS */
  fflush(stdout);
}
