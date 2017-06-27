/* $Id: initdsp.c,v 1.2 1999/01/03 02:07:08 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: initdsp.c,v 1.2 1999/01/03 02:07:08 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	file	:	initdsp.c
 *	Author	:	Osamu Nakamura
 */

#include <stdio.h>

#ifndef NOPIXRECT
#ifndef DOS
#ifdef XWINDOW
#include <sys/time.h>
#include <pixrect/pixrect_hs.h>
#include <sunwindow/notify.h>
#include <sunwindow/rect.h>
#include <sunwindow/rectlist.h>
#include <sunwindow/pixwin.h>
#include <sunwindow/pw_util.h>
#include <sunwindow/win_struct.h>
#include <sunwindow/win_environ.h>
#include <sunwindow/cms.h>
#include <sunwindow/win_input.h>
#else
#include <sunwindow/window_hs.h>
#include <sunwindow/cms.h>
#include <sunwindow/win_ioctl.h>
#endif /* XWINDOW */
/* #include <sunwindow/win_screen.h> */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pixrect/pixrect_hs.h>
#include <sun/fbio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#endif /* DOS */
#include <pixrect/pr_planegroups.h>
#endif /* NOPIXRECT */

#ifdef OS4
#include <vfork.h>
#endif /* OS4 */

#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "lspglob.h"
#include "emlglob.h"
#include "display.h"
#include "devconf.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"
#include "dbprint.h"

#ifdef ISC
#define getpagesize() 512
#endif /* ISC */

#ifdef DOS
#define getpagesize() 512
#endif /* DOS */

#if defined(XWINDOW) || defined(DOS)
#include "devif.h"
DLword *DisplayRegion68k_end_addr;
extern DspInterface currentdsp;
int DisplayWidth8;
extern DspInterfaceRec _curdsp, _coldsp;
#endif /* DOS */

/* from /usr/include/sun/fbio.h some machines don't have following def. */
#ifndef FBTYPE_SUNROP_COLOR
#define FBTYPE_SUNROP_COLOR 13 /* MEMCOLOR with rop h/w */
#define FBTYPE_SUNFAST_COLOR 12
#endif

#ifndef NOPIXRECT
struct screen LispScreen;
struct pixrect *CursorBitMap, *InvisibleCursorBitMap;
struct pixrect *SrcePixRect, *DestPixRect;
#ifdef DISPLAYBUFFER
int black = 0, white = -1;
struct pixrect *ColorDisplayPixrect, *DisplayRegionPixrect;
#define COPY_PIXRECT_TO_COLOR (PIX_SRC /*| PIX_DONTCLIP */)
#ifdef DEBUG
int oldred[2], oldgreen[2], oldblue[2];
#endif /* DEBUG */
#endif /* DISPLAYBUFFER */
#endif /* NOPIXRECT */

int LispWindowFd;
int FrameBufferFd;

int displaywidth, displayheight, DisplayRasterWidth, DisplayType;
int DisplayByteSize;
DLword *DisplayRegion68k; /* 68k addr of #{}22,0 */

#ifdef DISPLAYBUFFER
/* both vars has same value. That is the end of Lisp DisplayRegion */
DLword *DisplayRegion68k_end_addr;
#endif

/* some functions use this variable when undef DISPLAYBUFFER */
DLword *DISP_MAX_Address;

#ifdef SUNDISPLAY
struct cursor CurrentCursor, InvisibleCursor;
struct winlock DisplayLockArea;
#endif /* SUNDISPLAY */

extern DLword *EmCursorBitMap68K;
extern int errno;
extern IFPAGE *InterfacePage;

int DebugDSP = T;

#ifdef COLOR
extern DLword *ColorDisplayRegion68k;
extern int MonoOrColor;
#endif /* COLOR */

#ifdef XWINDOW
DLword *DisplayRegion68k_end_addr;
int LispDisplayWidth, LispDisplayHeight;
extern int *Xdisplay; /* DAANGER -jarl nilsson 27-apr-92 */
#endif                /* XWINDOW */

