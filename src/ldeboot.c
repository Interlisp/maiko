/* $Id: ldeboot.c,v 1.3 1999/01/03 02:07:13 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>      // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "unixfork.h"

#if defined(sun) && !defined(OS5)
#define USESUNSCREEN
#else
#undef USESUNSCREEN
#endif

#ifdef USESUNSCREEN
#include <sys/fbio.h>
#ifndef FBTYPE_SUNFAST_COLOR
#define FBTYPE_SUNFAST_COLOR 12
#endif

#define LDEMONO "ldesingle"
#define LDECOLOR "ldemulti"
#define LDETRUECOLOR "ldetruecolor"

#endif /* USESUNSCREEN */

#ifdef XWINDOW
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define LDEX "ldex"
#endif /* XWINDOW */

#ifdef SDL
#define LDESDL "ldesdl"
#endif

/************************************************************************/
/*									*/
/*				m a i n					*/
/*									*/
/*	Kick-start program for the "Lisp Development Environment" (lde)	*/
/*									*/
/************************************************************************/

int main(int argc, char *argv[]) {
  int i;
  char *filetorun = NULL;
  char filetorunpath[PATH_MAX];
  char *dirsepp = NULL;
  char *displayName = (char *)NULL;
  int doFork = 1;
#ifdef USESUNSCREEN
  int FrameBufferFd;
  struct fbtype my_screen;
  struct fbinfo FB_info;
  struct fbgattr FBattr;
#endif /* USESUNSCREEN */

  /* Kickstart program for the Lisp Development Environment (LDE).
     Display Device       emulator
     CG3, CG6             lde.multi
     BW2, CG2, CG4, CG9   lde.single

     FB-TYPE       REAL-TYPE
     BW2      2             x
     CG2      3             3
     CG3      8             6
     CG4      2             8
     CG6      8             12
     CG8      6             7
     CG9(GP1) 4             4    ;gpconfig -f -b
     CG9(GP1) 2            13    ;gpconfig gpone0 -f -b cgtwo0
     ;We assume This config for GXP model
     */

  /* look for a -display argument that could tell us X11 vs SDL for display
   */
  for (i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "-display") == 0)) {
      if (i == argc - 1) {
        (void)fprintf(stderr, "Missing argument to -display option.\n");
        exit(1);
      }
      displayName = argv[++i];
    }
  }

  /* Unless prevented by -NF option, fork the process, while small, to handle
   * process communications and subsequent forks
   */

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-NF") == 0) {
      doFork = 0;
      break;
    }
  }
  if (doFork) fork_Unix();

#ifdef SDL
#ifdef XWINDOW
  /* if we have SDL *and* XWINDOW we only do SDL if requested */
  if (displayName && (0 == strcmp(displayName, "SDL"))) {
#else
  /* otherwise SDL is it */
  {
#endif
    filetorun = LDESDL;
    goto run;
  }
#endif /* SDL */

#ifdef XWINDOW
  /* If an X server exists as specified in the -display option
   * or environment variable DISPLAY, ldex is started.
   */
  {
    Display *Xdisplay = XOpenDisplay(displayName);

    if (Xdisplay) {
      /* success connecting to X server. Close it now, it will be reopened by ldex */
      XCloseDisplay(Xdisplay);
      filetorun = LDEX;
      goto run;
    } else {
      (void)fprintf(stderr, "Unable to open X11 display %s\n",
              displayName ? displayName : "from DISPLAY");
      exit(1);
    }
  }
#endif /* XWINDOW */

#ifdef USESUNSCREEN
  if ((FrameBufferFd = open("/dev/fb", O_RDWR)) < 0) {
    (void)fprintf(stderr, "lde: can't open FrameBuffer\n");
    exit(1);
  }
  if (ioctl(FrameBufferFd, FBIOGTYPE, &my_screen) < 0) {
    perror(argv[0]);
    exit(1);
  }

  switch (my_screen.fb_type) {
    case FBTYPE_SUN4COLOR: /*  cg3 or cg6 */
      if (ioctl(FrameBufferFd, FBIOGATTR, &FBattr) >= 0) {
        if (FBattr.real_type == FBTYPE_SUN3COLOR ||   /* cg3 */
            FBattr.real_type == FBTYPE_SUNFAST_COLOR) /* cg6 */
        {
          filetorun = LDECOLOR;
        }
      }
      break;
    case FBTYPE_SUN2BW: /* bw2, cg4 or cg9  */
      filetorun = LDEMONO;
      break;
    case FBTYPE_SUN3COLOR: /* cg8 */
      if (ioctl(FrameBufferFd, FBIOGATTR, &FBattr) >= 0) {
        if (FBattr.real_type == FBTYPE_MEMCOLOR) { filetorun = LDETRUECOLOR; }
      }
      break;
    case FBTYPE_SUN2COLOR: /* cg2  */
      filetorun = LDEMONO;
      break;
    default: break;
  }
  if (filetorun == (char *)NULL) {
    perror("lde: This Display Model is not supported\n");
    exit(1);
  }
  close(FrameBufferFd);
#endif /* USESUNSCREEN */

  run:
  if (filetorun == NULL) {
    (void)fprintf(stderr, "Unable to determine what display program to run.\n");
    exit(1);
  }

  /* construct invocation path of display program parallel to this one,
   * so that it will have higher probability of finding a corresponding
   * display program if there are multiple versions findable via PATH
   */
  dirsepp = strrchr(argv[0], '/');
  if (dirsepp == NULL) {
    argv[0] = filetorun;
  } else {
    /* copy up to and including the final "/" in the path */
    dirsepp = stpncpy(filetorunpath, argv[0], dirsepp + 1 - argv[0]);

    /* dirsepp now points to the trailing null in the copy */
    strncpy(dirsepp, filetorun, PATH_MAX - (dirsepp - filetorunpath));
    argv[0] = filetorunpath;
  }
  execvp(argv[0], argv);
  perror(argv[0]);
  exit(1);

}
