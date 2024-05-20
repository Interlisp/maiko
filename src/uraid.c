/* @(#) uraid.c Version 1.52 (4/23/92). copyright Venue & Fuji Xerox  */

/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1989-1995 Venue. All Rights Reserved.             */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/************************************************************************/

#include "version.h"

/************************************************************************/
/************************************************************************/
/*                                                                      */
/*                          U R A I D . C                               */
/*                                                                      */
/*      URAID is the low-level debugger for Medley.                     */
/*                                                                      */
/************************************************************************/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef DOS
#include <sys/file.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/select.h>
#endif /* DOS */

#ifdef XWINDOW
#include <X11/Xlib.h>      // for ConnectionNumber
#endif

#ifdef OS5
#include <stropts.h>
#endif /* OS5 */

#if defined(MAIKO_OS_FREEBSD) || defined(MAIKO_OS_MACOS) || defined(OS5)
/* vfork is deprecated */
#define vfork fork
#endif

#include "version.h"             // for BIGVM

#include "adr68k.h"              // for NativeAligned4FromStackOffset, LAddr...
#include "cell.h"                // for DefCell, GetDEFCELL68k, GetVALCELL68k
#include "dbgtooldefs.h"         // for bt1, bt, sf
#include "devif.h"               // for DspInterfaceRec, DevRec, DspInterface
#include "display.h"             // for DISPLAYBUFFER
#include "gcarraydefs.h"         // for get_package_atom
#include "gcfinaldefs.h"         // for printarrayblock
#include "ifpage.h"              // for IFPAGE
#include "initdspdefs.h"         // for clear_display, flush_display_buffer
#include "initkbddefs.h"         // for init_keyboard
#include "kprintdefs.h"          // for print
#include "lispemul.h"            // for T, DLword, LispPTR, POINTERMASK, NIL
#include "lispmap.h"             // for DISPLAY_HI, ATOM_HI, DEFS_HI, DISPLA...
#include "llstkdefs.h"           // for stack_check
#include "lspglob.h"             // for InterfacePage, Plistspace
#include "lsptypes.h"            // for GETWORD, GetDTD, GetTypeNumber, dtd
#include "mkatomdefs.h"          // for make_atom
#include "returndefs.h"          // for contextsw
#include "stack.h"               // for FX, ResetFXP
#include "testtooldefs.h"        // for all_stack_dump, doko, printPC, print...
#include "timerdefs.h"           // for int_block, int_init, int_unblock
#include "uraiddefs.h"           // for copy_region, device_after_raid, devi...
#include "uraidextdefs.h"        // for URMAXCOMM, URMAXFXNUM, URSCAN_ALINK
#include "vmemsavedefs.h"        // for vmem_save

#ifdef MAIKO_ENABLE_ETHERNET
#include "etherdefs.h"
#endif

#ifdef DOS
#define vfork() printf("No forking around here.\n")
#endif /* DOS */

#if defined(DOS) || defined(XWINDOW)
#include "devif.h"
extern DspInterface currentdsp;
#endif /* DOS || XWINDOW */

#ifdef DOS
extern MouseInterface currentmouse;
extern KbdInterface currentkbd;
#endif /* DOS */

#ifdef COLOR
extern int MonoOrColor;
extern int Dispcolorsize;
extern DLword *ColorDisplayRegion68k;
#endif /* COLOR */

/***********************************************************************/
/*
        File name : uraid.c

        Written by : Takeshi Shimizu(Take)
        Date:   6-May-1988

        This File Supports the following commands:

**URAID command summary**
<<Displaying a Stack>>
c                       Checks all user stack contents
f number                Displays frame extension for the number(decimal)
k type                  Changes the type of stack link following.(a|c)
l [type]                Back trace for specified type stack.(k|m|r|g|p|u)
<CR>                    Display next frame.
<<Displaying the Memory contents>>
a litatom               Displays the top-level value of the litatom
d litatom               Displays the definition cell for the litatom
M                       Displays TOS,CSP,PVAR,IVAR,PC
m func1 func2           MOVED func1 to func2
t Xaddress              Displays the type of this object
p litatom               Displays the property list of the litatom
w                       Displays the current function-name and PC
x Xaddress [xnum]       Hex-Dump xnum word(16-bits) starting at Xaddress
@ litatom val           Sets TOPVAL of litatom to Decimal-val
< Xaddress val          Sets the word(16-bits) at the address to val
<<Return or Exit>>
e                       Exit to UNIX / DOS
h                       Context switch to HARDRESET
q                       Returns from URAID with NO-change
<<Misc>>
s                       Invoke Shell
v filename              Save the virtual memory on the filename(Not Bootable)
( [num]                 Sets Print level
!                       Prints the error message passed from the emulator
?                       Display this summary

*/
/************************************************************************/