#ifdef SUNDISPLAY
/* For SunOS4.1 Window Security Feature. */
int Win_security_p;
#include <sys/wait.h>
#define SV_ACQUIRE "/bin/sunview1/sv_acquire"
#define SV_RELEASE "/bin/sunview1/sv_release"
#endif /* SUNDISPLAY */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void init_cursor() {
#ifndef NOPIXRECT
  CursorBitMap = mem_create(CURSORWIDTH, CURSORHEIGHT, 1);
  mpr_mdlinebytes(CursorBitMap) = CURSORWIDTH >> 3; /* 2(byte) */
#endif                                              /* NOPIXRECT */

#ifdef SUNDISPLAY
  CurrentCursor.cur_xhot = 0;
  CurrentCursor.cur_yhot = 0;
  CurrentCursor.cur_shape = CursorBitMap;
  CurrentCursor.cur_function = PIX_SRC | PIX_DST;
#endif /* SUNDISPLAY */

/*  Invisible Cursor */

#ifndef NOPIXRECT
  InvisibleCursorBitMap = mem_create(0, 0, 1);
#endif /* NOPIXRECT */

#ifdef SUNDISPLAY
  InvisibleCursor.cur_xhot = 0;
  InvisibleCursor.cur_yhot = 0;
  InvisibleCursor.cur_shape = InvisibleCursorBitMap;
  InvisibleCursor.cur_function = /*PIX_SRC |*/ PIX_DST;
  win_setcursor(LispWindowFd, &InvisibleCursor);
  win_setmouseposition(LispWindowFd, 0, 0);
#endif /* SUNDISPLAY */
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void set_cursor() {
#ifdef SUNDISPLAY
#ifdef OLD_CURSOR
  (mpr_d(CursorBitMap))->md_image = (short *)(IOPage->dlcursorbitmap);
  /* BitmapBase of CurrentCursor
   * is set to IOPage->dlcursorbitmap
   */
  if (win_setcursor(LispWindowFd, &CurrentCursor) == -1) perror("SET Cursor");
  if (win_setmouseposition(LispWindowFd, 0, 0) == -1) perror("SET Mouse POS");
#else
  if (win_setcursor(LispWindowFd, &InvisibleCursor) == -1) perror("SET Cursor:");
#endif
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  Init_XCursor();
#endif /* XWINDOW */

  DBPRINT(("After Set cursor\n"));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifndef COLOR
void clear_display() {
#ifdef SUNDISPLAY
#ifndef DISPLAYBUFFER
  register short *word;
  register int w, h;
  word = (short *)DisplayRegion68k;
  for (h = displayheight; (h--);) {
    for (w = DisplayRasterWidth; (w--);) { *word++ = 0; }
  }
#else
  pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight, PIX_CLR, ColorDisplayPixrect, 0,
         0);
/* Original images are still kept in SYSOUT(DisplayRegion)  */
/*  clear_CG6; */
#endif /* DISPLAYBUFFER */

#endif /* SUNDISPLAY */

#ifdef DOS
  TPRINT(("Enter Clear_display\n"));
  (currentdsp->cleardisplay)(currentdsp);
  TPRINT(("Exit Clear_display\n"));
#endif /* DOS */
}

#else /* COLOR */

void clear_display() {
  register short *word;
  register int w, h;
  if (MonoOrColor == MONO_SCREEN) {
#ifndef DISPLAYBUFFER
    word = DisplayRegion68k;
    for (h = displayheight; (h--);) {
      for (w = DisplayRasterWidth; (w--);) { *word++ = 0; }
    }      /* end for(h) */
#else  /* DISPLAYBUFFER */
    pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight, PIX_CLR, ColorDisplayPixrect, 0,
           0);
#endif /* DISPLAYBUFFER */
  } else { /* MonoOrColo is COLOR_SCREEN */
    word = (short *)ColorDisplayRegion68k;
    for (h = displayheight; (h--);) {
      for (w = DisplayRasterWidth * 8; (w--);) { *word++ = 0; }
    } /* end for(h) */
  }   /* end if(MonoOrColor) */
}
#endif /* COLOR */

/*  ================================================================  */
/*  Now takes 68k address, function renamed for safety  */

