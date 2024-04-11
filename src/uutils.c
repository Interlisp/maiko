/* $Id: uutils.c,v 1.3 1999/05/31 23:35:47 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*			    U U T I L S . C				*/
/*									*/
/*	Utility subrs for dealing with the host OS.			*/
/*									*/
/************************************************************************/

#ifndef DOS
#include <pwd.h>         // for getpwuid, passwd
#endif
#include <signal.h>      // for killpg, SIGTSTP
#include <stddef.h>      // for size_t
#include <stdio.h>       // for printf, NULL, snprintf, size_t
#include <stdlib.h>      // for getenv
#include <string.h>      // for strcmp, strcpy, strlen, strncpy
#include <unistd.h>      // for getuid, gethostid, gethostname, getpgrp
#include "adr68k.h"      // for NativeAligned4FromLAddr
#include "keyboard.h"    // for KBEVENT, KB_ALLUP, RING, KEYEVENTSIZE, MAXKE...
#include "lispemul.h"    // for LispPTR, DLword, NIL, ATOM_T
#include "lspglob.h"
#include "lsptypes.h"    // for GETWORD, OneDArray, GetTypeNumber, THIN_CHAR...
#include "osmsg.h"       // for OSMESSAGE_PRINT
#include "uraiddefs.h"   // for device_after_raid, device_before_raid
#include "uutilsdefs.h"  // for c_string_to_lisp_string, check_unix_password

/************************************************************************/
/*									*/
/*		l i s p _ s t r i n g _ t o _ c _ s t r i n g		*/
/*									*/
/*	converts lisp string up to maximum length; returns 0 if it	*/
/*	succeeds, -1 if error (not a simple string, or too long, or	*/
/*	contains NS characters outside charset 0).			*/
/*									*/
/************************************************************************/

static int lisp_string_to_c_string(LispPTR Lisp, char *C, size_t length) {
  OneDArray *arrayp;
  char *base;

  if (GetTypeNumber(Lisp) != TYPE_ONED_ARRAY) { return (-1); }

  arrayp = (OneDArray *)NativeAligned4FromLAddr(Lisp);
  if (arrayp->fillpointer >= length) { return (-1); } /* too long */

  switch (arrayp->typenumber) {
    case THIN_CHAR_TYPENUMBER:
      base = ((char *)NativeAligned2FromLAddr(arrayp->base)) + ((int)(arrayp->offset));
#ifndef BYTESWAP
      strncpy(C, base, arrayp->fillpointer);
#else
      {
        size_t l = arrayp->fillpointer;
        char *dp = C;
        for (size_t i = 0; i < l; i++) {
          *dp++ = (char)(GETBYTE(base++));
        }
      }
#endif /* BYTESWAP */

      C[arrayp->fillpointer] = '\0';
      return 0;
    default: return -1;
  }
}

/************************************************************************/
/*									*/
/*		c _ s t r i n g _ t o _ l i s p _ s t r i n g		*/
/*									*/
/*	copies a C string into an existing Lisp string (it does not 	*/
/*	create a new Lisp string).  Returns 0 if succeeds, -1 if	*/
/*	error (Lisp string is not a simple string, or not long enough,	*/
/*	or is FATP).  The string will end with a null, which the	*/
/*	length must include.						*/
/*									*/
/************************************************************************/

static int c_string_to_lisp_string(char *C, LispPTR Lisp) {
  OneDArray *arrayp;
  char *base;
  size_t length;

  length = strlen(C);
  if (GetTypeNumber(Lisp) != TYPE_ONED_ARRAY) { return (-1); }

  arrayp = (OneDArray *)NativeAligned4FromLAddr(Lisp);
  if (arrayp->totalsize < length + 1) { return (-1); }
  /* too short for C string */

  switch (arrayp->typenumber) {
    case THIN_CHAR_TYPENUMBER:
      base = ((char *)NativeAligned2FromLAddr(arrayp->base)) + ((int)(arrayp->offset));
#ifndef BYTESWAP
      strcpy(base, C);
#else
      {
        char *dp = C;
        for (size_t i = 0; i < length + 1; i++) {
          char ch = *dp++;
#ifdef DOS
          if (ch == '\\') dp++; /* skip 2nd \ in \\ in C strings */
#endif /* DOS */
          GETBYTE(base++) = ch;
        }
      }
#endif /* BYTESWAP */

      return 0;
    default: return -1;
  }
}

