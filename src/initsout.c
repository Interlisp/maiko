/* $Id: initsout.c,v 1.3 1999/05/31 23:35:34 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
/************************************************************************/
/*									*/
/*		Make connections between lisp and C emulator		*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#include "version.h"

#ifndef DOS
#include <pwd.h>           // for getpwuid, passwd
#endif
#include <stdio.h>         // for fprintf, NULL, stderr
#include <stdlib.h>        // for malloc, exit
#include <string.h>        // for strlen, strncpy
#include <time.h>          // for time_t
#include <unistd.h>        // for gethostid, getuid
#include "adr68k.h"        // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#ifdef BYTESWAP
#include "byteswapdefs.h"   // for word_swap_page
#endif
#include "cell.h"          // for GetVALCELL68k
#include "dbprint.h"       // for DBPRINT
#include "etherdefs.h"     // for init_ifpage_ether
#include "gcarraydefs.h"   // for get_package_atom
#include "gcdata.h"        // for ADDREF, GCLOOKUP
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "ifpage.h"        // for IFPAGE, MACHINETYPE_MAIKO
#include "initsoutdefs.h"  // for build_lisp_map, fixp_value, init_for_bitblt
#include "iopage.h"        // for IOPAGE
#include "lispemul.h"      // for LispPTR, DLword, NIL, BYTESPER_DLWORD, POINTERMASK
#include "lispmap.h"       // for ATMHT_OFFSET, ATOMS_OFFSET, DEFS_OFFSET
#include "lspglob.h"       // for InterfacePage, IOPage, AtomHT, Closure_Cac...
#include "lsptypes.h"      // for GetDTD, TYPE_FIXP, TYPE_LISTP
#include "miscstat.h"      // for MISCSTATS
#include "mkcelldefs.h"    // for N_OP_createcell
#include "testtooldefs.h"  // for MakeAtom68k, MAKEATOM

#if defined(MAIKO_ENABLE_ETHERNET) || defined(MAIKO_ENABLE_NETHUB)
#include "etherdefs.h"
#endif

/********** definitions for bitblt. add by osamu **********/
DLword TEXTURE_atom;
DLword MERGE_atom;
DLword INPUT_atom;
DLword INVERT_atom;
DLword ERASE_atom;
DLword PAINT_atom;
DLword REPLACE_atom;
/************ end definitions for bitblt.*****************/

#ifdef BIGVM
#define GCENTRY LispPTR
#else
#define GCENTRY DLword
#endif

/************************************************************************/
/*									*/
/*	f i x p _ v a l u e			*/
/*									*/
/*	Set up a system value to be a FIXP cell.					*/
/*									*/
/************************************************************************/

#ifdef BIGVM
LispPTR *fixp_value(LispPTR *ptr) {
  LispPTR val = *ptr;

  if (val < (S_NEGATIVE | 0xFFFF)) {
    LispPTR newval = N_OP_createcell(S_POSITIVE | TYPE_FIXP);
    LispPTR *newcell;
    newcell = (LispPTR *)NativeAligned4FromLAddr(newval);
    *newcell = val & 0xFFFF; /* it was smallp, so fill in */
    *ptr = newval;
    GCLOOKUP(newval, ADDREF); /* so it has a refcount */
    return ((LispPTR *)newcell);
  }
  return ((LispPTR *)NativeAligned4FromLAddr(val));
}

#else
/* In old system, this is just a NO-OP */
#define fixp_value
#endif /* BIGVM */

/************************************************************************/
/*									*/
/*			   i n i t _ i f p a g e			*/
/*									*/
/*	Set up the interface page:  Fill in the machine-type, the	*/
/*	ethernet-ID, the virtual-memory limit (32Mb or less, depending	*/
/*	on what the -m specified), make space for display type.		*/
/*									*/
/*									*/
/************************************************************************/

#define PAGES_IN_MBYTE 2048

