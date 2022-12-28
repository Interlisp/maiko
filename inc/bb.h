#ifndef BB_H
#define BB_H 1
/* $Id: bb.h,v 1.2 1999/01/03 02:05:53 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	bb.h
		written by don charnley
						*/




/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1991 Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/



/*
		COMMENTS:

	This code tries to make very few assumptions about the
	underlying hardware, and some are required.  The following
	assumptions are made:
		memory addresses are "byte addresses"
		data is effectively 32 bits
		the memory is most efficient at 32-bit words
			(always 32-bit aligned)

	Left and right shift amounts are always less than 32 bits.
	(there is one exception, B_postloop_mask, which may have its
	shift count = 32, in which case it is not used)
	Right shifted data is masked if necessary to compensate for
	possible arithmetic shifts.  Arithmetic shifts are assumed as
	the default.

	The backwards bit of the control block is followed blindly,
	except that gray is always executed forwards.  Gray bricks
	are always 16 bits wide, and 1 to 16 bits high.  These
	assumptions are identical to those made in the D-machine
	microcode.
*/

/*	INDEX

	CONSTANTS
	op_repl_src
	op_fn_and
	op_fn_or
	op_fn_xor

	CONDITIONS
	aligned_loop		-- it's all 32-bit-word aligned.
	F_single_dst_word	-- All the dest bits lie in 1 word
	F_postloop_dst_word
	F_two_preloop_src
	F_src_preloop_normal
	F_src_word_in_postloop
	B_two_preloop_src
	B_src_preloop_normal
	B_single_dst_word	-- All the dest bits lie in 1 word
	B_postloop_dst_word
	B_src_word_in_postloop

	VARIABLES
	F_num_loop
	F_preloop_mask
	F_postloop_mask
	B_num_loop
	B_preloop_mask
	B_postloop_mask

	OTHER

	TRANSFER LOOP THINGS
	ForInner
	DestGetsF
	DestGetsB
	GetSrcF
	GetSrcB

	INIT
	some_init
	do_gray_init
	F_do_init
	B_do_init
	F_dst_init
	gray_src_init
	F_src_init
	B_dst_init
	B_src_init

	SETUPS
	do_src_gray_setup
	F_do_src_setup
	B_do_src_setup

	TRANSFERS
	do_gray_transfer
	F_do_transfer
	B_do_transfer
	do_partial_transfer

	POSTLOOP
	F_do_postloop_src_prep
	B_do_postloop_src_prep

	ADVANCES
	do_gray_advance
	F_do_advance
	B_do_advance
	do_src_gray_advance
	F_do_dst_advance
	F_do_src_advance
	B_do_src_advance
	B_do_dst_advance

	NAMED VARIABLES
	variables
*/

/*   CONSTANTS   */
#define	 op_repl_src	0
#define	 op_fn_and	1
#define	 op_fn_or	2
#define	 op_fn_xor	3

/*   CONDITIONS   */
#define  aligned_loop  src32lbit == dst32lbit
#define  F_single_dst_word  (dst32lbit + w) <= 32
#define  F_postloop_dst_word  dst32rbit != 31
#define  F_two_preloop_src  (src32lbit > dst32lbit) && ((src32lbit + w) > 32)
#define  F_src_preloop_normal  src32lbit <= dst32lbit
#define  F_src_word_in_postloop  src32rbit <= dst32rbit
#define  B_single_dst_word  (dst32lbit + w) <= 32
#define  B_two_preloop_src  (src32rbit < dst32rbit) && ((src32lbit + w) > (dst32lbit + 1))
#define  B_src_preloop_normal  src32rbit >= dst32rbit
#define  B_postloop_dst_word  dst32lbit != 0
#define  B_src_word_in_postloop  src32lbit >= dst32lbit

/*   VARIABLES   */
#define  F_num_loop  (((dst32lbit + w) >> 5) - 1)
#define  B_num_loop  ((w - dst32rbit - 1) > 0) ? ((w - dst32rbit - 1) >> 5) : 0
#define  F_preloop_mask  ((dst32lbit) ? (~(0xFFFFFFFF << (32 - dst32lbit))) : 0xFFFFFFFF)
#define  F_postloop_mask  0xFFFFFFFF << (31 - dst32rbit)
#define  B_preloop_mask  0xFFFFFFFF << (31 - dst32rbit)
#define  B_postloop_mask  ((dst32lbit) ? (~(0xFFFFFFFF << (32 - dst32lbit))) : 0xFFFFFFFF)