extern int DisplayRasterWidth;
extern int LispKbdFd;
extern fd_set LispReadFds;
extern struct cursor CurrentCursor, InvisibleCursor;
extern struct screen LispScreen;
extern int displaywidth, displayheight;
extern DLword *DisplayRegion68k;
extern int FrameBufferFd, ether_fd, RS232C_Fd;

#ifdef COLOR
DLword *HideColorDisp68k;
extern int Inited_Color;
#endif /* COLOR */

#ifdef DOS
static const char *URaid_summary1 =
    "\n-- Stack display commands\n\
c\t\t\tChecks all user stack contents\n\
f number\t\tDisplays stack frame for that frame number (decimal)\n\
k type\t\t\tChanges the type of stack link following. (a|c)\n\
l [type]\t\tBack Trace for specified type stack. (k|m|r|g|p|u|<null>)\n\
<CR>\t\t\tDisplay next frame.\n";

static const char *URaid_summary2 =
    "\n-- Memory display commands\n\
a litatom\t\tDisplays the top-level value of the litatom\n\
B Xaddress\t\tPrint the contents of the arrayblock at that address.\n\
d litatom\t\tDisplays the definition cell for the litatom\n\
M\t\t\tDisplays TOS,CSP,PVAR,IVAR,PC\n\
m func1 func2\t\tMOVD func1 to func2\n\
O Xaddress\t\tDisplays the object with that address\n\
t Xaddress\t\tDisplays the type of this object\n\
p litatom\t\tDisplays the property list of the litatom\n\
w\t\t\tDisplays the current function-name and PC\n\
x Xaddress [xnum]\tHex-Dump xnum (16-bit) words starting at Xaddress\n\
@ litatom val\t\tSets TOPVAL of litatom to Decimal-val\n\
< Xaddress Xval\t\tSets the (16-bit) word at the address to Xval\n";

static const char *URaid_summary3 =
    "\n-- Continue or exit commands\n\
e\t\t\tExit to DOS\n\
h\t\t\tDo a HARDRESET\n\
q\t\t\tReturns from URAID with NO change\n\
<<Misc>>\ns\t\t\tInvoke Shell\n\
v filename\t\tSave the virtual memory on the filename(Not Bootable)\n\
( [num]\t\t\tSets Print level\n\
!\t\t\tPrints the error message passed from the emulator\n\
?\t\t\tDisplay this summary";
#else
static const char *URaid_summary =
    "---URAID command summary---\n\
\n-- Stack display commands\n\
c\t\t\tChecks all user stack contents\n\
f number\t\tDisplays stack frame for that frame number (decimal)\n\
k type\t\t\tChanges the type of stack link following. (a|c)\n\
l [type]\t\tDisplays backtrace for specified type of stack. (k|m|r|g|p|u|<null>)\n\
<CR>\t\t\tDisplays next frame.\n\
\n-- Memory display commands\n\
a litatom\t\tDisplays the top-level value of the litatom\n\
B Xaddress\t\tDisplays the contents of the arrayblock at that address.\n\
d litatom\t\tDisplays the definition cell of the litatom\n\
M\t\t\tDisplays TOS,CSP,PVAR,IVAR,PC\n\
m func1 func2\t\tMoves definition of func1 to func2 (MOVD)\n\
O Xaddress\t\tDisplays the object with that address\n\
t Xaddress\t\tDisplays the type of the object with that address\n\
p litatom\t\tDisplays the property list of the litatom\n\
w\t\t\tDisplays the current function name and PC\n\
x Xaddress [xnum]\tDisplays in hexadecimal xnum (16-bit) words starting at Xaddress\n\
@ litatom val\t\tSets the TOPVAL of litatom to (decimal) val \n\
< Xaddress Xval\t\tSets the (16-bit) word at the address to Xval\n\
\n-- Continue or exit commands\n\
e\t\t\tExits to UNIX\n\
h\t\t\tPerforms a HARDRESET\n\
q\t\t\tReturns from URAID with NO change\n\
\n-- Miscellaneous commands\n\
s\t\t\tInvokes Shell\n\
v filename\t\tSaves the virtual memory on the filename (Not Bootable)\n\
( [num]\t\t\tSets the print level\n\
!\t\t\tDisplays the error message passed from the emulator\n\
?\t\t\tDisplays this summary";
#endif /* DOS */