void init_ifpage(unsigned sysout_size) {
  extern const time_t MDate;
  extern int DisplayType;
  extern int Storage_expanded;
  unsigned new_lastvmem;
  /*
    Initialize IFPAGE
   */
  InterfacePage->machinetype = MACHINETYPE_MAIKO;
#if defined(MAIKO_ENABLE_ETHERNET) || defined(MAIKO_ENABLE_NETHUB)
  init_ifpage_ether(); /* store ethernet ID in IF page */
#endif /* MAIKO_ENABLE_ETHERNET or MAIKO_ENABLE_NETHUB */
  /*InterfacePage->dl24bitaddressable = (sysout_size == 32? 0xffff : 0);*/
  InterfacePage->dl24bitaddressable = (sysout_size == 8 ? 0 : 0xffff);
  new_lastvmem = (sysout_size * PAGES_IN_MBYTE) - 1;

  if ((!Storage_expanded) && (InterfacePage->dllastvmempage != new_lastvmem)) {
    (void)fprintf(stderr, "You can't expand VMEM\n");
    exit(-1);
  } else { /* Set value which will be set to \\LASTVMEMFILEPAGE in LISP */
    InterfacePage->dllastvmempage = new_lastvmem;
    /* Also you can expand lisp space even if \\STOAGEFULL was T */
    *STORAGEFULL_word = NIL;
  }
  /* Set current process size */
  InterfacePage->process_size = sysout_size;

#ifdef BIGVM
  /* For BIGVM system, save the value in \LASTVMEMFILEPAGE for lisp's use */
  *LASTVMEMFILEPAGE_word = InterfacePage->dllastvmempage;
#endif /* BIGVM */

  /* unfortunately, Lisp only looks at a 16 bit serial number */
#if !defined(DOS) && !defined(MAIKO_OS_HAIKU)
  InterfacePage->serialnumber = 0xffff & gethostid();
#endif /* DOS MAIKO_OS_HAIKU */

/* get user name and stuff into vmem; this is the VMEM buffer;
This is a BCPL string -- it starts with a length count. C strings
are null terminated instead */
  InterfacePage->usernameaddr = 0;
#ifndef DOS
  {
    struct passwd *pwd;
    char *s;
    int len;
    /* Get username from getpwuid */
    /* The page/offset we are using is hardcoded in LLFAULT in functions */
    /*  \MAIKO.NEWFAULTINIT and \MAIKO.ASSIGNBUFFERS */
    if ((pwd = getpwuid(getuid())) != NULL) {
      InterfacePage->usernameaddr = 0155001;
      s = (char *)NativeAligned2FromLAddr(InterfacePage->usernameaddr);
      len = (int)strlen(pwd->pw_name);
      /* Lisp reserves 32 words for the BCPL String */
      len = (len < 32 * BYTESPER_DLWORD) ? len : 32 * BYTESPER_DLWORD - 1;
      *s = (char)len;
      strncpy(s + 1, pwd->pw_name, len);
#ifdef BYTESWAP
      /* we must swap the area we have written into, starting at 0155000 */
      /* rounding up to 4-byte words					 */
      word_swap_page(NativeAligned2FromLAddr(0155000), (len + 1 + 2 + 3) / 4);
#endif
    }

  }
#endif /* DOS */

  /* Days from Oct-13-87 12:00  It's Takeshi's birthday. */
  /* MDate may be set by vdate.c, generated by mkvdate.c. */
  InterfacePage->rversion = (MDate - 561150000) / (60 * 60 * 24);

  /* For DisplayType ,I couldn't insert this line into init_display */
  InterfacePage->devconfig &= 0xff87;
  InterfacePage->devconfig |= DisplayType;
}

/************************************************************************/
/*									*/
/*			    i n i t _ i o p a g e			*/
/*									*/
/*	Clean up the IO page:  Set the keyboard map to "all keys up."	*/
/*									*/
/************************************************************************/