/*   OTHER   */
/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*   TRANSFER LOOP THINGS (assume cnt's value is not used!)  */
#define ForInner   for (cnt = dstnumL; --cnt >= 0; )

#define DestGetsF(FN)   *(dst32addr++) FN shS;
#define DestGetsB(FN)   *(dst32addr--) FN shS;


#define GetSrcF   newS = *(src32addr++);			\
  shS        = savedS | ((newS >> srcRshift)/* & srcRmask*/);	\
  savedS     = newS << srcLshift;


#define GetSrcCF  newS = *(src32addr++);			\
  shS        = ~(savedS | ((newS >> srcRshift) /* & srcRmask */));	\
  savedS     = newS << srcLshift;


#define GetSrcB  newS = *(src32addr--);				\
  shS    = savedS | (newS << srcLshift);			\
  savedS = (newS >> srcRshift) /* & srcRmask*/;


#define GetSrcCB newS = *(src32addr--);				\
  shS    = ~(savedS | (newS << srcLshift));			\
  savedS = (newS >> srcRshift)/* & srcRmask*/;



/************************************************************************/
/*									*/
/*			I N I T I A L I Z A T I O N S			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#define  some_init						\
 num_lines_remaining = h;					\
 fwd = !backwardflg;						\
 bb_fast = !(31 & (srcbpl | dstbpl));				\
 if (gray) {do_gray_init}					\
 else if (fwd) {F_do_init}					\
 else {B_do_init}


#define  do_gray_init						\
 F_dst_init							\
 gray_src_init


#define  F_do_init						\
 F_dst_init							\
 F_src_init


#define  B_do_init						\
 B_dst_init							\
 B_src_init


#define  F_dst_init						\
 if (dx < 0)							\
 {								\
   x32byta  = (UNSIGNED)dstbase - ((7 - dx) >> 3);		\
 }								\
 else								\
 {								\
   x32byta  = (UNSIGNED)dstbase + (dx >> 3);		\
 }								\
 x32nbyt  = x32byta & 3;					\
 x32ia  = x32byta - x32nbyt;					\
 dst32addr = (unsigned int *)x32ia;					\
 dst32lbit = (x32nbyt << 3) + (dx & 7);				\
 dst32rbit = 31 & (dst32lbit + w - 1);				\
 OrigDstAddr = dst32addr;					\
 preloop_mask = F_preloop_mask;					\
 postloop_mask = F_postloop_mask;				\
 sdw_mask = preloop_mask & postloop_mask;			\
 dstnumL = F_num_loop;


#define  gray_src_init						\
 bb_fast = !(dstbpl & 31);					\
 src32lbit = 15 & sx;


#define  F_src_init						\
 if (sx < 0)							\
 {								\
   x32byta  = (UNSIGNED)srcbase - ((7 - sx) >> 3);		\
 }								\
 else								\
 {								\
   x32byta  = (UNSIGNED)srcbase + (sx >> 3);		\
 }								\
 x32nbyt  = x32byta & 3;					\
 x32ia  = x32byta - x32nbyt;					\
 src32addr = (unsigned int *)x32ia;				\
 src32lbit = (x32nbyt << 3) + (sx & 7);				\
 src32rbit = 31 & (src32lbit + w - 1);				\
 srcRshift = 31 & (dst32lbit - src32lbit);			\
 srcLshift = 31 & (src32lbit - dst32lbit);			\
 srcRmask  = ((srcLshift) ? ~(0xFFFFFFFF << srcLshift) : 0xFFFFFFFF);	\
 OrigSrcAddr = src32addr;


#define  B_dst_init						\
 abc = dx + w - 1;						\
 if (abc < 0)							\
 {								\
   x32byta  = (UNSIGNED)dstbase - ((7 - abc) >> 3);			\
 }								\
 else								\
 {								\
   x32byta  = (UNSIGNED)dstbase + (abc >> 3);			\
 }								\
 x32nbyt  = x32byta & 3;					\
 x32ia  = x32byta - x32nbyt;					\
 dst32addr = (unsigned int *)x32ia;				\
 dst32rbit = (x32nbyt << 3) + (abc & 7);			\
 dst32lbit = 31 & (dst32rbit - w + 1);				\
 OrigDstAddr = dst32addr;					\
 preloop_mask = B_preloop_mask;					\
 postloop_mask = B_postloop_mask;				\
 sdw_mask = preloop_mask & postloop_mask;			\
 dstnumL = B_num_loop;


#define  B_src_init						\
 abc = sx + w - 1;						\
 if (abc < 0)							\
 {								\
   x32byta  = (UNSIGNED)srcbase - ((7 - abc) >> 3);			\
 }								\
 else								\
 {								\
   x32byta  = (UNSIGNED)srcbase + (abc >> 3);			\
 }								\
 x32nbyt  = x32byta & 3;					\
 x32ia  = x32byta - x32nbyt;					\
 src32addr = (unsigned int *)x32ia;					\
 src32rbit = (x32nbyt << 3) + (abc & 7);			\
 src32lbit = 31 & (src32rbit - w + 1);				\
 srcRshift = 31 & (dst32lbit - src32lbit);			\
 srcLshift = 31 & (src32lbit - dst32lbit);			\
 srcRmask  = ((srcLshift) ? ~(0xFFFFFFFF << srcLshift) : 0);	\
 OrigSrcAddr = src32addr;


/************************************************************************/
/*									*/
/*	  S O U R C E   &   D E S T I N A T I O N   S E T - U P S	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*   SETUPS   */

#define  do_src_gray_setup					\
 srcLshift = 15 & (src32lbit - dst32lbit);			\
 shS = *srcbase;						\
 shS |= (shS << 16);	/* replicate the word */		\
 shS <<= srcLshift;			/* rotate left */	\
 shS |= (0xFFFF & (shS >> 16));		/*    "     "  */	\
 if (src_comp) shS = ~shS;


#define  F_do_src_setup						\
 if (F_two_preloop_src)						\
 {								\
   newS = *(src32addr++);					\
   savedS = newS << srcLshift;					\
   newS = *(src32addr++);					\
   shS = savedS | ((newS >> srcRshift) & srcRmask);		\
   savedS = (newS << srcLshift) & ~srcRmask;			\
 }								\
 else if (F_src_preloop_normal)					\
 {								\
   newS = *(src32addr++);					\
   shS = ((newS >> srcRshift) & srcRmask);			\
   savedS = (newS << srcLshift) & ~srcRmask;			\
 }								\
 else								\
 {								\
   newS = *(src32addr++);					\
   shS = newS << srcLshift;					\
 }								\
 if (src_comp) shS = ~shS;


#define  B_do_src_setup						\
 if (B_two_preloop_src)						\
 {								\
   newS = *(src32addr--);					\
   savedS = (newS >> srcRshift) & srcRmask;			\
   newS = *(src32addr--);					\
   shS = savedS | (newS << srcLshift);				\
   savedS = (newS >> srcRshift) & srcRmask;			\
 }								\
 else if (B_src_preloop_normal)					\
 {								\
   newS = *(src32addr--);					\
   shS = newS << srcLshift;					\
   savedS = (newS >> srcRshift) & srcRmask;			\
 }								\
 else								\
 {								\
   newS = *(src32addr--);					\
   shS = (newS >> srcRshift) & srcRmask;			\
 }								\
 if (src_comp) shS = ~shS;



/************************************************************************/
/*									*/
/*			T R A N S F E R   L O O P S			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#define  do_gray_transfer					\
 if (F_single_dst_word)						\
 {								\
   mask = sdw_mask;						\
   goto do_fpt;							\
 }								\
 mask = preloop_mask;						\
 do_partial_transfer						\
 dst32addr++;							\
 switch (op)							\
 { int cnt;						\
   case op_repl_src: ForInner { DestGetsF(=)  } break;		\
   case op_fn_and:   ForInner { DestGetsF(&=) } break;		\
   case op_fn_or:    ForInner { DestGetsF(|=) } break;		\
   case op_fn_xor:   ForInner { DestGetsF(^=) } break;		\
 }								\
 if (F_postloop_dst_word)					\
 {								\
   mask = postloop_mask;					\
   goto do_fpt;							\
 }								\
 goto next_line;



#define  F_do_transfer						\
 if (F_single_dst_word)						\
 {								\
   mask = sdw_mask;						\
   goto do_fpt;							\
 }								\
 mask = preloop_mask;						\
 do_partial_transfer						\
 dst32addr++;							\
 if (aligned_loop)						\
 {								\
   if (src_comp) switch (op)					\
   { int cnt;							\
     case op_repl_src: ForInner {*dst32addr++  = ~*src32addr++;} break;	\
     case op_fn_and:   ForInner {*dst32addr++ &= ~*src32addr++;} break;	\
     case op_fn_or:    ForInner {*dst32addr++ |= ~*src32addr++;} break;	\
     case op_fn_xor:   ForInner {*dst32addr++ ^= ~*src32addr++;} break;	\
   }									\
   else switch (op)							\
   { int cnt;							\
     case op_repl_src: ForInner {*dst32addr++  = *src32addr++;} break;	\
     case op_fn_and:   ForInner {*dst32addr++ &= *src32addr++;} break;	\
     case op_fn_or:    ForInner {*dst32addr++ |= *src32addr++;} break;	\
     case op_fn_xor:   ForInner {*dst32addr++ ^= *src32addr++;} break;	\
   }									\
 }									\
 else									\
 {									\
   if (src_comp) switch (op)						\
   { int cnt;							\
     case op_repl_src: ForInner {GetSrcCF DestGetsF(=)  } break;	\
     case op_fn_and:   ForInner {GetSrcCF DestGetsF(&=) } break;	\
     case op_fn_or:    ForInner {GetSrcCF DestGetsF(|=) } break;	\
     case op_fn_xor:   ForInner {GetSrcCF DestGetsF(^=) } break;	\
   }									\
   else switch (op)							\
   { int cnt;							\
     case op_repl_src: ForInner {GetSrcF  DestGetsF(=)  } break;	\
     case op_fn_and:   ForInner {GetSrcF  DestGetsF(&=) } break;	\
     case op_fn_or:    ForInner {GetSrcF  DestGetsF(|=) } break;	\
     case op_fn_xor:   ForInner {GetSrcF  DestGetsF(^=) } break;	\
   }									\
 }								\
 if (F_postloop_dst_word)					\
 {								\
   F_do_postloop_src_prep					\
   mask = postloop_mask;					\
   goto do_fpt;							\
 }								\
 goto next_line;




#define  B_do_transfer						\
 if (B_single_dst_word)						\
 {								\
   mask = sdw_mask;						\
   goto do_fpt;							\
 }								\
 mask = preloop_mask;						\
 do_partial_transfer						\
 dst32addr--;							\
 if (aligned_loop)						\
 {								\
   if (src_comp) switch (op)					\
   { int cnt;							\
     case op_repl_src: ForInner {*dst32addr--  = ~*src32addr--;} break;	\
     case op_fn_and:   ForInner {*dst32addr-- &= ~*src32addr--;} break;	\
     case op_fn_or:    ForInner {*dst32addr-- |= ~*src32addr--;} break;	\
     case op_fn_xor:   ForInner {*dst32addr-- ^= ~*src32addr--;} break;	\
   }									\
   else switch (op)							\
   { int cnt;							\
     case op_repl_src: ForInner {*dst32addr--  = *src32addr--;} break;	\
     case op_fn_and:   ForInner {*dst32addr-- &= *src32addr--;} break;	\
     case op_fn_or:    ForInner {*dst32addr-- |= *src32addr--;} break;	\
     case op_fn_xor:   ForInner {*dst32addr-- ^= *src32addr--;} break;	\
   }									\
 }									\
 else									\
 {									\
   if (src_comp) switch (op)						\
   { int cnt;							\
     case op_repl_src: ForInner {GetSrcCB DestGetsB(=)  } break;	\
     case op_fn_and:   ForInner {GetSrcCB DestGetsB(&=) } break;	\
     case op_fn_or:    ForInner {GetSrcCB DestGetsB(|=) } break;	\
     case op_fn_xor:   ForInner {GetSrcCB DestGetsB(^=) } break;	\
   }									\
   else switch (op)							\
   { int cnt;							\
     case op_repl_src: ForInner {GetSrcB  DestGetsB(=)  } break;	\
     case op_fn_and:   ForInner {GetSrcB  DestGetsB(&=) } break;	\
     case op_fn_or:    ForInner {GetSrcB  DestGetsB(|=) } break;	\
     case op_fn_xor:   ForInner {GetSrcB  DestGetsB(^=) } break;	\
   }									\
 }								\
 if (B_postloop_dst_word)					\
 {								\
   B_do_postloop_src_prep					\
   mask = postloop_mask;					\
   goto do_fpt;							\
 }								\
 goto next_line;




#define  do_partial_transfer					\
 dstdata = *dst32addr;						\
 dstold = dstdata & ~mask;					\
 switch (op)							\
 {								\
   case op_repl_src: dstdata  = shS; break;			\
   case op_fn_and:   dstdata &= shS; break;			\
   case op_fn_or:    dstdata |= shS; break;			\
   case op_fn_xor:   dstdata ^= shS; break;			\
 }								\
 dstdata &= mask;						\
 dstdata |= dstold;						\
 *dst32addr = dstdata;



/************************************************************************/
/*									*/
/*			P O S T - L O O P  C O D E			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/


#define F_do_postloop_src_prep					\
 if (F_src_word_in_postloop)					\
 {								\
   newS = *src32addr;						\
   shS = savedS | ((newS >> srcRshift) & srcRmask);		\
 }								\
 else								\
 {								\
   shS = savedS;						\
 }								\
 if (src_comp) shS = ~shS;



#define B_do_postloop_src_prep					\
 if (B_src_word_in_postloop)					\
 {								\
   newS = *src32addr;						\
   shS = savedS | (newS << srcLshift);				\
 }								\
 else								\
 {								\
   shS = savedS;						\
 }								\
 if (src_comp) shS = ~shS;




/************************************************************************/
/*									*/
/*	    L O O P - C O U N T E R   A D V A N C E   C O D E		*/
/*									*/
/*									*/
/*									*/
/************************************************************************/


#define  do_gray_advance					\
 F_do_dst_advance						\
 do_src_gray_advance

#define  F_do_advance						\
 F_do_dst_advance						\
 F_do_src_advance

#define  B_do_advance						\
 B_do_dst_advance						\
 B_do_src_advance

#define F_do_dst_advance					\
 if (bb_fast)							\
 {								\
   OrigDstAddr += dstbpl >> 5;				\
   dst32addr = OrigDstAddr;					\
 }								\
 else								\
 {								\
   dst32addr = OrigDstAddr;					\
   dst32lbit += dstbpl;						\
   dst32addr += dst32lbit >> 5;					\
   dst32lbit &= 31;						\
   dst32rbit = 31 & (dst32lbit + w - 1);			\
   OrigDstAddr = dst32addr;					\
   preloop_mask = F_preloop_mask;				\
   postloop_mask = F_postloop_mask;				\
   sdw_mask = preloop_mask & postloop_mask;			\
   dstnumL = F_num_loop;					\
 }

#define do_src_gray_advance					\
 if (++curr_gray_line >= num_gray)				\
 {								\
   curr_gray_line = 0;						\
   srcbase = srcbase - (num_gray - 1);				\
 }								\
 else ++srcbase;

#define F_do_src_advance					\
 if (bb_fast)							\
 {								\
   OrigSrcAddr += srcbpl >> 5;				\
   src32addr = OrigSrcAddr;					\
 }								\
 else								\
 {								\
   src32addr = OrigSrcAddr;					\
   src32lbit += srcbpl;						\
   src32addr += src32lbit >> 5;					\
   src32lbit &= 31;						\
   src32rbit = 31 & (src32lbit + w - 1);			\
   OrigSrcAddr = src32addr;					\
   srcRshift = 31 & (dst32lbit - src32lbit);			\
   srcLshift = 31 & (src32lbit - dst32lbit);			\
   srcRmask  = ((srcLshift) ? ~(0xFFFFFFFF << srcLshift) : 0xFFFFFFFF);	\
 }

#define B_do_src_advance					\
 if (bb_fast)							\
 {								\
   OrigSrcAddr += srcbpl >> 5;				\
   src32addr = OrigSrcAddr;					\
 }								\
 else								\
 {								\
   src32addr = OrigSrcAddr;					\
   src32rbit += srcbpl;						\
   if (src32rbit < 0)						\
   {								\
     src32addr -= (31 - src32rbit) >> 5;					\
   }								\
   else								\
   {								\
     src32addr += src32rbit >> 5;					\
   }								\
   src32rbit &= 31;						\
   src32lbit = 31 & (src32rbit - w + 1);			\
   srcRshift = 31 & (dst32lbit - src32lbit);			\
   srcLshift = 31 & (src32lbit - dst32lbit);			\
   srcRmask  = ((srcLshift) ? ~(0xFFFFFFFF << srcLshift) : 0);	\
   OrigSrcAddr = src32addr;					\
 }

#define B_do_dst_advance					\
 if (bb_fast)							\
 {								\
   OrigDstAddr += dstbpl >> 5;					\
   dst32addr = OrigDstAddr;					\
 }								\
 else								\
 {								\
   dst32addr = OrigDstAddr;					\
   dst32rbit += dstbpl;						\
   if (dst32rbit < 0)						\
   {								\
     dst32addr -= (31 - dst32rbit) >> 5;			\
   }								\
   else								\
   {								\
     dst32addr += dst32lbit >> 5;				\
   }								\
   dst32rbit &= 31;						\
   dst32lbit = 31 & (dst32rbit - w + 1);			\
   OrigDstAddr = dst32addr;					\
   preloop_mask = B_preloop_mask;				\
   postloop_mask = B_postloop_mask;				\
   sdw_mask = preloop_mask & postloop_mask;			\
   dstnumL = B_num_loop;					\
 }



/************************************************************************/
/*									*/
/*		V A R I A B L E   D E C L A R A T I O N S		*/
/*									*/
/*	This sets up the strictly-internal variables for bitblt.	*/
/*									*/
/*	However, YOU must set up the control variables that are used	*/
/*	as "arguments" to the bitblt code:				*/
/*									*/
/*	DLword	*srcbase, *dstbase;				*/
/*	int	sx, dx, w, h, srcbpl, dstbpl, backwardflg;		*/
/*	int	src_comp, op, gray, num_gray, curr_gray_line;		*/
/*									*/
/************************************************************************/

#define variables						\
int num_lines_remaining = 0;					\
int dstnumL = 0, src32lbit = 0, srcLshift = 0, dst32lbit = 0;	\
unsigned int srcRmask = 0, dstold = 0, dstdata = 0, mask = 0;	\
UNSIGNED x32byta = 0, x32nbyt = 0, x32ia = 0;			\
int abc, dst32rbit = 0, src32rbit = 0, fwd;			\
unsigned int *OrigSrcAddr = 0, *OrigDstAddr = 0;		\
int bb_fast;							\
unsigned int preloop_mask, postloop_mask, sdw_mask;		\
unsigned int *dst32addr = 0, *src32addr = 0;			\
unsigned int shS = 0, savedS = 0, newS = 0;			\
int srcRshift = 0;



/************************************************************************/
/*									*/
/*			n e w _ b i t b l t _ c o d e			*/
/*									*/
/*	Generic bitblt-code macro; generates bitblt code for the	*/
/*	general cases.  Requires a number of symbols be defined		*/
/*	by the calling code, in lieu of arguments:			*/
/*									*/
/*	srcbase								*/
/*	dstbase								*/
/*	srcbpl								*/
/*	dstbpl								*/
/*	backwardflg							*/
/*	sx, dx								*/
/*	w, h								*/
/*									*/
/************************************************************************/

#define  new_bitblt_code			\
do {						\
variables					\
some_init					\
while (num_lines_remaining-- > 0)		\
{ /* begin line loop */				\
  if (gray)					\
  {						\
    do_src_gray_setup				\
    do_gray_transfer				\
  }						\
  if (fwd)					\
  {						\
    F_do_src_setup				\
    F_do_transfer				\
  }						\
  {						\
    B_do_src_setup				\
    B_do_transfer				\
  }						\
do_fpt:						\
  {						\
    do_partial_transfer				\
    goto next_line;				\
  }						\
next_line:					\
  if (gray)					\
  {						\
    do_gray_advance				\
    continue;					\
  }						\
  if (fwd)					\
  {						\
    F_do_advance				\
    continue;					\
  }						\
  {						\
    B_do_advance				\
    continue;					\
  }						\
} /* end line loop */				\
} while (0)



/************************************************************************/
/*									*/
/*		n e w _ g r a y _ b i t b l t _ c o d e			*/
/*									*/
/*	Handles texture case of bitblt, for BLTSHADE functions.		*/
/*									*/
/************************************************************************/

#define  new_gray_bitblt_code			\
do {						\
variables					\
some_init					\
while (num_lines_remaining-- > 0)		\
{ /* begin line loop */				\
  do_src_gray_setup				\
  do_gray_transfer				\
do_fpt:						\
  do_partial_transfer				\
next_line:					\
  do_gray_advance				\
} /* end line loop */				\
} while (0)



/************************************************************************/
/*									*/
/*		 n e w _ c h a r _ b i t b l t _ c o d e		*/
/*									*/
/*	Optimized slightly for bltchar.					*/
/*									*/
/************************************************************************/

#define  new_char_bitblt_code			\
do {						\
variables					\
some_init					\
while (num_lines_remaining-- > 0)		\
{ /* begin line loop */				\
  F_do_src_setup				\
  F_do_transfer					\
do_fpt:						\
  do_partial_transfer				\
next_line:					\
  F_do_advance					\
} /* end line loop */				\
} while (0)



#endif /* BB_H */
