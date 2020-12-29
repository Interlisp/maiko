/* $Id: testdsp.c,v 1.2 1999/01/03 02:07:36 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <sunwindow/window_hs.h>
#include <sunwindow/cms.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sunwindow/win_ioctl.h>
#include <pixrect/pixrect_hs.h>
#include <sun/fbio.h>
#include <sundev/kbd.h>
#include <sundev/kbio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pixrect/pr_planegroups.h>

/**************************************
#include "lispemul.h"
#include "lispmap.h"
#include "address.h"
#include "adr68k.h"
#include "lspglob.h"
#include "emlglob.h"
#include "display.h"
#include "devconf.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"
#include "dbprint.h"		****/

struct screen LispScreen;
struct pixrect *CursorBitMap, *InvisibleCursorBitMap;
struct pixrect *SrcePixRect, *DestPixRect;

int LispWindowFd;
int FrameBufferFd;
int KeyboardFd;

int DisplayWidth, DisplayHeight, DisplayRasterWidth, DisplayType;
int DisplayByteSize;
short *DisplayRegion68k; /* 68k addr of #{}22,0 */
struct cursor CurrentCursor, InvisibleCursor;
struct winlock DisplayLockArea;

/**************************************
extern DLword *EmCursorBitMap68K;
extern IFPAGE *InterfacePage;

int DebugDSP = T;
******************************************/
/************** provide below so can skip the include files *******/
#define BITSPER_DLWORD 16
#define SUN2BW 1
#define SUN4COLOR 2
#define CURSORWIDTH 16
#define CURSORHEIGHT 16
#define DBPRINT(X) printf X
/************** provide above so can skip the include files *******/

/*  ================================================================  */