void init_display2(INT display_addr, INT display_max)
{
  int mmapstat;
  int fbgattr_result;
  char *texture_base;

#ifdef SUNDISPLAY
  struct fbtype my_screen;
  char groups[PIXPG_OVERLAY + 1];
  struct fbgattr FBattr;
  struct pixrect *ColorFb;
#endif /* SUNDISPLAY */

#ifdef SUNDISPLAY

  /* For SunOS4.1 Window Security Feature. */

  union wait status;

  /* Window Security is available? */
  if (!access(SV_ACQUIRE, F_OK))
    Win_security_p = 1;
  else
    Win_security_p = 0;

  if (Win_security_p) {
    switch (vfork()) {
      case -1: /* Error */ (void)fprintf(stderr, "init_display: Fork failed.\n"); exit(1);

      case 0: /* Child */
        (void)execl(SV_ACQUIRE, "sv_acquire", "0", "256", "250", 0);
        /* should not return */
        (void)fprintf(stderr, "init_display: exec for sv_acquire failed\n");
        exit(1);

      default: /* Parent */
        /* do nothing */
        break;
    }
    (void)wait(&status); /* child dies after changing 6 */

    if (status.w_retcode != 0) {
      (void)fprintf(stderr, "init_display: failed to set ownership of win devices\n");
      exit(1);
    }
  }

#endif /* SUNDISPLAY */

#ifdef SUNDISPLAY
  if ((LispWindowFd = win_screennew(&LispScreen)) == -1) {
    perror("init_display: can't create LispWindow\n");
    exit(-1);
  } else {
#ifdef KBINT
/* int_io_open(LispWindowFd);  JDS 4/27/94 move to initkbd, to try preventing the
 * move-mouse-never-get-kbd bug */
#endif
    fcntl(LispWindowFd, F_SETFL, fcntl(LispWindowFd, F_GETFL, 0) | FNDELAY);
  }
#endif /* SUNDISPLAY */

  DisplayRegion68k = (DLword *)display_addr;

#ifdef SUNDISPLAY
  if ((FrameBufferFd = open(LispScreen.scr_fbname, 2)) == -1) {
    perror("init_display: can't open FrameBuffer\n");
    exit(-1);
  }

  /* initialize Display parameters */
  if (ioctl(FrameBufferFd, FBIOGTYPE, &my_screen) == -1) {
    perror("init_display: can't find screen parameters\n");
    exit(-1);
  }
  displaywidth = my_screen.fb_width;
  displayheight = my_screen.fb_height;
#endif /* SUNDISPLAY */

#if (defined(XWINDOW) || defined(DOS))
  (currentdsp->device.enter)(currentdsp);
  displaywidth = currentdsp->Display.width;
  displayheight = currentdsp->Display.height;
#endif /* XWINDOW */

  DisplayRasterWidth = displaywidth / BITSPER_DLWORD;

  if ((displaywidth * displayheight) > display_max) { displayheight = display_max / displaywidth; }
  DISP_MAX_Address = DisplayRegion68k + DisplayRasterWidth * displayheight;
  DBPRINT(("FBIOGTYPE w x h = %d x %d\n", displaywidth, displayheight));

#ifdef DOS
  DisplayWidth8 = displaywidth / 8;
#endif /* DOS */

  DBPRINT(("FBIOGTYPE w x h = %d x %d\n", displaywidth, displayheight));
#if (!defined(XWINDOW) && !defined(DOS))
  DBPRINT(("   (real) type  = %d\n", my_screen.fb_type));
  DBPRINT(("   (real) bpp   = %d\n", my_screen.fb_depth));
#endif /* XWINDOW */

#ifdef SUNDISPLAY
  /** now attempt to use the FBIOGATTR call for more information **/

  fbgattr_result = ioctl(FrameBufferFd, FBIOGATTR, &FBattr);
  if (fbgattr_result >= 0) {
    DBPRINT(("FBIOGATTR realtype = %d\n", FBattr.real_type));
    DBPRINT(("   (real) size = %d x %d\n", FBattr.fbtype.fb_width, FBattr.fbtype.fb_height));
    DBPRINT(("   (real) type = %d\n", FBattr.fbtype.fb_type));
    DBPRINT(("   (real) bpp  = %d\n", FBattr.fbtype.fb_depth));
    DBPRINT(("          emuls= %d %d %d %d\n", FBattr.emu_types[0], FBattr.emu_types[1],
             FBattr.emu_types[2], FBattr.emu_types[3]));
  } else { /* fill in defaults */
    FBattr.real_type = my_screen.fb_type;
  }
  DBPRINT(("init_display: FBIOGATTR_result = %d\n", fbgattr_result));
/* probe for DISPLAY type */
/**********************************************************************
 *	         FB-TYPE       REAL-TYPE
 * 	BW2      2             x
 *	CG2      3             3
 *	CG3      8             6
 *	CG4      2             8
 *	CG6      8             12
 *	CG8      6             7
 *	CG9(GP1) 4             4    ;gpconfig -f -b
 *	CG9(GP1) 2            13    ;gpconfig gpone0 -f -b cgtwo0
 *	                            ;We assume This config for GXP model
 ***********************************************************************/
/* Medley supports real (or emulated) BW2, so check for that first */
#ifndef DISPLAYBUFFER

  if ((my_screen.fb_type == FBTYPE_SUN2BW)        /* real or emulated bwtwo */
      || (my_screen.fb_type == FBTYPE_SUN3COLOR)) /* Sun 3 color? */
    switch (FBattr.real_type) {
      case FBTYPE_SUN2BW: DisplayType = SUN2BW; break;

      case FBTYPE_MEMCOLOR: /* memory 24-bit (CG8) */
      {
        DisplayType = SUNMEMCOLOR;
        ColorFb = pr_open("/dev/fb");
        pr_available_plane_groups(ColorFb, sizeof(groups), groups);
        if (groups[PIXPG_OVERLAY] && groups[PIXPG_OVERLAY_ENABLE]) {
          pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
          pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
          pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
        }
      } break;

      case FBTYPE_SUN4COLOR:    /* cg4 */
      case FBTYPE_SUNROP_COLOR: /* cg9(GXP) */
      {                         /* need to clear overlay plane */
        DisplayType = SUN4COLOR;
        ColorFb = pr_open("/dev/fb");
        pr_available_plane_groups(ColorFb, sizeof(groups), groups);
        if (groups[PIXPG_OVERLAY] && groups[PIXPG_OVERLAY_ENABLE]) {
          pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
          pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
          pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
        }
        break;
      }

      default:
        printf("initdisplay: Unsupported FBreal_type %d\n", FBattr.real_type);
        DisplayType = (my_screen.fb_type) << 3;
        /* should be able to proceed, since its an emulated bwtwo */
    }
  else {                       /* not currently a SUN2BW, perhaps can emulate it? */
    if (fbgattr_result >= 0) { /* got gattrs, can try sattr */
#ifdef DEBUG
      error("fb bwtwo emulation not implemented, q to proceed\n");
#endif /* DEBUG */
    } else {
      error("Not cgfour or bwtwo, q to attempt to proceed");
    }
    printf("initdisplay: Unsupported FBreal_type %d\n", FBattr.real_type);
    DisplayType = (my_screen.fb_type) << 3;
    /* try to muddle on */
  }
#else /* DISPLAYBUFFER is defined, then: */
  DisplayRegion68k_end_addr = DisplayRegion68k + DisplayRasterWidth * displayheight;
  if (my_screen.fb_type == FBTYPE_SUN2BW) /* real or emulated bwtwo */
    switch (FBattr.real_type) {
      case FBTYPE_SUN4COLOR: { /* need to clear overlay plane */
        DisplayType = SUN4COLOR;
        ColorFb = pr_open("/dev/fb");
        pr_available_plane_groups(ColorFb, sizeof(groups), groups);
        if (groups[PIXPG_OVERLAY] && groups[PIXPG_OVERLAY_ENABLE]) {
          pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
          pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_CLR, 0, 0, 0);
          pr_set_plane_group(ColorFb, PIXPG_8BIT_COLOR);
        }
        ColorDisplayPixrect = ColorFb;
        break;
      }

      default: ColorDisplayPixrect = pr_open("/dev/fb"); break;
    }

  else if (my_screen.fb_type == FBTYPE_SUN4COLOR) { /* cg3 or cg6 */
    switch (FBattr.real_type) {
      case FBTYPE_SUN3COLOR: /* cg3 */
                             /*	DisplayType = SUN3COLOR; */
        DisplayType = (FBattr.real_type) << 3;
        break;
      case FBTYPE_SUNFAST_COLOR: /* cg6 */
                                 /*	DisplayType = SUNFASTCOLOR; */
        DisplayType = (FBattr.real_type) << 3;
        break;
      default: /* unknown display */ DisplayType = (FBattr.real_type) << 3; break;
    } /* end switch */
    ColorDisplayPixrect = pr_open("/dev/fb");
  } /* end else if() */

  else
    ColorDisplayPixrect = pr_open("/dev/fb");
  DisplayRegionPixrect = mem_point(displaywidth, displayheight, 1, display_addr);
