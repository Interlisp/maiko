/* $Id: allocmds.c,v 1.4 1999/05/31 23:35:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/**********************************************************************/
/*
                File Name :	allocmds.c

                Allocate Data Type(MDS)

                                Date :		August 18, 1987
                                Edited by :	Takeshi Shimizu

                Including :	initmdspage
                                alloc_mdspage

*/
/**********************************************************************/

#include "address.h"       // for LOLOC                                                                                                                                     
#include "adr68k.h"        // for LAddrFromNative, LPageFromNative, Addr68k_fr...                                                                                             
#include "allocmdsdefs.h"  // for alloc_mdspage, initmdspage                                                                                                                
#include "commondefs.h"    // for error                                                                                                                                     
#include "lispemul.h"      // for DLword, LispPTR, DLWORDSPER_PAGE, MDSINCRE...                                                                                             
#include "lispmap.h"       // for S_POSITIVE                                                                                                                                
#include "lspglob.h"       // for MDStypetbl                                                                                                                                
#include "lsptypes.h"      // for GETWORD, GetTypeNumber, TYPE_SMALLP                                                                                                       
#include "storagedefs.h"   // for newpage, checkfor_storagefull                                                                                                             

/************************************************************************/
/*									*/
/*			Make_MDSEntry					*/
/*									*/
/*	Fill in the MDS type-table entry for a new page (see		*/
/*	lsptypes.h for the meaning of the entry bits).			*/
/*									*/
/************************************************************************/
/* I consider that there is no case the variable named \GCDISABLED is set to T */
static inline void Make_MDSentry(UNSIGNED page, DLword pattern) {
  GETWORD((DLword *)MDStypetbl + (page >> 1)) = (DLword)pattern;
}

/**********************************************************************/
/*
        Func name :	initmdspage

                Write the specified MDSTT entry with specified pattern.
                returns Top entry for free chain lisp address

                                Date :		December 24, 1986
                                Edited by :	Takeshi Shimizu
                                Changed :	Jun. 5 87 take

*/
/**********************************************************************/

LispPTR initmdspage(LispPTR *base, DLword size, LispPTR prev)
/* MDS page base */
/* object cell size you need (WORD) */
/* keeping top of previous MDS cell */

{
  int remain_size; /* (IREMAINDER WORDSPERPAGE SIZE) */
  short num_pages;
  int limit;
  int used; /* used space in MDS page */
  int i;

#ifdef TRACE2
  printf("TRACE: initmdspage()\n");
#endif

  remain_size = DLWORDSPER_PAGE % size;

  if ((remain_size != 0) && (remain_size < (size >> 1) && (size < DLWORDSPER_PAGE))) {
    num_pages = MDSINCREMENT / DLWORDSPER_PAGE; /* on 1121 maybe 2 */
    limit = DLWORDSPER_PAGE;
  } else {
    num_pages = 1;
    limit = MDSINCREMENT;
  }

  for (i = 0; i < num_pages; i++) {
    used = 0;
    while ((used += size) <= limit) {
      *base = prev;                /* write prev MDS address to the top of MDS page */
      prev = LAddrFromNative(base); /* exchanging pointers */
      base = (LispPTR *)((DLword *)base + size);
    } /* while end */

    base = (LispPTR *)((DLword *)base + remain_size);

  } /* for end */

  return (prev);
} /* initmdspage end */

/**********************************************************************/
/*
                Func name :	alloc_mdspage

                        This version works only for Maiko

                        Date :		January 13, 1987
                        Edited by :	Takeshi Shimizu
                        Changed :	3-Apr-87 (take)
                                        20-Aug-87(take) ifdef
                                        08-Oct-87(take) checkfull
                                        22-Dec-87(Take)

*/
/**********************************************************************/

LispPTR *alloc_mdspage(short int type) {
  LispPTR *ptr; /* points Top 32 bit of the MDS page */
  LispPTR next_page;

  /* Next_Array=(DLword *)NativeAligned2FromLAddr(((*Next_Array_word)& 0xffff ) << 8); */

  if (LOLOC(*MDS_free_page_word) != NIL) {
    ptr = (LispPTR *)NativeAligned4FromLPage(LOLOC(*MDS_free_page_word));

    if (((next_page = LOLOC(*ptr)) != 0) && (GetTypeNumber((*ptr)) != TYPE_SMALLP))
      error("alloc_mdspage: Bad Free Page Link");
    else {
      *MDS_free_page_word = S_POSITIVE | next_page;
    }
  } else {
    /* I guess Next_MDSpage is redundant */
    checkfor_storagefull(NIL);
#ifdef BIGVM
    Next_MDSpage = (DLword *)NativeAligned2FromLAddr(((*Next_MDSpage_word)) << 8);
#else
    Next_MDSpage = (DLword *)NativeAligned2FromLAddr(((*Next_MDSpage_word) & 0xffff) << 8);
#endif
    ptr = (LispPTR *)Next_MDSpage;       /* Get Pointer to First Page */
    Next_MDSpage -= DLWORDSPER_PAGE * 2; /* decrement MDS count */
#ifdef BIGVM
    *Next_MDSpage_word = LPageFromNative(Next_MDSpage);
#else
    *Next_MDSpage_word = S_POSITIVE | LPageFromNative(Next_MDSpage);
#endif
    newpage(newpage(LAddrFromNative(ptr)) + DLWORDSPER_PAGE);
  }

  Make_MDSentry(LPageFromNative(ptr), type);
  return (ptr);
} /* alloc_mdspage end */
