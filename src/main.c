/* $Id: main.c,v 1.4 2001/12/26 22:17:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	main.c
 *	This file includes main()
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>

#ifndef DOS
#include <sys/param.h>
#include <unistd.h>
#else /* DOS */
#include <i32.h>
#define MAXPATHLEN 128
#define R_OK 04
#endif /* DOS */

#ifdef MAIKO_ENABLE_ETHERNET
#if defined(USE_NIT)
#include <net/nit.h> /* needed for Ethernet stuff below */
#endif               /* USE_DLPI */
#endif               /* MAIKO_ENABLE_ETHERNET */

#include "emlglob.h"
#include "adr68k.h"
#include "stack.h"
#include "return.h"

#include "lispemul.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "ifpage.h"
#include "iopage.h"

#include "maindefs.h"
#include "commondefs.h"
#include "dirdefs.h"
#include "dspifdefs.h"
#include "etherdefs.h"
#include "initdspdefs.h"
#include "initkbddefs.h"
#include "initsoutdefs.h"
#include "ldsoutdefs.h"
#include "miscstat.h"
#include "storagedefs.h"
#include "timerdefs.h"
#include "unixcommdefs.h"
#include "xcdefs.h"
#include "xrdoptdefs.h"

DLword *Lisp_world; /* lispworld */

/********** 68k address for Lisp Space **********/
DLword *Stackspace;
DLword *Plistspace;
DLword *DTDspace;
DLword *MDStypetbl;
DLword *AtomHT;
DLword *Pnamespace;
DLword *AtomSpace;
DLword *Defspace;
DLword *Valspace;

/********** For Virtual Memory Management **********/
#ifdef BIGVM
LispPTR *FPtoVP;
#else
DLword *FPtoVP;
#endif /* BIGVM */
DLword *PAGEMap;
DLword *PageMapTBL;
DLword *LockedPageTable;

/********** For Interface to LispMicro/Device **********/
DLword *IOCBPage;
IOPAGE *IOPage;
IFPAGE *InterfacePage;
MISCSTATS *MiscStats;

/********** UFN Table **********/
DLword *UFNTable;

/********** Tables for GC **********/
#ifdef BIGVM
LispPTR *HTmain;
LispPTR *HToverflow;
LispPTR *HTbigcount;
LispPTR *HTcoll;
#else
DLword *HTmain;
DLword *HToverflow;
DLword *HTbigcount;
DLword *HTcoll;
#endif /* BIGVM */

/********** Display **********/
DLword *DisplayRegion;
int DisplayInitialized = NIL;

DLword *MDS_space_bottom;
DLword *PnCharspace;
struct dtd *ListpDTD;

/********** For Lisp Emulator **********/
struct state MachineState;

/**********************************/
/*** Share val with LISP code ******/

DLword *MDS_free_page;
DLword *Next_MDSpage;
DLword *Next_Array;
/*******************************************/

/** CACHE LISP SYSVAL ***/
LispPTR *Next_MDSpage_word;
LispPTR *Next_Array_word;
LispPTR *MDS_free_page_word;

LispPTR *Reclaim_cnt_word;

/*** Cache Values for reclaimer by Tomtom 30-Sep-1987 ***/
LispPTR *GcDisabled_word;
LispPTR *CdrCoding_word;
LispPTR *FreeBlockBuckets_word;
LispPTR *Array_Block_Checking_word;
LispPTR *ArrayMerging_word;
LispPTR *ArraySpace_word;
LispPTR *ArraySpace2_word;
LispPTR *ArrayFrLst_word;
LispPTR *ArrayFrLst2_word;
LispPTR *Hunk_word;
LispPTR *System_Buffer_List_word;

/*** The end of the addition of cache values on reclaimer ***/

/*** cache values for the top level reclaimer's implementation ***/

