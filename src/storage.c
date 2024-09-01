/* $Id: storage.c,v 1.5 2001/12/26 22:17:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-94 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*****************************************************************/
/*
                File Name :	storage.c

*/
/*****************************************************************/
#include <stdio.h>         // for printf
#include "address.h"       // for POINTER_PAGE
#include "adr68k.h"        // for NativeAligned4FromLAddr, LAddrFromNative
#include "car-cdrdefs.h"   // for cdr, car, rplacd, rplaca
#include "commondefs.h"    // for error
#include "conspagedefs.h"  // for cons
#include "gcdata.h"        // for ADDREF, GCLOOKUP
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "gcfinaldefs.h"   // for makefreearrayblock, mergebackward
#include "ifpage.h"        // for IFPAGE, MACHINETYPE_MAIKO
#include "lispemul.h"      // for LispPTR, NIL, GETFPTOVP, INTSTAT, ATOM_T
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"       // for InterfacePage, FPtoVP, SYSTEMCACHEVARS_word
#include "lsptypes.h"      // for Listp
#include "storagedefs.h"   // for checkfor_storagefull, init_storage, newpage
#include "testtooldefs.h"  // for MAKEATOM

#define MINARRAYBLOCKSIZE 4
#define GUARDVMEMFULL 500
#define IFPVALID_KEY 5603

static void advance_array_seg(unsigned int nxtpage);
static void advance_storagestate(DLword flg);
static LispPTR dremove(LispPTR x, LispPTR l);
static void set_storage_state(void);

/*****************************************************************/
/*
                Func Name :	checkfor_storagefull(npages)

                Created	:	Oct. 7, 1987 Takeshi Shimizu
                Changed :	Oct. 12,1987 take

                Used to be LispPTR T/NIL return, but result never used
*/
/*****************************************************************/

void checkfor_storagefull(unsigned int npages) {
  int pagesleft;
  INTSTAT *int_state;

#ifdef BIGVM
  pagesleft = (*Next_MDSpage_word) - (*Next_Array_word) - PAGESPER_MDSUNIT;
#else
  pagesleft = (*Next_MDSpage_word & 0xffff) - (*Next_Array_word & 0xffff) - PAGESPER_MDSUNIT;
#endif

  if ((pagesleft < GUARDSTORAGEFULL) || (npages != 0)) {
    if (*STORAGEFULLSTATE_word == NIL) set_storage_state();

    switch (*STORAGEFULLSTATE_word & 0xffff) {
      case SFS_NOTSWITCHABLE:
      case SFS_FULLYSWITCHED:
        if (pagesleft < 0) {
          while (T) { error("MP9320:Storage completely full"); }
        } else if ((pagesleft <= GUARD1STORAGEFULL) && (*STORAGEFULL_word != NIL)) {
          *STORAGEFULL_word = S_POSITIVE;
          error(
              "MP9325:Space getting VERY full.\
		Please save and reload a.s.a.p.");
        } else if (*STORAGEFULL_word == NIL) {
          *STORAGEFULL_word = ATOM_T;
          int_state = (INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word);
          int_state->storagefull = T;
          *PENDINGINTERRUPT_word = ATOM_T;
        }
#ifdef DEBUG
        printf("\n checkfor_storagefull:DORECLAIM.....\n");
#endif
        return; /*(NIL); */

      case SFS_SWITCHABLE:
        if (npages == NIL) {
          if (pagesleft <= 0) {
            *LeastMDSPage_word = *Next_Array_word;
            *Next_MDSpage_word = *SecondMDSPage_word;
            advance_storagestate(SFS_FULLYSWITCHED);
            advance_array_seg(*SecondArrayPage_word);
            return;
          }
        } else if (npages > pagesleft) {
          /* Have to switch array space over,
            but leave MDS to fill the rest of the low pages   */
          *LeastMDSPage_word = *Next_Array_word;
          advance_storagestate(SFS_ARRAYSWITCHED);
          advance_array_seg(*SecondArrayPage_word);
          return;
        }
        break;
#ifdef BIGVM
      case SFS_ARRAYSWITCHED:
        if ((*Next_MDSpage_word) < (*LeastMDSPage_word))
#else
      case SFS_ARRAYSWITCHED:
        if ((*Next_MDSpage_word & 0xffff) < (*LeastMDSPage_word & 0xffff))
#endif
        {
          *Next_MDSpage_word = *SecondMDSPage_word;
          advance_storagestate(SFS_FULLYSWITCHED);
          return;
        } else if (npages != NIL)
          if ((npages + GUARDSTORAGEFULL) >=
#ifdef BIGVM
              ((*SecondMDSPage_word) - (*Next_Array_word)))
#else
              ((*SecondMDSPage_word & 0xffff) - (*Next_Array_word & 0xffff)))
#endif
            return; /*  (NIL); */
        return;     /* (T); */
      /* break; */

      default:
        error("checkfor_storagefull: Shouldn't"); /* (*STORAGEFULLSTATE_word) & 0xffff) */
        break;
    }
  } else
    return; /*(NIL); */
} /* checkfor_storagefull end */

