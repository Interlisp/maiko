/* $Id: ldeboot.c,v 1.3 1999/01/03 02:07:13 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: ldeboot.c,v 1.3 1999/01/03 02:07:13 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"

#if defined(sun) && !defined(OS5)
#define USESUNSCREEN
#else
#undef USESUNSCREEN
#endif

#include <stdio.h>
#include <ctype.h>

#include <sys/ioctl.h>
#ifdef USESUNSCREEN
#ifdef OS5
#include <sys/fbio.h>
#else
#include <sun/fbio.h>
#endif /* OS5 */
#endif /* USESUNSCREEN */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef XWINDOW
#ifndef SYSVONLY
#include <strings.h>
#endif /* SYSVONLY */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "XVersion.h"
#define LDEX "ldex"
extern char *getenv();
#endif /* XWINDOW */

#define LDEMONO "ldesingle"
#define LDECOLOR "ldemulti"
#define LDETRUECOLOR "ldetruecolor"

#define FBTYPE_SUNFAST_COLOR 12

char filetorun[30];

/************************************************************************/
/*									*/
/*				m a i n					*/
/*									*/
/*	Kick-start program for the "Lisp Development Environment" (lde)	*/
/*									*/
/************************************************************************/

main(argc, argv) int argc;
char **argv;
{
  char Earg[30], Ename[30], **newargv;
  int i;
#ifdef USESUNSCREEN
  struct fbtype my_screen;
#endif /* USESUNSCREEN */

  int FrameBufferFd;

#ifdef USESUNSCREEN
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

#ifdef XWINDOW
  /* If X-Server exists on the host specified in -display option
     or environment variable DISPLAY, ldex is started. Othewise
     ldesingle or ldemulti.
     */
  {
    char *Display_Name = (char *)NULL;
    Display *Xdisplay = (Display *)NULL;
    int i, option;
    char *pos;

    for (i = 1; i < argc; i++) {
#ifdef XV11R1
      pos = strchr(argv[i], ':');
      if (pos != NULL) {
        Display_Name = argv[i];
        continue;
      }
#endif /* XV11R1 */

#ifndef XV11R1
      if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "-display") == 0)) {
        if (i == argc) break;
        pos = (char *)strchr(argv[++i], ':');
        if (pos != NULL) { Display_Name = argv[i]; }
        continue;
      }
#endif /* XV11R1 */

    } /*end for() */

    if ((Xdisplay = XOpenDisplay(Display_Name)) != (Display *)NULL) {
      /* success to connect X-server */
      XCloseDisplay(Xdisplay);
      strcpy(filetorun, LDEX);

#ifdef FORKCOMM
      /* JRB - call fork_Unix here, while we're REALLY small, unless -NF is
              specified, of course... */
      for (i = 0; i < argc; i++)
        if (!strcmp(argv[i], "-NF")) break;
      if (i == argc) /* -NF not in arguments */
        fork_Unix();
#endif /* FORKCOMM */

      argv[0] = filetorun;
      execvp(filetorun, argv);
      perror(filetorun);
      exit(1);
    } else { /* failed to connect X-server */
#define NAME_LEN 100
      char host_name[NAME_LEN];
      gethostname(host_name, NAME_LEN);
      if (Display_Name == NULL) {
        if ((Display_Name = getenv("DISPLAY")) != NULL) {
          if (strncmp(Display_Name, host_name, strlen(host_name)) == 0) {
            fprintf(stderr, "ldeboot: can't find X-Server\n");
            exit(-1);
          } /* end if */
        }
        /* end if */
      } else {
        fprintf(stderr, "ldeboot: can't find X-Server\n");
        exit(-1);
      } /* end if */
    }   /* end if */
  }
#endif /* XWINDOW */

#ifdef USESUNSCREEN
  if ((FrameBufferFd = open("/dev/fb", O_RDWR)) < 0) {
    fprintf(stderr, "ldeboot: can't open FrameBuffer\n");
    exit(-1);
  }
  if (ioctl(FrameBufferFd, FBIOGTYPE, &my_screen) < 0) {
    perror("initdisplay0:");
    exit(-1);
  }

  if (my_screen.fb_type == FBTYPE_SUN4COLOR) { /*  cg3 or cg6 */
    if (ioctl(FrameBufferFd, FBIOGATTR, &FBattr) >= 0) {
      if (FBattr.real_type == FBTYPE_SUN3COLOR ||   /* cg3 */
          FBattr.real_type == FBTYPE_SUNFAST_COLOR) /* cg6 */
      {
        strcpy(filetorun, LDECOLOR);
      }
    } else { /* if( ioctl... */
      perror("lde: This Display Model does not supported\n");
      exit(-1);
    }
  } else if (my_screen.fb_type == FBTYPE_SUN2BW) { /* bw2, cg4 or cg9  */
    strcpy(filetorun, LDEMONO);
  } else if (my_screen.fb_type == FBTYPE_SUN3COLOR) {
    if (ioctl(FrameBufferFd, FBIOGATTR, &FBattr) >= 0) {
      if (FBattr.real_type == FBTYPE_MEMCOLOR) /* cg8 */
      {
        strcpy(filetorun, LDETRUECOLOR);
      }
    } else { /* if( ioctl... */
      perror("lde: This Display Model does not supported\n");
      exit(-1);
    }

  } else if (my_screen.fb_type == FBTYPE_SUN2COLOR) { /* cg2  */
    strcpy(filetorun, LDEMONO);
  } else {
    perror("lde: This Display Model does not supported\n");
    exit(-1);
  }; /* endif( my_screen... */

  close(FrameBufferFd);

#endif /* USESUNSCREEN */

#ifdef FORKCOMM
  /* JRB - call fork_Unix here, while we're REALLY small, unless -NF is
          specified, of course... */
  for (i = 0; i < argc; i++)
    if (!strcmp(argv[i], "-NF")) break;
  if (i == argc) /* -NF not in arguments */
    fork_Unix();
#endif

  /* start ldemono or ldecolor */

  argv[0] = filetorun; /* or whatever... */

  /* then execve the LDE executable */
  execvp(filetorun, argv);
  perror(filetorun);

  exit(1);
}