#define ADD_RANGEP(address)                             \
  do {                                                  \
    if (((address) < 0) || (POINTERMASK < (address))) { \
      printf("Address out of range.\n");                \
      return (T);                                       \
    }                                                   \
  } while (0)

/*** URaid G vals ***/
int URaid_scanlink = URSCAN_ALINK;
int URaid_currentFX = URMAXFXNUM + 1;
FX *URaid_FXarray[URMAXFXNUM];
int URaid_ArrMAXIndex;

char URaid_inputstring[URMAXCOMM];
char URaid_comm[2];
char URaid_arg1[URMAXCOMM / 2];
char URaid_arg2[URMAXCOMM / 2];
int URaid_argnum;
const char *URaid_errmess;

extern int PrintMaxLevel; /* for print level */

/***********************************************************************/
/*
        func name : parse_atomstring(string)
        Written by : Takeshi Shimizu
        Date:   6-May-1988
        Pass the atomstring(e.g. "XCL:EVAL")
         Then Returns atomindex .
        If fail to find, return -1

        If there is no package prefix,it will be treated as IL:
*/
/***********************************************************************/

LispPTR parse_atomstring(char *string)
{
  char *start, *packageptr, *nameptr;
  DLword packagelen = 0;
  DLword namelen = 0;
  DLword cnt;
  LispPTR aindex;

  for (cnt = 0, start = string; *string != '\0'; string++, cnt++) {
    if (*string == ':') {
      packagelen = cnt;
      packageptr = start;
      nameptr = string + 1;
      cnt = 0;
      *string = 0;
    }
  }
  if (packagelen == 0) { /* treat as IL: */
    nameptr = start;
    namelen = cnt;
  } else
    namelen = cnt - 1;

  if ((packagelen == 0) || (strncmp(packageptr, "IL", packagelen) == 0)) { /* default IL: */
    aindex = make_atom(nameptr, 0, namelen);
    if (aindex == 0xffffffff) {
      printf("trying IL:\n");
      aindex = get_package_atom(nameptr, namelen, "INTERLISP", 9, 0);
    }
  } else
    aindex = get_package_atom(nameptr, namelen, packageptr, packagelen, 0);

  if (aindex == 0xffffffff) return (0xffffffff);
  printf("INDEX : %d\n", aindex & 0xffff);
  return (aindex & 0xffff);
}

/***********************************************************************/
/*
        func name : uraid_commclear()
        Written by : Takeshi Shimizu
        Date:   6-May-1988

                Clear Command buffer

*/
/***********************************************************************/

void uraid_commclear(void) {
  memset(URaid_inputstring, 0, URMAXCOMM);
  memset(URaid_arg1, 0, URMAXCOMM / 2);
  memset(URaid_arg2, 0, URMAXCOMM / 2);

  URaid_comm[0] = 0;
  URaid_argnum = 0;
}

void copy_region(const DLword *src, DLword *dst, int width, int h)
{
  int w;

  for (; (h--);) {
    for (w = width; (w--);) { GETWORD(dst++) = GETWORD(src++); }
  }
}

struct dtd * uGetDTD(unsigned int typenum) {
    return (struct dtd *)GetDTD(typenum);
}

unsigned int uGetTN(unsigned int address) {
    return GetTypeNumber(address);
}

/***********************************************************************/
/*
        func name : uraid_commands()
        Written by : Takeshi Shimizu
        Date:   6-May-1988

                Execute URaid commands
                To continue, return T
                To exit, return NIL
*/
/***********************************************************************/