LispPTR *GcMess_word;
LispPTR *ReclaimMin_word;
LispPTR *GcTime1_word;
LispPTR *GcTime2_word;
LispPTR *MaxTypeNumber_word;

/*** The end of the addition of cache values for top reclaimer by Tomtom
                                                15-Oct-1987             ***/

/*  Pointers for closure caching */

LispPTR *Package_from_Index_word;
LispPTR *Package_from_Name_word;
LispPTR *Keyword_Package_word;
LispPTR *Closure_Cache_Enabled_word;
LispPTR *Closure_Cache_word;
LispPTR *Deleted_Implicit_Hash_Slot_word;
LispPTR First_index;

/*** The end of Pointers for closure caching ***/

/* CACHE values for 32Mb MDS/Array by Take */
LispPTR *STORAGEFULLSTATE_word;
LispPTR *STORAGEFULL_word;
LispPTR *PENDINGINTERRUPT_word;
LispPTR *LeastMDSPage_word;
LispPTR *SecondMDSPage_word;
LispPTR *SecondArrayPage_word;
LispPTR *INTERRUPTSTATE_word;
LispPTR *SYSTEMCACHEVARS_word;
LispPTR *MACHINETYPE_word;

LispPTR STORAGEFULLSTATE_index;
LispPTR *LASTVMEMFILEPAGE_word;
LispPTR *VMEM_FULL_STATE_word;

/** Array for N-tran **/

int native_load_address;
LispPTR native_closure_env = NOBIND_PTR;

/** Pipes for Unix Interface **/
int UnixPipeIn;
int UnixPipeOut;
int UnixPID;
int please_fork = 1;
/* disable X11 scroll bars if requested */
int noscroll = 0;
/*** STACK handle staff(Takeshi) **/
LispPTR *STACKOVERFLOW_word;
LispPTR *GuardStackAddr_word;
LispPTR *LastStackAddr_word;
LispPTR *NeedHardreturnCleanup_word;

/*** Ethernet stuff (JRB) **/
#ifdef MAIKO_ENABLE_ETHERNET
extern int ether_fd;
extern u_char ether_host[6];
#endif /* MAIKO_ENABLE_ETHERNET */

extern struct sockaddr_nit snit;

#ifdef INIT
int for_makeinit = 1;
#else
int for_makeinit = 0;
#endif /* INIT */

int kbd_for_makeinit = 0;
int save_argc;
char **save_argv;
int display_max = 65536 * 16 * 2;

/* diagnostic flag for sysout dumping */
extern unsigned maxpages;

char sysout_name[MAXPATHLEN]; /* Set by read_Xoption, in the X version. */
unsigned sysout_size = 0;    /* ditto */

int flushing = FALSE; /* see dbprint.h if set, all debug/trace printing will call fflush(stdout) after each printf */

#if defined(DOS) || defined(XWINDOW)
#include "devif.h"
extern DspInterface currentdsp;
#endif /* DOS || XWINDOW */

extern const time_t MDate;
extern int nokbdflag;
extern int nomouseflag;
#ifdef DOS
int dosdisplaymode = 0;
int twobuttonflag = FALSE;
int eurokbd = TRUE; /* Assume eurokbd by default. */
const char *helpstring =
    "\n\
medley [sysout-name] [<options>] ...\n\
Where <options> are:\n\
 sysout-name  The filename of your sysout.(see manual.)\n\
 -m <size>    Virtual memory size in  Mega Bytes(from 8 to 32)\n\
 -vga         Use standard VGA 640x480 screen resolution\n\
 -vesa102     Use VESA 800x600 screen resolution\n\
 -vesa104     Use VESA 1024x768 screen resolution\n\
 -2button     Force two button mouse handling\n\
 -3button     Force three button mouse handling\n\
 -noeurokbd   Force old style kbd handling (for 2.0 and earlier sysouts)\n\
 -eurokbd     Force new style kbd handling (for 2.01 and later sysouts)\n\
 -nokbd       Turn the kbd handling off (for debugging only)\n\
 -nomouse     Turn the mouse handling off (for debugging only)\n\
 -info        Print general info about the system\n\
 -help        Print this message\n";
