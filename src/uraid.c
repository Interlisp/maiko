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

#include <fcntl.h>
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

#ifndef XWINDOW
#ifdef SUNDISPLAY
#include <sundev/kbd.h>
#include <sundev/kbio.h>
#endif /* SUNDISPLAY */
#include <errno.h>
#ifndef NOPIXRECT
#include <sunwindow/window_hs.h>
#include <sunwindow/win_ioctl.h>
#include <pixrect/pixrect_hs.h>
#include <sunwindow/win_cursor.h>
#include <sunwindow/cms.h>
#include <sys/mman.h>
#ifdef SUNDISPLAY
extern int Win_security_p;
#endif /* SUNDISPLAY */

#endif /* NOPIXRECT */
#endif /* XWINDOW */

#ifdef OS5
#include <stropts.h>
#endif /* OS5 */

#if defined(FREEBSD) || defined(MACOSX) || defined(OS5)
/* vfork is deprecated */
#define vfork fork
#endif

#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "emlglob.h"
#include "cell.h"
#include "ifpage.h"
#include "debug.h"
#include "devconf.h"

#include "display.h"
#include "bitblt.h"

#include "uraiddefs.h"
#include "dbgtooldefs.h"
#include "gcarraydefs.h"
#include "initdspdefs.h"
#include "initkbddefs.h"
#include "kprintdefs.h"
#include "llstkdefs.h"
#include "mkatomdefs.h"
#include "returndefs.h"
#include "testtooldefs.h"
#include "timerdefs.h"
#include "vmemsavedefs.h"

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
#ifndef NOPIXRECT
extern int black, white;
extern struct pixrect *ColorDisplayPixrect, *DisplayRegionPixrect;
#endif /* NOPIXRECT */

extern int DisplayRasterWidth;
extern unsigned int LispWindowFd, LispKbdFd;
extern fd_set LispReadFds;
#ifndef NOPIXRECT
extern struct pixrect *CursorBitMap, *InvisibleCursorBitMap;
#endif /* NOPIXRECT */
extern struct cursor CurrentCursor, InvisibleCursor;
extern struct screen LispScreen;
extern int displaywidth, displayheight;
extern DLword *DisplayRegion68k;
extern int FrameBufferFd, ether_fd, RS232C_Fd;

LispPTR RadiAtomIndex;
LispPTR RaidPackageIndex;
DLword *HideDisp68k;

#ifdef COLOR
DLword *HideColorDisp68k;
extern int Inited_Color;
#endif /* COLOR */

#ifdef DOS
char *URaid_summary1 =
    "\n-- Stack display commands\n\
c\t\t\tChecks all user stack contents\n\
f number\t\tDisplays stack frame for that frame number (decimal)\n\
k type\t\t\tChanges the type of stack link following. (a|c)\n\
l [type]\t\tBack Trace for specified type stack. (k|m|r|g|p|u|<null>)\n\
<CR>\t\t\tDisplay next frame.\n";

char *URaid_summary2 =
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

char *URaid_summary3 =
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
char *URaid_summary =
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

#define ADD_RANGEP(address)                       \
  if ((address < 0) || (POINTERMASK < address)) { \
    printf("Address out of range.\n");            \
    return (T);                                   \
  }

#define URMAXCOMM 512
#define URMAXFXNUM 2000
#define URSCAN_ALINK 0
#define URSCAN_CLINK 1

/*** URaid G vals ***/
int URaid_scanlink = URSCAN_ALINK;
int URaid_currentFX = URMAXFXNUM + 1;
FX *URaid_FXarray[URMAXFXNUM];
int URaid_ArrMAXIndex;

char URaid_inputstring[URMAXCOMM];
char URaid_comm;
char URaid_arg1[URMAXCOMM / 2];
char URaid_arg2[URMAXCOMM / 2];
int URaid_argnum;
char *URaid_errmess;

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
  int flag = 0;
  int packagelen = 0;
  int namelen = 0;
  int cnt;
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
    aindex = make_atom(nameptr, 0, namelen, T);
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

