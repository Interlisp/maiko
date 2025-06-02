/* $Id: xrdopt.c,v 1.6 2001/12/26 22:17:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"        // for MAIKO_ENABLE_ETHERNET

#include <X11/Xlib.h>       // for XPointer, True, XParseGeometry, XResource...
#include <X11/Xresource.h>  // for XrmoptionSepArg, XrmGetResource, Xrmoptio...
#include <errno.h>          // for errno
#include <limits.h>         // for PATH_MAX
#include <stdio.h>          // for fprintf, NULL, stderr, sscanf
#include <stdlib.h>         // for getenv, exit, strtol
#include <string.h>         // for strncpy, strlcat, strlcpy, strcmp
#include <sys/types.h>      // for u_char
#include <unistd.h>         // for access, R_OK
#include "xdefs.h"          // for WINDOW_NAME
#include "xrdoptdefs.h"     // for print_Xusage, read_Xoption

#ifdef MAIKO_ENABLE_ETHERNET
#if defined(USE_NIT)
#include <net/nit.h> /* needed for Ethernet stuff below */
#endif               /* USE_NIT */
#endif               /* MAIKO_ENABLE_ETHERNET */

extern int LispWindowRequestedX, LispWindowRequestedY;
extern unsigned LispWindowRequestedWidth, LispWindowRequestedHeight;

extern int LispDisplayRequestedX, LispDisplayRequestedY;
extern unsigned LispDisplayRequestedWidth, LispDisplayRequestedHeight;

extern int xsync;
extern int TIMER_INTERVAL;

static XrmOptionDescRec opTable[] = {
    {"-help", "*help", XrmoptionIsArg, (XPointer)NULL},
    {"-h", "*help", XrmoptionIsArg, (XPointer)NULL},
    {"-sysout", "*sysout", XrmoptionSepArg, (XPointer)NULL},
    {"-display", "*display", XrmoptionSepArg, (XPointer)NULL},
    {"-d", "*display", XrmoptionSepArg, (XPointer)NULL},
    {"-borderWidth", "*borderWidth", XrmoptionSepArg, (XPointer)NULL},
    {"-bw", "*borderWidth", XrmoptionSepArg, (XPointer)NULL},
    {"-screen", "*screen", XrmoptionSepArg, (XPointer)NULL},
    {"-sc", "*screen", XrmoptionSepArg, (XPointer)NULL},
    {"-geometry", "*geometry", XrmoptionSepArg, (XPointer)NULL},
    {"-g", "*geometry", XrmoptionSepArg, (XPointer)NULL},
    {"-foreground", "*foreground", XrmoptionSepArg, (XPointer)NULL},
    {"-fg", "*foreground", XrmoptionSepArg, (XPointer)NULL},
    {"-background", "*background", XrmoptionSepArg, (XPointer)NULL},
    {"-bg", "*background", XrmoptionSepArg, (XPointer)NULL},
    {"-cursorColor", "*cursorColor", XrmoptionSepArg, (XPointer)NULL},
    {"-title", "*title", XrmoptionSepArg, (XPointer)NULL},
    {"-t", "*title", XrmoptionSepArg, (XPointer)NULL},
    {"-icontitle", "*icontitle", XrmoptionSepArg, (XPointer)NULL},
    {"-it", "*icontitle", XrmoptionSepArg, (XPointer)NULL},
    {"-iconbitmap", "*iconbitmap", XrmoptionSepArg, (XPointer)NULL},
    {"-ibm", "*iconbitmap", XrmoptionSepArg, (XPointer)NULL},
    {"-timer", "*timer", XrmoptionSepArg, (XPointer)NULL},
    {"-xpages", "*maxpages", XrmoptionSepArg, (XPointer)NULL},
    {"-m", "*memory", XrmoptionSepArg, (XPointer)NULL},
    {"-NF", "*NoFork", XrmoptionIsArg, (XPointer)NULL},
    {"-NoFork", "*NoFork", XrmoptionIsArg, (XPointer)NULL},
    {"-noscroll", "*noscroll", XrmoptionIsArg, (XPointer) NULL},
    {"-INIT", "*Init", XrmoptionIsArg, (XPointer)NULL},
    {"-EtherNet", "*EtherNet", XrmoptionSepArg, (XPointer)NULL},
    {"-E", "*EtherNet", XrmoptionSepArg, (XPointer)NULL},
    {"-autorepeat", "*autorepeat", XrmoptionSepArg, (XPointer)NULL},
    {"-xsync", "*xsync", XrmoptionIsArg, (XPointer)NULL},
};
/* autorepeat is a global setting for X, not anything that
 Medley has to be concerned with. Item kept for historical
 reasons /jarl
 */