#elif XWINDOW
const char *helpstring =
    "\n\
 either setenv LDESRCESYSOUT or do:\n\
 medley [<sysout-name>] [<options>]\n\
 -info                    Print general info about the system\n\
 -help                    Print this message\n\
 -d[isplay] host:srv.scr  The host, X server and screen you want Medley on\n\
 -bw <pixels>             The Medley screen borderwidth\n\
 -g[eometry] <geom>]      The Medley screen geometry\n\
 -sc[reen] <w>x<h>]       The Medley screen geometry\n";
#else  /* not DOS, not XWINDOW */
const char *helpstring =
    "\n\
 either setenv LDESRCESYSOUT or do:\n\
 lde[ether] [sysout-name] [<options>]\n\
 -info        Print general info about the system\n\
 -help        Print this message\n";
#endif /* DOS */

#if defined(MAIKO_ENABLE_NETHUB)
const char *nethubHelpstring =
 "\
 -nh-host dodo-host        Hostname for Dodo Nethub (no networking if missing)\n\
 -nh-port port-number      Port for Dodo Nethub (optional, default: 3333)\n\
 -nh-mac XX-XX-XX-XX-XX-XX Machine-ID for Maiko-VM (optional, default: CA-FF-EE-12-34-56) \n\
 -nh-loglevel level        Loglevel for Dodo networking (0..2, optional, default: 0)\n\
 ";
#else
const char *nethubHelpstring = "";
#endif

#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
extern int insnsCountdownForTimerAsyncEmulation;
#endif

/************************************************************************/
/*									*/
/*		     M A I N   E N T R Y   P O I N T			*/
/*									*/
/*									*/
/************************************************************************/

int main(int argc, char *argv[])
{
  int i;
  char *envname;
  extern int TIMER_INTERVAL;
  extern fd_set LispReadFds;
  long tmpint;
#ifdef MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE
  if (dld_find_executable(argv[0]) == 0) {
    perror("Name of executable not found.");
  } else if (dld_init(dld_find_executable(argv[0])) != 0) {
    dld_perror("Can't init DLD.");
  }
#endif /* MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE */

#ifdef XWINDOW
  read_Xoption(&argc, argv);
#endif /* XWINDOW */

  save_argc = argc;
  save_argv = argv;

#ifdef PROFILE
  moncontrol(0); /* initially stop sampling */
#endif           /* PROFILE */

/* Sysout is found as follows:
        If the first argument doesn't begin with '-', assume it's the sysout
        Look at the environment variable LDESRCESYSOUT if that fails
        Look for ~/lisp.virtualmem if that fails
        Barf and print the command line if tha fails
*/

  i = 1;

  if (argv[i] && ((strcmp(argv[i], "-info") == 0) || (strcmp(argv[i], "-INFO") == 0))) {
    print_info_lines();
    exit(0);
  }

  if (argv[i] && ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-HELP") == 0))) {
    fprintf(stderr, "%s%s", helpstring, nethubHelpstring);
    exit(0);
  }

  if (argc > 1 && argv[1][0] != '-') {
    strncpy(sysout_name, argv[1], MAXPATHLEN);
    i++;
  } else if ((envname = getenv("LDESRCESYSOUT")) != NULL) {
    strncpy(sysout_name, envname, MAXPATHLEN);
  } else if ((envname = getenv("LDESOURCESYSOUT")) != NULL)
    strncpy(sysout_name, envname, MAXPATHLEN);
  else {
#ifdef DOS
    strncpy(sysout_name, "lisp.vm", MAXPATHLEN);
#else
    if ((envname = getenv("HOME")) != NULL) {
      strncpy(sysout_name, envname, MAXPATHLEN);
      strncat(sysout_name, "/lisp.virtualmem", MAXPATHLEN - 17);
    }
#endif /* DOS */
  }
  if (access(sysout_name, R_OK)) {
    perror("Couldn't find a sysout to run");
    fprintf(stderr, "%s%s", helpstring, nethubHelpstring);
    exit(1);
  }
  /* OK, sysout name is now in sysout_name, and i is moved past a supplied name */

  for (; i < argc; i += 1) { /* step by 1 in case of typo */

    /* -t and -m are undocumented and somewhat dangerous... */

    if (!strcmp(argv[i], "-t")) { /**** timer interval	****/
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          TIMER_INTERVAL = tmpint;
        } else {
          fprintf(stderr, "Bad value for -t (integer > 0)\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -t\n");
        exit(1);
      }
    }

    else if (!strcmp(argv[i], "-m")) { /**** sysout size	****/
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          sysout_size = (unsigned)tmpint;
        } else {
          fprintf(stderr, "Bad value for -m (integer > 0)\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -m\n");
        exit(1);
      }
    }

    else if (!strcmp(argv[i], "-NF")) { /****  Don't fork (for dbxing)	****/
      please_fork = 0;
    }

    else if (!strcmp(argv[i], "-INIT")) { /*** init sysout, no packaged */
      for_makeinit = 1;
    }
