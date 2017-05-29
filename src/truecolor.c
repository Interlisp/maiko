/* $Id: truecolor.c,v 1.2 1999/01/03 02:07:38 sybalsky Exp $ (C) Copyright Venue, All Rights
 * Reserved  */
static char *id = "$Id: truecolor.c,v 1.2 1999/01/03 02:07:38 sybalsky Exp $ Copyright (C) Venue";

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

#include <stdio.h>
#include <sys/mman.h>
#include <pixrect/pixrect_hs.h>

#include "lispemul.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "lispmap.h"
#include "adr68k.h"
#include "arith.h"
#include "devconf.h"

#include "picture.h"

#define FALSE 0
#define TRUE !FALSE

#define COLOR_INIT 0
#define COLOR_OVERLAYREGION 1
#define COLOR_VIDEOREGION 2
#define COLOR_VIDEOINIT 3

#define IntToFixp(C, Lisp)                                \
  {                                                       \
    int *base;                                            \
    base = (int *)Addr68k_from_LADDR((unsigned int)Lisp); \
    *base = C;                                            \
  }

extern int displaywidth, displayheight, DisplayRasterWidth;
extern int ScreenLocked;
extern DLword *EmCursorX68K, *EmCursorY68K;
extern Pixrect *TrueColorFb;
extern int FrameBufferFd;
extern int Inited_Video;

int Inited_TrueColor = NIL;

int TrueColor_Op(args) LispPTR *args;
{
  int op, ret_value = NIL;
  /*
          N_GETNUMBER( args[0], op, bad_arg );
  */
  op = (DLword)args[0];

  switch (op) {
    case COLOR_INIT: ret_value = TrueColor_Initialize(args[1]); break;
    case COLOR_VIDEOINIT: ret_value = TrueColor_VideoInitialize(args[1]); break;
    case COLOR_OVERLAYREGION: {
      int left, top, bottom, width, height;
      if (args[1] != NIL_PTR) {
        LispPTR region, value;
        int bottom;
        /* get lisp region elements */
        region = args[1];
        value = car(region);
        N_GETNUMBER(value, left, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, bottom, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, width, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, height, bad_arg);
        top = displayheight - (bottom + height);
        if (args[2] == ATOM_T) {
          Overlay_Region(left, top, width, height, TRUE);
        } else {
          Overlay_Region(left, top, width, height, FALSE);
        } /* end if( args[2] ) */
        ret_value = T;
      } /* end if( arg[1] ) */
    }   /* end case */
    break;
    case COLOR_VIDEOREGION: {
      int left, top, bottom, width, height;
      if (args[1] != NIL_PTR) {
        LispPTR region, value;
        int bottom;
        /* get lisp region elements */
        region = args[1];
        value = car(region);
        N_GETNUMBER(value, left, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, bottom, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, width, bad_arg);
        region = cdr(region);
        value = car(region);
        N_GETNUMBER(value, height, bad_arg);
        top = displayheight - (bottom + height);
      } else {
        left = 0;
        top = 0;
        width = displaywidth;
        height = displayheight;
      } /* end if( arg[1] ) */

      if (args[2] == NIL_PTR) { /* video region clear */
        cgeight_video_region(left, top, width, height, FALSE);
      } else { /* video region set */
        cgeight_video_region(left, top, width, height, TRUE);
      } /* end if( args[2] ) */
      ret_value = T;
    } /* end case */
    break;
    defaults:
      break;
  } /* end switch( op ) */

bad_arg:
  return (ret_value);

} /* end TrueColor_Op */

extern int Inited_Color;
extern int DisplayType;

Pixrect *OverlayCursor, *OverlaySave;
DLword *OverlayRegion68k;