void init_display2(int display_addr, int display_max)
{
  int mmapstat;
  int ioctlresult;
  char *texture_base, *malloc();
  struct fbtype my_screen;
  char groups[PIXPG_OVERLAY + 1];
  struct fbgattr FBattr;
  struct pixrect *ColorFb;
  int mask, kbtype;

  if ((LispWindowFd = win_screennew(&LispScreen)) == -1) {
    perror("init_display: can't create LispWindow\n");
    exit(-1);
  } else {
#ifdef KBINT
    int_io_open(LispWindowFd);
#endif
    fcntl(LispWindowFd, F_SETFL, fcntl(LispWindowFd, F_GETFL, 0) | O_NONBLOCK);
  }

  DisplayRegion68k = (short *)display_addr;

  if ((FrameBufferFd = open(LispScreen.scr_fbname, 2)) == -1) {
    perror("init_display: can't open FrameBuffer\n");
    exit(-1);
  }

  if ((KeyboardFd = open(LispScreen.scr_kbdname, 2)) == -1) {
    perror("init_display: can't open Keyboard\n");
    exit(-1);
  }

  DBPRINT(("LispScreen.scr_kbdname %s\n", LispScreen.scr_kbdname));
  DBPRINT(("LispScreen.scr_fbname  %s\n", LispScreen.scr_fbname));

  if (ioctl(KeyboardFd, KIOCTYPE, &kbtype) != 0) {
    perror("ioctl(KIOCTYPE,..) fails");
  } else {
    DBPRINT(("ioctl(KIOCTYPE): %d\n", kbtype));
  }

  /* initialize Display parameters */
  if (ioctl(FrameBufferFd, FBIOGTYPE, &my_screen) == -1) {
    perror("init_display: can't find screen parameters\n");
    exit(-1);
  }
  DisplayWidth = my_screen.fb_width;
  DisplayHeight = my_screen.fb_height;
  DisplayRasterWidth = DisplayWidth / BITSPER_DLWORD;
  if ((DisplayWidth * DisplayHeight) > display_max) { DisplayHeight = display_max / DisplayWidth; }

  DBPRINT(("FBIOGTYPE w x h = %d x %d\n", DisplayWidth, DisplayHeight));
  DBPRINT(("          type  = %d\n", my_screen.fb_type));
  DBPRINT(("          bpp   = %d\n", my_screen.fb_depth));

  /** now attempt to use the FBIOGATTR call for more information **/

  ioctlresult = ioctl(FrameBufferFd, FBIOGATTR, &FBattr);
  if (ioctlresult >= 0) {
    DBPRINT(("FBIOGATTR realtype = %d\n", FBattr.real_type));
    DBPRINT(("   (real) w x h = %d x %d\n", FBattr.fbtype.fb_width, FBattr.fbtype.fb_height));
    DBPRINT(("   (real) type  = %d\n", FBattr.fbtype.fb_type));
    DBPRINT(("   (real) bpp   = %d\n", FBattr.fbtype.fb_depth));
    DBPRINT(("          emuls    = %d %d %d %d %d\n", FBattr.emu_types[0], FBattr.emu_types[1],
             FBattr.emu_types[2], FBattr.emu_types[3], FBattr.emu_types[4]));
  } else {
    DBPRINT(("ioctl(fd,FBIOGATTR,&FBattr) => %d\n", ioctlresult));
  }

  ColorFb = pr_open("/dev/fb");

  DBPRINT(("pixrect w, h, depth = %d %d %d\n", ColorFb->pr_size.x, ColorFb->pr_size.y,
           ColorFb->pr_depth));

  pr_getattributes(ColorFb, &mask);
  DBPRINT(("        getattrmask = %d\n", mask));

  groups[0] = 0;
  groups[1] = 0;
  groups[2] = 0;
  groups[3] = 0;
  groups[4] = 0;
  pr_available_plane_groups(ColorFb, sizeof(groups), groups);
  DBPRINT(("plane groups = current: %d\n", groups[0]));
  DBPRINT(("               mono   : %d\n", groups[1]));
  DBPRINT(("               8bitcol: %d\n", groups[2]));
  DBPRINT(("               ovrlyen: %d\n", groups[3]));
  DBPRINT(("               ovrly  : %d\n", groups[4]));

  pr_available_plane_groups(ColorFb, sizeof(groups), groups);

  /* try to clear enable plane if it exists */

  if (groups[PIXPG_OVERLAY] && groups[PIXPG_OVERLAY_ENABLE]) {
    pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
    pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
    pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
  }

  {
    int currgroup;
    currgroup = pr_get_plane_group(ColorFb);
    DBPRINT(("current planegroup: %d, unknown: %d\n", currgroup, currgroup == PIXPG_CURRENT));
  }
/*  ================================================================ */
#ifdef NOTUSED
  if (my_screen.fb_type == FBTYPE_SUN2BW) {
    if (ioctlresult) >= 0) {
        if (FBattr.real_type == FBTYPE_SUN4COLOR) {
          /* must be color display cgfour */
          /* we've already gotten the plane groups from above */
          if (groups[PIXPG_OVERLAY] && groups[PIXPG_OVERLAY_ENABLE]) {
            pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
            pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
            pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
          }
        } else { /* not cgfour */
          printf("initdisplay: Unsupported FBTYPE %d\n", FBattr.real_type);
        }
      } /* if ioctlresult...... */
  }     /* if ...FBTYPE_SUN2BW end */
#endif
  /*  ================================================================ */

  DisplayLockArea.wl_rect.r_width = DisplayWidth;
  DisplayLockArea.wl_rect.r_height = DisplayHeight;
  init_cursor();
  DisplayByteSize = ((DisplayWidth * DisplayHeight / 8 + (getpagesize() - 1)) & -getpagesize());

  DBPRINT(("Display Addr: 0x%x\n", DisplayRegion68k));
  DBPRINT(("length: 0x%x\n", DisplayByteSize));
  DBPRINT(("page size: 0x%x\n", getpagesize()));

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

  DBPRINT(("before clear display()\n"));

  clear_display();

  DBPRINT(("after  clear_display()\n"));

  /* initialize pixrects used in pilotbitblt (internal will change) */
  SrcePixRect = mem_point(0, 0, 1, NULL);
  DestPixRect = mem_point(0, 0, 1, NULL);

  DBPRINT(("before set_cursor\n"));
}

