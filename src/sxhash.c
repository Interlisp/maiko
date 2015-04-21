/* $Id: sxhash.c,v 1.4 2001/12/24 01:09:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: sxhash.c,v 1.4 2001/12/24 01:09:06 sybalsky Exp $ Copyright (C) Venue";



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


#include <stdio.h>



#include "lispemul.h"
#include "lspglob.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "adr68k.h"
#include "address.h"
#include "stack.h"
#include "cell.h"
#include "array.h"

#include "arith.h"



/** Follows definition in LLARRAYELT: **/
#define EQHASHINGBITS(item)  ( (((item)>>16)&0xFFFF) ^ ( (((item)&0x1FFF)<<3) ^ (((item)>>9)& 0x7f) ) )

  


/****************************************************************/
/*                                                              */
/*                            SXHASH                            */
/*								*/
/*         C-coded version of the hashing function SXHASH       */
/*								*/
/****************************************************************/
typedef
   struct
    {
	LispPTR      object;
     } SXHASHARG;

LispPTR SX_hash (register SXHASHARG *args)
{
    return(S_POSITIVE | ( 0xFFFF & (sxhash(args->object))));
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
sxhash (LispPTR obj)
{
   /* unsigned short hashOffset; Not Used */
    unsigned int cell;
    unsigned typen;
    OneDArray* str;
    switch (SEGMASK & obj)
      {
	case S_POSITIVE:
	case S_NEGATIVE: return(obj & 0xFFFF);
	default: switch (typen=GetTypeNumber(obj))
		   {
		     case TYPE_FIXP:   return((FIXP_VALUE(obj)) & 0xFFFF);
		     case TYPE_FLOATP: cell = (unsigned int) FIXP_VALUE(obj);
				       return((cell&0xFFFF)^(cell>>16));
#ifdef BIGATOMS
		     case TYPE_NEWATOM: /* as for LITATOM... */
#endif /* BIGATOMS */

		     case TYPE_LITATOM: return(EQHASHINGBITS(obj));
		     case TYPE_LISTP: return(sxhash_list(obj));
		     case TYPE_PATHNAME: return(sxhash_pathname(obj));
		     case TYPE_ONED_ARRAY:
		     case TYPE_GENERAL_ARRAY: str = (OneDArray *)
						     Addr68k_from_LADDR(obj);
					      if (str->stringp)
						     return(sxhash_string(str));
					      if (str->bitp)
						     return(sxhash_bitvec(str));
					      return(EQHASHINGBITS(obj));
		     case TYPE_BIGNUM:
			{
			  LispPTR contents;
			  contents = ((BIGNUM *)Addr68k_from_LADDR(obj))
					  -> contents;
			  return ( (unsigned short) car(contents) + 
				 ( ((unsigned short) car(cdr(contents))) <<12));
			}

		     case TYPE_COMPLEX:	{
					  COMPLEX *object;
					  object = (COMPLEX *)
						     Addr68k_from_LADDR(obj);
					  return (sxhash(object->real)
						   ^ sxhash(object->imaginary));
					}
		     case TYPE_RATIO:	{
					  RATIO *object;
					  object = (RATIO *)
						     Addr68k_from_LADDR(obj);
					  return (sxhash(object->numerator) ^
						   sxhash(object->denominator));
					}


		     default: return(EQHASHINGBITS(obj));
		   }
      }
  }


#ifndef SUN3_OS3_OR_OS4_IL
/* Rotates the 16-bit work to the left 7 bits (or to the right 9 bits) */
short unsigned
sxhash_rotate(short unsigned int value)
{
    return ((value<<7) | ((value>>9) & 0x7f));
  }

#endif


sxhash_string(OneDArray *obj)
{
    unsigned i, len, offset;
    register unsigned short hash = 0;
    len = (unsigned)obj ->fillpointer;
    if (len > 13) len = 13;
    offset = (unsigned)obj -> offset;
    switch (obj -> typenumber)
      {
	case THIN_CHAR_TYPENUMBER: {
				     register char *thin;
				     register unsigned i;
				     thin = ((char *)
					     (Addr68k_from_LADDR(obj->base)))
						 + offset;
				    for (i=0;i<len;i++)
				      hash = sxhash_rotate(hash^GETBYTE(thin++));
				    }
				    break;
	case FAT_CHAR_TYPENUMBER:  {
				     register unsigned short *fat;
				     register unsigned i;
				     fat = ((unsigned short *)
					    (Addr68k_from_LADDR(obj->base)))
						 + offset;
				     for (i=0;i<len;i++)
				       hash = sxhash_rotate(hash^GETWORD(fat++));
				    }
				    break;
	default: error("SXHASH of a string not made of chars!\n");

      }
    return(hash);
  }

sxhash_bitvec(OneDArray *obj)
{
    unsigned short *base;
    unsigned i, len, offset, bitoffset;
    unsigned short hash = 0;
    len = (unsigned)obj -> fillpointer;
    offset = (unsigned)obj -> offset;
    base = ((unsigned short *)(Addr68k_from_LADDR(obj->base))) + (offset>>4);
    if (offset == 0)
      {
	hash = (*base);
	if (len<16) hash = hash >> (16-len);
      }
     else
      {
	bitoffset = offset & 15;
	hash = (GETWORD(base++)<<(bitoffset)) | (GETWORD(base)>>(16-bitoffset));
	if ((len-offset) < 16) hash = hash >>(16-(len-offset));
      }
    return(hash);
  }


sxhash_list(LispPTR obj)
{
    unsigned short hash = 0;
    int counter;
    for (counter = 0; (counter<13)&&(GetTypeNumber(obj)==TYPE_LISTP); counter++)
      {
	hash = sxhash_rotate(hash^sxhash(car(obj)));
	obj = cdr(obj);
      }
    return(hash);
  }

sxhash_pathname(LispPTR obj)
{
    unsigned short hash = 0;
    PATHNAME *path;
    path = (PATHNAME *)(Addr68k_from_LADDR(obj));
    hash = sxhash_rotate( sxhash(path->host) ^ sxhash(path->device));
    hash = sxhash_rotate(hash^sxhash(path->type));
    hash = sxhash_rotate(hash^sxhash(path->version));
    hash = sxhash_rotate(hash^sxhash(path->directory));
    hash = sxhash_rotate(hash^sxhash(path->name));
   return(hash);
  }

/****************************************************************/
/* 								*/
/*                     STRING-EQUAL-HASHBITS       		*/
/*								*/
/*	C-coded version of the hashing function 		*/
/*	STRING-EQUAL-HASHBITS in LLARRAYELT.			*/
/* 								*/
/****************************************************************/


LispPTR STRING_EQUAL_HASHBITS(SXHASHARG *args)
{
     return(S_POSITIVE | ( 0xFFFF & (stringequalhash(args->object))));
  } /* STRING_EQUAL_HASHBITS */

stringequalhash(LispPTR obj)
{
    unsigned i, len, offset, fatp, ind;
    register unsigned short hash = 0;
    PNCell *pnptr;
    DLword *base;
    PLCell *Prop;
    OneDArray *str;
    switch (GetTypeNumber(obj))
      {
#ifdef BIGATOMS
	case TYPE_NEWATOM:  /* as for LITATOM; it's all in the macros below. */
#endif /* BIGATOMS */

	case TYPE_LITATOM: ind = ((int)obj)&POINTERMASK;
			   pnptr = (PNCell *) GetPnameCell(ind);
			   base= (DLword *)Addr68k_from_LADDR(pnptr->pnamebase);
			   Prop = (PLCell *) GetPropCell(ind);
			   fatp = Prop->fatpnamep;
			   offset = 1;
			   len = GETBYTE((unsigned char *)base);
			   break;
	case TYPE_ONED_ARRAY:
	case TYPE_GENERAL_ARRAY: str = (OneDArray *)
				     Addr68k_from_LADDR(obj);
			      if (str->stringp)
				{
				  fatp=(str->typenumber) == FAT_CHAR_TYPENUMBER;
				  base = Addr68k_from_LADDR(str->base);
				  offset = str->offset;
				  len = str->fillpointer;
				}
			       else return(EQHASHINGBITS(obj));
			      break;
	default: return(EQHASHINGBITS(obj));
      };


    if (fatp)
     {
	register unsigned short *fat;
	register unsigned i;
	fat = ((unsigned short *) base) + offset;
	for (i=0;i<len;i++)
	  {
	    hash = hash + ((hash&0xFFF)<<2);
	    hash = hash + (0x20 | GETWORD(fat++)) + ((hash & 0xFF)<<8);
	  }
     }
    else
     {
	register char *thin;
	register unsigned i;
	thin = ((char *) base) + offset;
	for (i=0;i<len;i++)
	  {
	    hash = hash + ((hash&0xFFF)<<2);
	    hash = hash + (0x20 | GETBYTE(thin++)) + ((hash & 0xFF)<<8);
	  }
     }
    return(hash);
  }


/****************************************************************/
/* 								*/
/*                        STRING-HASHBITS         		*/
/*								*/
/*	C-coded version of the hashing function 		*/
/*	STRINGHASHBITS in LLARRAYELT.				*/
/* 								*/
/****************************************************************/


LispPTR STRING_HASHBITS(SXHASHARG *args)
{
     return(S_POSITIVE | ( 0xFFFF & (stringhash(args->object))));
  } /* STRING_HASHBITS */

stringhash(LispPTR obj)
{
    unsigned i, len, offset, fatp, ind;
    register unsigned short hash = 0;
    PNCell *pnptr;
    DLword *base;
    PLCell *Prop;
    OneDArray *str;
    switch (GetTypeNumber(obj))
      {
#ifdef BIGATOMS
	case TYPE_NEWATOM:  /* as for LITATOM; it's all in the macros below. */
#endif /* BIGATOMS */

	case TYPE_LITATOM: ind = ((int)obj)&POINTERMASK;
			   pnptr = (PNCell *) GetPnameCell(ind);
			   base= (DLword *)Addr68k_from_LADDR(pnptr->pnamebase);
			   Prop = (PLCell *) GetPropCell(ind);
			   fatp = Prop->fatpnamep;
			   offset = 1;
			   len = GETBYTE((unsigned char *) base);
			   break;
	case TYPE_ONED_ARRAY:
	case TYPE_GENERAL_ARRAY: str = (OneDArray *)
				     Addr68k_from_LADDR(obj);
			      if (str->stringp)
				{
				  fatp=(str->typenumber) == FAT_CHAR_TYPENUMBER;
				  base = Addr68k_from_LADDR(str->base);
				  offset = str->offset;
				  len = str->fillpointer;
				}
			       else return(EQHASHINGBITS(obj));
			      break;
	default: return(EQHASHINGBITS(obj));
      }; /* switch */


    if (fatp)
     {
	register unsigned short *fat;
	register unsigned i;
	fat = ((unsigned short *) base) + offset;
	for (i=0;i<len;i++)
	  {
	    hash = hash + ((hash&0xFFF)<<2);
	    hash = hash + GETWORD(fat++) + ((hash & 0xFF)<<8);
	  }
     }
    else
     {
	register char *thin;
	register unsigned i;
	thin = ((char *) base) + offset;
	for (i=0;i<len;i++)
	  {
	    hash = hash + ((hash&0xFFF)<<2);
	    hash = hash + GETBYTE(thin++) + ((hash & 0xFF)<<8);
	  }
     }
    return(hash);
  }  /* stringhash */
