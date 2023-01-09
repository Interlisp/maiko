/* $Id: kprint.c,v 1.2 1999/05/31 23:35:36 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>         // for printf
#include "address.h"       // for LOLOC
#include "adr68k.h"        // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#include "car-cdrdefs.h"   // for cdr, car
#include "emlglob.h"
#include "kprintdefs.h"    // for prindatum, print, print_NEWstring, print_fixp
#include "lispemul.h"      // for LispPTR, DLbyte, DLword, POINTERMASK, NIL
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"      // for NEWSTRINGP, GETBYTE, GetTypeNumber, TYPE_L...
#include "print.h"         // for DOUBLEQUOTE, RIGHT_PAREN, LEFT_PAREN, SPACE
#include "testtooldefs.h"  // for print_atomname

int PrintMaxLevel = 3;
int Printdepth = 0;
int PrintMaxLen = 10;
int PrintLen[20];

/************************************************************************/
/*									*/
/*			P R I N D A T U M				*/
/*									*/
/************************************************************************/

void prindatum(LispPTR x) {
  NEWSTRINGP *newstring;
  struct dtd *dtd_base;
  int typen;
  LispPTR typename;

  if (Printdepth >= PrintMaxLevel) {
    if (Printdepth == PrintMaxLevel) {
      if (GetTypeNumber(x) == TYPE_LISTP) {
        printf("(-)");
      } else
        printf("*");
    }
    return;
  }

  x = x & POINTERMASK;
  switch (typen = GetTypeNumber(x)) {
    case TYPE_LITATOM:
#ifdef BIGATOMS
    case TYPE_NEWATOM:
#endif /* BIGATOMS */
      print_atomname(x);
      break;
    case TYPE_LISTP:
      Printdepth++;
      PrintLen[Printdepth] = 0;
      printf("%c", LEFT_PAREN); /* print "(" */
    lp:
      if (PrintLen[Printdepth]++ > PrintMaxLen) {
        printf("%c", RIGHT_PAREN);
        Printdepth--;
        break;
      }
      prindatum(car(x));
      if (Listp(cdr(x)) == 0) { /* print dotted pair */
        if ((x = cdr(x)) != NIL) {
          printf(" . ");
          prindatum(x);
        }
      } else {
        printf("%c", SPACE);
        x = cdr(x);
        goto lp;
      }
      printf("%c", RIGHT_PAREN); /* print ")" */
      Printdepth--;
      break;

    case TYPE_SMALLP:
      if ((x & SEGMASK) == S_POSITIVE)
        printf("%d", LOLOC(x)); /* print positive smallp */
      else
        printf("%d", (LOLOC(x) | 0xffff0000)); /* print negative smallp */
      break;
    case TYPE_FIXP:
      print_fixp(x); /* print fixp  */
      break;
    case TYPE_FLOATP: print_floatp(x); break;
    case TYPE_STRINGP:
      print_string(x); /* print string */
      break;
    case TYPE_ONED_ARRAY:
    case TYPE_GENERAL_ARRAY:
      newstring = (NEWSTRINGP *)NativeAligned4FromLAddr(x);
      if (newstring->stringp) {
        print_NEWstring(x);
      }
      break;
    default: dtd_base = (struct dtd *)GetDTD(typen); printf("{");
#ifdef BIGVM
      if ((typename = dtd_base->dtd_name) != 0)
#else
      if ((typename = dtd_base->dtd_namelo + (dtd_base->dtd_namehi << 16)) != 0)
#endif
        print_atomname(typename);
      else
        printf("unknown");
      printf("}0x");
      printf("%x", x); /* print lisp address in hex */
  }
}

/************************************************************************/
/*									*/
/*				P R I N T				*/
/*									*/
/*	Equivalent to the Lisp function PRINT,  prints the object.	*/
/*									*/
/************************************************************************/

LispPTR print(LispPTR x) {
  Printdepth = 0;
  prindatum(x & POINTERMASK);
  /* printf("\n"); */ /* print CR */
  return (x);
}

/************************************************************************/
/*									*/
/*			   p r i n t _ s t r i n g			*/
/*									*/
/*	Print a Lisp string.						*/
/*									*/
/************************************************************************/

void print_string(LispPTR x) {
  struct stringp *string_point;
  DLword st_length;
  DLbyte *string_base;

  int i;

  string_point = (struct stringp *)NativeAligned4FromLAddr(x);
  st_length = string_point->length;
  string_base = (DLbyte *)NativeAligned2FromLAddr(string_point->base);

  printf("%c", DOUBLEQUOTE); /* print %" */

  for (i = 1; i <= st_length; i++) {
    printf("%c", GETBYTE(string_base));
    string_base++;
  }

  printf("%c", DOUBLEQUOTE); /* print %" */
}

/************************************************************************/
/*									*/
/*		      p r i n t _ N E W s t r i n g			*/
/*									*/
/*	Print a Lyric-style string (the Commonlisp-array kind), as	*/
/*	opposed to the older special-case STRINGP kind from Koto.	*/
/*									*/
/************************************************************************/

void print_NEWstring(LispPTR x) {
  NEWSTRINGP *string_point;
  DLword st_length;
  DLbyte *string_base;

  int i;

  string_point = (NEWSTRINGP *)NativeAligned4FromLAddr(x);
  st_length = string_point->fillpointer;
  string_base = (DLbyte *)NativeAligned2FromLAddr(string_point->base);
  string_base += string_point->offset;

  printf("%c", DOUBLEQUOTE); /* print %" */

  for (i = 0; i < st_length; i++) {
    printf("%c", GETBYTE(string_base));
    string_base++;
  }

  printf("%c", DOUBLEQUOTE); /* print %" */
}

/************************************************************************/
/*									*/
/*			p r i n t _ f i x p				*/
/*									*/
/*	Print a lisp integer (but not a bignum!)			*/
/*									*/
/************************************************************************/

void print_fixp(LispPTR x) {
  int *addr_fixp;

  addr_fixp = (int *)NativeAligned4FromLAddr(x);
  printf("%d", *addr_fixp);
}

/************************************************************************/
/*									*/
/*			p r i n t _ f l o a t p				*/
/*									*/
/*	Print a lisp floating-point value.				*/
/*									*/
/************************************************************************/

void print_floatp(LispPTR x) {
  float *addr_floatp;

  addr_floatp = (float *)NativeAligned4FromLAddr(x);
  printf("%f", *addr_floatp);
}