extern char Display_Name[128];
char Display_Name[128];
extern char iconpixmapfile[1024];
char iconpixmapfile[1024];
extern char iconTitle[255];
char iconTitle[255];
extern char cursorColor[255];
char cursorColor[255] = {0};
extern char foregroundColorName[64];
extern char backgroundColorName[64];
extern char windowTitle[255];


extern char sysout_name_cl[];
extern char sysout_name_xrm[];
extern unsigned sysout_size;
extern int for_makeinit, please_fork, noscroll;
/* diagnostic flag for sysout dumping */
/* extern unsigned maxpages; */

/*** Ethernet stuff (JRB) **/
#ifdef MAIKO_ENABLE_ETHERNET
extern int ether_fd;
extern u_char ether_host[6];
#if defined(USE_NIT)
extern struct sockaddr_nit snit;
#endif /* USE_NIT */
#endif /* MAIKO_ENABLE_ETHERNET */

/************************************************************************/
/*									*/
/*			p r i n t _ X u s a g e				*/
/*									*/
/*	Print out command-line options for X, to help user.		*/
/*									*/
/************************************************************************/

void print_Xusage(const char *prog)
{
  (void)fprintf(stderr, " %s options:\n", prog);
  (void)fprintf(stderr, " [-sysout] [<sysout>]                 -path to the Medley image\n");
  (void)fprintf(stderr, " -h[elp]                              -prints this text\n");
  (void)fprintf(stderr, " -info                                -prints configuration info\n");
  (void)fprintf(stderr, " -cursorColor X11-color-spec          -sets foreground cursor color\n");
  (void)fprintf(stderr, " -fg|-foreground X11-color-spec       -sets foreground display color\n");
  (void)fprintf(stderr, " -bg|-background X11-color-spec       -sets background display color\n");
  (void)fprintf(stderr, " -d[isplay] <host>:<display>.<screen>\n");
  (void)fprintf(stderr,
          " -g[eometry] <geom>                   -size & placement for the medley window on your X "
          "screen\n");
  (void)fprintf(stderr,
          " -sc[reen] <geom>                     -size & placement for the medley display\n");
  (void)fprintf(stderr, " -t[itle] <string>                    -titlebar text for the window manager\n");
  (void)fprintf(stderr, " -icontitle <string> | -it <string>   -text for the medley icon\n");
  (void)fprintf(stderr, " -iconbitmap <path> | -ibm <path>     -bitmap for the medley icon\n");
  (void)fprintf(stderr,
          " -xsync                               -turn  XSynchronize on. (default is off)\n\n");
#if defined(MAIKO_ENABLE_NETHUB)
  (void)fprintf(stderr,"\
 -nh-host dodo-host        Hostname for Dodo Nethub (no networking if missing)\n\
 -nh-port port-number      Port for Dodo Nethub (optional, default: 3333)\n\
 -nh-mac XX-XX-XX-XX-XX-XX Machine-ID for Maiko-VM (optional, default: CA-FF-EE-12-34-56) \n\
 -nh-loglevel level        Loglevel for Dodo networking (0..2, optional, default: 0)\n\n");
#endif
  (void)fprintf(stderr, "Please refer to the manual for further information.\n\n");
  exit(EXIT_FAILURE);
} /* end print_Xusage() */

/************************************************************************/
/*									*/
/*			r e a d _ X o p t i o n				*/
/*									*/
/*	Parse command-line options related to X windows.		*/
/*									*/
/************************************************************************/