int TrueColor_Initialize(overlay_bmbase) LispPTR overlay_bmbase;
{
  unsigned int pict, *ret_value;
  Pixrect *source;
  int mmapstat, size;

  if (Inited_Color) {
    printf("cgeight_init_color_display: 8 bits color display has already initialized.\n");
  } /* end if( Inited_Color ) */

  if (Inited_TrueColor) {
    printf("cgeight_init_color_display: 24 bits color display has already initialized.\n");
  } /* end if( Inited_TrueColor ) */

  if (DisplayType != SUNMEMCOLOR) {
    error("cgeight_init_color_display: Unsupported FBreal_type %d\n", DisplayType);
  } /* end if( DisplayType ) */

  OverlayRegion68k = Addr68k_from_LADDR(overlay_bmbase);
  size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

  ScreenLocked = T;
  taking_mouse_down();

  pict = (unsigned int)cgeight_init_color_display();

  source = mem_point(displaywidth, displayheight, 1, OverlayRegion68k);
  pr_set_plane_group(TrueColorFb, PIXPG_OVERLAY_ENABLE);
  pr_rop(TrueColorFb, 0, 0, displaywidth, displayheight, PIX_SRC, source, 0, 0);
  pr_set_plane_group(TrueColorFb, PIXPG_24BIT_COLOR);

  mmapstat = (int)mmap(OverlayRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                       MAP_FIXED |
#endif
                           MAP_SHARED,
                       FrameBufferFd, 0x20000);

  if (mmapstat == -1) {
    perror("TrueColor_Initialize: ERROR at mmap system call\n");
    exit(0);
  } /* end if( mmapstat ) */

  OverlayCursor = mem_create(16, 16, 32);
  OverlaySave = mem_create(16, 16, 32);

  Inited_TrueColor = TRUE;

  taking_mouse_up(*EmCursorX68K, *EmCursorY68K);
  overlay_mouse_up(*EmCursorX68K, *EmCursorY68K);
  ScreenLocked = NIL;

  ret_value = (unsigned int *)createcell68k(TYPE_FIXP);
  *ret_value = pict;
  return (LADDR_from_68k(ret_value));

} /* end TrueColor_Initialize */

DLword *VideoEnableRegion68k;

int TrueColor_VideoInitialize(videoenable_bmbase) LispPTR videoenable_bmbase;
{
  Pixrect *source;
  int mmapstat, size;

  ScreenLocked = T;

  clear_video_region();
  Video_Initialize(FrameBufferFd);

  VideoEnableRegion68k = Addr68k_from_LADDR(videoenable_bmbase);
  size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

  source = mem_point(displaywidth, displayheight, 1, VideoEnableRegion68k);
  pr_set_plane_group(TrueColorFb, PIXPG_VIDEO_ENABLE);
  pr_rop(TrueColorFb, 0, 0, displaywidth, displayheight, PIX_SRC, source, 0, 0);
  pr_set_plane_group(TrueColorFb, PIXPG_24BIT_COLOR);

  mmapstat = (int)mmap(VideoEnableRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                       MAP_FIXED |
#endif
                           MAP_SHARED,
                       FrameBufferFd, 0x533000);

  if (mmapstat == -1) {
    perror("TrueColor_VideoInitialize: ERROR at mmap system call\n");
    exit(0);
  } /* end if( mmapstat ) */

  ScreenLocked = NIL;
  return (T);

} /* TrueColor_VideoInitialize */

int Overlay_Region(left, top, width, height, flg) int left, top, width, height, flg;
{
  ScreenLocked = T;
  overlay_mouse_down();

  cgeight_overlay_region(left, top, width, height, (flg ? FALSE : TRUE));

  overlay_mouse_up(*EmCursorX68K, *EmCursorY68K);
  ScreenLocked = NIL;

} /* end Overlay_Region */

#ifdef TRUECOLOR
#define TrackCursor(cx, cy)                          \
  {                                                  \
    *CLastUserActionCell68k = MiscStats->secondstmp; \
    overlay_mouse_down();                            \
    taking_mouse_down();                             \
    taking_mouse_up(cx, cy);                         \
    overlay_mouse_up(cx, cy);                        \
    *EmCursorX68K = cx;                              \
    *EmCursorY68K = cy;                              \
  }
#endif /* TRUECOLOR */

extern Pixrect *OverlaySave, *OverlayCursor;

overlay_mouse_up(newx, newy) int newx, newy;
{
  pr_set_plane_group(TrueColorFb, PIXPG_OVERLAY_ENABLE);
  pr_rop(TrueColorFb, newx, newy, 16, 16, PIX_SRC, OverlaySave, 0, 0);
  pr_rop(TrueColorFb, 0, 0, 16, 16, PIX_SRC | PIX_DST, OverlayCursor, newx, newy);
  pr_set_plane_group(TrueColorFb, PIXPG_24BIT_COLOR);

} /* end overlay_mouse_up */

extern int LastCursorX, LastCursorY;

overlay_mouse_down() {
  pr_set_plane_group(TrueColorFb, PIXPG_OVERLAY_ENABLE);
  pr_rop(OverlaySave, 0, 0, 16, 16, PIX_SRC, TrueColorFb, LastCursorX, LastCursorY);
  pr_set_plane_group(TrueColorFb, PIXPG_24BIT_COLOR);

} /* end overlay_mouse_down */

