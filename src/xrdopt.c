/* $Id: xrdopt.c,v 1.6 2001/12/26 22:17:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef MAIKO_ENABLE_ETHERNET
#ifndef USE_DLPI
#include <net/nit.h> /* needed for Ethernet stuff below */
#endif               /* USE_DLPI */
#endif               /* MAIKO_ENABLE_ETHERNET */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "xdefs.h"
#include "dbprint.h"

#include "xrdoptdefs.h"

extern int LispWindowRequestedX, LispWindowRequestedY;
extern unsigned LispWindowRequestedWidth, LispWindowRequestedHeight;

extern int LispDisplayRequestedX, LispDisplayRequestedY;
extern unsigned LispDisplayRequestedWidth, LispDisplayRequestedHeight;

extern int xsync;

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

char Display_Name[128];
char iconpixmapfile[1024];
char Window_Title[255];
char Icon_Title[255];

extern char sysout_name[];
extern int sysout_size, for_makeinit, please_fork;
/* diagnostic flag for sysout dumping */
/* extern int maxpages; */

/*** Ethernet stuff (JRB) **/
#ifdef MAIKO_ENABLE_ETHERNET
extern int ether_fd;
extern u_char ether_host[6];
#ifndef USE_DLPI
extern struct sockaddr_nit snit;
#endif /* USE_DLPI */
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
  fprintf(stderr, " %s options:\n", prog);
  fprintf(stderr, " [-sysout] [<sysout>]                 -path to the Medley image\n");
  fprintf(stderr, " -h[elp]                              -prints this text\n");
  fprintf(stderr, " -info                                -prints configuration info\n");
  fprintf(stderr, " -d[isplay] <host>:<display>.<screen>\n");
  fprintf(stderr,
          " -g[eometry] <geom>                   -size & placement for the medley window on your X "
          "screen\n");
  fprintf(stderr,
          " -sc[reen] <geom>                     -size & placement for the medley display\n");
  fprintf(stderr, " -t[itle] <string>                    -titlebar text for the window manager\n");
  fprintf(stderr, " -icontitle <string> | -it <string>   -text for the medley icon\n");
  fprintf(stderr, " -iconbitmap <path> | -ibm <path>     -bitmap for the medley icon\n");
  fprintf(stderr,
          " -xsync                               -turn  XSynchronize on. (default is off)\n\n");
  fprintf(stderr, "Please refer to the manual for further information.\n\n");
  exit(0);
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

  sysout_name[0] = '\0';
  if (*argc == 2) /* There was probably a sysoutarg */
  {
    (void)strcpy(sysout_name, argv[1]);
  } else if ((envname = getenv("LDESRCESYSOUT")) != NULL) {
    strcpy(sysout_name, envname);
  } else if ((envname = getenv("LDESOURCESYSOUT")) != NULL)
    strcpy(sysout_name, envname);
  else {
    envname = getenv("HOME");
    (void)strcat(sysout_name, envname);
    (void)strcat(sysout_name, "/lisp.virtualmem");

    if (access(sysout_name, R_OK) != 0) { (void)strcat(sysout_name, ""); }
  }

  /* In order to access other DB's we have to open the main display now */
  /* This is just temporary. We'll need this display struct to open the */
  /* main databases for medley. After it is used the display will be */
  /* closed and the opening of the other displays will follow the standard */
  /* protocol. */

  if (XrmGetResource(commandlineDB, "ldex.display", "Ldex.Display", str_type, &value) == True) {
    (void)strncpy(Display_Name, value.addr, (int)value.size);
  } else if (getenv("DISPLAY") == (char *)NULL) {
    fprintf(stderr, "Can't find a display. Either set the shell\n");
    fprintf(stderr, "variable DISPLAY to an appropriate display\n");
    fprintf(stderr, "or provide a -display argument.\n");
    print_Xusage(argv[0]);
  } else {
    envname = getenv("DISPLAY");
    (void)strcpy(Display_Name, envname);
  }
  if ((xdisplay = XOpenDisplay(Display_Name)) != NULL) {
    /* read the other databases */
    /* Start with app-defaults/medley */
    (void)strcpy(tmp, "/usr/lib/X11/app-defaults/");
    (void)strcat(tmp, "medley");
    applicationDB = XrmGetFileDatabase(tmp);
    if (applicationDB != NULL) { (void)XrmMergeDatabases(applicationDB, &rDB); }
    /* Then try the displays defaults */
    if (XResourceManagerString(xdisplay) != NULL) {
      serverDB = XrmGetStringDatabase(XResourceManagerString(xdisplay));
      if (serverDB != NULL) { (void)XrmMergeDatabases(serverDB, &rDB); }
    }
    XCloseDisplay(xdisplay);
  } else {
    fprintf(stderr, "Open_Display: cannot connect to display %s.", XDisplayName(Display_Name));
    exit(-1);
  }

  envname = getenv("HOME");
  (void)strcat(tmp, envname);
  (void)strcat(tmp, "/.Xdefaults");
  if (access(tmp, R_OK) != 0) {
    serverDB = XrmGetFileDatabase(tmp);
    if (serverDB != NULL) { (void)XrmMergeDatabases(serverDB, &rDB); }
  }

  /* Now for the commandline */
  (void)XrmMergeDatabases(commandlineDB, &rDB);

  if (XrmGetResource(rDB, "ldex.sysout", "Ldex.Sysout", str_type, &value) == True) {
    /* Get Sysout */
    (void)strncpy(sysout_name, value.addr, (int)value.size);
  }
  if (sysout_name[0] == '\0') {
    fprintf(stderr, "Couldn't find a sysout to run;\n");
    print_Xusage(argv[0]);
  }

  if (XrmGetResource(rDB, "ldex.title", "Ldex.Title", str_type, &value) == True) {
    (void)strncpy(Window_Title, value.addr, (int)value.size);
  } else {
    (void)strcpy(Window_Title, WINDOW_NAME);
  }
  if (XrmGetResource(rDB, "ldex.icontitle", "Ldex.icontitle", str_type, &value) == True) {
    (void)strncpy(Icon_Title, value.addr, (int)value.size);
  } else {
    (void)strcpy(Icon_Title, "Medley");
  }

  if (XrmGetResource(rDB, "ldex.iconbitmap", "Ldex.Iconbitmap", str_type, &value) == True) {
    (void)strncpy(iconpixmapfile, value.addr, (int)value.size);
  }

  /* Old style geometry definition. */
  if (XrmGetResource(rDB, "ldex.geometry", "Ldex.geometry", str_type, &value) == True) {
    /* Get Geometry */
    (void)strncpy(tmp, value.addr, (int)value.size);
    bitmask = XParseGeometry(tmp, &LispWindowRequestedX, &LispWindowRequestedY,
                             &LispWindowRequestedWidth, &LispWindowRequestedHeight);
  }
  if (XrmGetResource(rDB, "ldex.screen", "Ldex.screen", str_type, &value) == True) {
    /* Get Geometry */
    (void)strncpy(tmp, value.addr, (int)value.size);
    bitmask = XParseGeometry(tmp, &LispDisplayRequestedX, &LispDisplayRequestedY,
                             &LispDisplayRequestedWidth, &LispDisplayRequestedHeight);
  }

  (void)strcpy(tmp, ""); /* Clear the string */

  if (XrmGetResource(rDB, "ldex.NoFork", "Ldex.NoFork", str_type, &value) == True) {
    please_fork = 0;
  }

  /*    if (XrmGetResource(rDB,
                         "ldex.maxpages",
                         "Ldex.maxpages",
                         str_type, &value) == True) {
        (void)strncpy(tmp, value.addr, (int)value.size);
        maxpages = atoi(tmp);
      }
  */
  if (XrmGetResource(rDB, "ldex.memory", "Ldex.memory", str_type, &value) == True) {
    (void)strncpy(tmp, value.addr, (int)value.size);
    sysout_size = atoi(tmp);
  }

  if (XrmGetResource(rDB, "ldex.Init", "Ldex.Init", str_type, &value) == True) { for_makeinit = 1; }

  if (XrmGetResource(rDB, "ldex.xsync", "Ldex.xsync", str_type, &value) == True) { xsync = True; }
#ifdef MAIKO_ENABLE_ETHERNET
  if (XrmGetResource(rDB, "ldex.EtherNet", "Ldex.EtherNet", str_type, &value) == True) {
    int b0, b1, b2, b3, b4, b5;
    (void)strncpy(tmp, value.addr, (int)value.size);
#ifdef USE_DLPI
    if (sscanf(tmp, "%d:%x:%x:%x:%x:%x:%x", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5) == 7)
#else
    if (sscanf(tmp, "%d:%x:%x:%x:%x:%x:%x:%s", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5,
               snit.snit_ifname) == 8)
#endif /* USE_DLPI */
    {
      ether_host[0] = b0;
      ether_host[1] = b1;
      ether_host[2] = b2;
      ether_host[3] = b3;
      ether_host[4] = b4;
      ether_host[5] = b5;
    } else {
      fprintf(stderr, "Missing or bogus -E argument\n");
      ether_fd = -1;
      exit(1);
    }
  }
#endif /* MAIKO_ENABLE_ETHERNET */
} /* end readXoption */
