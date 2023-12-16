/* $Id: initkbd.c,v 1.2 1999/01/03 02:07:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef DOS
#include <sys/file.h>
#include <sys/select.h>
#endif /* DOS */

#ifdef DOS
#include <i32.h>   /* "#pragma interrupt" & '_chain_intr'*/
#include <dos.h>   /* defines REGS & other structs       */
#include <stdio.h> /* define NULL                        */
#include <conio.h>
#include <time.h>
#include <stk.h>
#endif /* DOS */


#ifdef XWINDOW
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "XKeymap.h"
#include "xdefs.h"
#endif /* XWINDOW */

#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "adr68k.h"
#include "address.h"

#include "devconf.h"
#include "ifpage.h"
#include "keyboard.h"

#include "initkbddefs.h"
#include "initdspdefs.h"

#ifdef XWINDOW
#include "devif.h"
#include "xinitdefs.h"
extern DspInterface currentdsp;
#endif /* XWINDOW */

#ifdef DOS
#include "devif.h"
extern MouseInterface currentmouse;
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
#endif /* DOS */

extern int LispKbdFd;
int LispKbdFd = -1;

extern fd_set LispReadFds;

extern DLword *EmMouseX68K;
extern DLword *EmMouseY68K;
extern DLword *EmCursorX68K;
extern DLword *EmCursorY68K;
extern DLword *EmRealUtilin68K;
extern DLword *EmUtilin68K;
extern DLword *EmKbdAd068K;
extern DLword *EmKbdAd168K;
extern DLword *EmKbdAd268K;
extern DLword *EmKbdAd368K;
extern DLword *EmKbdAd468K;
extern DLword *EmKbdAd568K;
extern DLword *EmDispInterrupt68K;
extern DLword *EmCursorBitMap68K;

DLword *EmMouseX68K;
DLword *EmMouseY68K;
DLword *EmCursorX68K;
DLword *EmCursorY68K;
DLword *EmRealUtilin68K;
DLword *EmUtilin68K;
DLword *EmKbdAd068K;
DLword *EmKbdAd168K;
DLword *EmKbdAd268K;
DLword *EmKbdAd368K;
DLword *EmKbdAd468K;
DLword *EmKbdAd568K;
DLword *EmDispInterrupt68K;
DLword *EmCursorBitMap68K;

/*u_char SUNLispKeyMap[128];*/
extern u_char *SUNLispKeyMap;
u_char *SUNLispKeyMap;

/* keymap for type3 */
static u_char SUNLispKeyMap_for3[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 255,
    /*   8 */ 100, 255,  67, 255,  68, 255, 101, 255,
    /*  16 */  66, 104,  80,  47, 255,  73,  74,  75,
    /*  24 */ 255,  92,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  13, 255,  81,  82,  83,
    /*  48 */ 255,  14, 255,  62, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  15, 255,  84,  85,  87, 255,
    /*  72 */ 111,  89, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44, 255,  94,  69,  70, 255,  90,
    /*  96 */ 255,  46, 255,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  98,  76,  72, 255, 255, 255, 255,  56,
    /* 120 */  31,  57,  93, 255, 255, 255, 255, 255
};

/* for type4 */

static u_char SUNLispKeyMap_for4[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  30,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  94,  69,  70, 255, 255, 255,  92,  56,
    /* 120 */  86,  57,  88, 255, 103, 102, 255, 255
};

/* for jle */

static u_char SUNLispKeyMap_jle[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  71,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  59,  45,  30,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  10,
    /* 112 */  94,  69,  70,  72, 103, 109,  92,  56,
    /* 120 */  86,  57,  88, 255, 255, 102, 255, 255
};
/* [40]   10 -> 59  */
/* [41]   59 -> 45  */
/* [42]   45 -> 30  */
/* [111]  71 -> 10  */
/* [115] 255 -> 72  Kakutei */
/* [116] 255 -> 103 Henkan */
/* [117] 255 -> 109 Nihongo On-Off */

static u_char *XGenericKeyMap; /* filled in with malloc if needed */

/* For the IBM-101 kbd FF marks exceptions */