#ifdef DOS
    else if ((strcmp(argv[i], "-vga") == 0) || (strcmp(argv[i], "-VGA") == 0)) {
      dosdisplaymode = 1;
    } else if ((strcmp(argv[i], "-vesa102") == 0) || (strcmp(argv[i], "-VESA102") == 0)) {
      dosdisplaymode = 0x102;
    } else if ((strcmp(argv[i], "-vesa104") == 0) || (strcmp(argv[i], "-VESA104") == 0)) {
      dosdisplaymode = 0x104;
    } else if ((strcmp(argv[i], "-2button") == 0) || (strcmp(argv[i], "-2BUTTON") == 0)) {
      twobuttonflag = TRUE;
    } else if ((strcmp(argv[i], "-3button") == 0) || (strcmp(argv[i], "-3BUTTON") == 0)) {
      twobuttonflag = FALSE;
    } else if ((strcmp(argv[i], "-noeurokbd") == 0) || (strcmp(argv[i], "-NOEUROKBD") == 0)) {
      eurokbd = FALSE;
    } else if ((strcmp(argv[i], "-eurokbd") == 0) || (strcmp(argv[i], "-EUROKBD") == 0)) {
      eurokbd = TRUE;
    } else if ((strcmp(argv[i], "-nokbd") == 0) || (strcmp(argv[i], "-NOKBD") == 0)) {
      nokbdflag = TRUE;
    } else if ((strcmp(argv[i], "-nomouse") == 0) || (strcmp(argv[i], "-NOMOUSE") == 0)) {
      nomouseflag = TRUE;
    }