#ifdef I386
  ((struct mpr_data *)DisplayRegionPixrect->pr_data)->md_flags |= MP_I386;
  ((struct mpr_data *)ColorDisplayPixrect->pr_data)->md_flags |= MP_I386;
#endif

#ifdef DEBUG
  pr_getcolormap(ColorDisplayPixrect, 0, 2, oldred, oldgreen, oldblue);
#endif /* DEBUG */
  DBPRINT(("Color map for color pixrect:\n  0:  R: %d, G: %d, B: %d\n  1:  R: %d, G: %d, B: %d\n",
           oldred[0], oldgreen[0], oldblue[0], oldred[1], oldgreen[1], oldblue[1]));
  DBPRINT(("Color depth = %d.\n", ColorDisplayPixrect->pr_depth));
  pr_putcolormap(ColorDisplayPixrect, 1, 1, &black, &black, &black);
  pr_putcolormap(ColorDisplayPixrect, 0, 1, &white, &white, &white);
  pr_putcolormap(ColorDisplayPixrect, 255, 1, &black, &black, &black);
  pr_putcolormap(ColorDisplayPixrect, (1 << ColorDisplayPixrect->pr_depth) - 1, 1, &black, &black,
                 &black);
#endif

  DisplayLockArea.wl_rect.r_width = displaywidth;
  DisplayLockArea.wl_rect.r_height = displayheight;

