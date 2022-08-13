/* $Id: gcmain3.c,v 1.4 1999/05/31 23:35:31 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*************************************************************************/
/*                                                                       */
/*                       File Name : gcmain3.c                           */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                      Creation Date : July-7-1987                      */
/*                      Written by Tomoru Teruuchi                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions : gcmapscan();                                    */
/*                       gcmapunscan();                                  */
/*                       gcscanstack();                                  */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/*           Description :                                               */
/*                                                                       */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include <stdio.h>        // for sprintf
#include "address.h"      // for VAG2
#include "adr68k.h"       // for Addr68k_from_LADDR, Addr68k_from_StkOffset
#include "commondefs.h"   // for error
#include "emlglob.h"
#include "gcdata.h"       // for GCENTRY, REC_GCLOOKUP, STKREF, hashentry
#include "gcmain3defs.h"  // for gcmapscan, gcmapunscan, gcscanstack
#include "gcrcelldefs.h"  // for gcreccell
#include "gcscandefs.h"   // for gcscan1, gcscan2
#include "ifpage.h"       // for IFPAGE
#include "lispemul.h"     // for LispPTR, DLword, NIL, FRAMESIZE, POINTERMASK
#include "lispmap.h"      // for HTMAIN_SIZE, STK_HI
#include "lspglob.h"      // for HTcoll, HTmain, InterfacePage
#include "lsptypes.h"
#include "stack.h"        // for Bframe, fnhead, frameex1, STK_BF, STK_FSB

#define WORDSPERCELL 2
#define MAXHTCNT 63
#define PADDING 4
#define FNOVERHEADWORDS 8
#define ADD_OFFSET(ptr, dloffset) ((LispPTR *)(((DLword *)(ptr)) + (dloffset)))

#ifdef BIGVM
#define BIND_BITS(value) ((unsigned int)(value) >> 28)
#define BF_FLAGS(value) ((unsigned int)(value) >> 29)
#define PTR_BITS(entry) ((unsigned int)(entry)&POINTERMASK)
#define GetSegnuminColl(entry1) (((entry1)&0xFFFE) >> 1)
#define GetStkCnt(entry1) ((entry1) >> 16)
#define StkCntIsZero(entry1) (!((entry1)&0xFFFF0000))
#define StkrefP(entry1) ((entry1)&0x10000)
#define SinglerefP(entry1) (((entry1)&0xFFFE0000) == 0x20000)
#define Boundp(frame_field) ((frame_field) == 0)
#define Stkref(ptr) REC_GCLOOKUP(ptr, STKREF)
#define GcreclaimLp(ptr) \
  while (((ptr) = gcreccell(ptr)) != NIL) REC_GCLOOKUP(ptr, ADDREF)
#define HTLPTR ((struct htlinkptr *)(entry))
#define HENTRY ((struct hashentry *)(entry))
#define HTMAIN_ENTRY_COUNT (HTMAIN_SIZE >> 1)
#define STKREFBIT 0x10000 /* the bit that says "I'm on the stack" */

#else
#define BIND_BITS(value) ((unsigned int)(value) >> 24)
#define BF_FLAGS(value) ((unsigned int)(value) >> 29)
#define PTR_BITS(entry) ((unsigned int)((unsigned int)((entry) << 8) >> 8))
#define GetSegnuminColl(entry1) ((entry1 & 0x01fe) >> 1)
#define GetStkCnt(entry1) ((entry1 & 0x0fe00) >> 9)
#define StkCntIsZero(entry1) (!((entry1)&0x0fe00))
#define StkrefP(entry1) ((entry1)&0x0200)
#define SinglerefP(entry1) (((entry1)&0xFC00) == 0x0400)
#define Boundp(frame_field) ((frame_field) == 0)
#define Stkref(ptr) REC_GCLOOKUP(ptr, STKREF)
#define GcreclaimLp(ptr) \
  while ((ptr = gcreccell(ptr)) != NIL) REC_GCLOOKUP(ptr, ADDREF)
#define HTLPTR ((struct htlinkptr *)WORDPTR(entry))
#define HENTRY ((struct hashentry *)WORDPTR(entry))
#define HTMAIN_ENTRY_COUNT HTMAIN_SIZE
#define STKREFBIT 0x200
#endif /* BIGVM */


