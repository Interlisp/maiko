/* $Id: mkatom.c,v 1.4 2001/12/24 01:09:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: mkatom.c,v 1.4 2001/12/24 01:09:05 sybalsky Exp $ Copyright (C) Venue";

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
                        parse_number
*/
/**********************************************************************/

#include "lispemul.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "cell.h"
#include "dbprint.h"

#include "mkatomdefs.h"
#include "commondefs.h"
#include "mkcelldefs.h"

#define ATOMoffset 2         /* NIL NOBIND  */
#define MAX_ATOMINDEX 0xffff /* max number of atoms */

#define Atom_reprobe(hash, char) ((((char)^(hash)) | 1) & 63)

extern DLword *Lisp_world;

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

DLword compute_hash(char *char_base, DLword offset, DLword length) {
  DLword hash;
  DLword number;
  DLword temp1, temp2;
  DLword *word_base;
  char_base += offset;
  hash = (int)(*(char_base)) << 8; /* get first byte */
  char_base++;                     /* skip length area */

  for (number = 1; number <= length - 1; char_base++, number++) {
    hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
    temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
    hash = (int)(temp1 + (*(char_base))) & 0x0ffff;
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

DLword compute_lisp_hash(char *char_base, DLword offset, DLword length, DLword fatp) {
  DLword hash;
  DLword number;
  DLword temp1, temp2;
  DLword *word_base;

  if (length == 0) return (0);

  if (fatp) { /* fat characters in the string to be searched. */
    word_base = (DLword *)char_base;
    word_base += offset;
    hash = (DLword)(0xFF & GETWORD(word_base)) << 8; /* get first byte */
    word_base++;                                     /* skip length area */

    for (number = 1; number <= length - 1; word_base++, number++) {
      hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
      temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
      hash = (int)(temp1 + (0xFF & GETWORD(word_base))) & 0x0ffff;
    }
  } else {
    char_base += offset;
    hash = (int)(0xFF & GETBYTE(char_base)) << 8; /* get first byte */
    char_base++;                                  /* skip length area */

    for (number = 1; number <= length - 1; char_base++, number++) {
      hash = (hash + ((hash & 4095) << 2)) & 0x0ffff;
      temp1 = (hash + ((hash & 255) << 8)) & 0x0ffff;
      hash = (int)(temp1 + (0xFF & GETBYTE(char_base))) & 0x0ffff;
    }
  }
  return (hash);

} /* end compute_lisp_hash */

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

LispPTR compare_chars(register char *char1, register char *char2, register DLword length) {
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
#ifdef BYTESWAP
int bytecmp(char *char1, char *char2, int len)
{
  int index;
  for (index = 0; index < len; index++) {
    if (GETBYTE(char1++) != *(char2++)) return (0);
  }
  return (1);
}
#endif /* BYTESWAP */

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

LispPTR compare_lisp_chars(register char *char1, register char *char2, register DLword length,
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
      if (lispcmp((DLword *)char1, char2, length))
      return (T);
    else
      return (NIL);
  } else { /* char2 is fat, char1 isn't */
      if (lispcmp((DLword *)char2, char1, length))
      return (T);
    else
      return (NIL);
  }

} /* end compare_lisp_chars */

int lispcmp(DLword *char1, unsigned char *char2, int len) {
  int index;
  for (index = 0; index < len; index++) {
      if (GETWORD(char1++) != (unsigned char)GETBYTE(char2++)) return (0);
  }
  return (1);
}

/**********************************************************************/
/*
        Func name :	make_atom

        If the atom already existed then return
        else create new atom .  Returns the Atom's index.

        This function does not handle FAT pname's.

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

LispPTR make_atom(char *char_base, DLword offset, DLword length, short int non_numericp)
/* if it is NIL then these chars are treated as NUMBER */
{
  extern DLword *Spospspace;
  extern DLword *AtomHT;
  extern DLword *Pnamespace;
  extern DLword *AtomSpace;

  DLword hash;
  LispPTR hash_entry; /* hash entry contents */
  DLword atom_index;
  DLword reprobe;

  PNCell *pnptr;
  char *pname_base;
  unsigned short first_char;

#ifdef TRACE2
  printf("TRACE: make_atom( %s , offset= %d, len= %d, non_numericp = %d)\n", char_base, offset,
         length, non_numericp);
#endif

  first_char = (*(char_base + offset)) & 0xff;
  if (length != 0) {
    if (length == 1) /* one char. atoms */
    {
      if (first_char > 57) /* greater than '9 */
        return ((LispPTR)(ATOMoffset + (first_char - 10)));
      else if (first_char > 47) /* between '0 to '9 */
        return ((LispPTR)(S_POSITIVE + (first_char - 48)));
      /* fixed S_... mar-27-87 take */
      else /* other one char. atoms */
        return ((LispPTR)(ATOMoffset + first_char));
    } /* if(length==1.. end */
    else if ((non_numericp == NIL) && (first_char <= '9'))
    /* more than 10 arithmetic  aon + - mixed atom process */
    {
      if ((hash_entry = parse_number(char_base + offset, length)) != 0)
        return ((LispPTR)hash_entry); /* if NIL that means THE ATOM is +- mixed litatom */
                                      /* 15 may 87 take */
    }

    hash = compute_hash(char_base, offset, length);

  } /* if(lengt.. end */
  else {
    hash = 0;
    first_char = 255;
  }

  /* This point corresponds with LP in Lisp source */

  /* following for loop never exits until it finds new hash enty or same atom */
  for (reprobe = Atom_reprobe(hash, first_char); (hash_entry = GETWORD(AtomHT + hash)) != 0;
       hash = ((hash + reprobe) & 0xffff)) {
    atom_index = hash_entry - 1;
    /* get pname pointer */
    pnptr = (PNCell *)GetPnameCell(atom_index);
    pname_base = (char *)Addr68k_from_LADDR(POINTERMASK & pnptr->pnamebase);

    if ((length == GETBYTE(pname_base)) &&
        (compare_chars(++pname_base, char_base + offset, length) == T)) {
      DBPRINT(("FOUND the atom. \n"));
      return (atom_index); /* find already existed atom */
    }
    DBPRINT(("HASH doesn't hit. reprobe!\n"));

  } /* for end */

  /* we can't find that atom, then we should make new atom */
  DBPRINT(("HASH NEVER HIT.  Returning -1.\n"));
  return (0xffffffff);
  /** Don't create newatom now **/
} /* make_atom end */

/*********************************************************************/
/*
        Func name :	parse_number

        Desc	:	It can treat -65534 to 65535 integer
                        Returns SMALLP PTR
        Date :		1,May 1987 Take
                        15 May 87 take
*/
/*********************************************************************/

/* Assume this func. should be called with C string in "char_base" */
LispPTR parse_number(char *char_base, short int length) {
  register LispPTR sign_mask;
  register LispPTR val;
  register int radix;
  register int *cell68k;

#ifdef TRACE2
  printf("TRACE: parse_number()\n");
#endif

  /* Check for Radix 8(Q) postfixed ?? */
  if ((*(char_base + (length - 1))) == 'Q') {
    radix = 8;
    length--;
  } else
    radix = 10;

  /* Check for Sign */
  sign_mask = S_POSITIVE;

  if ((*(char_base) == '+') || (*(char_base) == '-')) {
    sign_mask = ((*char_base++) == '+') ? S_POSITIVE : S_NEGATIVE;
    length--;
  }

  for (val = 0; length > 0; length--) {
    if ((((*char_base)) < '0') || ('9' < ((*char_base)))) return (NIL);
    val = radix * val + (*char_base++) - '0';
  }

  if (val > 0xffffffff) error("parse_number : Overflow ...exceeded range of FIXP");

  if ((sign_mask == S_POSITIVE) && (val > 0xffff)) {
    cell68k = (int *)createcell68k(TYPE_FIXP);
    *cell68k = val;
    return (LADDR_from_68k(cell68k));
  } else if ((sign_mask == S_NEGATIVE) && (val > 0xffff)) {
    cell68k = (int *)createcell68k(TYPE_FIXP);
    *cell68k = ~val + 1;
    return (LADDR_from_68k(cell68k));
  }

  else if (sign_mask == S_NEGATIVE)
    return (sign_mask | (~((DLword)val) + 1));
  else {
    return (sign_mask | val);
  }
}
/* end parse_number */