LispPTR uraid_commands(void) {
  int num, val;
  LispPTR address;
  char *endpointer;
  LispPTR index;
  DefCell *defcell68k;
#ifndef DOS
  int status;
#endif /* DOS */

  if (URaid_argnum == -1) {
    /* disp next FX */
    if (URaid_currentFX > URaid_ArrMAXIndex) {
      printf("There is no more stack.\n");
      return (T);
    } else {
      sf(URaid_FXarray[URaid_currentFX++]);
      return (T);
    }
  }
  switch (URaid_comm[0]) {
/*** Displaying STACK stuff */
#ifdef DOS
    case '1': printf("%s\n", URaid_summary1); break;
    case '2': printf("%s\n", URaid_summary2); break;
    case '3': printf("%s\n", URaid_summary3); break;
#endif /* DOS */
    case 'c': stack_check(0); break;
    case 'C': all_stack_dump(0, 0, T); break;
    case 'f':                /**if((URaid_arg1[0] < '0') || (URaid_arg1[0] > '9')){
                                printf("Illegal argument, not a number\n");
                                 return(T);
                             }**/
      if (URaid_argnum == 1) /* f comm only */
      {
        printf("DUMP-STACK: f decimal-FXnumber\n");
        return (T);
      }
      errno = 0;
      num = (int)strtoul(URaid_arg1, &endpointer, 10);
      if (errno != 0 || *endpointer != '\0') { /* com read fails */
        printf("Illegal argument, not decimal number\n");
        return (T);
      }
      if ((num > URaid_ArrMAXIndex) || (num < 0)) {
        printf("Frame number doesn't exist.\n");
        return (T);
      }
      sf(URaid_FXarray[num]);
      URaid_currentFX = num + 1;
      break;
    case 'k':
      if ((URaid_arg1[0] == 'A') || (URaid_arg1[0] == 'a'))
        URaid_scanlink = URSCAN_ALINK;
      else if ((URaid_arg1[0] == 'C') || (URaid_arg1[0] == 'c'))
        URaid_scanlink = URSCAN_CLINK;
      else
        printf("Link type should be A or C\n");
      break;
    case 'l':
      if (URaid_argnum == 1) {
        bt(); /* default case CURRENTFX */
        return (T);
      }
      switch (URaid_arg1[0]) {
        case 'k':
          printf("IFP->KBDFXP :\n");
          bt1((FX *)NativeAligned4FromStackOffset(InterfacePage->kbdfxp));
          break;
        case 'm':
          printf("IFP->MISCFXP :\n");
          bt1((FX *)NativeAligned4FromStackOffset(InterfacePage->miscfxp));
          break;
        case 'r':
          printf("IFP->RESETFXP :\n");
          bt1((FX *)NativeAligned4FromStackOffset(InterfacePage->resetfxp));
          break;
        case 'g':
          printf("IFP->GCFXP :\n");
          bt1((FX *)NativeAligned4FromStackOffset(InterfacePage->gcfxp));

          break;
        case 'p':
          printf("IFP->FAULTFXP :\n");
          bt1((FX *)NativeAligned4FromStackOffset(InterfacePage->faultfxp));

          break;
        case 'u': bt(); break;
        default: printf("2nd argument should be k,m,r,g,p,u or null.\n"); break;
      } /* switch end */
      break;

    /* Displaying the memory contents stuff */
    case 'a': /* GETTOPVAL */
      if (URaid_argnum != 2) {
        printf("GETTOPVAL: a litatom\n");
        return (T);
      }
      if ((index = parse_atomstring(URaid_arg1)) == 0xffffffff) {
        printf("No such atom.\n");
        return (T);
      } else {
	printf("Atom: %s index %d\n", URaid_arg1, index);
      }
      print(*((LispPTR *)GetVALCELL68k(index)));
      break;
    case 'B': { /* print array block */
      LispPTR objaddr;
      if (URaid_argnum == 1) {
        printf("PRINT-ARRAY-BLOCK: B HEX-LispAddress\n");
        return (T);
      }
      errno = 0;
      objaddr = (LispPTR)strtoul(URaid_arg1, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg not HEX number\n");
        return (T);
      }
      printarrayblock(objaddr);
    }
    break;

    case 'd': /* DEFCELL */
      if (URaid_argnum != 2) {
        printf("GETD: d litatom\n");
        return (T);
      }
      if ((index = parse_atomstring(URaid_arg1)) == 0xffffffff) {
        printf("No such atom.\n");
        return (T);
      } else {
	printf("Atom: %s index %d\n", URaid_arg1, index);
      }
      defcell68k = (DefCell *)GetDEFCELL68k(index);
      if (defcell68k->ccodep) {
        printf("{CCODEP}0x%x\n", defcell68k->defpointer);
        return (T);
      } else {
        print(defcell68k->defpointer);
        return (T);
      }

    case 'M': /* Machine States */
      printf("TOS   : 0x%x\n", TopOfStack);
      printf("CSTKP : 0x%x\n", LAddrFromNative(CurrentStackPTR));
      printf("PVAR  : 0x%x\n", LAddrFromNative(PVar));
      printf("IVAR  : 0x%x\n", LAddrFromNative(IVar));
      printPC();
      putchar('\n');
      break;
    case 'm': /* MOVD */
      if (URaid_argnum != 3) {
        printf("MOVD: m <from funcname> <to funcname>\n");
        return (T);
      }
      {
        DefCell *fromfunc, *tofunc;
        LispPTR fromindex, toindex;
        if ((fromindex = parse_atomstring(URaid_arg1)) == 0xffffffff) {
          printf("No such function (from)\n");
          return (T);
        }
        if ((toindex = parse_atomstring(URaid_arg2)) == 0xffffffff) {
          printf("No such function (to)\n");
          return (T);
        }

        fromfunc = (DefCell *)GetDEFCELL68k(fromindex);
        tofunc = (DefCell *)GetDEFCELL68k(toindex);
        tofunc->defpointer = fromfunc->defpointer;
        print(toindex);
        printf(" is smashed with ");
        print(fromindex);
        putchar('\n');
      }

      break;
    case 'O': { /* print instance from Laddr. Not documented */
      LispPTR objaddr;
      if (URaid_argnum == 1) {
        printf("PRINT-INSTANCE: O HEX-LispAddress\n");
        return (T);
      }
      errno = 0;
      objaddr = (LispPTR)strtoul(URaid_arg1, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg not HEX number\n");
        return (T);
      }
      print(objaddr);
    } break;
    case 't': /* Object TYPE */
      if (URaid_argnum != 2) {
        printf("PRINTTYPENAME: t Xaddress\n");
        return (T);
      }

      /**HEXNUMP(URaid_arg1,"Not Address");**/
      errno = 0;
      address = (LispPTR)strtoul(URaid_arg1, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg not HEX number\n");
        return (T);
      }
      ADD_RANGEP(address);

      switch (address >> 16) {
        case ATOM_HI:
          printf("{ATOM}");
          printf("0x%x\n", address);
          break;
        case STK_HI:
          printf("{STK}");
          printf("0x%x\n", address);
          break;
        case PLIS_HI:
          printf("{PLIST}");
          printf("0x%x\n", address);
          break;
        case FPTOVP_HI:
          printf("{FPTOVP}");
          printf("0x%x\n", address);
          break;
        case PNP_HI:
          printf("{PNP}");
          printf("0x%x\n", address);
          break;
        case DEFS_HI:
          printf("{DEF}");
          printf("0x%x\n", address);
          break;
        case VALS_HI:
          printf("{VAL}");
          printf("0x%x\n", address);
          break;
        case DISPLAY_HI:
        case DISPLAY_HI + 1:
          printf("{DISPLAY}");
          printf("0x%x\n", address);
          break;
      default: {
          struct dtd *dtd = (struct dtd*)GetDTD(GetTypeNumber(address));
#ifdef BIGVM
          index = dtd->dtd_name;
#else
          index = (dtd->dtd_namehi << 16) + dtd->dtd_namelo;
#endif
          putchar('{');
          if (index != 0) {
              print_atomname(index);
          } else {
              printf("unknown");
          }
          printf("}\n");
          break;
      }
      } /* switch end */

      break;
#define GetPROPCELL68k(index) ((LispPTR *)Plistspace + (index))

    case 'p': /* property list */
      if (URaid_argnum != 2) {
        printf("GETPROPLIST : p litatom\n");
        return (T);
      }
      if ((index = parse_atomstring(URaid_arg1)) == 0xffffffff) {
        printf("No such atom\n");
        return (T);
      }
      print(*(GetPROPCELL68k(index)) & POINTERMASK);
      break;

    case 'w': /* Disp CurrentFunc name & PC */ doko(); break;

#define XDUMPW 8
    case 'x': /* HEX dump "x Xaddress Xnum" */
      /* Obsolete
      HEXNUMP(URaid_arg1,"Not Address");
      HEXNUMP(URaid_arg2,"Not number");
      ***/
      if (URaid_argnum == 1) { /* for help */
        printf("HEX-DUMP: x Xaddress [Xnum]\n");
        return (T);
      }
      errno = 0;
      address = (LispPTR)strtoul(URaid_arg1, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg(Xaddress) not Xaddress\n");
        return (T);
      }
      if (URaid_arg2[0] == '\0') {
        num = XDUMPW;
      } else {
        errno = 0;
        num = strtol(URaid_arg2, &endpointer, 16);
        if (errno != 0 || *endpointer != '\0') {
          printf("Arg(Xnum) not Xnum\n");
          return (T);
        }
      }
      if (num < 0) {
        printf("Dump words num should be positive\n");
        return (T);
      }
      /* Address range check */
      ADD_RANGEP(address);
      ADD_RANGEP(address + num);

      {
        int i;
        DLword *ptr, *endptr;
        ptr = (DLword *)NativeAligned2FromLAddr(address);
        endptr = ptr + num;

        while (ptr < endptr) {
          printf("0x%x : ", LAddrFromNative(ptr));

          for (i = 0; ((ptr < endptr) && (i < XDUMPW)); ptr++, i++) { printf("%4x ", *ptr); }
          putchar('\n');
        }
      }
      break;
    case '@': /* SETTOPVAL */
      if (URaid_argnum != 3) {
        printf("SETTOPVAL: @ litatom Decimal-val\n");
        return (T);
      }
      if ((index = parse_atomstring(URaid_arg1)) == 0xffffffff) {
        printf("No such atom\n");
        return (T);
      }
      if (strncmp(URaid_arg2, "NIL", 3) == 0) {
        val = NIL;
      } else if (*URaid_arg2 == 'T')
        val = ATOM_T;
      else {
        errno = 0;
        val = strtol(URaid_arg2, &endpointer, 10);
        if (errno != 0 || *endpointer != '\0') {
          printf(" Bad value\n");
          return (T);
        } else {
          if ((val < -65536) || (65535 < val)) {
            printf("Bad Val range\n");
            return (T);
          }
          if (val >= 0)
            val |= S_POSITIVE;
          else {
            val = val & 0xffff;
            val |= S_NEGATIVE;
          }
        }
      }
      printf("Old value is ");
      print(*((LispPTR *)GetVALCELL68k(index)));
      putchar('\n');
      *((LispPTR *)GetVALCELL68k(index)) = (LispPTR)val;
      print(val);
      break;

    case '<':
      if (URaid_argnum != 3) {
        printf("Mem modify: < Xaddre Xval\n");
        return (T);
      }
      /* Assignment */
      /* OBSOLETE
      HEXNUMP(URaid_arg1,"Not Address");
      HEXNUMP(URaid_arg2,"Not Proper Value");
      ***/

      errno = 0;
      address = (LispPTR)strtol(URaid_arg1, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg(Xaddress) not Xaddress\n");
        return (T);
      }
      errno = 0;
      val = strtol(URaid_arg2, &endpointer, 16);
      if (errno != 0 || *endpointer != '\0') {
        printf("Arg(Xval) not Xaddress\n");
        return (T);
      }

      ADD_RANGEP(address);
      if (val < 0 || val > 0xffff) {
        printf("Xval invalid (16 bit range exceeded).\n");
        return (T);
      }

      {
        DLword *ptr;
        ptr = (DLword *)NativeAligned2FromLAddr(address);
        *ptr = val;
        printf("0x%x : 0x%x\n", address, *ptr);
      }
      break;

    /****MISC ****/
    case 'q': /* return with do Nothing */
      printf("Return to Lisp?[confirm](Y or N)<");
      {
        int c;
        c = getchar();
        if ((c == 'Y') || (c == 'y')) {
          /*TopOfStack = NIL;*/
          return (NIL);
        }
      }
      fflush(stdin);
      URaid_currentFX = URMAXFXNUM + 1;
      return (T);
    /* break; */
    case 'h': /* HARDRESET */
      printf("HARDRESET?[confirm](Y or N)<");
      {
        int c;
        c = getchar();
        if ((c == 'Y') || (c == 'y')) {
          /*PC+= 3; for currentfx->pc ajust:MOve to subr */
          contextsw(ResetFXP, 2, 0);
          /*PC -= 3;  in subr.c it increments by 3 */
          fflush(stdin);
          return (NIL); /* return to dispatch */
        }
      }
      fflush(stdin);
      break;
    case 'e': /* exit to UNIX */
#ifdef DOS
      printf("Exit to DOS?[confirm](Y or N)<");
#else  /* DOS */
      printf("Exit to UNIX?[confirm](Y or N)<");
#endif /* DOS */
      {
        int c;
        c = getchar();
        if ((c == 'Y') || (c == 'y')) exit(0);
      }
      fflush(stdin);
      URaid_currentFX = URMAXFXNUM + 1;
      break;
    case 's': /* SHELL */
      switch (vfork()) {
        case -1: (void)(void)fprintf(stderr, "uraid: Fork failed.\n"); exit(1);

        case 0: (void)execl("/bin/sh", "sh", "-i", NULL); exit(1);

        default: break;
      }
#ifndef DOS
      (void)wait(&status);
/* system("/bin/sh -i"); */
#endif /* DOS */
      return (T);
    /* break; */
    case 'v':
      if (URaid_argnum != 2) {
        printf("VMEMSAVE: v filename (it's NOT bootable)\n");
        return (T);
      }
      if (vmem_save(URaid_arg1) != NIL) {
#ifndef DISPLAYBUFFER
        clear_display();
#endif /* DISPLAYBUFFER */

        printf("VMEMSAVE fails\n");
      } else {
        clear_display();
        printf("VMEMSAVE finished, but it's not bootable\n");
      }
      break;
    case '(':
      if (URaid_argnum == 1)
        num = 2;
      else {
        errno = 0;
        num = strtoul(URaid_arg1, &endpointer, 10);
        if (errno != 0 || *endpointer != '\0') { 
          printf("Illegal argument, not number\n");
          return (T);
        }
      }
      PrintMaxLevel = num;
      printf("PrintLevel is set to %d.", num);
      break;
    case '?':
#ifdef DOS
      printf(
          " 1: <<Displaying the Stack>>\n 2: <<Displaying memory contents>>\n 3: <<Return or "
          "Exit>>\n");
#else
      printf("%s\n", URaid_summary);
#endif /* DOS */
      break;
    case '!': printf("Error message is: %s\n", URaid_errmess); break;

    default: printf("Unsupported command.\n"); break;

  } /* switch end */
  return (T);
}

