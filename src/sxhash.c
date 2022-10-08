/* $Id: sxhash.c,v 1.4 2001/12/24 01:09:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"       // for NativeAligned4FromLAddr
#include "arith.h"        // for FIXP_VALUE
#include "car-cdrdefs.h"  // for car, cdr
#include "cell.h"         // for PLCell, PNCell, GetPnameCell, GetPropCell
#include "commondefs.h"   // for error
#include "emlglob.h"
#include "lispemul.h"     // for LispPTR, DLword, POINTERMASK, SEGMASK
#include "lispmap.h"      // for S_POSITIVE, S_NEGATIVE
#include "lspglob.h"
#include "lsptypes.h"     // for OneDArray, PATHNAME, GETBYTE, GETWORD, COMPLEX
#include "sxhashdefs.h"   // for STRING_EQUAL_HASHBITS, STRING_HASHBITS, SX_...

/** Follows definition in LLARRAYELT: **/
#define EQHASHINGBITS(item) \
  ((((item) >> 16) & 0xFFFF) ^ ((((item)&0x1FFF) << 3) ^ (((item) >> 9) & 0x7f)))

static unsigned short sxhash(LispPTR obj);
static unsigned short sxhash_rotate(short unsigned int value);
static unsigned short sxhash_string(OneDArray *obj);
static unsigned short sxhash_bitvec(OneDArray *obj);
static unsigned short sxhash_list(LispPTR obj);
static unsigned short sxhash_pathname(LispPTR obj);
static unsigned short stringequalhash(LispPTR obj);
static unsigned short stringhash(LispPTR obj);

/****************************************************************/
/*                                                              */
/*                            SXHASH                            */
/*								*/
/*         C-coded version of the hashing function SXHASH       */
/*								*/
/****************************************************************/

LispPTR SX_hash(LispPTR object) {
    return (S_POSITIVE | (0xFFFF & (sxhash(object))));
  /* Smash the top of the stack to a 0xe, offset */
}

/*****************************************************************/
/*                             sxhash                            */
/*                                                               */
/* Internal function, called from SXHASH, and used for recursive */
/*     calls, e.g., for hashing lists and compound objects.      */
/* Fails to handle ratios, complex's, bitvectors pathnames & odd */
/* cases */
/*****************************************************************/
static unsigned short sxhash(LispPTR obj) {
  /* unsigned short hashOffset; Not Used */
  unsigned int cell;
  OneDArray *str;
  switch (SEGMASK & obj) {
    case S_POSITIVE:
    case S_NEGATIVE: return (obj & 0xFFFF);
    default:
      switch (GetTypeNumber(obj)) {
        case TYPE_FIXP: return ((FIXP_VALUE(obj)) & 0xFFFF);
        case TYPE_FLOATP:
          cell = (unsigned int)FIXP_VALUE(obj);
          return ((cell & 0xFFFF) ^ (cell >> 16));
#ifdef BIGATOMS
        case TYPE_NEWATOM: /* as for LITATOM... */
#endif                     /* BIGATOMS */

        case TYPE_LITATOM: return (EQHASHINGBITS(obj));
        case TYPE_LISTP: return (sxhash_list(obj));
        case TYPE_PATHNAME: return (sxhash_pathname(obj));
        case TYPE_ONED_ARRAY:
        case TYPE_GENERAL_ARRAY:
          str = (OneDArray *)NativeAligned4FromLAddr(obj);
          if (str->stringp) return (sxhash_string(str));
          if (str->bitp) return (sxhash_bitvec(str));
          return (EQHASHINGBITS(obj));
        case TYPE_BIGNUM: {
          LispPTR contents;
          contents = ((BIGNUM *)NativeAligned4FromLAddr(obj))->contents;
          return ((unsigned short)car(contents) + (((unsigned short)car(cdr(contents))) << 12));
        }

        case TYPE_COMPLEX: {
          COMPLEX *object;
          object = (COMPLEX *)NativeAligned4FromLAddr(obj);
          return (sxhash(object->real) ^ sxhash(object->imaginary));
        }
        case TYPE_RATIO: {
          RATIO *object;
          object = (RATIO *)NativeAligned4FromLAddr(obj);
          return (sxhash(object->numerator) ^ sxhash(object->denominator));
        }

        default: return (EQHASHINGBITS(obj));
      }
  }
}