void uraid_commclear() {
  memset(URaid_inputstring, 0, URMAXCOMM);
  memset(URaid_arg1, 0, URMAXCOMM / 2);
  memset(URaid_arg2, 0, URMAXCOMM / 2);

  URaid_comm = 0;
  URaid_argnum = 0;
}

void copy_region(DLword *src, DLword *dst, int width, int h)
{
  register int w;

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

LispPTR uraid_commands() {
  int num, address, val, tmp;
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
  switch (URaid_comm) {
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
      if (sscanf(URaid_arg1, "%d", &num) <= 0) { /* com read fails */
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
          bt1(Addr68k_from_StkOffset(InterfacePage->kbdfxp));
          break;
        case 'm':
          printf("IFP->MISCFXP :\n");
          bt1(Addr68k_from_StkOffset(InterfacePage->miscfxp));
          break;
        case 'r':
          printf("IFP->RESETFXP :\n");
          bt1(Addr68k_from_StkOffset(InterfacePage->resetfxp));
          break;
        case 'g':
          printf("IFP->GCFXP :\n");
          bt1(Addr68k_from_StkOffset(InterfacePage->gcfxp));

          break;
        case 'p':
          printf("IFP->FAULTFXP :\n");
          bt1(Addr68k_from_StkOffset(InterfacePage->faultfxp));

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
      break;

    case 'M': /* Machine States */
      printf("TOS   : 0x%x\n", TopOfStack);
      printf("CSTKP : 0x%x\n", LADDR_from_68k(CurrentStackPTR));
      printf("PVAR  : 0x%x\n", LADDR_from_68k(PVar));
      printf("IVAR  : 0x%x\n", LADDR_from_68k(IVar));
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
      int objaddr;
      if (URaid_argnum == 1) {
        printf("PRINT-INSTANCE: O HEX-LispAddress\n");
        return (T);
      }
      if (sscanf(URaid_arg1, "%x", &objaddr) <= 0) {
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
      if (sscanf(URaid_arg1, "%x", &address) <= 0) {
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
          struct dtd *dtd = GetDTD(GetTypeNumber(address));
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
      if (sscanf(URaid_arg1, "%x", &address) <= 0) { /* arg1 not HEX */
        printf("Arg(Xaddress) not Xaddress\n");
        return (T);
      }
      switch (sscanf(URaid_arg2, "%x", &num)) {
        case -1: /* Use defaultval for word-num */ num = XDUMPW; break;
        case 0: /* Illegal number */
          printf("Arg(Xnum) not Xnum\n");
          return (T);
        /* break; */
        default: break;
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
        ptr = (DLword *)Addr68k_from_LADDR(address);
        endptr = ptr + num;

        while (ptr < endptr) {
          printf("0x%x : ", LADDR_from_68k(ptr));

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
        if (sscanf(URaid_arg2, "%d", &val) == -1) {
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

      if (sscanf(URaid_arg1, "%x", &address) <= 0) {
        printf("Arg(Xaddress) not Xaddress\n");
        return (T);
      }
      if (sscanf(URaid_arg2, "%x", &val) <= 0) {
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
        ptr = (DLword *)Addr68k_from_LADDR(address);
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
        case -1: (void)fprintf(stderr, "uraid: Fork failed.\n"); exit(1);

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
#ifndef DISPLAYBUFFER
      copy_region(HideDisp68k, DisplayRegion68k, DisplayRasterWidth, displayheight);
#endif /* DISPLAYBUFFER */

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

      else if ((URaid_arg1[0] < '0') || (URaid_arg1[0] > '9')) {
        printf("Illegal argument, not number\n");
        return (T);
      } else
        sscanf(URaid_arg1, "%d", &num);

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

#ifdef SUNDISPLAY
#define SV_ACQUIRE "/bin/sunview1/sv_acquire"
#define SV_RELEASE "/bin/sunview1/sv_release"
#endif /* SUNDISPLAY */

int device_before_raid() {
  int keytrans;
  int size;
  int munmapstat;
  struct pixrect *fb;
  /*  extern char *alloc_hideDISP(); */
#ifdef SUNDISPLAY
  union wait status;
#endif /* SUNDISPLAY */

  int_timer_off();

#if (defined(XWINDOW) && defined(SYSVSIGNALS) && defined(SIGIO))
  sigrelse(SIGIO); /* So X events still get recognized. */
#endif             /* XWINDOW + SYSVSIGNALS + SIGIO */

#ifdef SUNDISPLAY
  win_setcursor(LispWindowFd, &InvisibleCursor);
#ifdef KBINT
  int_io_close(LispWindowFd);
#endif
#endif /* SUNDISPLAY */

#ifdef NOETHER
#else
#ifdef ETHERINT
  if (ether_fd > 0) /* check ether is used or not */
    int_io_close(ether_fd);
#endif
#endif /* NOETHER */

#ifdef RS232INT
  int_io_close(RS232C_Fd);
#endif

#ifdef SUNDISPLAY
  mess_reset(); /* turn off console-msg handling */

#ifdef FX_AR_124
  /* For AR 124. Type4 driver bug?? by m.matsuda */
  {
    long i;
    for (i = 0; i < 900000; i++)
      ;
  }
#endif /*  FX_AR_124 */

  if ((LispKbdFd = open(LispScreen.scr_kbdname, O_RDWR)) == -1) {
    fprintf(stderr, "can't open %s\n", LispScreen.scr_kbdname);
    return (-1);
  }

  keytrans = TR_EVENT; /* keyboard encodes key */
  if (ioctl(LispKbdFd, KIOCTRANS, &keytrans) == -1) {
    fprintf(stderr, "Error at ioctl errno =%d\n", errno);
    return (-1);
  }
  close(LispKbdFd);
  close(LispWindowFd);

#ifdef TRUECOLOR
  truecolor_before_raid();
#endif /* TRUECOLOR */

#ifndef DISPLAYBUFFER
  size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

  if ((HideDisp68k = malloc(size)) == 0) {
    printf("can't malloc hide space\n");
    return (-1);
  }

  copy_region(DisplayRegion68k, HideDisp68k, DisplayRasterWidth, displayheight);
#endif /* DISPLAYBUFFER */

#ifdef COLOR
  save_colormap();

#ifndef DISPLAYBUFFER
  if (Inited_Color) {
#else
  if (MonoOrColor == COLOR_SCREEN) {
#endif /* DISPLAYBUFFER */

    /* save color image */
    size =
        ((displaywidth * displayheight + (getpagesize() - 1)) & -getpagesize()); /* 8 bit depth */
    if ((HideColorDisp68k = malloc(size)) == 0) {
      printf("can't malloc hide space\n");
      return (-1);
    }
    copy_region(ColorDisplayRegion68k, HideColorDisp68k, DisplayRasterWidth * 8, displayheight);
  }    /* end if(MonoOrColor) */
#endif /* COLOR */

  clear_display();

#ifdef DISPLAYBUFFER
  pr_close(ColorDisplayPixrect);
  close(FrameBufferFd);
#endif

  if (Win_security_p) {
    switch (vfork()) {
      case -1: /* Error */ (void)fprintf(stderr, "display_before_exit: Fork failed.\n"); exit(1);

      case 0: /* Child */
        (void)execl(SV_RELEASE, "sv_release", NULL);
        /* should not return */
        (void)fprintf(stderr, "display_before_exit: exec for sv_release failed\n");
        exit(1);

      default: /* Parent */
        /* do nothing */
        break;
    }
    (void)wait(&status); /* child dies after changing 16 */

    if (status.w_retcode != 0) {
      (void)fprintf(stderr, "device_before_raid: failed to set ownership of win devices\n");
      exit(1);
    }
  }

#endif /* SUNDISPLAY */

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
                        fprintf(stderr,"can't alloc hide space\n");
                 }
                break;
        case 0 :
        case SFS_SWITCHABLE :
                retaddr =
                 (char*)Addr68k_from_LADDR((*ArraySpace2_word) & POINTERMASK);
                printf("Hidespace inside Lisp(2)\n");

                break;
        case SFS_ARRAYSWITCHED :
                retaddr=(char*)Addr68k_from_LADDR(*Next_Array_word & 0xffff);
;
                printf("Hidespace inside Lisp(3)\n");

                break;
        case SFS_FULLYSWITCHED :
                if((UNSIGNED)Addr68k_from_LADDR(*Next_MDSpage_word & 0xffff)
                        - (UNSIGNED)Addr68k_from_LADDR(*Next_Array_word & 0xffff)
                 >size) {
                        retaddr= (char*)Addr68k_from_LADDR(*Next_Array_word & 0xffff);
                        printf("Hidespace inside Lisp(4)\n");
                 }
                else if((retaddr=malloc(size)) ==0){
                        fprintf(stderr,"can't alloc hide disp\n");
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

int device_after_raid() {
  extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K;
  extern DLword *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K;
  FD_ZERO(&LispReadFds);
  if (re_init_display(DISPLAY_OFFSET, 65536 * 16 * 2) == -1) return (-1);
  set_cursor();
  init_keyboard(1);

#ifdef TRUECOLOR
  truecolor_after_raid();
#endif /* TRUECOLOR */

#ifdef NOETHER
#else
  init_ether();
#ifdef USE_DLPI
  if (ether_fd > 0)
    if (ioctl(ether_fd, I_SETSIG, S_INPUT) != 0) {
      perror("after-uraid: I_SETSIG for ether failed:\n");
      close(ether_fd);
      ether_fd = -1;
      return (-1);
    }
#endif /* USE_DLPI */
#endif /* NOETHER */

#ifdef XWINDOW
#ifdef I_SETSIG
  if (ioctl(ConnectionNumber(currentdsp->display_id), I_SETSIG, S_INPUT) < 0)
    perror("SETSIG on X fd failed");
#endif /* I_SETSIG */
#endif /* XWINDOW */

  int_init();

#ifdef SUNDISPLAY
  FD_SET(LispWindowFd, &LispReadFds);
#endif /* SUNDISPLAY */

#ifdef NOETHER
#else
  FD_SET(ether_fd, &LispReadFds);
#endif /* NOETHER */

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

  int_timer_on();
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
  int mmapstat, size;
  struct pixrect *ColorFb;
  extern struct pixrect *ColorDisplayPixrect, *DisplayRegionPixrect;
  extern int DisplayType;

#ifdef SUNDISPLAY

  union wait status;

  if (Win_security_p) {
    switch (vfork()) {
      case -1: /* Error */ (void)fprintf(stderr, "re_init_display: Fork failed.\n"); exit(1);

      case 0: /* Child */
        (void)execl(SV_ACQUIRE, "sv_acquire", "0", "256", "250", NULL);
        /* should not return */
        (void)fprintf(stderr, "re_init_display: exec for sv_acquire failed\n");
        exit(1);

      default: /* Parent */
        /* do nothing */
        break;
    }
    (void)wait(&status); /* child dies after changing 6 */

    if (status.w_retcode != 0) {
      (void)fprintf(stderr, "re_init_display: failed to set ownership of win devices\n");
      exit(1);
    }
  }

  mess_init();
  if ((LispWindowFd = win_screennew(&LispScreen)) == -1) {
    fprintf(stderr, "init_display: can't create LispWindow\n");
    return (-1);
  } else {
#ifdef KBINT
    int_io_open(LispWindowFd);
    fcntl(LispWindowFd, F_SETFL, fcntl(LispWindowFd, F_GETFL, 0) | O_NONBLOCK);

#ifdef FX_AR_124
    /* For AR 124. Type4 driver bug?? by m.matsuda */
    {
      long i;
      for (i = 0; i < 900000; i++)
        ;
    }
#endif /* FX_AR_124 */

#endif
  }

#ifndef DISPLAYBUFFER
  /* for CGFOUR dev */
  if (DisplayType == SUN4COLOR) {
    ColorFb = pr_open("/dev/fb");
    pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
    pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
    pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
  }
#else  /* DISPLAYBUFFER is T */
/*  ColorDisplayPixrect = pr_open("/dev/fb");
    pr_putcolormap(ColorDisplayPixrect, 1, 1, &black, &black, &black);
    pr_putcolormap(ColorDisplayPixrect, 0, 1, &white, &white, &white);
    pr_putcolormap(ColorDisplayPixrect, 255, 1, &black, &black, &black);
    pr_putcolormap(ColorDisplayPixrect,
                   (1<<ColorDisplayPixrect->pr_depth)-1, 1,
                   &black, &black, &black);
***/
#endif /* DISPLAYBUFFER */

  init_cursor();

#ifndef DISPLAYBUFFER
  size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());
  mmapstat = (int)mmap(DisplayRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                       MAP_FIXED |
#endif /* OS4 */

                           MAP_SHARED,
                       FrameBufferFd, 0);

  if (mmapstat == -1) {
    fprintf(stderr, "re_init_display: ERROR at mmap system call\n");
    fprintf(stderr, "errno = %d\n\n", errno);
    return (0);
  }

  copy_region(HideDisp68k, DisplayRegion68k, DisplayRasterWidth, displayheight);

  free(HideDisp68k);
#endif /* DISPLAYBUFFER */

#ifdef DISPLAYBUFFER
  ColorDisplayPixrect = pr_open("/dev/fb");
  flush_display_buffer();
/*     refresh_CG6; */

#endif /* DISPLAYBUFFER */

#endif /* SUNDISPLAY */

  return (0);
}

#else /* COLOR */

static int re_init_display(int lisp_display_addr, int display_max)
{
  int mmapstat, size;
  struct pixrect *ColorFb;
  /*
      extern struct pixrect *color_source;
  */
  struct pixrect *color_source;
  extern int DisplayType;

#ifdef SUNDISPLAY
  union wait status;

  if (Win_security_p) {
    switch (vfork()) {
      case -1: /* Error */ (void)fprintf(stderr, "re_init_display: Fork failed.\n"); exit(1);

      case 0: /* Child */
        (void)execl(SV_ACQUIRE, "sv_acquire", "0", "256", "250", NULL);
        /* should not return */
        (void)fprintf(stderr, "re_init_display: exec for sv_acquire failed\n");
        exit(1);

      default: /* Parent */
        /* do nothing */
        break;
    }
    (void)wait(&status); /* child dies after changing 6 */

    if (status.w_retcode != 0) {
      (void)fprintf(stderr, "re_init_display: failed to set ownership of win devices\n");
      exit(1);
    }
  }

  mess_init();
  if ((LispWindowFd = win_screennew(&LispScreen)) == -1) {
    fprintf(stderr, "init_display: can't create LispWindow\n");
    return (-1);
  } else {
#ifdef KBINT
    int_io_open(LispWindowFd);
    fcntl(LispWindowFd, F_SETFL, fcntl(LispWindowFd, F_GETFL, 0) | O_NONBLOCK);
#endif
  }

#ifdef DISPLAYBUFFER
  if ((FrameBufferFd = open(LispScreen.scr_fbname, 2)) == -1) {
    perror("init_display: can't open FrameBuffer\n");
    exit(-1);
  }
#endif /* DISPLAYBUFFER */

  restore_colormap();

  if (MonoOrColor == MONO_SCREEN) {
#ifndef DISPLAYBUFFER
    /* for CGFOUR dev */
    if (DisplayType == SUN4COLOR) {
      ColorFb = pr_open("/dev/fb");
      pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
      pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_SET, 0, 0, 0);
      pr_set_plane_group(ColorFb, PIXPG_OVERLAY);
    }
#else  /* DISPLAYBUFFER is T */
/*  ColorDisplayPixrect = pr_open("/dev/fb");
    pr_putcolormap(ColorDisplayPixrect, 1, 1, &black, &black, &black);
    pr_putcolormap(ColorDisplayPixrect, 0, 1, &white, &white, &white);
    pr_putcolormap(ColorDisplayPixrect, 255, 1, &black, &black, &black);
    pr_putcolormap(ColorDisplayPixrect,
                   (1<<ColorDisplayPixrect->pr_depth)-1, 1,
                   &black, &black, &black);
***/
#endif /* DISPLAYBUFFER */

    init_cursor();

#ifndef DISPLAYBUFFER
    size = ((displaywidth * displayheight / 8 + (getpagesize() - 1)) & -getpagesize());

    mmapstat = (int)mmap(DisplayRegion68k, size, PROT_READ | PROT_WRITE,
#ifdef OS4
                         MAP_FIXED |
#endif /* OS4 */

                             MAP_SHARED,
                         FrameBufferFd, 0);
    if (Inited_Color)
      mmapstat = (int)mmap(ColorDisplayRegion68k, Dispcolorsize, PROT_READ | PROT_WRITE,
#ifdef OS4
                           MAP_FIXED |
#endif /* OS4 */

                               MAP_SHARED,
                           FrameBufferFd, 0x40000);

    if (mmapstat == -1) {
      fprintf(stderr, "re_init_display: ERROR at mmap system call\n");
      fprintf(stderr, "errno = %d\n\n", errno);
      return (0);
    }
    /* restore mono image */
    copy_region(HideDisp68k, DisplayRegion68k, DisplayRasterWidth, displayheight);

    free(HideDisp68k);

    if (Inited_Color) { /* restore color image */
      copy_region(HideColorDisp68k, ColorDisplayRegion68k, DisplayRasterWidth * 8, displayheight);
      free(HideColorDisp68k);
    } /* end if( Inited_Color ) */
#endif /* DISPLAYBUFFER */

#ifdef DISPLAYBUFFER
    ColorDisplayPixrect = pr_open("/dev/fb");
    flush_display_buffer();
/*     refresh_CG6; */
#endif /* DISPLAYBUFFER */

  } else { /* MonoOrColor is COLOR_SCREEN */
    ColorFb = pr_open("/dev/fb");
#ifdef DISPLAYBUFFER
    ColorDisplayPixrect = ColorFb;
#endif /* DISPLAYBUFFER */

    color_source = mem_point(displaywidth, displayheight, 8, ColorDisplayRegion68k);
    pr_rop(ColorFb, 0, 0, displaywidth, displayheight, PIX_SRC, color_source, 0, 0);
#ifndef DISPLAYBUFFER
    pr_set_plane_group(ColorFb, PIXPG_OVERLAY_ENABLE);
    pr_rop(ColorFb, 0, 0, ColorFb->pr_width, ColorFb->pr_height, PIX_CLR, 0, 0, 0);
#endif /* DISPLAYBUFFER */

    pr_set_plane_group(ColorFb, PIXPG_8BIT_COLOR);
    init_cursor();
    mmapstat = (int)mmap(ColorDisplayRegion68k, Dispcolorsize, PROT_READ | PROT_WRITE,
#ifdef OS4
                         MAP_FIXED |
#endif
                             MAP_SHARED,
                         FrameBufferFd, 0x40000);
    if (mmapstat == -1) {
      perror("cgfour_init_color_display: ERROR at mmap system call\n");
      error(
          "cgfour_init_color_display: ERROR at mmap system call\n You may be able to continue by "
          "typing 'q'");
      /*                  printf("MMAP FAIL:BMBASE=0x%x\nNATIVE:= 0x%x\nLISPBASEN:= 0x%x\n",
                              color_bitmapbase,ColorDisplayRegion68k,Lisp_world);
      */
      return (NIL);
    } /* end if(mmapstat) */

#ifndef DISPLAYBUFFER
    /* restore mono image */
    copy_region(HideDisp68k, DisplayRegion68k, DisplayRasterWidth, displayheight);

    free(HideDisp68k);
#endif /* DISPLAYBUFFER */

    /* restore coloe image */
    copy_region(HideColorDisp68k, ColorDisplayRegion68k, DisplayRasterWidth * 8, displayheight);

    free(HideColorDisp68k);
  } /* end if(MonoOrColor) */
#endif /* SUNDISPLAY */

  return (0);
}

#endif /* COLOR */
