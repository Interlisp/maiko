#ifndef LSPGLOB_H
#define LSPGLOB_H 1

/* $Id: lspglob.h,v 1.2 1999/01/03 02:06:15 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/**********************************************************************/
/*
 		File Name :	lspglob.h

		Global variables for LispSYSOUT
 
 					Date :		December 16, 1986
 					Edited by :	Takeshi Shimizu
 					Changed : Jan 13 1987 Take.
					Changed : Feb 16 1987 Take.
					Changed : Mar 25 1987 Take.
					Changed : Apr 24 1987 take
					Changed : Jul 02 1987 take
*/
/**********************************************************************/
#include "version.h" /* for BIGVM */
#include "ifpage.h" /* for IFPAGE */
#include "iopage.h" /* for IOPAGE */
#include "lispemul.h" /* for LispPTR, DLword */
#include "miscstat.h" /* for MISCSTAT */

 extern  DLword *Stackspace;		/* STACKSPACE*/
 extern  DLword *Plistspace;		/* PLISTSPACE */
 extern  DLword *DTDspace;		/* DTDSPACE */
 extern  DLword *MDStypetbl;		/* MDSTT  */
 extern  DLword *AtomHT;		/* AtomHashTable */
 extern  DLword *Pnamespace;		/* PNSPACE */
 extern  DLword *AtomSpace;		/* New atoms, initial set */
 extern  DLword *Defspace;		/* DEFSPACE */
 extern  DLword *Valspace;		/* VALSPACE */
 
/* For Virtual Mem Management */
#ifdef BIGVM
extern LispPTR *FPtoVP;
#else
extern DLword *FPtoVP;
#endif /* BIGVM */
extern DLword *PAGEMap;
extern DLword *PageMapTBL;
extern DLword *LockedPageTable;

/* For Interface to Micro or Device */
extern DLword *IOCBPage;
extern IOPAGE *IOPage;
extern IFPAGE *InterfacePage;
extern MISCSTATS *MiscStats;

/* UFN Tbl */
extern DLword *UFNTable;

/* Internal Hash Table for GC */
#ifdef BIGVM
 extern  LispPTR *HTmain;
 extern  LispPTR *HToverflow;
 extern  LispPTR *HTbigcount;
 extern  LispPTR *HTcoll;
#else
 extern  DLword *HTmain;
 extern  DLword *HToverflow;
 extern  DLword *HTbigcount;
 extern  DLword *HTcoll;
#endif /* BIGVM */


/* DISPLAY */
 extern DLword *DisplayRegion;


/* FLEX STORAGES */
 extern  DLword *MDS_space_bottom;  /* Start of MDS (pre -2) */
 extern  DLword *PnCharspace;	/* Space for PN char codes (Thin only) */

 extern  struct dtd *ListpDTD;	/* DTD for LISTP chang. 25-mar-87 take */
 extern  DLword *Next_Array;	/* Next available ARRAY space */
 extern  DLword *Next_MDSpage;	/* Next available MDS space */
  
 /* Pointers in Cell or any object means DLword offset from Lisp_world.
 So, 24 bit Lisp pointers can points 32Mbyte area.
 But, ATOMSPACE may be treated as special index space for LITATOM.
 In another way, it means that the Pointers points ATOMSPACE has no
 allocated memory, and these are used as index for access one of ATOM's prop. */
  extern DLword *Lisp_world;  /* Lisp Start BASE  */
 


/******* CACHE 68k address for LISP SYSVAL *******/
extern LispPTR *Next_MDSpage_word;
extern LispPTR *Next_Array_word;
extern LispPTR *MDS_free_page_word;

extern LispPTR *Reclaim_cnt_word;

/*** cache values for array reclaimer by Tomtom 30-Sep-1987 ***/

extern LispPTR *GcDisabled_word;
extern LispPTR *CdrCoding_word;
extern LispPTR *FreeBlockBuckets_word;
extern LispPTR *Array_Block_Checking_word;
extern LispPTR *ArrayMerging_word;
extern LispPTR *ArraySpace_word;
extern LispPTR *ArraySpace2_word;
extern LispPTR *ArrayFrLst_word;
extern LispPTR *ArrayFrLst2_word;
extern LispPTR *Hunk_word;
extern LispPTR *System_Buffer_List_word;

/*** The end of the addition of cache values for reclaimer by Tomtom ***/

/*** cache values for top level reclaimer Tomtom 15-Oct-1987 ***/
extern LispPTR *GcMess_word;
extern LispPTR *ReclaimMin_word;
extern LispPTR *GcTime1_word;
extern LispPTR *GcTime2_word;
extern LispPTR *MaxTypeNumber_word;
/*** The end of the addition of cache values for the top level reclaimer ***/

/*** The addition cache for closure-caching ***/

extern LispPTR *Package_from_Index_word;
extern LispPTR *Package_from_Name_word;
extern LispPTR *Keyword_Package_word;
extern LispPTR *Deleted_Implicit_Hash_Slot_word;
extern LispPTR *Closure_Cache_Enabled_word;
extern LispPTR *Closure_Cache_word;
extern LispPTR First_index;
/*** The end of cache value for closure-caching ***/

/* CACHE values for 32Mb MDS/Array by Take */
extern LispPTR *STORAGEFULLSTATE_word;
extern LispPTR *STORAGEFULL_word;
extern LispPTR *PENDINGINTERRUPT_word;
extern LispPTR *LeastMDSPage_word;
extern LispPTR *SecondMDSPage_word;
extern LispPTR *SecondArrayPage_word;
extern LispPTR *INTERRUPTSTATE_word;
extern LispPTR *IOINTERRUPTFLAGS_word;
extern LispPTR *SYSTEMCACHEVARS_word;
extern LispPTR *MACHINETYPE_word;

extern LispPTR STORAGEFULLSTATE_index;
/******* 7-Oct-87 take********/
extern LispPTR *LASTVMEMFILEPAGE_word;
extern LispPTR *VMEM_FULL_STATE_word;

/* Array for N-tran */
extern int native_load_address;
extern LispPTR native_closure_env;

/* Vars for Stack operations */
extern LispPTR *STACKOVERFLOW_word;
extern LispPTR *GuardStackAddr_word;
extern LispPTR *LastStackAddr_word;
extern LispPTR *NeedHardreturnCleanup_word;

/* I/O Pipe for Unix Interface */
extern int UnixPipeIn;
extern int UnixPipeOut;
extern int UnixPID;

/* Interrupt frame calls */
extern LispPTR DOBUFFEREDTRANSITION_index;
extern LispPTR INTERRUPTFRAME_index;
extern LispPTR PERIODIC_INTERRUPTFRAME_index;
extern LispPTR DORECLAIM_index;

/* BITBLT related atoms */
extern LispPTR BITBLTBITMAP_index;
extern LispPTR BLTSHADEBITMAP_index;
extern LispPTR BLTCHAR_index;
extern LispPTR TEDIT_BLTCHAR_index;
#ifdef COLOR
extern LispPTR SLOWBLTCHAR_index;
extern LispPTR COLORSCREEN_index;
#endif

/* BITBLT operation atoms */
extern DLword TEXTURE_atom;
extern DLword MERGE_atom;
extern DLword INPUT_atom;
extern DLword INVERT_atom;
extern DLword ERASE_atom;
extern DLword PAINT_atom;
extern DLword REPLACE_atom;

/* Atom index for IL:\MVLIST, for the VALUES opcode */
extern LispPTR MVLIST_index;

#endif