/************************************************************************/
/*                                                                      */
/*                      device_before_raid()                            */
/*                                                                      */
/*      Written by : Takeshi Shimizu                                    */
/*      Date:    6-May-1988                                             */
/*                                                                      */
/*                                                                      */
/*      Do whatever cleanup is necessary before leaving Lisp            */
/*      for URAID or for SUSPEND-LISP:  Cache the display buffer,       */
/*      close the frame buffer, and any buffering pixrects.             */
/*                                                                      */
/************************************************************************/

#ifdef DISPLAYBUFFER
extern struct pixrect *ColorDisplayPixrect, *DisplayRegionPixrect;
#endif


int device_before_raid(void) {
#ifdef XWINDOW
  sigset_t signals;
#endif

  int_block();

#ifdef XWINDOW
  /* So X events still get recognized. */
  sigemptyset(&signals);
#ifndef MAIKO_OS_HAIKU
  sigaddset(&signals, SIGIO);
#endif
  sigprocmask(SIG_UNBLOCK, &signals, NULL);
#endif


#ifdef MAIKO_ENABLE_ETHERNET
#ifdef ETHERINT
  if (ether_fd > 0) /* check ether is used or not */
    int_io_close(ether_fd);
#endif
#endif /* MAIKO_ENABLE_ETHERNET */

#ifdef RS232INT
  int_io_close(RS232C_Fd);
#endif


#if defined(XWINDOW) || defined(DOS)
  (currentdsp->cleardisplay)(currentdsp);
  (currentdsp->device.before_raid)(currentdsp);
#ifdef DOS
  (currentmouse->device.before_raid)(currentmouse);
  (currentkbd->device.before_raid)(currentkbd);
#endif /* DOS */
#endif /* XWINDOW || DOS */

  return (0);
}

