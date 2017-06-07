/* $Id: gcmain3.c,v 1.4 1999/05/31 23:35:31 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: gcmain3.c,v 1.4 1999/05/31 23:35:31 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
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
/*           Descreption :                                               */
/*                                                                       */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/

#include <stdio.h> /* for sprintf */
#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "lspglob.h"
#include "emlglob.h"
#include "stack.h"
#include "cell.h"
#include "ifpage.h"
#include "gc.h"

#ifdef GCC386
#include "inlnPS2.h"
#endif

#define WORDSPERCELL 2
#define MAXHTCNT 63
#define PADDING 4
#define FNOVERHEADWORDS 8
#define ADD_OFFSET(ptr, dloffset) ((LispPTR *)((DLword *)(ptr) + (dloffset)))

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
  while ((ptr = gcreccell(ptr)) != NIL) REC_GCLOOKUP(ptr, ADDREF)
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

#ifdef GCC386
/* byte-swapped, 386 assembler version */
LispPTR gcmapscan() {
  volatile DLword probe;
  volatile DLword *entry;
  volatile DLword offset;
  volatile LispPTR ptr;

  asm volatile(
      " \n\
	movl $32768,%%edi	/ probe = HTSIZE.			\n\
	.align 4							\n\
nextentry:			/ nextentry:				\n\
	decl %%edi							\n\
	js returNIL							\n\
	leal 0(,%%edi,2),%%esi						\n\
	addl HTmain,%%esi	/htlptr = (struct htlinkptr *)(HTmain+probe);	\n\
	.align 4							\n\
scanloop:								\n\
	movl %%esi,%%edx						\n\
	xorb $2,%%dl							\n\
	movzwl (%%edx),%%eax	/ contents = ((struct htlinkptr *)WORDPTR(htlptr))->contents	\n\
	testl %%eax,%%eax	/ if (contents && 			\n\
	je scanbot							\n\
	testb $1,%%al							\n\
	jne scanok							\n\
	testb $254,%%ah							\n\
	jne scanbot							\n\
scanok:									\n\
	jmp scandone							\n\
	.align 4							\n\
scanbot:								\n\
	addl $-2,%%esi		/ end of while loop.			\n\
	decl %%edi							\n\
	jns scanloop							\n\
	jmp returNIL							\n\
									\n\
									\n\
scandone:								\n\
	movl %%edx,%0	/ entry = (DLword *) HTmain + probe.		\n\
retry:	/ retry:							\n\
	movl %0,%%ecx							\n\
	movzwl (%%ecx),%%eax						\n\
	testb $1,%%al		/ if HENTRY->collision,			\n\
	je nocollision							\n\
	xorl %%esi,%%esi		/ prev = 0			\n\
	andl $65534,%%eax						\n\
linkloop:		// linkloop:					\n\
	leal 0(,%%eax,2),%%ecx						\n\
	addl HTcoll,%%ecx						\n\
	movw 2(%%ecx),%%ax	/ offset = ((struct htcoll *)link)->free_ptr;	\n\
	testb $254,%%ah		/ if StkCountIsZero(offset)		\n\
	jne stknz							\n\
	sall $15,%%eax		/	, (probe << 1));		\n\
	andl $16711680,%%eax						\n\
	leal 0(,%%edi,2),%%edx						\n\
	orl %%eax,%%edx							\n\
	movl %%edx,%2	/ to ptr.					\n\
	testl %%esi,%%esi		/ DelLink.  if (prev != 0)	\n\
	je prevZ							\n\
	leal 2(%%esi),%%edx	/ GETWORD((DLword *)prev + 1) = GETWORD((DLword *)link + 1)	\n\
	xorb $2,%%dl							\n\
	leal 2(%%ecx),%%eax						\n\
	xorb $2,%%al							\n\
	movw (%%eax),%%ax						\n\
	jmp freelink							\n\
	.align 4							\n\
prevZ:	\n\
	movl %0,%%edx	/ else GETWORD((DLword *)entry) = GETWORD((DLword *)link + 1)	\n\
	leal 2(%%ecx),%%eax	\n\
	xorb $2,%%al	\n\
	movw (%%eax),%%ax	\n\
	orb $1,%%al	\n\
freelink:	/ FreeLink	\n\
	movw %%ax,(%%edx)	\n\
	movl %%ecx,%%eax	\n\
	xorb $2,%%al	\n\
	movw $0,(%%eax)	\n\
	leal 2(%%ecx),%%eax	\n\
	xorb $2,%%al	\n\
	movl HTcoll,%%edx	/ GETWORD(link+1) = GETWORD(HTcoll);	\n\
	xorb $2,%%dl	\n\
	movw (%%edx),%%dx	\n\
	movw %%dx,(%%eax)	\n\
	movl HTcoll,%%edx	/ GETWORD(HTcoll) = (link - HTcoll);	\n\
	xorb $2,%%dl	\n\
	movl %%ecx,%%eax	\n\
	subl HTcoll,%%eax	\n\
	sarl $1,%%eax	\n\
	movw %%ax,(%%edx)	\n\
	movl %0,%%esi	/ link = (DLword *)HTcoll + GetLinkptr(GETWORD((DLword *)entry	\n\
	movw (%%esi),%%ax	\n\
	andl $65534,%%eax	\n\
	addl %%eax,%%eax	\n\
	movl %%eax,%%ecx	\n\
	addl HTcoll,%%ecx	\n\
	leal 2(%%ecx),%%edx	/ if (GETWORD((DLword *)link + 1) == 0) {	\n\
	xorb $2,%%dl	\n\
	cmpw $0,(%%edx)	\n\
	jne sgclp1	\n\
	movl %%ecx,%%eax	/ GETWORD((DLword *)entry) = GETWORD((DLword *)link)	\n\
	xorb $2,%%al	\n\
	movw (%%eax),%%bx	\n\
	movw %%bx,(%%esi)	\n\
	movw $0,(%%eax)		/ FreeLink: GETWORD(link) = 0	\n\
	movl HTcoll,%%eax	/ GETWORD(link+1) = GETWORD(HTcoll)	\n\
	xorb $2,%%al	\n\
	movw (%%eax),%%bx	\n\
	movw %%bx,(%%edx)	\n\
	movl %%ecx,%%ebx	\n\
	subl HTcoll,%%ebx	\n\
	sarl $1,%%ebx	\n\
	movw %%bx,(%%eax)	\n\
	.align 4	\n\
sgclp1:	/ start of gcloop 1 - do setup		\n\
	movl GcDisabled_word,%%ecx		\n\
	movl	MDStypetbl,%%ebx		\n\
gclp1:		/ GcreclaimLp:	\n\
	pushl %2	\n\
	call gcreccell	\n\
	addl $4,%%esp	\n\
	movl %%eax,%2	\n\
	testl %%eax,%%eax	\n\
	je eogclp1	\n\
	shrl $9,%%eax	\n\
	leal 0(%%ebx,%%eax,2),%%eax		\n\
	xorb $2,%%al	\n\
	cmpw $0,(%%eax)	\n\
	jl gclp1	\n\
	cmpl $76,(%%ecx)	\n\
	je gclp1	\n\
	pushl $0	\n\
	pushl %2	\n\
	call rec_htfind	\n\
	addl $8,%%esp	\n\
	jmp gclp1	\n\
	.align 4	\n\
	.align 4	\n\
eogclp1:	\n\
	movl %0,%%eax	/ if (HTLPTR->contents == 0)	\n\
	cmpw $0,(%%eax)	\n\
	je nextentry		/ goto nextentry;	\n\
	jmp retry	/ else goto retry;	\n\
	.align 4	\n\
	.align 4	\n\
	\n\
stknz:	\n\
	movw (%%ecx),%%ax	/ if ((offset = ((struct htcoll *)link)->next_free))	\n\
	testw %%ax,%%ax	\n\
	je nextentry	\n\
	movl %%ecx,%%esi	\n\
	andl $65535,%%eax	\n\
	jmp linkloop	\n\
	.align 4	\n\
	.align 4	\n\
nocollision:	\n\
	testw $65024,(%%ecx)	/ if (StkCntIsZero(HTLPTR->contents)) {	\n\
	jne nextentry	\n\
	movw (%%ecx),%%dx	/ptr = VAG2(HENTRY->segnum, (probe << 1));	\n\
	sall $15,%%edx	\n\
	andl $16711680,%%edx	\n\
	leal (,%%edi,2),%%eax	\n\
	orl %%eax,%%edx	\n\
	movl %%edx,%2	\n\
	movw $0,(%%ecx)	/ HTLPTR->contents = 0	\n\
	.align 4	\n\
	movl GcDisabled_word,%%ecx	\n\
	movl MDStypetbl,%%ebx	\n\
gclp2:	/ GcreclaimLp	\n\
	pushl %2	\n\
	call gcreccell	\n\
	addl $4,%%esp	\n\
	movl %%eax,%2	\n\
	testl %%eax,%%eax	\n\
	je nextentry	\n\
	shrl $9,%%eax	\n\
	leal 0(%%ebx,%%eax,2),%%eax	\n\
	xorb $2,%%al	\n\
	cmpw $0,(%%eax)	\n\
	jl gclp2	\n\
	cmpl $76,(%%ecx)	\n\
	je gclp2	\n\
	pushl $0	\n\
	pushl %2	\n\
	call rec_htfind	\n\
	addl $8,%%esp	\n\
	jmp gclp2	\n\
	.align 4	\n\
returNIL:	\n\
    "
      : "=g"(entry), "=g"(offset), "=g"(ptr)
      :
      : "ax", "dx", "cx", "bx", "si", "di");

  return NIL;
}
#else