#ifdef NEVER
u_char DOSLispKeyMap_101[0x80] = {
    /*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */

    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   81,   82,   83,   96,   84,   85,   87,   102,  94,
    /* 5*/ 69,   70,   98,   13,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif /* NEVER */

#ifdef DOS
/* For the IBM-101 kbd FF marks exceptions */
static u_char DOSLispKeyMap_101[0x80] = {
    /*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */

    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   62,   130,  63,   96,   129,  85,   132,  102,  90,
    /* 5*/ 131,  91,   89,   46,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif

void init_keyboard(int flg) /* if 0 init else re-init */
{
  set_kbd_iopointers();
  /* if using a raw keyboard, LispKbdFd would be the result of opening /dev/kbd
   * and it would be added to LispReadFds to generate keyboard events
   */
  if (flg == 0) { keyboardtype(LispKbdFd); }

#if   XWINDOW
  init_Xevent(currentdsp);
#elif DOS
  if (flg == 0) { /* Install the handlers ONLY when we */
    /* init the kbd the init the kbd the */
    /* first time. */

    /* turn on kbd */
    make_kbd_instance(currentkbd);
    (currentkbd->device.enter)(currentkbd);

    /* turn on mouse */
    make_mouse_instance(currentmouse);
    (currentmouse->device.enter)(currentmouse, currentdsp);
  }
#endif /* XWINDOW DOS */
}

/*  ----------------------------------------------------------------*/

void device_before_exit(void) {
#ifdef   DOS
  (currentmouse->device.exit)(currentmouse);
  (currentkbd->device.exit)(currentkbd);
#endif /* SUNDISPLAY DOS*/
  display_before_exit();
}

/*  ----------------------------------------------------------------*/

void set_kbd_iopointers(void) {
  EmMouseX68K = (DLword *)&(IOPage->dlmousex);
  EmMouseY68K = (DLword *)&(IOPage->dlmousey);
  EmCursorX68K = (DLword *)&(IOPage->dlcursorx);
  EmCursorY68K = (DLword *)&(IOPage->dlcursory);
  EmRealUtilin68K = (DLword *)&(IOPage->dlutilin);
  /* EmUtilin68K is for KEYDOWNP1 macro or etc. */
  EmUtilin68K = (DLword *)&(InterfacePage->fakemousebits);
  EmKbdAd068K = (DLword *)&(IOPage->dlkbdad0);
  EmKbdAd168K = (DLword *)&(IOPage->dlkbdad1);
  EmKbdAd268K = (DLword *)&(IOPage->dlkbdad2);
  EmKbdAd368K = (DLword *)&(IOPage->dlkbdad3);
  EmKbdAd468K = (DLword *)&(IOPage->dlkbdad4);
  EmKbdAd568K = (DLword *)&(IOPage->dlkbdad5);
  EmDispInterrupt68K = (DLword *)&(IOPage->dldispinterrupt);
  EmCursorBitMap68K = (DLword *)(IOPage->dlcursorbitmap);

  *EmRealUtilin68K = KB_ALLUP;
  *EmKbdAd068K = KB_ALLUP;
  *EmKbdAd168K = KB_ALLUP;
  *EmKbdAd268K = KB_ALLUP;
  *EmKbdAd368K = KB_ALLUP;
  *EmKbdAd468K = KB_ALLUP;
  *EmKbdAd568K = KB_ALLUP;
}

/*  ----------------------------------------------------------------*/


#define MIN_KEYTYPE 3
#define KB_AS3000J (7 + MIN_KEYTYPE)
#define KB_RS6000 (8 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_DEC3100 (9 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_HP9000 (10 + MIN_KEYTYPE)  /* TODO: Can we remove this? */
#define KB_X (11 + MIN_KEYTYPE)
#define KB_DOS (12 + MIN_KEYTYPE)
#define KB_SDL (13 + MIN_KEYTYPE)

/* KB_SUN4 not defined in older OS versions */
#ifndef KB_SUN4
#define KB_SUN4 4
#endif

#ifndef KB_SUN2
/* These KB types nog defined outside Sun world,so define them here */
#define KB_SUN2 2
#define KB_SUN3 3
#endif /* KB_SUN2 */

/* For the JLE keyboard */
#define KB_JLE 5


#ifdef XWINDOW
/*
 * 
 */

static int find_unused_key(KeySym *map, int minkey, int codecount, int symspercode, int sym, u_char *table)
{
  int i;

  for (i = 0; i < (codecount * symspercode); i++) {
    if (sym == map[i]) {
      int code = minkey + (i / symspercode);
      if (table[code - 7] != 255) {
#ifdef DEBUG
          printf("table[%d - 7] != 255\n", code);
#endif
          continue;
      }
      return (code);
    }
  }
  return (0);
}

/************************************************************************/
/*									*/
/*			m a k e _ X _ k e y m a p			*/
/*									*/
/*	Starting from the generic-X-keyboard mapping in XKeymap.h,	*/
/*	construct a keyboard map for this machine, using the rules	*/
/*	shown in the header file.					*/
/*									*/
/*									*/
/************************************************************************/

static u_char *make_X_keymap(void) {
  u_char *table = (u_char *)malloc(256); /* the final result table */
  int lisp_codes_used[256];              /* Keep track of the Lisp key #s we've used */
  int last_KEYSYM = -1;
  int sym_used = 0;
  const int *key_sym_pairs = generic_X_keymap;
  int i = 0;
  KeySym *mapping;
  int codecount, symspercode, minkey, maxkey;

  for (; i < 256; i++) { /* clear the tables we just allocated */
    lisp_codes_used[i] = 0;
    table[i] = 255; /* The "no key assigned" code */
  }

  XLOCK;
  XDisplayKeycodes(currentdsp->display_id, &minkey, &maxkey);
  codecount = maxkey + 1 - minkey;
  mapping = XGetKeyboardMapping(currentdsp->display_id, minkey, codecount, &symspercode);
  XUNLOCK(currentdsp);

  for (; *key_sym_pairs != -1;) {
    int reusable = *key_sym_pairs++, code = *key_sym_pairs++, sym = *key_sym_pairs++, xcode;

    if (sym_used && (sym == last_KEYSYM)) continue;

    sym_used = 0;
    last_KEYSYM = sym;

    xcode = find_unused_key(mapping, minkey, codecount, symspercode, sym, table);

    if (xcode == 0) continue;
    if ((!reusable) && (lisp_codes_used[code] != 0)) continue;

    sym_used = 1;
    last_KEYSYM = sym;
    lisp_codes_used[code] = 1;
    table[xcode - 7] = code;
  }

#ifdef DEBUG
  printf("\n\n\tXGetKeyboardMapping table\n\n");
  for (i = 0; i < codecount * symspercode; i += symspercode) {
      printf("%d:", minkey + (i / symspercode));
      for (int j = 0; j < symspercode; j++) {
        printf("\t %8lx", (unsigned long)mapping[i+j]);
      }
      printf("\n");
  }

  printf("\n\n\tKeyboard mapping table\n\n");
  for (i = 0; i < 256; i += 8) {
    printf("%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, table[i], table[i + 1], table[i + 2], table[i + 3], table[i + 4],
           table[i + 5], table[i + 6], table[i + 7]);
  }
#endif /* DEBUG */

  XFree(mapping); /* No locking required? */

  return (table);
}

#endif /* XWINDOW */

/************************************************************************/
/*									*/
/*			  k e y b o a r d t y p e			*/
/*									*/
/*	Determine what kind of keyboard we're dealing with, by		*/
/*	checking the LDEKBDTYPE shell variable.  It it's not set,	*/
/*	either default it (for Sun, IBM), or complain and exit.		*/
/*	Valid LDEKBDTYPE values:					*/
/*		type3	Sun type-3 keyboard				*/
/*		type4	Sun type-4 keyboard				*/
/*		rs6000	IBM RS/6000					*/
/*		x	generic X keyboard map				*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void keyboardtype(int fd)
{
  int type;
  int i;
  char *key;

#if defined(OS4)
  for (i = 0; i < 5000; i++) { /* IDLE LOOP */
  }                            /* because of a SunOS bug */
#endif

  /* clear the keyboard field in devconfig */
  InterfacePage->devconfig &= 0xfff8;

  /************************************************************
   Due to the problems of SunOS 4.0 & 4.0.1
   calling ioctl never return the correct keyboard type.
   So,these 2 lines are commented out ...(Take)->AR11100
  *************************************************************/

  /* Get keytype from LDEKBDTYPE  */
  if ((key = getenv("LDEKBDTYPE")) == 0) {
#ifdef XWINDOW
    type = KB_X;
#elif DOS
    type = KB_DOS;
#elif SDL
    type = KB_SDL;
#endif /* XWINDOW */
  } /* if end */
  else {
    if (strcmp("as3000j", key) == 0)
      type = KB_AS3000J;
    else if (strcmp("type4", key) == 0)
      type = KB_SUN4;
    else if (strcmp("type2", key) == 0)
      type = KB_SUN2;
    else if (strcmp("jle", key) == 0)
      type = KB_JLE;
    else if (strcmp("X", key) == 0 || strcmp("x", key) == 0)
      type = KB_X;
    else if (strcmp("sdl", key) == 0)
      type = KB_SDL;
    else
      type = KB_SUN3; /* default */
  }

  switch (type) {
    case KB_SUN2: /* type2, we still use keymap for type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      /* MIN_KEYTYPE is 3,so we can't set devconfig correctly */
      /* Therefore type2 may treat as type3 */
      InterfacePage->devconfig |= 0;
      break;
    case KB_SUN3: /* type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_SUN4: /* type4 */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_JLE: /* JLE */
      /*printf("jle\n"); */
      SUNLispKeyMap = SUNLispKeyMap_jle;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      /* InterfacePage->devconfig |= 4-MIN_KEYTYPE; */
      break;
    case KB_AS3000J: /* for AS, use type4 map */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE; /* 7 */
      break;
#ifdef XWINDOW
    case KB_X:
      XGenericKeyMap = (u_char *)make_X_keymap();
      SUNLispKeyMap = XGenericKeyMap;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
      break;
#endif /* XWINDOW */
#ifdef SDL
  case KB_SDL:
    InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
    break;
#endif /* SDL */
#ifdef DOS
    case KB_DOS:
      SUNLispKeyMap = DOSLispKeyMap_101;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
      break;
#endif /* DOS */
    default: {
      char errmsg[200];
      sprintf(errmsg, "Unsupported keyboard type: %d", type);
      printf("%s\n", errmsg);
      printf("Configuring keyboard for type-3\n");
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE;
      break;
    }
  }

} /* end keyboardtype*/