#endif /* SUNDISPLAY */

#ifdef XWINDOW
  DisplayType = SUN2BW;
  DisplayRegion68k_end_addr = DisplayRegion68k + DisplayRasterWidth * displayheight;
#endif /* XWINDOW */

  init_cursor();
  DisplayByteSize = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

  DBPRINT(("Display address: 0x%x\n", DisplayRegion68k));
  DBPRINT(("        length : 0x%x\n", DisplayByteSize));
  DBPRINT(("        pg size: 0x%x\n", getpagesize()));

#ifdef SUNDISPLAY
#ifndef DISPLAYBUFFER
  mmapstat = (int)mmap(DisplayRegion68k, DisplayByteSize, PROT_READ | PROT_WRITE,
#ifdef OS4
                       MAP_FIXED |
#endif
                           MAP_SHARED,
                       FrameBufferFd, 0);

  DBPRINT(("after mmap: 0x%x\n", mmapstat));

  if (mmapstat == -1) {
    perror("init_display: ERROR at mmap system call\n");
    exit(0);
  }
#endif /* ifndef DISPLAYBUFFER */

  DBPRINT(("after mem_point\n"));
#endif /* SUNDISPLAY */

#ifdef DOS
  (currentdsp->cleardisplay)(currentdsp);
#else  /* DOS */
  clear_display();
#endif /* DOS */

  DBPRINT(("after clear_display()\n"));

#ifndef NOPIXRECT
  /* initialize pixrect used in pilotbitblt */
  SrcePixRect = mem_point(0, 0, 1, NULL);
  DestPixRect = mem_point(0, 0, 1, NULL);