LispPTR gcmapscan(void) {
  register GCENTRY probe;
  register GCENTRY *entry;
  GCENTRY offset, dbgcontents;
  register LispPTR ptr;

  probe = HTMAIN_ENTRY_COUNT;
nextentry:
  while ((probe = gcscan1(probe)) != NIL) {
    entry = (GCENTRY *)HTmain + probe;
  retry:
    if (HENTRY->collision) {
      register GCENTRY *prev;
      register GCENTRY *link;
      LispPTR content;

      prev = (GCENTRY *)0;
      link = (GCENTRY *)HTcoll + GetLinkptr((content = HTLPTR->contents));
    linkloop:
      offset = ((struct htcoll *)link)->free_ptr;
      if (StkCntIsZero(offset)) {
        /* Reclaimable object */
        ptr = VAG2(GetSegnuminColl(offset), (probe << 1));
        DelLink(link, prev, entry);
        GcreclaimLp(ptr);
        if (HTLPTR->contents == 0)
          goto nextentry;
        else
          goto retry;
      }
      if ((offset = ((struct htcoll *)link)->next_free)) {
        prev = link;
        link = (GCENTRY *)(HTcoll + offset);
        goto linkloop;
      }
      goto nextentry;
    }
    if (StkCntIsZero(dbgcontents = HTLPTR->contents)) {
      ptr = VAG2(HENTRY->segnum, (probe << 1));
      HTLPTR->contents = 0;
      GcreclaimLp(ptr);
    }
  }
  return (NIL);
}

LispPTR gcmapunscan(void) {
  register GCENTRY probe;
  register GCENTRY *entry;
  GCENTRY offset;

  probe = HTMAIN_ENTRY_COUNT;
  while ((probe = gcscan2(probe)) != NIL) {
    entry = (GCENTRY *)HTmain + probe;
  retry:
    if (HENTRY->collision) {
      register GCENTRY *prev;
      register GCENTRY *link;

      prev = (GCENTRY *)0;
      link = (GCENTRY *)(HTcoll + GetLinkptr(HTLPTR->contents));
    scnlp:
      offset = ((struct htcoll *)link)->free_ptr;
      if (StkrefP(offset)) {
        if (SinglerefP(offset)) {
          DelLink(link, prev, entry);
          goto retry;
        } else {
          GETGC((GCENTRY *)link) = offset & (~STKREFBIT);
        }
      }
      offset = ((struct htcoll *)link)->next_free;
      if (offset) {
        prev = link;
        link = (GCENTRY *)(HTcoll + offset);
        goto scnlp;
      }
    } else if (HENTRY->stackref) {
      if (HENTRY->count == 1)
        HTLPTR->contents = 0;
      else
        HENTRY->stackref = 0;
    }
  }
  return (NIL);
}