/*****************************************************************/
/*
                Func Name :	advance_array_seg(nxtpage)

                Created	:	Oct. 7, 1987 Takeshi Shimizu
                Changed :

*/
/*****************************************************************/

static void advance_array_seg(unsigned int nxtpage)
/* rare page num */
{
  unsigned int ncellsleft;

/* Called when 8Mb are exhausted,and we want to switch array space
 into the extended area(Secondary space),starting with nextpage.
 We MUST clean up old area first.   */

#ifdef BIGVM
  nxtpage &= 0xFFFFF; /* new VM, limit is 20 bits of page */
#else
  nxtpage &= 0xFFFF; /* for old VM size, limit is 16 bits of page */
#endif

  ncellsleft = (*Next_Array_word - POINTER_PAGE(*ArrayFrLst_word) - 1) * CELLSPER_PAGE +
               (CELLSPER_PAGE - (((*ArrayFrLst_word) & 0xff) >> 1));

  if (ncellsleft >= MINARRAYBLOCKSIZE) {
    mergebackward(makefreearrayblock(*ArrayFrLst_word, ncellsleft));
#ifdef BIGVM
    *ArrayFrLst2_word = (((*LeastMDSPage_word)) << 8);
#else
    *ArrayFrLst2_word = (((*LeastMDSPage_word) & 0xffff) << 8);
#endif
  } else {
    *ArrayFrLst2_word = *ArrayFrLst_word;
  }
#ifdef BIGVM
  *Next_Array_word = nxtpage;
#else
  *Next_Array_word = S_POSITIVE | nxtpage;
#endif
  *ArrayFrLst_word = nxtpage << 8;
  *ArraySpace2_word = *ArrayFrLst_word;

  /* return(S_POSITIVE); making function void as result never used */

} /* advance_array_seg end */

/*****************************************************************/
/*
                Func Name :	advance_storagestate(flg)

                Created	:	Oct. 7, 1987 Takeshi Shimizu
                Changed :

*/
/*****************************************************************/

/* DLword */
static void advance_storagestate(DLword flg) {
#ifdef DEBUG
  printf("STORAGEFULLSTATE is now set to %d \n", flg);
#endif

  *STORAGEFULLSTATE_word = (S_POSITIVE | flg);
  InterfacePage->fullspaceused = 65535;
  *SYSTEMCACHEVARS_word = dremove(STORAGEFULLSTATE_index, *SYSTEMCACHEVARS_word);
}

/*****************************************************************/
/*
                Func Name :	set_storage_state()

                Created	:	Oct. 7, 1987 Takeshi Shimizu
                Changed :

*/
/*****************************************************************/
static void set_storage_state(void) {
  if ((*MACHINETYPE_word & 0xffff) == MACHINETYPE_MAIKO) {
    if (InterfacePage->dl24bitaddressable != 0)
      *STORAGEFULLSTATE_word = S_POSITIVE | SFS_SWITCHABLE;
    else
      *STORAGEFULLSTATE_word = S_POSITIVE | SFS_NOTSWITCHABLE;

    *SYSTEMCACHEVARS_word = cons(STORAGEFULLSTATE_index, *SYSTEMCACHEVARS_word);

    GCLOOKUP(*SYSTEMCACHEVARS_word, ADDREF);

#ifdef DEBUG
    printf("SETSTATE: set to %d \n", (*STORAGEFULLSTATE_word) & 0xffff);
#endif
  } else {
    error("set_storage_state: Sorry! We can work on only Maiko");
  }

} /* set_storage_state() end */

static LispPTR dremove(LispPTR x, LispPTR l) {
  LispPTR z;

  if (Listp(l) == NIL)
    return (NIL);
  else if (x == car(l)) {
    if (cdr(l) != NIL) {
      rplaca(l, car(cdr(l)));
      rplacd(l, cdr(cdr(l)));
      return (dremove(x, l));
    } else 
        return (NIL);
  } else {
    z = l;
  lp:
    if (Listp(cdr(l)) == NIL)
      return (z);
    else if (x == car(cdr(l)))
      rplacd(l, cdr(cdr(l)));
    else
      l = cdr(l);
    goto lp;
  }
}

