/* $Id: mkatom.c,v 1.4 2001/12/24 01:09:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/**********************************************************************/
/*
        File Name :	makeatom.c

        Desc. :		Create atom

        Date :		January 29, 1987
        Edited by :	Takeshi Shimizu
        Change : create_symbol
                          take,30-Jun
                          1 May 1987 take
                          28 Aug. 1987 take

        Including :	make_atom
                        compute_hash
                        create_symbol
                        compare_chars
*/
/**********************************************************************/

#ifndef BYTESWAP
#include <string.h>      // for memcmp
#endif
#include "adr68k.h"      // for NativeAligned2FromLAddr
#include "cell.h"        // for PNCell, GetPnameCell
#include "dbprint.h"     // for DBPRINT
#include "lispemul.h"    // for DLword, LispPTR, T, NIL, POINTERMASK
#include "lspglob.h"     // for AtomHT
#include "lispmap.h"     // for S_POSITIVE
#include "lsptypes.h"    // for GETBYTE, GETWORD
#include "mkatomdefs.h"  // for compare_chars, compare_lisp_chars, compute_hash

#define ATOMoffset 2         /* NIL NOBIND  */
#define MAX_ATOMINDEX 0xffff /* max number of atoms */

#define Atom_reprobe(hash, char) ((((char)^(hash)) | 1) & 63)

/**********************************************************************/
/*
        Func name :	compute_hash

        Compute hash value from chars.
        THIS ONLY WORKS CORRECTLY ON EMULATOR STRINGS.
        Don't use it with strings in lisp-space.

        Date :		January 29, 1987
        Chan.		Aug. 27 87 take
        Edited by :	Takeshi Shimizu
*/
/**********************************************************************/

DLword compute_hash(const char *char_base, DLword offset, DLword length) {
  DLword hash;
  DLword number;
  DLword temp1;

  char_base += offset;
  hash = (DLword)((*char_base) << 8); /* get first byte */
  char_base++;                     /* skip length area */

  for (number = 1; number <= length - 1; char_base++, number++) {
    hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
    temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
    hash = (temp1 + (*char_base)) & 0x0ffff;
  }

  return (hash);

} /* end compute_hash */

/**********************************************************************/
/*
        Func name :	compute_lisp_hash

        Compute hash value from chars. WORKS ONLY ON LISP CHARS.

        Date :		January 29, 1987
        Chan.		Aug. 27 87 take
        Edited by :	Takeshi Shimizu
*/
/**********************************************************************/

DLword compute_lisp_hash(const char *char_base, DLword offset, DLword length, DLword fatp) {
  DLword hash;
  DLword number;
  DLword temp1;
  const DLword *word_base;

  if (length == 0) return (0);

  if (fatp) { /* fat characters in the string to be searched. */
    word_base = (const DLword *)char_base;
    word_base += offset;
    hash = (DLword)((0xFF & GETWORD(word_base)) << 8); /* get first byte */
    word_base++;                                     /* skip length area */

    for (number = 1; number <= length - 1; word_base++, number++) {
      hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
      temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
      hash = (temp1 + (0xFF & GETWORD(word_base))) & 0x0ffff;
    }
  } else {
    char_base += offset;
    hash = (DLword)((0xFF & GETBYTE(char_base)) << 8); /* get first byte */
    char_base++;                                  /* skip length area */

    for (number = 1; number <= length - 1; char_base++, number++) {
      hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
      temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
      hash = (temp1 + (0xFF & GETBYTE(char_base))) & 0x0ffff;
    }
  }
  return (hash);

} /* end compute_lisp_hash */

#ifdef BYTESWAP
static int bytecmp(const char *char1, const char *char2, int len)
{
  int index;
  for (index = 0; index < len; index++) {
      if (GETBYTE(char1++) != *(const uint8_t *)(char2++)) return (0);
  }
  return (1);
}
#endif /* BYTESWAP */

/**********************************************************************/
/*
        Func name :	compare_chars

        Compare two strings, char1, char2
                char1 -- in the LISP address space (& potentially
                         byte swapped!)
                char2 -- in emulator space, and obeying the "natural"
                         ordering of bytes in a string.

        Date :		January 29, 1987
        Edited by :	Takeshi Shimizu
        (why not call strncmp directly??)
        Because we need to account for byte ordering!! --JDS
        AND we need to compare NS chars (which have 0s in the string) correctly!
*/
/**********************************************************************/

LispPTR compare_chars(const char *char1, const char *char2, DLword length) {
#ifndef BYTESWAP
  if (memcmp(char1, char2, length) == 0)
#else
  if (bytecmp(char1, char2, length))
#endif /* BYTESWAP */

  {
    return (T);
  } else {
    return (NIL);
  }

} /* end compare_chars */

static int lispcmp(const DLword *char1, const char *char2, int len) {
  int index;
  for (index = 0; index < len; index++) {
      if (GETWORD(char1++) != GETBYTE(char2++)) return (0);
  }
  return (1);
}