LispPTR gcmapscan(void) {
  register GCENTRY probe;
  register GCENTRY *entry;
  GCENTRY offset, dbgentry, dbgcontents;
  register LispPTR ptr;

  probe = HTMAIN_ENTRY_COUNT;
nextentry:
  while ((probe = gcscan1(probe)) != NIL) {
    entry = (GCENTRY *)HTmain + probe;
    dbgentry = GETGC(entry);
  retry:
    if (HENTRY->collision) {
      register GCENTRY *prev;
      register GCENTRY *link;
      LispPTR content, dbgfree;

      prev = (GCENTRY *)0;
      link = (GCENTRY *)HTcoll + GetLinkptr((content = HTLPTR->contents));
    linkloop:
      offset = ((struct htcoll *)link)->free_ptr;
      dbgfree = ((struct htcoll *)link)->next_free;
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
#endif /* GCC386 */

LispPTR gcmapunscan(void) {
  register GCENTRY probe;
  register GCENTRY *entry;
  GCENTRY offset;
  register LispPTR ptr;

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
      if ((offset = ((struct htcoll *)link)->next_free)) {
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
  register Bframe *bascframe;
  Bframe *obascframe;
  LispPTR scanptr, scanend;
  UNSIGNED scanend68K;
  struct fnhead *nametable;
  int ftyp;
  int pvcount;

  scanptr = VAG2(STK_HI, InterfacePage->stackbase);
  scanend = VAG2(STK_HI, InterfacePage->endofstack);
  scanend68K = (UNSIGNED)Addr68k_from_LADDR(scanend);
  bascframe = (Bframe *)Addr68k_from_LADDR(scanptr);

  if (0 != (3 & (UNSIGNED)bascframe)) {
    char debugStr[100];
    sprintf(debugStr,
            "Frame ptr (0x%x) not to word bound "
            "at start of gcscanstack.",
            bascframe);
    error(debugStr);
  }

  while (1) {
    /*This is endless loop until encountering tail of stack */

    ftyp = (int)bascframe->flags;
    switch (ftyp) {
      case STK_FX: {
        {
          register struct frameex1 *frameex;
          register struct fnhead *fnheader;
          frameex = (struct frameex1 *)bascframe;
          scanptr = LADDR_from_68k(frameex);
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
            pvars = (LispPTR *)((DLword *)bascframe + FRAMESIZE);
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

            obascframe = bascframe;
            bascframe =
                (Bframe *)ADD_OFFSET(bascframe, FRAMESIZE + PADDING + (((fnheader->pv) + 1) << 2));

            if (0 != (3 & (UNSIGNED)bascframe)) {
              char debugStr[100];
              sprintf(debugStr,
                      "Frame ptr (0x%x) not to word bound "
                      "in gcscanstack() STK_FX case; old frame = 0x%x.",
                      bascframe, obascframe);
              error(debugStr);
            }

          scantemps:
            while ((UNSIGNED)bascframe < (UNSIGNED)qtemp) {
              register LispPTR value;
              value = *((LispPTR *)bascframe);
              if
                Boundp(BIND_BITS(value)) Stkref(value);
              bascframe++;
            }; /* while */

            if (ntend != 0) {
              obascframe = bascframe;
              bascframe = (Bframe *)Addr68k_from_StkOffset(ntend);
              if (0 != (3 & (UNSIGNED)bascframe)) {
                char debugStr[100];
                sprintf(debugStr,
                        "Frame ptr (0x%x) not to word bound "
                        "in gcscanstack() scantemps; old frame = 0x%x.",
                        bascframe, obascframe);
                error(debugStr);
              }

              qtemp = next;
              ntend = 0;
              goto scantemps;
            };

            obascframe = bascframe;
            bascframe = (Bframe *)next;

            if (0 != (3 & (UNSIGNED)bascframe)) {
              char debugStr[100];
              sprintf(debugStr,
                      "Frame ptr (0x%x) not to word bound "
                      "in gcscanstack(), end scantemps; old frame = 0x%x.",
                      bascframe, obascframe);
              error(debugStr);
            }

          }; /* LOCAL regs qtemp next */
        };   /* local regs fnheader frameex */
        break;
      };
      case STK_GUARD: /* stack's tail ? */ {
        if ((UNSIGNED)bascframe >= (UNSIGNED)scanend68K)
          return (NIL);
        else {
          obascframe = bascframe;
          bascframe = (Bframe *)((DLword *)bascframe + bascframe->ivar);

          if (0 != (3 & (UNSIGNED)bascframe)) {
            char debugStr[100];
            sprintf(debugStr,
                    "Frame ptr (0x%x) not to word bound "
                    "in gcscanstack() STK_GUARD; old frame = 0x%x.",
                    bascframe, obascframe);
            error(debugStr);
          }
        };
        break;
      };
      case STK_FSB: {
        obascframe = bascframe;
        bascframe = (Bframe *)((DLword *)bascframe + bascframe->ivar);

        if (0 != (3 & (UNSIGNED)bascframe)) {
          char debugStr[100];
          sprintf(debugStr,
                  "Frame ptr (0x%x) not to word bound "
                  "in gcscanstack() STK_FSB; old frame = 0x%x.",
                  bascframe, obascframe);
          error(debugStr);
        }

        break;
      };
      default: /* must be basic frame !! */
      {
        register LispPTR bf_word;
        while (STK_BF != BF_FLAGS(bf_word = *((LispPTR *)bascframe))) {
          Stkref(PTR_BITS(bf_word));
          bascframe++;
        };
        bascframe++;
      };

        /* **** NOTE THIS CODE DOES NOT COMPILE CORRECTLY ON THE SUN 4
           {register LispPTR bf_word;
           while(STK_BF != BF_FLAGS(
           bf_word = *((LispPTR *)bascframe++)))
           { Stkref(PTR_BITS(bf_word));
           };
           };
           **** */
    }; /* switch */
  };   /* while(1) */
}