/*  ================================================================  */

init_cursor() {
  CursorBitMap = mem_create(CURSORWIDTH, CURSORHEIGHT, 1);
  mpr_mdlinebytes(CursorBitMap) = CURSORWIDTH >> 3; /* 2(byte) */
  CurrentCursor.cur_xhot = 0;
  CurrentCursor.cur_yhot = 0;
  CurrentCursor.cur_shape = CursorBitMap;
  CurrentCursor.cur_function = PIX_SRC | PIX_DST;

  /*  Invisible Cursor */
  InvisibleCursorBitMap = mem_create(0, 0, 1);
  InvisibleCursor.cur_xhot = 0;
  InvisibleCursor.cur_yhot = 0;
  InvisibleCursor.cur_shape = InvisibleCursorBitMap;
  InvisibleCursor.cur_function = /*PIX_SRC |*/ PIX_DST;

  win_setcursor(LispWindowFd, &InvisibleCursor);
  win_setmouseposition(LispWindowFd, 0, 0);
}

/*  ================================================================  */

display_before_exit() {
  clear_display();

  win_screendestroy(LispWindowFd);
#ifdef KBINT
  int_io_close(LispWindowFd);
#endif
  close(LispWindowFd);
}

/*  ================================================================  */

clear_display() {
  register short *word;
  register int w, h;
  word = DisplayRegion68k;
  for (h = DisplayHeight; (h--);) {
    for (w = DisplayRasterWidth; (w--);) { *word++ = 0; }
  }
}

/*  ================================================================  */

paint_display() {
  register short *word;
  register int w, h;
  word = DisplayRegion68k;
  for (h = DisplayHeight; (h--);) {
    for (w = DisplayRasterWidth; (w--);) { *word++ = w; }
  }
}

/*  ================================================================  */

#define BYTESPER_PAGE 512

read_datum(char *lispworld)
{
  int srcefile; /* fd */
  int i, j, sysout_size;
  int lispworld_offset;
  struct stat stat_buf;
  char *charptr;
  int readresult;
  char bigbuff[BYTESPER_PAGE];

  srcefile = open("fake.sysout", O_RDONLY, NULL);

  /* get sysout file size in halfpage(256) */
  if (fstat(srcefile, &stat_buf) == -1) {
    perror("read_datum: can't get srcefile size (fstat fails)");
    exit(-1);
  }
  sysout_size = stat_buf.st_size / BYTESPER_PAGE * 2;

  DBPRINT(("file size       =   %d\n", stat_buf.st_size));
  DBPRINT(("sysout size / 2 = 0x%x\n", sysout_size / 2));

  lispworld_offset = 0;
  for (i = 0; i < (sysout_size / 2); i++) {
    lispworld_offset += BYTESPER_PAGE;
    charptr = (char *)lispworld + lispworld_offset;
#ifdef BUFFER
    readresult = read(srcefile, bigbuff, BYTESPER_PAGE);
    for (j = 0; j < BYTESPER_PAGE; j++) { *(charptr + j) = bigbuff[j]; }
#else
    readresult = read(srcefile, charptr, BYTESPER_PAGE);
#endif
    if (readresult == -1) {
      printf("read_datum: can't read srcefile file at %d\n", i);
      perror("read() error was");
      exit(-1);
    };
  };
  DBPRINT(("srcefile is read completely\n"));
  close(srcefile);
}

/*  ================================================================  */

int_io_open(){};  /* stubs for other parts of our prog. */
int_io_close(){}; /* stubs for other parts of our prog. */

main() {
  int maxdisplayregion; /* in what units? */
  int realaddr;

  realaddr = valloc(5000000);
  if (realaddr == 0) {
    printf("valloc returns 0\n");
  } else {
    maxdisplayregion = 4000000; /* assume 4Mb display reg. */
    init_display2(realaddr, maxdisplayregion);
    paint_display();
    sleep(4);

    DBPRINT(("before read_datum\n"));
    read_datum(realaddr);

    sleep(4);
    display_before_exit();
  }
}