/**
char *alloc_hideDISP(int size)
{
 char *retaddr;
 switch(*STORAGEFULLSTATE_word & 0xffff)
  {
        case SFS_NOTSWITCHABLE :
                if((retaddr =malloc(size)) ==0){
                        (void)fprintf(stderr,"can't alloc hide space\n");
                 }
                break;
        case 0 :
        case SFS_SWITCHABLE :
                retaddr =
                 (char *)NativeAligned2FromLAddr((*ArraySpace2_word) & POINTERMASK);
                printf("Hidespace inside Lisp(2)\n");

                break;
        case SFS_ARRAYSWITCHED :
                retaddr=(char *)NativeAligned2FromLAddr(*Next_Array_word & 0xffff);
;
                printf("Hidespace inside Lisp(3)\n");

                break;
        case SFS_FULLYSWITCHED :
                if((UNSIGNED)NativeAligned2FromLAddr(*Next_MDSpage_word & 0xffff)
                        - (UNSIGNED)NativeAligned2FromLAddr(*Next_Array_word & 0xffff)
                 >size) {
                        retaddr= (char *)NativeAligned2FromLAddr(*Next_Array_word & 0xffff);
                        printf("Hidespace inside Lisp(4)\n");
                 }
                else if((retaddr=malloc(size)) ==0){
                        (void)fprintf(stderr,"can't alloc hide disp\n");
                         }
                        printf("Hidespace new\n");
                break;

        default : printf("Illegal data in STORAGEFULLSTATE\n");
                  retaddr=0;
                 break;
  }
 return(retaddr);
}
****/