void init_iopage(void) {
  /*
   * Initialize IOPAGE
   */
  IOPage->dlkbdad0 = 65535; /* ALL UP */
  IOPage->dlkbdad1 = 65535; /* ALL UP */
  IOPage->dlkbdad2 = 65535; /* ALL UP */
  IOPage->dlkbdad3 = 65535; /* ALL UP */
  IOPage->dlkbdad4 = 65535; /* ALL UP */
  IOPage->dlkbdad5 = 65535; /* ALL UP */
  IOPage->dlutilin = 65535; /* ALL UP */
}

/************************************************************************/
/*									*/
/*			b u i l d _ l i s p _ m a p			*/
/*									*/
/*	Create the atom-pointers used by C to deal with the lisp	*/
/*	SYSOUT.								*/
/*									*/
/************************************************************************/

extern int for_makeinit;

void build_lisp_map(void) {
  DLword index;

  Stackspace = (DLword *)NativeAligned2FromLAddr(STK_OFFSET);
  Plistspace = (DLword *)NativeAligned2FromLAddr(PLIS_OFFSET);
  DTDspace = (DLword *)NativeAligned2FromLAddr(DTD_OFFSET);
  MDStypetbl = (DLword *)NativeAligned2FromLAddr(MDS_OFFSET);
  AtomHT = (DLword *)NativeAligned2FromLAddr(ATMHT_OFFSET);
  Pnamespace = (DLword *)NativeAligned2FromLAddr(PNP_OFFSET);
  AtomSpace = (DLword *)NativeAligned2FromLAddr(ATOMS_OFFSET);
  Defspace = (DLword *)NativeAligned2FromLAddr(DEFS_OFFSET);
  Valspace = (DLword *)NativeAligned2FromLAddr(VALS_OFFSET);

  DBPRINT(("Stackspace = %p.\n", (void *)Stackspace));
  DBPRINT(("AtomHT = %p.\n", (void *)AtomHT));

  ListpDTD = (struct dtd *)GetDTD(TYPE_LISTP);

#ifdef BIGVM
  FPtoVP = (LispPTR *)NativeAligned4FromLAddr(FPTOVP_OFFSET);
#else
  FPtoVP = (DLword *)NativeAligned2FromLAddr(FPTOVP_OFFSET);
#endif /* BIGVM */
  IOPage = (IOPAGE *)NativeAligned4FromLAddr(IOPAGE_OFFSET);
  InterfacePage = (IFPAGE *)NativeAligned4FromLAddr(IFPAGE_OFFSET);
  MiscStats = (MISCSTATS *)NativeAligned4FromLAddr(MISCSTATS_OFFSET);

  UFNTable = (DLword *)NativeAligned2FromLAddr(UFNTBL_OFFSET);
  DisplayRegion = (DLword *)NativeAligned2FromLAddr(DISPLAY_OFFSET);

  HTmain = (GCENTRY *)NativeAligned4FromLAddr(HTMAIN_OFFSET);
  HToverflow = (GCENTRY *)NativeAligned4FromLAddr(HTOVERFLOW_OFFSET);
  HTbigcount = (GCENTRY *)NativeAligned4FromLAddr(HTBIG_OFFSET);
  HTcoll = (GCENTRY *)NativeAligned4FromLAddr(HTCOLL_OFFSET);

  /**** cache values *****/
  Reclaim_cnt_word = MakeAtom68k("\\RECLAIM.COUNTDOWN");

  /*** cache values for gcreclaimer : added by T. Teruuchi 30-Sep-1987 ***/

  GcDisabled_word = MakeAtom68k("\\GCDISABLED");

  /*** following cache values are the solution for array reclaimer ***/

  FreeBlockBuckets_word = MakeAtom68k("\\FREEBLOCKBUCKETS");
  Array_Block_Checking_word = MakeAtom68k("ARRAYBLOCKCHECKING");
  ArrayMerging_word = MakeAtom68k("\\ARRAYMERGING");
  ArraySpace_word = MakeAtom68k("\\ARRAYSPACE");
  ArraySpace2_word = MakeAtom68k("\\ARRAYSPACE2");
  ArrayFrLst_word = MakeAtom68k("\\ArrayFrLst");
  ArrayFrLst2_word = MakeAtom68k("\\ArrayFrLst2");
  Hunk_word = MakeAtom68k("\\HUNKING?");
  System_Buffer_List_word = MakeAtom68k("SYSTEMBUFFERLIST");

  /*** The following cache values are for the top level reclaimer ***/

  GcMess_word = MakeAtom68k("GCMESS");
  ReclaimMin_word = MakeAtom68k("\\RECLAIMMIN");

  /*** The following caches are for Symbol lookup April-28,1988 Tomtom ***/

  Package_from_Index_word = MakeAtom68k("*PACKAGE-FROM-INDEX*");
  Package_from_Name_word = MakeAtom68k("*PACKAGE-FROM-NAME*");
  Keyword_Package_word = MakeAtom68k("*KEYWORD-PACKAGE*");
  DBPRINT(("Package_from_Index_word = %p\n", (void *)Package_from_Index_word));
  DBPRINT(("Package_from_Name_word  = %p\n", (void *)Package_from_Name_word));

  /*** The following atom-index cache is for CL:VALUES opcode JDS 4/5/89 ***/

  MVLIST_index = MAKEATOM("\\MVLIST");

  /* * * Atoms for closure-cache interface * * */

  if (for_makeinit) {
    Closure_Cache_Enabled_word = (LispPTR *)malloc(4);
    *Closure_Cache_Enabled_word = NIL;

    Closure_Cache_word = (LispPTR *)malloc(4);
    *Closure_Cache_word = NIL;

    Deleted_Implicit_Hash_Slot_word = (LispPTR *)malloc(4);
    *Deleted_Implicit_Hash_Slot_word = NIL;
  } else {
    DBPRINT(("%p %p %p %p %p", (void *)Stackspace, (void *)Plistspace, (void *)DTDspace, (void *)MDStypetbl, (void *)AtomHT));

    index = get_package_atom("*CLOSURE-CACHE-ENABLED*", 23, "SI", 2, NIL);
    Closure_Cache_Enabled_word = GetVALCELL68k(index);

    index = get_package_atom("*CLOSURE-CACHE*", 15, "SI", 2, NIL);
    Closure_Cache_word = GetVALCELL68k(index);

    index = get_package_atom("*DELETED-IMPLICIT-HASH-SLOT*", 28, "XCL", 3, NIL);
    Deleted_Implicit_Hash_Slot_word = GetVALCELL68k(index);
  }

  /*** THE following CACHE values are added by Take. ***/
  /* The fixp_value calls make sure that those values are held
     in FIXP cells (and the ..._word variable points to that cell)
     so that the BIGVM code can go beyond 16 bits of page # */

  STORAGEFULLSTATE_index = MAKEATOM("\\STORAGEFULLSTATE");
  STORAGEFULLSTATE_word = MakeAtom68k("\\STORAGEFULLSTATE");
  STORAGEFULL_word = MakeAtom68k("\\STORAGEFULL");
  PENDINGINTERRUPT_word = MakeAtom68k("\\PENDINGINTERRUPT");
  LeastMDSPage_word = fixp_value(MakeAtom68k("\\LeastMDSPage"));
  SecondMDSPage_word = fixp_value(MakeAtom68k("\\SecondMDSPage"));
  SecondArrayPage_word = fixp_value(MakeAtom68k("\\SecondArrayPage"));
  INTERRUPTSTATE_word = MakeAtom68k("\\INTERRUPTSTATE");
  SYSTEMCACHEVARS_word = MakeAtom68k("\\SYSTEMCACHEVARS");
  MACHINETYPE_word = MakeAtom68k("\\MACHINETYPE");
  LASTVMEMFILEPAGE_word = fixp_value(MakeAtom68k("\\LASTVMEMFILEPAGE"));
  VMEM_FULL_STATE_word = MakeAtom68k("\\VMEM.FULL.STATE");
  Next_MDSpage_word = fixp_value(MakeAtom68k("\\NxtMDSPage"));
  Next_Array_word = fixp_value(MakeAtom68k("\\NxtArrayPage"));
  MDS_free_page_word = MakeAtom68k("\\MDSFREELISTPAGE");
  MaxTypeNumber_word = MakeAtom68k("\\MaxTypeNumber");

  /*** The following are STK-OVER-FLOW stuff * Take **/
  STACKOVERFLOW_word = MakeAtom68k("\\STACKOVERFLOW");
  GuardStackAddr_word = MakeAtom68k("\\GuardStackAddr");
  LastStackAddr_word = MakeAtom68k("\\LastStackAddr");
  NeedHardreturnCleanup_word = MakeAtom68k("\\NEED.HARDRESET.CLEANUP");

  /*** The addition of cache values for Bitblt by osamu April 13, 1988 ***/
  TEXTURE_atom = MAKEATOM("TEXTURE");
  MERGE_atom = MAKEATOM("MERGE");
  INPUT_atom = MAKEATOM("INPUT");
  INVERT_atom = MAKEATOM("INVERT");
  ERASE_atom = MAKEATOM("ERASE");
  PAINT_atom = MAKEATOM("PAINT");
  REPLACE_atom = MAKEATOM("REPLACE");

  /*** Do initialization for specific devices ***/
  init_for_keyhandle(); /* keyboard-interrupt handler */
  init_for_bltchar();   /* BLTCHAR speed-up */
  init_for_bitblt();    /* BITBLT-to-display speed-up */
}