#endif /* DOS */

    /* Can only do this under SUNOs, for now */
    else if (!strcmp(argv[i], "-E")) { /**** ethernet info	****/
#ifdef MAIKO_ENABLE_ETHERNET
      int b0, b1, b2, b3, b4, b5;
#if defined(USE_DLPI)
      if (argc > ++i &&
          sscanf(argv[i], "%d:%x:%x:%x:%x:%x:%x", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5) == 7)
#else
      if (argc > ++i &&
          sscanf(argv[i], "%d:%x:%x:%x:%x:%x:%x:%s", &ether_fd, &b0, &b1, &b2, &b3, &b4, &b5,
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
#endif /* MAIKO_ENABLE_ETHERNET */

    }

#ifdef MAIKO_ENABLE_NETHUB
    else if (!strcmp(argv[i], "-nh-host")) {
      if (argc > ++i) {
        setNethubHost(argv[i]);
      } else {
        fprintf(stderr, "Missing argument after -nh-host\n");
        exit(1);
      }
    }
    else if (!strcmp(argv[i], "-nh-port")) {
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          setNethubPort(tmpint);
        } else {
          fprintf(stderr, "Bad value for -nh-port\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -nh-port\n");
        exit(1);
      }
    }
    else if (!strcmp(argv[i], "-nh-mac")) {
      if (argc > ++i) {
        int b0, b1, b2, b3, b4, b5;
        if (sscanf(argv[i], "%x-%x-%x-%x-%x-%x",  &b0, &b1, &b2, &b3, &b4, &b5) == 6) {
          setNethubMac(b0, b1, b2, b3, b4, b5);
        } else {
          fprintf(stderr, "Invalid argument for -nh-mac\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -nh-mac\n");
        exit(1);
      }
    }
    else if (!strcmp(argv[i], "-nh-loglevel")) {
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint >= 0) {
          setNethubLogLevel(tmpint);
        } else {
          fprintf(stderr, "Bad value for -nh-loglevel\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -nh-loglevel\n");
        exit(1);
      }
    }
#endif /* MAIKO_ENABLE_NETHUB */

#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
    else if (!strcmp(argv[i], "-intr-emu-insns")) {
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 1000) {
          insnsCountdownForTimerAsyncEmulation = tmpint;
        } else {
          fprintf(stderr, "Bad value for -intr-emu-insns (integer > 1000)\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -intr-emu-insns\n");
        exit(1);
      }
    }
#endif

    /* diagnostic flag for big vmem write() calls */
    else if (!strcmp(argv[i], "-xpages")) {
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          maxpages = (unsigned)tmpint;
        } else {
          fprintf(stderr, "Bad value for -xpages (integer > 0)\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Missing argument after -xpages\n");
        exit(1);
      }
    }
  }

/* Sanity checks. */
#ifdef DOS
  probemouse(); /* See if the mouse is connected. */
#else
  if (getuid() != geteuid()) {
    fprintf(stderr, "Effective user is not real user.  Resetting uid\n");
    if (setuid(getuid()) == -1) {
      fprintf(stderr, "Unable to reset user id to real user id\n");
      exit(1);
    }
  }
#endif /* DOS */

  FD_ZERO(&LispReadFds);

#ifdef MAIKO_ENABLE_ETHERNET
  init_ether(); /* modified by kiuchi Nov. 4 */
#endif          /* MAIKO_ENABLE_ETHERNET */

#ifdef MAIKO_ENABLE_NETHUB
  connectToHub();
#endif

#ifdef DOS
  init_host_filesystem();
#else
  /* Fork Unix was called in kickstarter; if we forked, look up the */
  /* pipe handles to the subprocess and set them up.		      */

  if (FindUnixPipes()) /* must call the routine to allocate storage, */
  {                    /* in case we're re-starting a savevm w/open ptys */
    if (please_fork) fprintf(stderr, "Failed to find UNIXCOMM file handles; no processes\n");
  }
#endif /* DOS */

#if defined(DOS) || defined(XWINDOW)
  make_dsp_instance(currentdsp, 0, 0, 0, 1); /* All defaults the first time */
#endif                                       /* DOS || XWINDOW */

  /* Load sysout to VM space and returns real sysout_size(not 0) */
  sysout_size = sysout_loader(sysout_name, sysout_size);

  build_lisp_map(); /* built up map */

  init_ifpage(sysout_size); /* init interface page */
  init_iopage();
  init_miscstats();
  init_storage();

  set_cursor();

  /* file system directory enumeration stuff */
  if (!init_finfo()) {
    fprintf(stderr, "Cannot allocate internal data.\n");
    exit(1);
  }
#ifdef RS232
  rs232c_init();
#endif

  /* Get OS message to ~/lisp.log and print the message to prompt window */
  if (!for_makeinit) {

    init_keyboard(0); /* can't turn on the keyboard yet or you will die
                         in makeinit.  Pilotbitblt will turn it on if
                         you used the proper switches when building LDE.
                            JDS -- 1/18/90 also BITBLTSUB does it now. */
  }

