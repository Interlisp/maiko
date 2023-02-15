/* $Id: gcscan.c,v 1.3 1999/05/31 23:35:33 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*************************************************************************/
/*                                                                       */
/*                         File Name : gcscan.c                         */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                         Creation Date : July-7-1987                   */
/*                         Written by Tomoru Teruuchi                    */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions :                                                 */
/*                        gcscan1(probe)                                 */
/*                        gcscan2(probe)                                 */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/* The functions "gcscan1" and "gcscan2" are the translated functions    */
/*  from the Lisp Functions "\GCSCAN1" & "\GCSCAN2".                     */
/* These functions' role is to scan the HTmain Table and return the      */
/*  existing entry(by "gcscan1") & the entry whose STKBIT field is ON    */
/*  (by "gcscan2").These functions are the UFN functions that are called */
/*  by OPCODES "GCSCAN1" & "GCSCAN2".                                    */
/*                                                                       */
/* gcscan1                                                               */
/*    INPUT : probe (the starting offset in the HTmain table)            */
/*    OUTPUT : the entry's offset or NIL (no more entry existing)        */
/*                                                                       */
/* gcscan2                                                               */
/*    INPUT : probe (the starting offset in the HTmain table)            */
/*    OUTPUT : the entry's offset or NIL (no more entry existing)        */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include "lispemul.h"
#include "lspglob.h"
#include "gcdata.h"
#include "lsptypes.h"

#include "gcscandefs.h"

#ifdef BIGVM
#define HTSTKBIT 0x10000 /* = 512 */
#define HTENDS ((struct hashentry *)htlptr)
#define GetStkCnt(entry1) ((entry1) >> 16)
#else
#define HTSTKBIT 0x200 /* = 512 */
#define HTENDS ((struct hashentry *)htlptr)
#define GetStkCnt(entry1) (entry1 >> 9)
#endif /* BIGVM */

int gcscan1(int probe)
/* probe is offset */
{
  struct htlinkptr *htlptr; /* overlay access method */
  int contents;
  while (--probe >= 0) /* End of HTmain Table ? */
  {
    /* Start addr. of scanning */
    htlptr = (struct htlinkptr *)(HTmain + probe);
    contents = ((struct htlinkptr *)GCPTR(htlptr))->contents;
    if (contents && (((struct hashentry *)GCPTR(HTENDS))->collision || (GetStkCnt(contents) == 0)))
      return (probe);
  }
  return (-1);
}

int gcscan2(int probe)
/* probe is offset */
{
  struct htlinkptr *htlptr; /* overlay access method */
  while (--probe >= 0)               /* End of HTmain Table ? */
  {
    htlptr = (struct htlinkptr *)(HTmain + probe);
    /* Start addr. of scanning */
    if (((HTSTKBIT | 1) & ((struct htlinkptr *)GCPTR(htlptr))->contents) != 0)
      return (probe); /* stackref or collision ON */
  }
  return (-1);
}