/************************************************************************/
/*									*/
/*		    c h e c k _ u n i x _ p a s s w o r d		*/
/*									*/
/*	Check this guy's password against what he gave us.		*/
/*									*/
/************************************************************************/

LispPTR check_unix_password(LispPTR *args) {
  return ATOM_T;
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR unix_username(LispPTR *args) {
#ifndef DOS
  struct passwd *pwd;

  if ((pwd = getpwuid(getuid())) == NULL) return NIL;
  if (c_string_to_lisp_string(pwd->pw_name, args[0])) return NIL;
#endif /* DOS */
  return ATOM_T;
}

/************************************************************************/
/*									*/
/*			u n i x _ g e t p a r m				*/
/*									*/
/*	Given a string name for a configuration parameter, return	*/
/*	a string with the value for that parameter.			*/
/*									*/
/*	PARAMETER	MEANING/VALUES					*/
/*									*/
/*	MACH		What kind of processor we're running on		*/
/*			(sparc, mc68020, or i386 possible)		*/
/*									*/
/*	ARCH		The kind of machine we're running on		*/
/*			(sun4, sun386, sun3 possible)			*/
/*									*/
/*	DISPLAY		What kind of display we're running with		*/
/*			(X, BUFFERED, DIRECT possible)			*/
/*									*/
/*	HOSTNAME	Name of the machine we're running on.		*/
/*									*/
/*	LOGNAME		Login ID of the user running Lisp.		*/
/*									*/
/*	FULLUSERNAME	??						*/
/*									*/
/*	HOSTID		Machine serial# or Ether ID.			*/
/*									*/
/*									*/
/************************************************************************/
/*
 * The code for "MACH" and "ARCH" are really not correct and it's not
 * clear what use they are. RS/6000 systems use a PowerPC processor,
 * and so did PowerBook Macintosh systems.
 * "MACH" and "ARCH" both seem to be a mix of instruction set architecture and
 * system types (rs/6000 used PowerPC).
 * The only usage seems to be checking "ARCH" == "dos" and for the existence
 * of *any* result from the call, which indicates it's an emulated system.
 */
LispPTR unix_getparm(LispPTR *args) {
  char envname[20], result[128], *envvalue;
  if (lisp_string_to_c_string(args[0], envname, sizeof envname)) return NIL;

  if (strcmp(envname, "MACH") == 0) {
#if defined(sparc)
    envvalue = "sparc";
#elif defined(DOS)
    envvalue = "386";
#elif defined(MAIKO_OS_MACOS)
    envvalue = "i386";
#else
    envvalue = "mc68020";
#endif
  } else if (strcmp(envname, "OSNAME") == 0) {
    envvalue = MAIKO_OS_NAME;
  } else if (strcmp(envname, "ARCH") == 0) {
    envvalue = MAIKO_ARCH_NAME;
  } else if (strcmp(envname, "DISPLAY") == 0) {
#if defined(XWINDOW)
    envvalue = "X";
#elif defined(SDL)
    envvalue = "SDL";
#elif defined(DISPLAYBUFFER)
    envvalue = "BUFFERED";
#else
    envvalue = "DIRECT";
#endif
  }
#ifndef DOS
  else if (strcmp(envname, "HOSTNAME") == 0) {
    if (gethostname(result, sizeof result)) return NIL;
    envvalue = result;
  } else if (strcmp(envname, "LOGNAME") == 0) {
    struct passwd *pwd;
    if ((pwd = getpwuid(getuid())) == NULL) return NIL;
    envvalue = pwd->pw_name;
  } else if (strcmp(envname, "FULLUSERNAME") == 0) {
    struct passwd *pwd;
    if ((pwd = getpwuid(getuid())) == NULL) return NIL;
    envvalue = pwd->pw_gecos;
  } 
#ifndef MAIKO_OS_HAIKU
  else if (strcmp(envname, "HOSTID") == 0) {
    snprintf(result, sizeof(result), "%lx", gethostid());
    envvalue = result;
  }
#endif /* MAIKO_OS_HAIKU */
#endif /* DOS */
  else
    return NIL;

  if (c_string_to_lisp_string(envvalue, args[1])) return NIL;
  return ATOM_T;
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR unix_getenv(LispPTR *args) {
  char envname[20], *envvalue;
  if (lisp_string_to_c_string(args[0], envname, sizeof envname)) return NIL;
  envvalue = getenv(envname);
  if (!envvalue) return NIL;
  if (c_string_to_lisp_string(envvalue, args[1])) return NIL;
  return ATOM_T;
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

LispPTR unix_fullname(LispPTR *args) {
#ifndef DOS
  struct passwd *pwd;

  if ((pwd = getpwuid(getuid())) == NULL) return NIL;
  if (c_string_to_lisp_string(pwd->pw_gecos, args[0])) return NIL;
#endif /* DOS */
  return ATOM_T;
}

/************************************************************************/
/*									*/
/*			s u s p e n d _ l i s p				*/
/*									*/
/*	Suspend execution, ala ^Z from the shell.			*/
/*									*/
/************************************************************************/

extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K, *EmUtilin68K;
extern DLword *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K;

LispPTR suspend_lisp(LispPTR *args) {
#ifndef DOS
  extern DLword *CTopKeyevent;
  extern LispPTR *KEYBUFFERING68k;

  DLword w, r;
  KBEVENT *kbevent;

  if (device_before_raid() < 0) {
    OSMESSAGE_PRINT(printf("Can't suspend\n"));
    return NIL;
  }

  OSMESSAGE_PRINT(printf("suspending...\n"));

/* Send a terminal-stop signal to the whole process-group, not
   just this process, so that if we are running as part of a
   C-shell file the shell will be suspended too. */
  killpg(getpgrp(), SIGTSTP);

  OSMESSAGE_PRINT(printf("resuming\n"));
  device_after_raid();

  r = RING_READ(CTopKeyevent);
  w = RING_WRITE(CTopKeyevent);

  /*NO CARE about event queue FULL */

  GETWORD(EmKbdAd068K) = KB_ALLUP;
  GETWORD(EmKbdAd168K) = KB_ALLUP;
  GETWORD(EmKbdAd268K) = KB_ALLUP;
  GETWORD(EmKbdAd368K) = KB_ALLUP;
  GETWORD(EmKbdAd468K) = KB_ALLUP;
  GETWORD(EmKbdAd568K) = KB_ALLUP;
  GETWORD(EmRealUtilin68K) = KB_ALLUP;

  kbevent = (KBEVENT *)(CTopKeyevent + w);

  /*    RCLK(kbevent->time); */

  kbevent->W0 = GETWORD(EmKbdAd068K);
  kbevent->W1 = GETWORD(EmKbdAd168K);
  kbevent->W2 = GETWORD(EmKbdAd268K);
  kbevent->W3 = GETWORD(EmKbdAd368K);
  kbevent->W4 = GETWORD(EmKbdAd468K);
  kbevent->W5 = GETWORD(EmKbdAd568K);
  kbevent->WU = GETWORD(EmRealUtilin68K);

  if (r == 0) /* Queue was empty */
    ((RING *)CTopKeyevent)->read = w;

  if (w >= MAXKEYEVENT)
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  else
    ((RING *)CTopKeyevent)->write = w + KEYEVENTSIZE;
#endif /* DOS, which doesn't support suspend-lisp */
  return ATOM_T;
}