void read_Xoption(int *argc, char *argv[])
{
  int bitmask;
  XrmDatabase commandlineDB = NULL, applicationDB, serverDB, rDB = NULL;
  XrmValue value;
  char *str_type[30], tmp[1024], *envname;
  Display *xdisplay;
  int i;

  /**********************************************/
  /*                                            */
  /*  Take care of -help flag, which            */
  /*  isn't handled correctly by the X code.    */
  /*                                            */
  /**********************************************/

  for (i = 1; i < *argc; i++) {
    if (argv[i] && ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-HELP") == 0))) {
      print_Xusage(argv[0]);
      exit(0);
    }
  }

  /* Now let X handle the parsing. */

  (void)XrmInitialize();
/* If the first argv lacks '-' in front it is the sysout. */
#ifdef NEVEFR
  /* JDS 12/20/01: app name should always be "ldex", not what's in argv?? */
  XrmParseCommand(&commandlineDB, opTable, opTableEntries, argv[0], argc, argv);
#endif
  XrmParseCommand(&commandlineDB, opTable, sizeof(opTable) / sizeof(opTable[0]), "ldex", argc, argv);

  if (XrmGetResource(commandlineDB, "ldex.help", "Ldex.Help", str_type, &value) == True) {
    print_Xusage(argv[0]);
  }

  if (XrmGetResource(commandlineDB, "ldex.sysout", "Ldex.Sysout", str_type, &value) == True) {
    /* Get Sysout from command line only */
    (void)strncpy(sysout_name_cl, value.addr, value.size);
  }

  /* In order to access other DB's we have to open the main display now */
  /* This is just temporary. We'll need this display struct to open the */
  /* main databases for medley. After it is used the display will be */
  /* closed and the opening of the other displays will follow the standard */
  /* protocol. */

  if (XrmGetResource(commandlineDB, "ldex.display", "Ldex.Display", str_type, &value) == True) {
    (void)strncpy(Display_Name, value.addr, value.size);
  } else if (getenv("DISPLAY") == (char *)NULL) {
    (void)fprintf(stderr, "Can't find a display. Either set the shell\n");
    (void)fprintf(stderr, "variable DISPLAY to an appropriate display\n");
    (void)fprintf(stderr, "or provide a -display argument.\n");
    print_Xusage(argv[0]);
  } else {
    envname = getenv("DISPLAY");
    (void)strlcpy(Display_Name, envname, sizeof(Display_Name));
  }
  if ((xdisplay = XOpenDisplay(Display_Name)) != NULL) {
    /* read the other databases */
    /* Start with app-defaults/medley */
    (void)strlcpy(tmp, "/usr/lib/X11/app-defaults/", sizeof(tmp));
    (void)strlcat(tmp, "medley", sizeof(tmp));
    applicationDB = XrmGetFileDatabase(tmp);
    if (applicationDB != NULL) { (void)XrmMergeDatabases(applicationDB, &rDB); }
    /* Then try the displays defaults */
    if (XResourceManagerString(xdisplay) != NULL) {
      serverDB = XrmGetStringDatabase(XResourceManagerString(xdisplay));
      if (serverDB != NULL) { (void)XrmMergeDatabases(serverDB, &rDB); }
    }
    XCloseDisplay(xdisplay);
  } else {
    (void)fprintf(stderr, "Open_Display: cannot connect to display %s.", XDisplayName(Display_Name));
    exit(-1);
  }

  envname = getenv("HOME");
  (void)strlcpy(tmp, envname, sizeof(tmp));
  (void)strlcat(tmp, "/.Xdefaults", sizeof(tmp));
  if (access(tmp, R_OK) != 0) {
    serverDB = XrmGetFileDatabase(tmp);
    if (serverDB != NULL) { (void)XrmMergeDatabases(serverDB, &rDB); }
  }

  /* Now for the commandline */
  (void)XrmMergeDatabases(commandlineDB, &rDB);

  if (XrmGetResource(rDB, "ldex.sysout", "Ldex.Sysout", str_type, &value) == True) {
    /* Get Sysout from x resource manager */
    (void)strncpy(sysout_name_xrm, value.addr, value.size);
  }

  if (XrmGetResource(rDB, "ldex.title", "Ldex.Title", str_type, &value) == True) {
    (void)strncpy(windowTitle, value.addr, sizeof(windowTitle) - 1);
  } else {
    (void)strncpy(windowTitle, WINDOW_NAME, sizeof(windowTitle) - 1);
  }
  if (XrmGetResource(rDB, "ldex.icontitle", "Ldex.icontitle", str_type, &value) == True) {
    (void)strncpy(iconTitle, value.addr, value.size);
  } else {
    (void)strlcpy(iconTitle, "Medley", sizeof(iconTitle));
  }

  if (XrmGetResource(rDB, "ldex.iconbitmap", "Ldex.Iconbitmap", str_type, &value) == True) {
    (void)strncpy(iconpixmapfile, value.addr, value.size);
  }

  /* Old style geometry definition. */
  if (XrmGetResource(rDB, "ldex.geometry", "Ldex.geometry", str_type, &value) == True) {
    /* Get Geometry */
    (void)strncpy(tmp, value.addr, value.size);
    bitmask = XParseGeometry(tmp, &LispWindowRequestedX, &LispWindowRequestedY,
                             &LispWindowRequestedWidth, &LispWindowRequestedHeight);
  }
  if (XrmGetResource(rDB, "ldex.screen", "Ldex.screen", str_type, &value) == True) {
    /* Get Geometry */
    (void)strncpy(tmp, value.addr, value.size);
    bitmask = XParseGeometry(tmp, &LispDisplayRequestedX, &LispDisplayRequestedY,
                             &LispDisplayRequestedWidth, &LispDisplayRequestedHeight);
  }

  if (XrmGetResource(rDB, "ldex.cursorColor", "Ldex.cursorColor", str_type, &value) == True) {
    (void)strncpy(cursorColor, value.addr, sizeof(cursorColor) - 1);
  }

  if (XrmGetResource(rDB, "ldex.foreground", "Ldex.foreground", str_type, &value) == True) {
    (void)strncpy(foregroundColorName, value.addr, sizeof(foregroundColorName) - 1);
  }

  if (XrmGetResource(rDB, "ldex.background", "Ldex.background", str_type, &value) == True) {
    (void)strncpy(backgroundColorName, value.addr, sizeof(backgroundColorName) - 1);
  }

  if (XrmGetResource(rDB, "ldex.NoFork", "Ldex.NoFork", str_type, &value) == True) {
    please_fork = 0;
  }

  if (XrmGetResource(rDB, "ldex.noscroll", "Ldex.noscroll", str_type, &value) == True) {
    noscroll = 1;
  }

  if (XrmGetResource(rDB, "ldex.timer", "Ldex.timer", str_type, &value) == True) {
    (void)strncpy(tmp, value.addr, value.size);
    errno = 0;
    i = (int)strtol(tmp, (char **)NULL, 10);
    if (errno == 0 && i > 0)
      TIMER_INTERVAL = i;
  }

  /*    if (XrmGetResource(rDB,
                         "ldex.maxpages",
                         "Ldex.maxpages",
                         str_type, &value) == True) {
        (void)strncpy(tmp, value.addr, value.size);
        maxpages = (unsigned)strtol((tmp, (char **)NULL, 10);
	// should check no error here 
      }
  */
  if (XrmGetResource(rDB, "ldex.memory", "Ldex.memory", str_type, &value) == True) {
    (void)strncpy(tmp, value.addr, value.size);
    errno = 0;
    i = (int)strtol(tmp, (char **)NULL, 10);
    if (errno == 0 && i > 0)
      sysout_size = (unsigned)i;
  }

  if (XrmGetResource(rDB, "ldex.Init", "Ldex.Init", str_type, &value) == True) { for_makeinit = 1; }

  if (XrmGetResource(rDB, "ldex.xsync", "Ldex.xsync", str_type, &value) == True) { xsync = True; }
#ifdef MAIKO_ENABLE_ETHERNET
  if (XrmGetResource(rDB, "ldex.EtherNet", "Ldex.EtherNet", str_type, &value) == True) {
    int b0, b1, b2, b3, b4, b5;
    (void)strncpy(tmp, value.addr, value.size);
#if defined(USE_DLPI)
    if (sscanf(tmp, "%d:%x:%x:%x:%x:%x:%x", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5) == 7)
#elif defined(USE_NIT)
    if (sscanf(tmp, "%d:%x:%x:%x:%x:%x:%x:%s", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5,
               snit.snit_ifname) == 8)
#endif /* USE_NIT */
    {
      ether_host[0] = b0;
      ether_host[1] = b1;
      ether_host[2] = b2;
      ether_host[3] = b3;
      ether_host[4] = b4;
      ether_host[5] = b5;
    } else {
      (void)fprintf(stderr, "Missing or bogus -E argument\n");
      ether_fd = -1;
      exit(1);
    }
  }
#endif /* MAIKO_ENABLE_ETHERNET */
} /* end readXoption */