truecolor_before_exit() {
  if (Inited_TrueColor) {
    { /* fill region */
      int h, w;
      unsigned short *ptr;
      ptr = (unsigned short *)OverlayRegion68k;
      for (h = displayheight; (h--);) {
        for (w = DisplayRasterWidth; (w--);) { *(ptr++) = ~0; } /* end for( w ) */
      }                                                         /* end for( h ) */
    }

    pr_set_plane_group(TrueColorFb, PIXPG_24BIT_COLOR);
    pr_rop(TrueColorFb, 0, 0, displaywidth, displayheight, PIX_SRC | PIX_COLOR(0xffffff), 0, 0, 0);

#ifdef VIDEO
    if (Inited_Video) {
      { /* clear video enable region */
        int h, w;
        unsigned short *ptr;
        ptr = (unsigned short *)VideoEnableRegion68k;
        for (h = displayheight; (h--);) {
          for (w = DisplayRasterWidth; (w--);) { *(ptr++) = 0xffff; } /* end for( w ) */
        }                                                             /* end for( h ) */
      }

      Video_Close();

    }  /* end if( Inited_Video ) */
#endif /* VIDEO */

    pr_close(TrueColorFb);

  } /* end if( Inited_TrueColor ) */

} /* truecolor_before_exit */

char *valloc();
char *HideOverlayRegion;
#ifdef VIDEO
char *HideVideoEnableRegion;
extern int Video_OnOff_Flg;
static int video_onoff;
#endif /* VIDEO */

truecolor_before_raid() {
  int size;

  if (Inited_TrueColor) {
    size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

    if ((HideOverlayRegion = valloc(size)) == 0) {
      printf("can't valloc hide space\n");
      return (-1);
    } /* end if( HideOverlayRegion ) */

    copy_region(OverlayRegion68k, HideOverlayRegion, DisplayRasterWidth, displayheight);

    { /* fill region */
      int h, w;
      unsigned short *ptr;
      ptr = (unsigned short *)OverlayRegion68k;
      for (h = displayheight; (h--);) {
        for (w = DisplayRasterWidth; (w--);) { *(ptr++) = 0xffff; } /* end for( w ) */
      }                                                             /* end for( h ) */
    }
  } /* end if( Inited_TrueColor ) */

#ifdef VIDEO
  if (Inited_Video) {
    if ((video_onoff = Video_OnOff_Flg)) Video_OnOff(FALSE);
    if ((HideVideoEnableRegion = valloc(size)) == 0) {
      printf("can't valloc hide space\n");
      return (-1);
    } /* end if( HideVideoEnableRegion ) */

    copy_region(VideoEnableRegion68k, HideVideoEnableRegion, DisplayRasterWidth, displayheight);

    { /* clear video enable region */
      int h, w;
      unsigned short *ptr;
      ptr = (unsigned short *)VideoEnableRegion68k;
      for (h = displayheight; (h--);) {
        for (w = DisplayRasterWidth; (w--);) { *(ptr++) = 0xffff; } /* end for( w ) */
      }                                                             /* end for( h ) */
    }
  }    /* end if( Inited_Video ) */
#endif /* VIDEO */

} /* end truecolor_before_raid */

truecolor_after_raid() {
  int size, mmapstat;

  if (Inited_TrueColor) {
    size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

    mmapstat = (int)mmap(OverlayRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                         MAP_FIXED |
#endif
                             MAP_SHARED,
                         FrameBufferFd, 0x20000);

    if (mmapstat == -1) {
      perror("TrueColor_Initialize: ERROR at mmap system call\n");
      exit(0);
    } /* end if( mmapstat ) */

    copy_region(HideOverlayRegion, OverlayRegion68k, DisplayRasterWidth, displayheight);
    free(HideOverlayRegion);
  } /* end if( Inited_TrueColor ) */

#ifdef VIDEO
  if (Inited_Video) {
    mmapstat = (int)mmap(VideoEnableRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                         MAP_FIXED |
#endif
                             MAP_SHARED,
                         FrameBufferFd, 0x533000);

    if (mmapstat == -1) {
      perror("TrueColor_VideoInitialize: ERROR at mmap system call\n");
      exit(0);
    } /* end if( mmapstat ) */

    copy_region(HideVideoEnableRegion, VideoEnableRegion68k, DisplayRasterWidth, displayheight);
    free(HideVideoEnableRegion);
    if (video_onoff) Video_OnOff(TRUE);

  }    /* end if( Inited_Video ) */
#endif /* VIDEO */

} /* end truecolor_after_raid */