#endif /* NOPIXRECT */

  DBPRINT(("exiting init_display\n"));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void display_before_exit() {
#ifdef SUNDISPLAY
  union wait status;
#endif /* SUNDISPLAY */

#ifdef TRUECOLOR
  truecolor_before_exit();
#endif /* TRUECOLOR */

  clear_display();
#ifdef SUNDISPLAY
  /*    win_remove( LispWindowFd ); */
  win_screendestroy(LispWindowFd);
#ifdef KBINT
  int_io_close(LispWindowFd);
#endif
  close(LispWindowFd);

  if (Win_security_p) {
    switch (vfork()) {
      case -1: /* Error */ (void)fprintf(stderr, "display_before_exit: Fork failed.\n"); exit(1);

      case 0: /* Child */
        (void)execl(SV_RELEASE, "sv_release", 0);
        /* should not return */
        (void)fprintf(stderr, "display_before_exit: exec for sv_release failed\n");
        exit(1);

      default: /* Parent */
        /* do nothing */
        break;
    }
    (void)wait(&status);
    if (status.w_retcode != 0) {
      (void)fprintf(stderr, "display_before_raid: failed to set ownership of win devices\n");
      exit(1);
    }
  }

#endif /* SUNDISPLAY */

#if defined(XWINDOW) || defined(DOS)
  (currentdsp->device.exit)(currentdsp);
#endif /* DOS */
}

#ifdef DISPLAYBUFFER

#ifdef I386
#define EVENADDR(ptr) (0xFFFFFFFE & (int)ptr)
#else
#define EVENADDR(ptr) (ptr)
#endif

#endif /* DISPLAYBUFFER */

#if defined(DISPLAYBUFFER) || defined(DOS)
/************************************************************************/
/*									*/
/*		    i n _ d i s p l a y _ s e g m e n t			*/
/*									*/
/*	Returns T if the base address for this bitblt is in the 	*/
/*	display segment.						*/
/*									*/
/************************************************************************/
/*  Change as MACRO by osamu '90/02/08
 *  new macro definition is in display.h
in_display_segment(baseaddr)
  register DLword *baseaddr;
  {
    if ((DisplayRegion68k <= baseaddr) &&
        (baseaddr <=DISP_MAX_Address))   return(T);
    return(NIL);
  }
------------------ */
#endif /* DISPLAYBUFFER */

/************************************************************************/
/*									*/
/*		 f l u s h _ d i s p l a y _ b u f f e r		*/
/*									*/
/*	Copy the entire Lisp display bank to the real frame buffer 	*/
/*	[Needs to be refined for efficiency.]				*/
/*									*/
/************************************************************************/

void flush_display_buffer() {
#ifdef SUNDISPLAY
#ifdef DISPLAYBUFFER
#ifdef I386
  bit_reverse_region(DisplayRegion68k, displaywidth, displayheight, DLWORD_PERLINE);
#endif

  pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight, COPY_PIXRECT_TO_COLOR,
         DisplayRegionPixrect, 0, 0);

#ifdef I386
  bit_reverse_region(DisplayRegion68k, displaywidth, displayheight, DLWORD_PERLINE);
#endif
#endif /* DISPLAYBUFFER */
#endif /* SUNDISPLAY */

#ifdef XWINDOW
  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, currentdsp->Visible.x,
                                 currentdsp->Visible.y, currentdsp->Visible.width,
                                 currentdsp->Visible.height);
#elif DOS
  TPRINT(("Enter flush_display_buffer\n"));
  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, 0, 0, currentdsp->Display.width,
                                 currentdsp->Display.height);
  TPRINT(("Exit flush_display_buffer\n"));
#endif /* DOS */
}

/************************************************************************/
/*									*/
/*		 f l u s h _ d i s p l a y _ r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	x								*/
/*	y								*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/
#define BITEPER_DLBYTE 8
#define DLBYTE_PERLINE (displaywidth / 8)

void flush_display_region(int x, int y, int w, int h)
{
#ifdef SUNDISPLAY
#ifdef DISPLAYBUFFER

  pr_rop(ColorDisplayPixrect, x, y, w, h, COPY_PIXRECT_TO_COLOR, DisplayRegionPixrect, x, y);
#endif /* DISPLAYBUFFER */

