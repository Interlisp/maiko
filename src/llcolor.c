/* $Id: llcolor.c,v 1.2 1999/01/03 02:07:15 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>

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

#include "llcolordefs.h"

extern int MonoOrColor;
extern DLword *ColorDisplayRegion68k;
extern int Dispcolorsize;
extern int Inited_Color;

int MonoOrColor = MONO_SCREEN;
DLword *ColorDisplayRegion68k = NULL;
int Dispcolorsize = 0;
int Inited_Color = NIL;
extern struct pixrect *ColorDisplayPixrect;
extern int displaywidth, displayheight, FrameBufferFd;

/*******************************************************************/
/*	Func name	: cgfour_init_color_display(args)
        Arg(s)		: COLOR BITMAP ADDRESS(LISPPTR)
        Desc		: Assign to SUBR 0210
                          mmap LispPTR to Color Display FB.
        By  Takeshi
*/
/*******************************************************************/
#ifdef COLOR
LispPTR cgfour_init_color_display(LispPTR color_bitmapbase) /* SUBR 0210 */ /* COLOR BITMAP ADDRESS */
{
  struct pixrect *ColorFb;
  struct pixrect *color_source;
  int mmapstat;

  if (MonoOrColor == COLOR_SCREEN) {
    printf("You can not initialize the color screen from inside color screen. \n");
  }

  ColorDisplayRegion68k = NativeAligned2FromLAddr(color_bitmapbase);

  Dispcolorsize =
      ((displaywidth * displayheight + (getpagesize() - 1)) & -getpagesize()); /* 8 bit depth */
#ifndef DISPLAYBUFFER
  ColorFb = pr_open("/dev/fb");
#else
  ColorFb = ColorDisplayPixrect;
#endif /* DISPLAYBUFFER */

#ifndef DISPLAYBUFFER
  color_source = mem_point(displaywidth, displayheight, 8, ColorDisplayRegion68k);

  pr_set_plane_group(ColorFb, PIXPG_8BIT_COLOR);
  pr_rop(ColorFb, 0, 0, displaywidth, displayheight, PIX_SRC, color_source, 0, 0);

  mmapstat = (int)mmap(ColorDisplayRegion68k, Dispcolorsize, PROT_READ | PROT_WRITE,
#ifdef OS4
                       MAP_FIXED |
#endif
                           MAP_SHARED,
                       FrameBufferFd, 0x40000);
  if (mmapstat == -1) {
    perror("cgfour_init_color_display: ERROR at mmap system call\n");
    error(
        "cgfour_init_color_display: ERROR at mmap system call\n You may be able to continue by "
        "typing 'q'");
  }
#endif /* DISPLAYBUFFER */

  printf("COLOR-INIT OK BMBASE=0x%x\nNATIVE:= 0x%x)\n", color_bitmapbase, ColorDisplayRegion68k);

  Inited_Color = T; /* Color display is active. */

  return (color_bitmapbase);
} /* end cgfour_init_color_display */

#else  /* COLOR */

LispPTR cgfour_init_color_display(LispPTR color_bitmapbase) /* SUBR 0210 */ /* COLOR BITMAP ADDRESS */
{
  printf("Color is not supported.\n");
  return (NIL);
}
#endif /* COLOR */

/*******************************************************************/
/*	Func name	: cgfour_change_screen_mode(which_screen)
        Arg(s)		: MONO_SCREEN OR COLOR_SCREEN
        Desc		: Assign to SUBR 0211
                          Change screen Mono to Color,vice versa.
        By  Takeshi
*/
/*******************************************************************/
#ifdef COLOR
LispPTR cgfour_change_screen_mode(LispPTR which_screen)
{ /* subr 0211 */
  struct pixrect *ColorFb;
  extern ScreenLocked;
  extern DLword *EmCursorX68K, *EmCursorY68K;

  int mmapstat;

#ifndef DISPLAYBUFFER
  ColorFb = pr_open("/dev/fb");
#else
  ColorFb = ColorDisplayPixrect;
#endif /* DISPLAYBUFFER */

  ScreenLocked = T;
  taking_mouse_down();

  switch (which_screen & 0xf) {
    case MONO_SCREEN: { /* resume mono screen */
#ifdef DISPLAYBUFFER
      mmapstat = (int)munmap(ColorDisplayRegion68k, Dispcolorsize);
      if (mmapstat == -1) {
        perror("cg_four_change_screen: ERROR at munmap system call\n");
        exit(0);
      } /* end if(mmapstat) */
      save_color_screen();
#endif /* DISPLAYBUFFER */

      pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
      pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
      pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
#ifdef DISPLAYBUFFER
      flush_display_buffer();
#endif /* DISPLAYBUFFER */

      MonoOrColor = MONO_SCREEN;
      break;
    }
    case COLOR_SCREEN: {
#ifndef DISPLAYBUFFER
      pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
      pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_CLR, 0, 0, 0);
#endif /* DISPLAYBUFFER */

      pr_set_plane_group(ColorFb, PIXPG_8BIT_COLOR);
#ifdef DISPLAYBUFFER
      restore_color_screen();
      mmapstat = (int)mmap(ColorDisplayRegion68k, Dispcolorsize, PROT_READ | PROT_WRITE,
#ifdef OS4
                           MAP_FIXED |
#endif
                               MAP_SHARED,
                           FrameBufferFd, 0x40000);
      if (mmapstat == -1) {
        perror("cg_four_change_screen: ERROR at mmap system call\n");
        exit(0);
      }
#endif /* DISPLAYBUFFER */

      MonoOrColor = COLOR_SCREEN;
      break;
    }
    default: { error("cgfour_change_screen_mode:Unknown mode:"); }
  }