/************************************************************************/
/*									*/
/*		    i n i t _ f o r _ k e y h a n d l e			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void init_for_keyhandle(void) {
  DLword index;
  extern DLword *CTopKeyevent;
  extern LispPTR *KEYBOARDEVENTQUEUE68k;
  extern LispPTR *KEYBUFFERING68k;
  extern LispPTR *TIMER_INTERRUPT_PENDING68k;
  extern LispPTR *PENDINGINTERRUPT68k;
  extern LispPTR *MOUSECHORDTICKS68k;
  extern LispPTR *LASTUSERACTION68k;
  extern LispPTR ATOM_STARTED;
  extern LispPTR *CLastUserActionCell68k;

  extern LispPTR *CURSORDESTHEIGHT68k;
  extern LispPTR *CURSORDESTWIDTH68k;

  extern LispPTR *CURSORHOTSPOTX68k;
  extern LispPTR *CURSORHOTSPOTY68k;
  extern LispPTR *SOFTCURSORUPP68k;
  extern LispPTR *SOFTCURSORWIDTH68k;
  extern LispPTR *SOFTCURSORHEIGHT68k;
  extern LispPTR *CURRENTCURSOR68k;

  extern LispPTR *PERIODIC_INTERRUPT68k;
  extern LispPTR *PERIODIC_INTERRUPT_FREQUENCY68k;

  CURSORDESTHEIGHT68k = MakeAtom68k("\\CURSORDESTHEIGHT");
  CURSORDESTWIDTH68k = MakeAtom68k("\\CURSORDESTWIDTH");
  CURSORHOTSPOTX68k = MakeAtom68k("\\CURSORHOTSPOTX");
  CURSORHOTSPOTY68k = MakeAtom68k("\\CURSORHOTSPOTY");

  SOFTCURSORUPP68k = MakeAtom68k("\\SOFTCURSORUPP");
  SOFTCURSORWIDTH68k = MakeAtom68k("\\SOFTCURSORWIDTH");
  SOFTCURSORHEIGHT68k = MakeAtom68k("\\SOFTCURSORHEIGHT");
  CURRENTCURSOR68k = MakeAtom68k("\\CURRENTCURSOR");

  ATOM_STARTED = MAKEATOM("STARTED");

  KEYBOARDEVENTQUEUE68k = MakeAtom68k("\\KEYBOARDEVENTQUEUE");
  KEYBUFFERING68k = MakeAtom68k("\\KEYBUFFERING");

  PENDINGINTERRUPT68k = MakeAtom68k("\\PENDINGINTERRUPT");
  MOUSECHORDTICKS68k = MakeAtom68k("\\MOUSECHORDTICKS");
  LASTUSERACTION68k = MakeAtom68k("\\LASTUSERACTION");

#ifndef INIT
  CLastUserActionCell68k = (LispPTR *)NativeAligned4FromLAddr(*LASTUSERACTION68k & POINTERMASK);
#endif

  DOBUFFEREDTRANSITION_index = MAKEATOM("\\DOBUFFEREDTRANSITIONS");
  INTERRUPTFRAME_index = MAKEATOM("\\INTERRUPTFRAME");

  CTopKeyevent = (DLword *)NativeAligned2FromLAddr(*KEYBOARDEVENTQUEUE68k);

  PERIODIC_INTERRUPT68k = MakeAtom68k("\\PERIODIC.INTERRUPT");
  PERIODIC_INTERRUPT_FREQUENCY68k = MakeAtom68k("\\PERIODIC.INTERRUPT.FREQUENCY");

  PERIODIC_INTERRUPTFRAME_index = MAKEATOM("\\PERIODIC.INTERRUPTFRAME");
  DORECLAIM_index = MAKEATOM("\\DORECLAIM");
#ifndef INIT
  index = get_package_atom("\\MAIKO.IO-INTERRUPT-FLAGS", 25, "INTERLISP", 9, NIL);
  IOINTERRUPTFLAGS_word = GetVALCELL68k(index);
#endif
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void init_for_bltchar(void) {
#ifdef COLOR
  LispPTR index;
#endif
  char *IL;

  extern LispPTR *TOPWDS68k;
#ifdef COLOR
  extern LispPTR *SCREENBITMAPS68k;
  extern LispPTR *COLORSCREEN68k; /*  \\COLORSCREEN */