#endif /* SUNDISPLAY */

#if (defined(XWINDOW) || defined(DOS))
  TPRINT(("Enter flush_display_region x=%d, y=%d, w=%d, h=%d\n", x, y, w, h));
  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, x, y, w, h);
  TPRINT(("Exit flush_display_region\n"));
#endif /* DOS */
}
#ifdef BYTESWAP
void byte_swapped_displayregion(int x, int y, int w, int h)
{
  extern unsigned char reversedbits[];
  register unsigned int *longptr, *lineptr;
  register int linecount, wordlimit;

  /* Get QUAD byte aligned pointer */
  longptr = (unsigned int *)(((UNSIGNED)((DLword *)DisplayRegion68k + (DLWORD_PERLINE * y)) +
                              ((x + 7) >> 3)) &
                             0xfffffffc);

  bit_reverse_region(longptr, w, h, DLWORD_PERLINE);

  return;

} /* byte_swapped_displayregion end */
#endif /* BYTESWAP */

/************************************************************************/
/*									*/
/*	    f l u s h _ d i s p l a y _ l i n e r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	x								*/
/*	ybase the offset from top of bitmap, as the address of the	*/
/*	       first word of the line to start on.			*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/

void flush_display_lineregion(UNSIGNED x, DLword *ybase, UNSIGNED w, UNSIGNED h)
#ifdef I386
{ /*flush_display_buffer(); */
  int y;
  y = ((DLword *)ybase - DisplayRegion68k) / DLWORD_PERLINE;

  bit_reverse_region(ybase, displaywidth, h, DLWORD_PERLINE);
  pr_rop(ColorDisplayPixrect, x, y, displaywidth, h, COPY_PIXRECT_TO_COLOR, DisplayRegionPixrect, x,
         y);

  bit_reverse_region(ybase, displaywidth, h, DLWORD_PERLINE);
}
#else
{
  int y;
  y = ((DLword *)ybase - DisplayRegion68k) / DLWORD_PERLINE;
#ifdef SUNDISPLAY
#ifdef DISPLAYBUFFER

  pr_rop(ColorDisplayPixrect, x, y, w, h, COPY_PIXRECT_TO_COLOR, DisplayRegionPixrect, x, y);
#endif /* DISPLAYBUFFER */

#endif /* SUNDISPLAY */

#if (defined(XWINDOW) || defined(DOS))
  TPRINT(("Enter flush_display_lineregion x=%d, y=%d, w=%d, h=%d\n", x, y, w, h));
  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, x, y, w, h);
  TPRINT(("Exit flush_display_lineregion\n"));
#endif /* DOS */
}
#endif /* I386 */

/************************************************************************/
/*									*/
/*	    f l u s h _ d i s p l a y _ p t r r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	bitoffset  bit offset into word pointed to by ybase		*/
/*	ybase the offset from top of bitmap, as the address of the	*/
/*	       word containing the upper-leftmost bit changed.		*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/

#define BITSPERWORD 16

void flush_display_ptrregion(DLword *ybase, UNSIGNED bitoffset, UNSIGNED w, UNSIGNED h)
#ifdef I386
{ flush_display_buffer(); }
#else
{
  int y, x, baseoffset, realw;
  baseoffset = (((DLword *)ybase) - DisplayRegion68k);
  y = baseoffset / DLWORD_PERLINE;
  x = bitoffset + (BITSPERWORD * (baseoffset - (DLWORD_PERLINE * y)));
#endif /* I386 */

#if (defined(SUNDISPLAY) && defined(DISPLAYBUFFER))
pr_rop(ColorDisplayPixrect, x, y, w, h, COPY_PIXRECT_TO_COLOR, DisplayRegionPixrect, x, y);
#elif (defined(XWINDOW) || defined(DOS))
  TPRINT(("Enter flush_display_ptrregion\n x=%d, y=%d, w=%d, h=%d\n", x, y, w, h));
  (currentdsp->bitblt_to_screen)(currentdsp, DisplayRegion68k, x, y, w, h);
  TPRINT(("Exit flush_display_ptrregion\n"));
#endif /* DOS */
}
