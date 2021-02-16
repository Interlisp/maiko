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

DspInterfaceRec curdsp;
DspInterface currentdsp = &curdsp;

#ifdef XWINDOW
extern int LispDisplayRequestedWidth;
extern int LispDisplayRequestedHeight;

extern DspInterface X_init(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint,
                           int depth_hint);
#endif /* XWINDOW */

void make_dsp_instance(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint,
                       int depth_hint) {
#if   XWINDOW
  /* lispbitmap is 0 when we call X_init the first time. */
  if (X_init(dsp, 0, LispDisplayRequestedWidth, LispDisplayRequestedHeight, depth_hint) == NULL) {
    fprintf(stderr, "Can't open display.");
    exit(-1);
  }
#endif /* DOS | XWINDOW */
} /* Now we know the maximum capabilities of the hardware. */

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
  printf("bitblt_to_screen= %p\n", dsp->bitblt_to_screen);
  printf("cleardisplay= %p\n", dsp->cleardisplay);
  fflush(stdout);
}
