/* $Id: byteswap.c,v 1.5 2002/01/02 08:15:16 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: byteswap.c,v 1.5 2002/01/02 08:15:16 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***************************************************************************/
/*                                                                         */
/*                               byteswap.c                             */
/*                                                                         */
/*         Support functions for byte-swapped architecture machines        */
/*                             (e.g., 80386's)                             */
/*                                                                         */
/***************************************************************************/

#include "hdw_conf.h"
#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "stack.h"

#include "byteswapdefs.h"

#if defined(ISC)
#include "inlnPS2.h"
#else

/****************************************************************/
/*                                                              */
/*                 swap halves of a single 4-byte word          */
/*                                                              */
/****************************************************************/
unsigned int swapx(unsigned int word) {
  return (((word >> 16) & 0xffff) | ((word & 0xffff) << 16));
}

/****************************************************************/
/*                                                              */
/*                 Byte-swap a single 2-byte word               */
/*                                                              */
/****************************************************************/
unsigned short byte_swap_word(unsigned short word) {
  return (((word >> 8) & 0xff) | ((word & 0xff) << 8));
}

/****************************************************************/
/*                                                              */
/*                   Word-swap a 2-word integer                 */
/*           Does NOT byte-swap the words themselves.           */
/*                                                              */
/****************************************************************/
/***
unsigned int word_swap_longword(word)
  unsigned int word;
  {
      return( ((word>>16)&0xffff)+((word&0xffff)<<16) );
  } ***/
#ifndef I386
#define word_swap_longword(word) (((word >> 16) & 0xffff) | ((word & 0xffff) << 16))
#endif
#endif /* !ISC */

/****************************************************************/
/*                                                              */
/*            Byte-swap a region wordcount words long           */
/*            This does NOT swap words in a long-word!          */
/*                                                              */
/****************************************************************/
void byte_swap_page(unsigned short *page, int wordcount) {
  int i;
  for (i = 0; i < wordcount; i++) { *(page + i) = byte_swap_word(*(page + i)); }
}

#ifndef GCC386
/****************************************************************/
/*                                                              */
/*     Byte- & word-swap a region wordcount long-words long     */
/*                                                              */
/****************************************************************/
void word_swap_page(unsigned short *page, int longwordcount) {
  register int i;
  register unsigned int *longpage;
  longpage = (unsigned int *)page;
  for (i = 0; i < (longwordcount + longwordcount); i++) {
    *(page + i) = byte_swap_word(*(page + i));
  }
  for (i = 0; i < longwordcount; i++) { *(longpage + i) = word_swap_longword(*(longpage + i)); }
}
#endif /* GCC386 */

/****************************************************************/
/*                                                              */
/*    		 Bit-reverse all the words in a region	  	*/
/*                                                              */
/****************************************************************/

unsigned char reversedbits[256] = {
    /* table of bytes with their bits reversed */
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,

    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,

    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,

    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,

    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,

    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,

    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,

    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,

    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,

    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,

    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,

    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,

    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,

    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,

    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,

    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

/*unsigned short reverse_bits(word)
  unsigned short word;
  {
    return ((reversedbits[(word>>8) & 0xFF] <<8) | reversedbits[word & 0xff]);
  }
******/
#define reverse_bits(word) ((reversedbits[((word) >> 8) & 0xFF] << 8) | reversedbits[(word)&0xff])

void bit_reverse_region(unsigned short *top, int width, int height, int rasterwidth) {
  register int i, j, wordwid = ((width + 31) >> 5) << 1;
  register unsigned short *word;

  for (i = 0; i < height; i++) {
    word = top;
    for (j = 0; j < wordwid; j++) { GETWORD(word + j) = reverse_bits(GETWORD(word + j)); }
    word_swap_page((unsigned short *)((UNSIGNED)word & 0xFFFFFFFE), (wordwid + 1) >> 1);
    top += rasterwidth;
  }
}

/************************************************************************/
/*									*/
/*		b y t e _ s w a p _ c o d e _ b l o c k			*/
/*									*/
/*	Byte-swap the opcodes in a piece of compiled code.  This 	*/
/*	can be used to make the compiled bytes be in machine-natural	*/
/*	order, so we can avoid pointer arithmetic on the PC in the	*/
/*	inner loop.  The performance effect isn't yet known (2/92)	*/
/*									*/
/************************************************************************/

#ifdef RESWAPPEDDCODESTREAM
unsigned int byte_swap_code_block(unsigned int *base) {
  UNSIGNED startpc, len;

  startpc = ((UNSIGNED)base) + ((struct fnhead *)base)->startpc;
  len = code_block_size(base);

  word_swap_page((unsigned short *)startpc, (len + 3) >> 2);

  return (UNSIGNED)base;

} /* end of byte_swap_code_block */
#endif /* RESWAPPEDCODESTREAM */