/***********************************************************************/
/*
        func name : device_after_raid()
        Written by : Takeshi Shimizu
        Date:   6-May-1988

        This should be called when returning LISP.

*/
/***********************************************************************/
/*
 * Seems like a bad return type or bad code, returning 0, -1, or NIL
 * NBriggs May 2017
 */
#define KB_ALLUP 0xffff

static int re_init_display(int, int);

int device_after_raid(void) {
  extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K;
  extern DLword *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K;
  FD_ZERO(&LispReadFds);
  if (re_init_display(DISPLAY_OFFSET, 65536 * 16 * 2) == -1) return (-1);
  set_cursor();
  init_keyboard(1);

#ifdef TRUECOLOR
  truecolor_after_raid();
#endif /* TRUECOLOR */

#ifdef MAIKO_ENABLE_ETHERNET
  init_ether();
#if defined(USE_DLPI)
  if (ether_fd > 0)
    if (ioctl(ether_fd, I_SETSIG, S_INPUT) != 0) {
      perror("after-uraid: I_SETSIG for ether failed:\n");
      close(ether_fd);
      ether_fd = -1;
      return (-1);
    }
#endif /* USE_DLPI */
#endif /* MAIKO_ENABLE_ETHERNET */

#ifdef XWINDOW
#ifdef I_SETSIG
  if (ioctl(ConnectionNumber(currentdsp->display_id), I_SETSIG, S_INPUT) < 0)
    perror("SETSIG on X fd failed");
#endif /* I_SETSIG */
#endif /* XWINDOW */

  int_init();


#ifdef MAIKO_ENABLE_ETHERNET
  if (ether_fd > 0) FD_SET(ether_fd, &LispReadFds);
#endif /* MAIKO_ENABLE_ETHERNET */

#ifdef XWINDOW
  (currentdsp->device.after_raid)(currentdsp);
  FD_SET(ConnectionNumber(currentdsp->display_id), &LispReadFds);
  flush_display_buffer();
#elif DOS
  (currentdsp->device.after_raid)(currentdsp);
  (currentmouse->device.after_raid)(currentmouse, currentdsp);
  (currentkbd->device.after_raid)(currentkbd);
  flush_display_buffer();
#endif /* XWINDOW | DOS */

  int_unblock();
  *EmKbdAd068K = *EmRealUtilin68K = *EmKbdAd168K = *EmKbdAd268K = *EmKbdAd368K = *EmKbdAd468K =
      *EmKbdAd568K = KB_ALLUP;
  return (0);
} /* device_after_raid */

/***********************************************************************/
/*
        func name :re_init_display(lisp_display_addr, display_max)
                        int lisp_display_addr, display_max;

        Written by : Takeshi Shimizu
        Date:   6-May-1988

        This should be called when returning LISP.
        Only called by device_after_raid

*/
/***********************************************************************/
#ifndef COLOR

static int re_init_display(int lisp_display_addr, int display_max)
{

  return (0);
}

#else /* COLOR */

static int re_init_display(int lisp_display_addr, int display_max)
{

  return (0);
}

#endif /* COLOR */