/* Rotates the 16-bit work to the left 7 bits (or to the right 9 bits) */
static unsigned short sxhash_rotate(short unsigned int value) {
  return ((value << 7) | ((value >> 9) & 0x7f));
}

static unsigned short sxhash_string(OneDArray *obj) {
  unsigned len, offset;
  unsigned short hash = 0;
  len = (unsigned)obj->fillpointer;
  if (len > 13) len = 13;
  offset = (unsigned)obj->offset;
  switch (obj->typenumber) {
    case THIN_CHAR_TYPENUMBER: {
      char *thin;
      unsigned i;
      thin = ((char *)(NativeAligned2FromLAddr(obj->base))) + offset;
      for (i = 0; i < len; i++) hash = sxhash_rotate(hash ^ GETBYTE(thin++));
    } break;
    case FAT_CHAR_TYPENUMBER: {
      unsigned short *fat;
      unsigned i;
      fat = ((unsigned short *)(NativeAligned2FromLAddr(obj->base))) + offset;
      for (i = 0; i < len; i++) hash = sxhash_rotate(hash ^ GETWORD(fat++));
    } break;
    default: error("SXHASH of a string not made of chars!\n");
  }
  return (hash);
}

static unsigned short sxhash_bitvec(OneDArray *obj) {
  unsigned short *base;
  unsigned len, offset, bitoffset;
  unsigned short hash = 0;
  len = (unsigned)obj->fillpointer;
  offset = (unsigned)obj->offset;
  base = ((unsigned short *)(NativeAligned2FromLAddr(obj->base))) + (offset >> 4);
  if (offset == 0) {
    hash = (*base);
    if (len < 16) hash = hash >> (16 - len);
  } else {
    bitoffset = offset & 15;
    hash = (GETWORD(base++) << (bitoffset));
    hash |= (GETWORD(base) >> (16 - bitoffset));
    if ((len - offset) < 16) hash = hash >> (16 - (len - offset));
  }
  return (hash);
}

static unsigned short sxhash_list(LispPTR obj) {
  unsigned short hash = 0;
  int counter;
  for (counter = 0; (counter < 13) && (GetTypeNumber(obj) == TYPE_LISTP); counter++) {
    hash = sxhash_rotate(hash ^ sxhash(car(obj)));
    obj = cdr(obj);
  }
  return (hash);
}

static unsigned short sxhash_pathname(LispPTR obj) {
  unsigned short hash = 0;
  PATHNAME *path;
  path = (PATHNAME *)(NativeAligned4FromLAddr(obj));
  hash = sxhash_rotate(sxhash(path->host) ^ sxhash(path->device));
  hash = sxhash_rotate(hash ^ sxhash(path->type));
  hash = sxhash_rotate(hash ^ sxhash(path->version));
  hash = sxhash_rotate(hash ^ sxhash(path->directory));
  hash = sxhash_rotate(hash ^ sxhash(path->name));
  return (hash);
}

/****************************************************************/
/* 								*/
/*                     STRING-EQUAL-HASHBITS       		*/
/*								*/
/*	C-coded version of the hashing function 		*/
/*	STRING-EQUAL-HASHBITS in LLARRAYELT.			*/
/* 								*/
/****************************************************************/

LispPTR STRING_EQUAL_HASHBITS(LispPTR object) {
  return (S_POSITIVE | (0xFFFF & (stringequalhash(object))));
} /* STRING_EQUAL_HASHBITS */