#ifdef DOS
  _setrealmode(0x3f); /* Don't interrupt on FP overflows */
  _getrealerror();

  tzset();
#endif

#ifdef OS5
  tzset();
#endif /* OS5 */

  /* now start up lisp */
  start_lisp();
  return (0);
}

/************************************************************************/
/*									*/
/*		  	   s t a r t _ l i s p				*/
/*									*/
/*	This is the function that actually starts up the lisp emulator.	*/
/*									*/
/*									*/
/************************************************************************/

void start_lisp(void) {
  DLword *freeptr, *next68k;

/*******************************/
/*  First, turn off any pending interrupts from during VMEMSAVE.	*/
/*  This keeps US from trying to handle OLD interrupts.		*/
/*******************************/
#ifndef INIT
  {
    INTSTAT *intstate = ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
    intstate->ETHERInterrupt = 0;
    intstate->LogFileIO = 0;
    intstate->IOInterrupt = 0;
    intstate->waitinginterrupt = 0;
    intstate->intcharcode = 0;
  }
#endif /* INIT */

  TopOfStack = 0;
  Error_Exit = 0;

  PVar = (DLword *)NativeAligned2FromLAddr(STK_OFFSET | InterfacePage->currentfxp) + FRAMESIZE;

  freeptr = next68k = NativeAligned2FromLAddr(STK_OFFSET | CURRENTFX->nextblock);

  if (GETWORD(next68k) != STK_FSB_WORD) error("Starting Lisp: Next stack block isn't free!");

  while (GETWORD(freeptr) == STK_FSB_WORD) EndSTKP = freeptr = freeptr + GETWORD(freeptr + 1);

  CurrentStackPTR = next68k - 2;

  FastRetCALL;

  /* JRB - The interrupt initialization must be done right before  */
  /*       entering the bytecode dispatch loop; interrupts get     */
  /*       unblocked here 					   */
  int_init();
#ifdef DOS
  _dpmi_lockregion((void *)&dispatch, 32768);
#endif /* DOS */
  dispatch();
}

void print_info_lines(void) {
#if (RELEASE == 200)
  printf("Emulator for Medley release 2.0\n");
#elif (RELEASE == 201)
  printf("Emulator for Medley release 2.01\n");
#elif (RELEASE == 210)
  printf("Emulator for Medley release 2.1\n");
#elif (RELEASE == 300)
  printf("Emulator for Medley release 3.0\n");
#elif (RELEASE == 350)
  printf("Emulator for Medley release 3.5\n");
#elif (RELEASE == 351)
  printf("Emulator for Medley release 3.51\n");
#endif /* RELEASE */
  printf("Compiled for %s (%s) (%d bit).\n", MAIKO_OS_NAME, MAIKO_ARCH_NAME, MAIKO_ARCH_WORD_BITS);
  printf("Creation date: %s", ctime(&MDate));
#ifdef LPSOLVE
  printf("Contains lp_solve LP solver.\n");
#endif /* LPSOLVE */
#ifdef BIGBIGVM
  printf("Supports 256Mb virtual memory.\n");
#elif BIGVM
  printf("Supports 64Mb virtual memory.\n");
#else
  printf("Supports 32Mb virtual memory.\n");
#endif /* BIGVM */
#ifdef NOVERSION
  printf("Does not enforce SYSOUT version matching.\n");
#endif /* NOVERSION */
#ifdef MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE
  printf("Has foreign-function-call interface.\n");
#else
  printf("Has no foreign-function-call interface.\n");
#endif /* MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE */
#ifdef NOEUROKBD
  printf("No support for European keyboards.\n");
#else
  printf("Supports Sun European Type-4/5 keyboards.\n");
#endif /* NOEUROKBD */
}