LispPTR gcscanstack(void) {
  register Bframe *basicframe;
  Bframe *obasicframe;
  LispPTR scanptr, scanend;
  UNSIGNED scanend68K;
  int ftyp;

  scanptr = VAG2(STK_HI, InterfacePage->stackbase);
  scanend = VAG2(STK_HI, InterfacePage->endofstack);
  scanend68K = (UNSIGNED)Addr68k_from_LADDR(scanend);
  basicframe = (Bframe *)Addr68k_from_LADDR(scanptr);

  if (0 != (3 & (UNSIGNED)basicframe)) {
    char debugStr[100];
    sprintf(debugStr,
            "Frame ptr (%p) not to word bound at start of gcscanstack.",
            (void *)basicframe);
    error(debugStr);
  }

  while (1) {
    /*This is endless loop until encountering tail of stack */

    ftyp = (int)basicframe->flags;
    switch (ftyp) {
      case STK_FX: {
        {
          register struct frameex1 *frameex;
          register struct fnhead *fnheader;
          frameex = (struct frameex1 *)basicframe;
          {
            register LispPTR fn_head;
#ifdef BIGVM
            fn_head = (LispPTR)(frameex->fnheader);
#else
            fn_head = (LispPTR)VAG2(frameex->hi2fnheader, frameex->lofnheader);
#endif /* BIGVM */
            Stkref(fn_head);
            fnheader = (struct fnhead *)Addr68k_from_LADDR(fn_head);
          };
          {
            register int pcou;
            register LispPTR *pvars;
            pvars = (LispPTR *)((DLword *)basicframe + FRAMESIZE);
            for (pcou = fnheader->nlocals; pcou-- != 0;) {
              register LispPTR value;
              value = *pvars;
              if
                Boundp(BIND_BITS(value)) Stkref(value);
              ++pvars;
            }; /* for */
          };   /* register int pcou */

          {
            register UNSIGNED qtemp;
            register UNSIGNED next;
            register UNSIGNED ntend;

            next = qtemp = (UNSIGNED)Addr68k_from_StkOffset(frameex->nextblock);
            /* this is offset */
            ntend = 0; /* init flag */
            if (frameex->validnametable) {
              register LispPTR nametable;
#ifdef BIGVM
              nametable = frameex->nametable;
#define hi2nametable (nametable >> 16)
#else
              register unsigned int hi2nametable;
              register unsigned int lonametable;
              lonametable = frameex->lonametable;
              hi2nametable = frameex->hi2nametable;
              nametable = VAG2(hi2nametable, lonametable);
#endif /* BIGVM */
              if (STK_HI == hi2nametable) {
                Stkref(fnheader->framename);
#ifdef BIGVM
                qtemp = (UNSIGNED)Addr68k_from_StkOffset(nametable & 0xFFFF);
#else
                qtemp = (UNSIGNED)Addr68k_from_StkOffset(lonametable);
#endif
                ntend = (UNSIGNED)(((DLword *)qtemp) + FNHEADSIZE +
                                   (((struct fnhead *)qtemp)->ntsize) * 2);
              } else
                Stkref(nametable);
            }; /* frameex->validnametable */

            obasicframe = basicframe;
            basicframe =
                (Bframe *)ADD_OFFSET(basicframe, FRAMESIZE + PADDING + (((fnheader->pv) + 1) << 2));

            if (0 != (3 & (UNSIGNED)basicframe)) {
              char debugStr[100];
              sprintf(debugStr,
                      "Frame ptr (%p) not to word bound "
                      "in gcscanstack() STK_FX case; old frame = %p.",
                      (void *)basicframe, (void *)obasicframe);
              error(debugStr);
            }

          scantemps:
            while ((UNSIGNED)basicframe < (UNSIGNED)qtemp) {
              register LispPTR value;
              value = *((LispPTR *)basicframe);
              if
                Boundp(BIND_BITS(value)) Stkref(value);
              basicframe++;
            }; /* while */

            if (ntend != 0) {
              obasicframe = basicframe;
              basicframe = (Bframe *)ntend;
              if (0 != (3 & (UNSIGNED)basicframe)) {
                char debugStr[100];
                sprintf(debugStr,
                        "Frame ptr (%p) not to word bound "
                        "in gcscanstack() scantemps; old frame = %p.",
                        (void *)basicframe, (void *)obasicframe);
                error(debugStr);
              }

              qtemp = next;
              ntend = 0;
              goto scantemps;
            };

            obasicframe = basicframe;
            basicframe = (Bframe *)next;

            if (0 != (3 & (UNSIGNED)basicframe)) {
              char debugStr[100];
              sprintf(debugStr,
                      "Frame ptr (%p) not to word bound "
                      "in gcscanstack(), end scantemps; old frame = %p.",
                      (void *)basicframe, (void *)obasicframe);
              error(debugStr);
            }

          }; /* LOCAL regs qtemp next */
        };   /* local regs fnheader frameex */
        break;
      };
      case STK_GUARD: /* stack's tail ? */ {
        if ((UNSIGNED)basicframe >= (UNSIGNED)scanend68K)
          return (NIL);
        else {
          obasicframe = basicframe;
          basicframe = (Bframe *)((DLword *)basicframe + basicframe->ivar);

          if (0 != (3 & (UNSIGNED)basicframe)) {
            char debugStr[100];
            sprintf(debugStr,
                    "Frame ptr (%p) not to word bound "
                    "in gcscanstack() STK_GUARD; old frame = %p.",
                    (void *)basicframe, (void *)obasicframe);
            error(debugStr);
          }
        };
        break;
      };
      case STK_FSB: {
        obasicframe = basicframe;
        basicframe = (Bframe *)((DLword *)basicframe + basicframe->ivar);

        if (0 != (3 & (UNSIGNED)basicframe)) {
          char debugStr[100];
          sprintf(debugStr,
                  "Frame ptr (%p) not to word bound "
                  "in gcscanstack() STK_FSB; old frame = %p.",
                  (void *)basicframe, (void *)obasicframe);
          error(debugStr);
        }

        break;
      };
      default: /* must be basic frame !! */
      {
        register LispPTR bf_word;
        while (STK_BF != BF_FLAGS(bf_word = *((LispPTR *)basicframe))) {
          Stkref(PTR_BITS(bf_word));
          basicframe++;
        };
        basicframe++;
      };

        /* **** NOTE THIS CODE DOES NOT COMPILE CORRECTLY ON THE SUN 4
           {register LispPTR bf_word;
           while(STK_BF != BF_FLAGS(
           bf_word = *((LispPTR *)basicframe++)))
           { Stkref(PTR_BITS(bf_word));
           };
           };
           **** */
    }; /* switch */
  };   /* while(1) */
}