/*****************************************************************/
/*
                Func Name :	newpage(addr)

                Created	:	Oct. 12, 1987 Takeshi Shimizu
                Changed :	Oct. 13, 1987 take
                                Oct. 20, 1987 take

*/
/**
 * Makes an entry in the FPtoVP table for the newly created page.
 * Ensures that the FPtoVP table has room for the additional entry,
 * extending the FPtoVP table by an additional  page, if necessary,
 * such that it is maintained as a contiguous region of file pages.
 */
/*****************************************************************/

LispPTR newpage(LispPTR base) {
#ifdef BIGVM
  unsigned int vp; /* Virtual Page we're creating */
#else
  DLword vp;        /* (built from base)           */
#endif /* BIGVM */

  INTSTAT *int_state;
  unsigned int nactive;

  vp = base >> 8; /* Compute virtual-page # from Lisp address of the page */

#ifdef DEBUG
  /************************/
  if (vp == 0) error("newpage: vp=0");
  printf("***newpage modify vmemsize %d ", InterfacePage->nactivepages);
/*************************/
#endif
  nactive = ++(InterfacePage->nactivepages);

/* if nactive is a multiple of the # of FPTOVP entries */
/* on a page, we need to create a new FPTOVP page.     */
#ifdef BIGVM
  if ((nactive & 127) == 0) /* in BIGVM, they're cells */
#else
  if ((nactive & 0xff) == 0) /* in old way, they're words */
#endif /* BIGVM */
  {    /* need to add virtual page for fptovp first */
    unsigned int vp_of_fp, fp_of_fptovp;

    /* compute virtual page of new FPtoVP table page */
    /* i.e., the vp that holds the next FPtoVP entry */
    vp_of_fp = (LAddrFromNative(FPtoVP + nactive) >> 8);

    /* compute file page where this entry has to be to ensure
       that FPtoVP is contiguous on the file */

    fp_of_fptovp = InterfacePage->fptovpstart + (vp_of_fp - (LAddrFromNative(FPtoVP) >> 8));

    /* debugging check: make sure FPtoVP is contiguous */

    if (GETFPTOVP(FPtoVP, fp_of_fptovp - 1) != vp_of_fp - 1) {
      printf("FPtoVP not contiguous\n");
      printf("vp_of_fp = 0x%x, fp = 0x%x\n", vp_of_fp, fp_of_fptovp);
      printf("FPTOVP(fp-1) = 0x%x.\n", GETFPTOVP(FPtoVP, fp_of_fptovp - 1));
    }

    /* move the file page for the previous VMEM page */
    GETFPTOVP(FPtoVP, nactive) = GETFPTOVP(FPtoVP, fp_of_fptovp);

    /* now, store virtual page of FPtoVP in FPtoVP table */

    GETFPTOVP(FPtoVP, fp_of_fptovp) = (vp_of_fp);

    /* now we can make room for the new page we're adding */
    nactive = ++(InterfacePage->nactivepages);
  }
  GETFPTOVP(FPtoVP, nactive) = vp;

#ifdef DEBUG
  /*************************/
  printf("to %d  with VP:%d \n", InterfacePage->nactivepages, vp);
/************************/
#endif

  if (InterfacePage->nactivepages >
#ifdef BIGVM
      (((*LASTVMEMFILEPAGE_word)) - GUARDVMEMFULL))
#else
      (((*LASTVMEMFILEPAGE_word) & 0xffff) - GUARDVMEMFULL))
#endif
  {
    /* set vmemfull state */
    if (*VMEM_FULL_STATE_word == NIL) {
      int_state = (INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word);
      int_state->vmemfull = T;
      *PENDINGINTERRUPT_word = ATOM_T;
    }
#ifdef BIGVM
    if (InterfacePage->nactivepages < ((*LASTVMEMFILEPAGE_word)))
#else
    if (InterfacePage->nactivepages < ((*LASTVMEMFILEPAGE_word) & 0xffff))
#endif
    {
      *VMEM_FULL_STATE_word = S_POSITIVE; /* set 0 */
    } else if (InterfacePage->key == IFPVALID_KEY) {
      *VMEM_FULL_STATE_word = ATOM_T;
    } else
      *VMEM_FULL_STATE_word = MAKEATOM("DIRTY");
  }

  return (base);

} /* newpage */

/*****************************************************************/
/*
                Func Name :	init_storage()
                Description:
                                Set values which are referenced by
                                Lisp Storage handling funcs

                Created	:	Apr-23 1990 Takeshi Shimizu
                Changed :
*/
/*****************************************************************/
void init_storage(void) {
#ifdef BIGVM
  *SecondMDSPage_word = (InterfacePage->dllastvmempage - PAGESPER_MDSUNIT - 1);
#else
  *SecondMDSPage_word = S_POSITIVE | (InterfacePage->dllastvmempage - PAGESPER_MDSUNIT - 1);
#endif
} /* init_storage */