#ifndef DISPLAYBUFFER
  pr_close(ColorFb);
#endif /* DISPLAYBUFFER */

  taking_mouse_up(*EmCursorX68K, *EmCursorY68K);
  ScreenLocked = NIL;
  return (which_screen);
}
#else  /* COLOR */

LispPTR cgfour_change_screen_mode(LispPTR which_screen)
{
  printf("Color is not supported.\n");
  return (NIL);
}
#endif /* COLOR */

/*******************************************************************/
/*	Func name	: cgfour_set_colormap(args)
        Arg(s)		: Passed by args
                          index: colormap index(0~255)
                          red,green,blue:(0~255)
        Desc		: Assign to SUBR 0212
                          Set Colormap entry
        By  Takeshi
*/
/*******************************************************************/
#ifdef COLOR
LispPTR cgfour_set_colormap(LispPTR args[])
{
  int index;
  struct pixrect *ColorFb;
  unsigned char RED_colormap;
  unsigned char GRN_colormap;
  unsigned char BLU_colormap;


  index = args[0] & 0xff;

  RED_colormap = (unsigned char)(args[1] & 0xff);
  GRN_colormap = (unsigned char)(args[2] & 0xff);
  BLU_colormap = (unsigned char)(args[3] & 0xff);

  ColorFb = pr_open("/dev/fb");

  /*
   *  pr_putcolormap(pr, index, count, red, green, blue)
   *     struct pixrect *pr;
   *     int index, count;
   *     unsigned char red[], green[], blue[];
   */
  if (pr_putcolormap(ColorFb, index, 1, &RED_colormap, &GRN_colormap, &BLU_colormap) == -1)
    perror("putcolormap:");
  pr_close(ColorFb);
  return (T);
}
#else  /* COLOR */

LispPTR cgfour_set_colormap(LispPTR args[])
{
  printf("Color is not supported.\n");
  return (NIL);
}
#endif /* COLOR */

#ifdef COLOR
#ifdef DISPLAYBUFFER
static struct pixrect *saved_screen;
static int Screen_Saved = T;

void save_color_screen(void) {
  if (!Screen_Saved) {
    saved_screen = mem_point(displaywidth, displayheight, 8, ColorDisplayRegion68k);
    pr_rop(saved_screen, 0, 0, displaywidth, displayheight, PIX_SRC, ColorDisplayPixrect, 0, 0);
    Screen_Saved = T;
  } /* end if(!Screen_Saved) */
} /* end save_color_screen() */

void restore_color_screen(void) {
  if (Screen_Saved) {
    saved_screen = mem_point(displaywidth, displayheight, 8, ColorDisplayRegion68k);
    pr_rop(ColorDisplayPixrect, 0, 0, displaywidth, displayheight, PIX_SRC, saved_screen, 0, 0);
    Screen_Saved = NIL;
  } /* end if(Screen_Saved) */
} /* end restore_color_screen() */
#endif /* DISPLAYBUFFER */

static unsigned char red_colormap[256], green_colormap[256], blue_colormap[256];
static int Saved_Colormap = NIL;
void save_colormap(void) {
  struct pixrect *Color_Fb;

  if (!Saved_Colormap) {
    Color_Fb = pr_open("/dev/fb");
    if ((pr_getcolormap(Color_Fb, 0, 256, red_colormap, green_colormap, blue_colormap)) == -1)
      perror("save_color_map:");
    else
      Saved_Colormap = T;

    pr_close(Color_Fb);
  } /* end if( !Saved_Colormap ) */
} /* end save_colormap() */

void restore_colormap(void) {
  struct pixrect *Color_Fb;

  if (Saved_Colormap) {
    Color_Fb = pr_open("/dev/fb");
    if ((pr_putcolormap(Color_Fb, 0, 256, red_colormap, green_colormap, blue_colormap)) == -1)
      perror("restore_color_map:");
    pr_close(Color_Fb);
    Saved_Colormap = NIL;
  } /* end if( Saved_Colormap ) */
} /* end restore_colormap() */
#endif /* COLOR */