/**********************************************************************/
/*
        Func name :	compare_lisp_chars

        Compare two strings, char1, char2
                in the LISP address space (& potentially
                 byte swapped!)

        Date :		January 29, 1987
        Edited by :	Takeshi Shimizu
        (why not call strncmp directly??)
        Because we need to account for byte ordering!! --JDS
        And for fat/thin differences.
        OFFSETs must be accounted for in the pointers already.
*/
/**********************************************************************/

LispPTR compare_lisp_chars(const char *char1, const char *char2, DLword length,
                           DLword fat1, DLword fat2) {
  if ((!fat1) == (!fat2)) { /* both fat or both non-fat. */
#ifdef BYTESWAP
    if (fat1) { /* both fat, so compare 'em a word at a time */
      int i;
      for (i = 0; i < length; i++) {
        if (GETWORD(char1) != GETWORD(char2)) return (NIL);
        /* increment by size of fat character, i.e., DLword, 2 bytes */
        char1+= sizeof(DLword); char2+=sizeof(DLword);
      }
      return (T);
    } else { /* both thin, so compare 'em a byte at a time */
             /* (it's this way in case we're byte-swapped.)*/
      int i;
      for (i = 0; i < length; i++)
        if (GETBYTE(char1++) != GETBYTE(char2++)) return (NIL);
      return (T);
    }
#else
    /* This one fails for byte-swapped machines */
    if (fat1) length = length + length;
    if (memcmp(char1, char2, length) == 0)
      return (T);
    else {
      return (NIL);
    }
#endif /* BYTESWAP */

  } else if (fat1) { /* char1 is fat, char2 isn't */
      if (lispcmp((const DLword *)char1, char2, length))
      return (T);
    else
      return (NIL);
  } else { /* char2 is fat, char1 isn't */
      if (lispcmp((const DLword *)char2, char1, length))
      return (T);
    else
      return (NIL);
  }

} /* end compare_lisp_chars */

/**********************************************************************/
/*
        Func name :	make_atom

        Look up the atom index of an existing atom, or return 0xFFFFFFFF

        This function is a subset of \MKATOM (in LLBASIC), but only handles
        thin text atom names (no numbers, no 2-byte pnames).
        It MUST return the same atom index number as \MKATOM

        Date :		January 29, 1987
        Edited by :	Takeshi Shimizu
        Changed : take 20-Jan
        Changed : March 27 '87  take
        Changed : May 1 '87 take
        Changed : May 9 '87 take
        Changed : May 13 '87 take
                May 15 '87 take
*/
/**********************************************************************/

LispPTR make_atom(const char *char_base, DLword offset, DLword length)
{
  DLword hash;
  LispPTR hash_entry; /* hash entry contents */
  DLword atom_index;
  DLword reprobe;

  PNCell *pnptr;
  char *pname_base;
  unsigned short first_char;

#ifdef TRACE2
  printf("TRACE: make_atom( %s , offset= %d, len= %d)\n", char_base, offset, length);
#endif

  first_char = (*(char_base + offset)) & 0xff;
  switch (length) {
  case 0:
    /* the zero-length atom has hashcode 0 */
    hash = 0;
    first_char = 255;
    break;

  case 1:
    /* One-character atoms live in well known places, no need to hash */
    if (first_char > '9')
      return ((LispPTR)(ATOMoffset + (first_char - 10)));
    if (first_char >= '0' ) /* 0..9 */
      return ((LispPTR)(S_POSITIVE + (first_char - '0')));
    /* other one character atoms */
    return ((LispPTR)(ATOMoffset + first_char));

  default:
    hash = compute_hash(char_base, offset, length);
    break;
  }

  /* This point corresponds with LP in Lisp source */

  /* following for loop does not exit until it finds new hash entry or same atom */
  for (reprobe = Atom_reprobe(hash, first_char); (hash_entry = GETWORD(AtomHT + hash)) != 0;
       hash = ((hash + reprobe) & 0xffff)) {
    atom_index = (DLword)(hash_entry - 1);
    /* get pname pointer */
    pnptr = (PNCell *)GetPnameCell(atom_index);
    pname_base = (char *)NativeAligned2FromLAddr(POINTERMASK & pnptr->pnamebase);

    if ((length == GETBYTE(pname_base)) &&
        (compare_chars(++pname_base, char_base + offset, length) == T)) {
      DBPRINT(("FOUND the atom. \n"));
      return (atom_index); /* found existing atom */
    }
    DBPRINT(("HASH doesn't hit. reprobe!\n"));

  } /* for end */

  /* we can't find that atom, then we should make new atom */
  DBPRINT(("HASH NEVER HIT.  Returning -1.\n"));
  return (0xffffffff);
  /** Don't create newatom now **/
} /* make_atom end */