static unsigned short stringequalhash(LispPTR obj) {
  unsigned len, offset, fatp, ind;
  unsigned short hash = 0;
  PNCell *pnptr;
  DLword *base;
  PLCell *Prop;
  OneDArray *str;
  switch (GetTypeNumber(obj)) {
#ifdef BIGATOMS
    case TYPE_NEWATOM: /* as for LITATOM; it's all in the macros below. */
#endif                 /* BIGATOMS */

    case TYPE_LITATOM:
      ind = ((int)obj) & POINTERMASK;
      pnptr = (PNCell *)GetPnameCell(ind);
      base = (DLword *)NativeAligned2FromLAddr(pnptr->pnamebase);
      Prop = (PLCell *)GetPropCell(ind);
      fatp = Prop->fatpnamep;
      offset = 1;
      len = GETBYTE((unsigned char *)base);
      break;
    case TYPE_ONED_ARRAY:
    case TYPE_GENERAL_ARRAY:
      str = (OneDArray *)NativeAligned4FromLAddr(obj);
      if (str->stringp) {
        fatp = (str->typenumber) == FAT_CHAR_TYPENUMBER;
        base = NativeAligned2FromLAddr(str->base);
        offset = str->offset;
        len = str->fillpointer;
      } else
        return (EQHASHINGBITS(obj));
      break;
    default: return (EQHASHINGBITS(obj));
  }

  if (fatp) {
    unsigned short *fat;
    unsigned i;
    fat = ((unsigned short *)base) + offset;
    for (i = 0; i < len; i++) {
      hash = hash + ((hash & 0xFFF) << 2);
      hash = hash + (0x20 | GETWORD(fat++)) + ((hash & 0xFF) << 8);
    }
  } else {
    char *thin;
    unsigned i;
    thin = ((char *)base) + offset;
    for (i = 0; i < len; i++) {
      hash = hash + ((hash & 0xFFF) << 2);
      hash = hash + (0x20 | GETBYTE(thin++)) + ((hash & 0xFF) << 8);
    }
  }
  return (hash);
}

/****************************************************************/
/* 								*/
/*                        STRING-HASHBITS         		*/
/*								*/
/*	C-coded version of the hashing function 		*/
/*	STRINGHASHBITS in LLARRAYELT.				*/
/* 								*/
/****************************************************************/

LispPTR STRING_HASHBITS(LispPTR object) {
  return (S_POSITIVE | (0xFFFF & (stringhash(object))));
} /* STRING_HASHBITS */

static unsigned short stringhash(LispPTR obj) {
  unsigned len, offset, fatp, ind;
  unsigned short hash = 0;
  PNCell *pnptr;
  DLword *base;
  PLCell *Prop;
  OneDArray *str;
  switch (GetTypeNumber(obj)) {
#ifdef BIGATOMS
    case TYPE_NEWATOM: /* as for LITATOM; it's all in the macros below. */
#endif                 /* BIGATOMS */

    case TYPE_LITATOM:
      ind = ((int)obj) & POINTERMASK;
      pnptr = (PNCell *)GetPnameCell(ind);
      base = (DLword *)NativeAligned2FromLAddr(pnptr->pnamebase);
      Prop = (PLCell *)GetPropCell(ind);
      fatp = Prop->fatpnamep;
      offset = 1;
      len = GETBYTE((unsigned char *)base);
      break;
    case TYPE_ONED_ARRAY:
    case TYPE_GENERAL_ARRAY:
      str = (OneDArray *)NativeAligned4FromLAddr(obj);
      if (str->stringp) {
        fatp = (str->typenumber) == FAT_CHAR_TYPENUMBER;
        base = NativeAligned2FromLAddr(str->base);
        offset = str->offset;
        len = str->fillpointer;
      } else
        return (EQHASHINGBITS(obj));
      break;
    default: return (EQHASHINGBITS(obj));
  } /* switch */

  if (fatp) {
    unsigned short *fat;
    unsigned i;
    fat = ((unsigned short *)base) + offset;
    for (i = 0; i < len; i++) {
      hash = hash + ((hash & 0xFFF) << 2);
      hash = hash + GETWORD(fat++) + ((hash & 0xFF) << 8);
    }
  } else {
    char *thin;
    unsigned i;
    thin = ((char *)base) + offset;
    for (i = 0; i < len; i++) {
      hash = hash + ((hash & 0xFFF) << 2);
      hash = hash + GETBYTE(thin++) + ((hash & 0xFF) << 8);
    }
  }
  return (hash);
} /* stringhash */