#endif
  IL = "INTERLISP";

  if (!for_makeinit) {
    BLTCHAR_index = get_package_atom("\\MAIKO.PUNTBLTCHAR", 18, IL, 9, NIL);
    TEDIT_BLTCHAR_index = get_package_atom("\\TEDIT.BLTCHAR", 14, IL, 9, NIL);
  }
  TOPWDS68k = MakeAtom68k("\\TOPWDS");

#ifdef COLOR
  if (!for_makeinit) {
    SLOWBLTCHAR_index = get_package_atom("\\PUNT.SLOWBLTCHAR", 17, "INTERLISP", 9, NIL);

    index = MAKEATOM("\\SCREENBITMAPS");
    SCREENBITMAPS68k = GetVALCELL68k(index);
    COLORSCREEN_index = MAKEATOM("\\COLORSCREEN");
    COLORSCREEN68k = GetVALCELL68k(COLORSCREEN_index);
  }

#endif /* COLOR */
}

/************************************************************************/
/*									*/
/*			i n i t _ f o r _ b i t b l t			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void init_for_bitblt(void) {
#ifdef COLOR
  extern LispPTR *COLORSCREEN68k;
#endif /* COLOR */

  if (!for_makeinit) {
    BITBLTBITMAP_index = get_package_atom("\\PUNT.BITBLT.BITMAP", 19, "INTERLISP", 9, NIL);
    BLTSHADEBITMAP_index = get_package_atom("\\PUNT.BLTSHADE.BITMAP", 21, "INTERLISP", 9, NIL);

#ifdef COLOR
    COLORSCREEN_index = MAKEATOM("\\COLORSCREEN");
    COLORSCREEN68k = GetVALCELL68k(COLORSCREEN_index);
#endif /* COLOR */
  }
}
