! @(#) bbtSPARC.s Version 1.2 (4/22/92). copyright Venue & Fuji Xerox  #
!
!************************************************************************/
!*									*/
!*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
!*	Manufactured in the United States of America.			*/
!*									*/
!*	The contents of this file are proprietary information 		*/
!*	belonging to Venue, and are provided to you under license.	*/
!*	They may not be further distributed or disclosed to third	*/
!*	parties without the specific permission of Venue.		*/
!*									*/
!************************************************************************/
!	Argument stack offsets, for rational naming.
!
#define srcbase [%fp+68]
#define dstbase [%fp+72]
#define sx [%fp+76]
#define dx [%fp+80]
#define w [%fp+84]
#define height [%fp+88]
#define srcbpl [%fp+92]
#define dstbpl [%fp+96]
#define backwardflg [%fp+100]
#define src_comp [%fp+104]
#define op [%fp+108]
#define gray [%fp+112]
#define num_gray [%fp+116]
#define curr_gray_line [%fp+120]
!
!
!
#define num_lines_remaining [%fp-4]
#define fwd [%fp-8]
#define bb_fast [%fp-12]
#define temp 
#define dstnumL
#define src32lbit
#define srcLshift
#define dst32lbit
#define srcRmask
#define dstold
#define dstdata
#define mask
#define x32byta
#define x32nbyt
#define x32ia
#define abc
#define dst32rbit
#define src32rbit [%fp-32]
#define OrigSrcAddr [%fp-36]
#define OrigDstAddr [%fp-20]
#define preloop_mask
#define postloop_mask
#define sdw_mask [%fp-24]
#define dst32addr [%fp-16]
#define src32addr [%fp-28]
#define shS
#define savedS 
#define newS
#define srcRshift
!
!	Computed intermediate values
!
!		srcbpl_rshift_3	saved value of (srcbpl>>3)
!		dstbpl_rshift_3	saved value of (dstbpl>>3)
!		num_gray_1	saved value of (num_gray-1)
!		ffff		saved value of 0xFFFF
#define srcbpl_rshift_3 [%fp-52]
#define num_gray_1 [%fp-48]
#define dstbpl_rshift_3 [%fp-56]	
#define ffff [%fp-44]
!
!	Register Usage
!
!	%i0		(arg srcbase)
!	%i1 srcLshift	(arg dstbase)
!	%i2 srcRshift	(arg sx)
!	%i3 dx, newS	(arg dx)
!	%i4		(arg width)
!	%i5 bb_fast	(arg height)
!	%i6
!	%i7
!
!	%o0
!	%o1
!	%o2
!	%o3
!	%04
!	%o5
!	%o6
!	%o7
!
!	%g0 [always zero]
!	%g1
!	%g2
!	%g3
!	%g4
!	%g5
!	%g6
!	%g7
!
!	%l0 srcRmask
!	%l1 op (left shifted 2 by JDS as of 11/7/89)
!	%l2 postloop_mask
!	%l3 preloop_mask
!	%l4 dst32rbit
!	%l5 src32lbit
!	%l6 dst32lbit
!	%l7
!
!	Definitions for register assignments, to make code clear.
!	All are of the form R<varname>, to indicate the R<egister>
!	version of the variable, in case there is also a storage place
!	for it.
!
#define RsrcRmask %l0
#define Rop %l1
#define Rpostloop_mask %l2
#define Rpreloop_mask %l3
#define Rdst32rbit %l4
#define Rsrc32lbit %l5
#define Rdst32lbit %l6
#define RsrcRshift %i2
!
	.seg	"text"			! [internal]
	.proc	4
	.global	_bitblt
_bitblt:
!#PROLOGUE# 0
!#PROLOGUE# 1
	save	%sp,-120,%sp
	ld	backwardflg,%o0
	st	%i5,num_lines_remaining
	tst	%o0
	st	%i4,w
	bne	L77003
	st	%i0,srcbase
	mov	1,%o1
	b	L77004
	st	%o1,fwd
L77003:
	st	%g0,fwd
L77004:
	ld	dstbpl,%o3
	ld	srcbpl,%o2
	or	%o2,%o3,%o2
	andcc	%o2,31,%g0
	bne,a	L77007
	mov	0,%i5		! bb_fast = %i5
	mov	1,%i5
L77007:
	ld	gray,%o5
	st	%i5,bb_fast
	tst	%o5		! are we doing the gray case?
	be,a	LY53		! no => LY53
	ld	fwd,%o5
	tst	%i3		! dx
	bge,a	LY52
	sra	%i3,3,%o4
	mov	7,%o7
	sub	%o7,%i3,%o7
	sra	%o7,3,%o7
	b	LY12
	sub	%i1,%o7,%i1
LY52:					! [internal]
	add	%i1,%o4,%i1
LY12:					! [internal]
	and	%i1,3,Rdst32lbit
	sub	%i1,Rdst32lbit,%i1
	st	%i1,dst32addr
	ld	w,%i1
	ld	dst32addr,%o2
	sll	Rdst32lbit,3,Rdst32lbit
	and	%i3,7,%i3
	add	Rdst32lbit,%i3,Rdst32lbit	!dst32lbit
	add	Rdst32lbit,%i1,%i1
	sub	%i1,1,Rdst32rbit
	tst	Rdst32lbit
	st	%o2,OrigDstAddr	! OrigDstAddr?
	be	L77013
	and	Rdst32rbit,31,Rdst32rbit	! dst32rbit
	mov	32,%o3
	sub	%o3,Rdst32lbit,%o3
	mov	-1,%i3
	sll	%i3,%o3,%i3
	b	L77014
	xor	%i3,-1,%i3	! preloop_mask
L77013:
	mov	-1,%i3
L77014:
	ld	dstbpl,%o3
	mov	31,%o2
	sub	%o2,Rdst32rbit,%o2
	mov	-1,Rpostloop_mask
	sll	Rpostloop_mask,%o2,Rpostloop_mask	! postloop_mask
	mov	%i3,Rpreloop_mask
	and	Rpreloop_mask,Rpostloop_mask,%o0
	andcc	%o3,31,%g0
	sra	%i1,5,%l7
	st	%o0,sdw_mask	! sdw_mask?
	bne	L77016
	dec	%l7		! dstnumL seems to be in %l7.
	b	L77017
	mov	1,%i3
L77016:
	mov	0,%i3 ! bb_fast
L77017:
	st	%i3,bb_fast
	b	L77396
	and	%i2,15,Rsrc32lbit !srclbit
!
!	Loop-advance, for either the forward or backward case
!
LY53:					! [internal]
	tst	%o5		! going forward?
	be,a	LY51		! No.  => LY51
	ld	w,%o3
	tst	%i3
	bge,a	LY50
	sra	%i3,3,%o4
	mov	7,%o7
	sub	%o7,%i3,%o7
	sra	%o7,3,%o7
	b	LY13
	sub	%i1,%o7,%i1
LY50:					! [internal]
	add	%i1,%o4,%i1
LY13:					! [internal]
	and	%i1,3,Rdst32lbit
	sub	%i1,Rdst32lbit,%i1
	st	%i1,dst32addr
	ld	w,%i1
	ld	dst32addr,%o2
	sll	Rdst32lbit,3,Rdst32lbit
	and	%i3,7,%i3
	add	Rdst32lbit,%i3,Rdst32lbit
	add	Rdst32lbit,%i1,%i1
	sub	%i1,1,Rdst32rbit
	tst	Rdst32lbit
	st	%o2,OrigDstAddr
	be	L77024
	and	Rdst32rbit,31,Rdst32rbit
	mov	32,%o3
	sub	%o3,Rdst32lbit,%o3
	mov	-1,%i3
	sll	%i3,%o3,%i3
	b	L77025
	xor	%i3,-1,%i3
L77024:
	mov	-1,%i3
L77025:
	mov	31,%o2
	sub	%o2,Rdst32rbit,%o2
	mov	-1,Rpostloop_mask
	sll	Rpostloop_mask,%o2,Rpostloop_mask
	mov	%i3,Rpreloop_mask
	and	Rpreloop_mask,Rpostloop_mask,%o0
	tst	%i2
	sra	%i1,5,%l7
	st	%o0,sdw_mask
	bge	L77027
	dec	%l7
	ld	srcbase,%i3
	mov	7,%o3
	sub	%o3,%i2,%o3
	sra	%o3,3,%o3
	b	LY14
	sub	%i3,%o3,%i3
L77027:
	ld	srcbase,%i3
	sra	%i2,3,%o2
	add	%i3,%o2,%i3
LY14:					! [internal]
	ld	w,%o4
	and	%i3,3,Rsrc32lbit
	sub	%i3,Rsrc32lbit,%i3
	sll	Rsrc32lbit,3,Rsrc32lbit
	and	%i2,7,%i2
	add	Rsrc32lbit,%i2,Rsrc32lbit
	add	Rsrc32lbit,%o4,%o4
	sub	Rsrc32lbit,Rdst32lbit,%i1
	and	%i1,31,%i1
	dec	%o4
	and	%o4,31,%o4
	tst	%i1
	sub	Rdst32lbit,Rsrc32lbit,RsrcRshift
	st	%i3,src32addr
	st	%o4,src32rbit
	be	L77030
	and	RsrcRshift,31,RsrcRshift
	mov	-1,%i3
	sll	%i3,%i1,%i3
	b	L77031
	xor	%i3,-1,%i3
L77030:
	mov	-1,%i3
L77031:
	ld	src32addr,%o2
	mov	%i3,RsrcRmask
	b	L77396
	st	%o2,OrigSrcAddr
!
!
!	Loop advance for the blt-backward case.
!
LY51:					! [internal]
	add	%i3,%o3,%i3
	deccc	%i3
	bpos,a	LY49
	sra	%i3,3,%o4
	mov	7,%o7
	sub	%o7,%i3,%o7
	sra	%o7,3,%o7
	b	LY15
	sub	%i1,%o7,%i1
LY49:					! [internal]
	add	%i1,%o4,%i1
LY15:					! [internal]
	and	%i1,3,Rdst32rbit
	sub	%i1,Rdst32rbit,%i1
	sll	Rdst32rbit,3,Rdst32rbit
	and	%i3,7,%i3
	add	Rdst32rbit,%i3,Rdst32rbit
	sub	Rdst32rbit,%o3,Rdst32lbit
	inc	Rdst32lbit
	and	Rdst32lbit,31,Rdst32lbit
	tst	Rdst32lbit ! replaced by andCC
	st	%i1,OrigDstAddr
	mov	-1,Rpostloop_mask
	be	Ldstl0
	st	%i1,dst32addr
	mov	32,%o1
	sub	%o1,Rdst32lbit,%o1
	sll	Rpostloop_mask,%o1,Rpostloop_mask
	xor	Rpostloop_mask,-1,Rpostloop_mask
Ldstl0:	ld	w,%o1
	mov	31,%o3
	sub	%o3,Rdst32rbit,%o3
	sub	%o1,Rdst32rbit,%i3
	mov	-1,Rpreloop_mask
	sll	Rpreloop_mask,%o3,Rpreloop_mask
	dec	%i3
	tst	%i3
	and	Rpreloop_mask,Rpostloop_mask,%o0
	ble	L77037
	st	%o0,sdw_mask
	b	L77038
	sra	%i3,5,%i3
L77037:
	mov	0,%i3
L77038:
	mov	%i3,%l7
	add	%i2,%o1,%i3
	deccc	%i3
	bpos	LY48
	ld	srcbase,%i1
	mov	7,%o1
	sub	%o1,%i3,%o1
	sra	%o1,3,%o1
	b	LY16
	sub	%i1,%o1,%i1
LY48:					! [internal]
	sra	%i3,3,%o0
	add	%i1,%o0,%i1
LY16:					! [internal]
	ld	w,Rsrc32lbit
	and	%i1,3,%i2
	sub	%i1,%i2,%i1
	sll	%i2,3,%i2
	and	%i3,7,%i3
	add	%i2,%i3,%i2	!src32rbit
	st	%i2,src32rbit	! moved from below for clarity
	sub	%i2,Rsrc32lbit,Rsrc32lbit
	inc	Rsrc32lbit
	and	Rsrc32lbit,31,Rsrc32lbit
	st	%i1,src32addr
	sub	Rsrc32lbit,Rdst32lbit,%i1
	and	%i1,31,%i1
!	st	%i2,src32rbit
	sub	Rdst32lbit,Rsrc32lbit,RsrcRshift
	tst	%i1
	be	L77043
	and	RsrcRshift,31,RsrcRshift
	mov	-1,%i3
	sll	%i3,%i1,%i3
	b	L77044
	xor	%i3,-1,%i3
L77043:
	mov	0,%i3
L77044:
	ld	src32addr,%o1
	mov	%i3,RsrcRmask		! srcRmask
	st	%o1,OrigSrcAddr
L77396:
	ld	srcbpl,%o3
	ld	num_lines_remaining,%i5
	ld	num_gray,%o0
	ld	dstbpl,%o5
	ld	op,Rop
	sll	Rop,2,Rop	! op << 2, for jump dispatch
	sra	%o3,3,%o3
	st	%o3,srcbpl_rshift_3	! srcbpl >> 3, for later use.
	ld	num_lines_remaining,%o3
	dec	%o0
	sethi	%hi(0xffff),%o2
	or	%o2,%lo(0xffff),%o2	! [internal]
	sll	%o0,1,%o0
	dec	%o3
	sra	%o5,3,%o5
	tst	%i5
	st	%o2,ffff
	st	%o0,num_gray_1
	st	%o3,num_lines_remaining
	ble	LY1
	st	%o5,dstbpl_rshift_3
LY8:					! [internal]
	ld	gray,%o4
	tst	%o4
	be,a	LY47
	ld	fwd,%o3
	ld	srcbase,%i5
	ld	ffff,%o0
	lduh	[%i5],%i5
	sub	Rsrc32lbit,Rdst32lbit,%i1
	sll	%i5,16,%o3
	or	%i5,%o3,%i5
	ld	src_comp,%o3
	and	%i1,15,%i1
	sll	%i5,%i1,%i5
	srl	%i5,16,%o7
	and	%o7,%o0,%o7
	tst	%o3
	be	L77051
	or	%i5,%o7,%i5
	xor	%i5,-1,%i5
L77051:
	ld	w,%o5
	add	Rdst32lbit,%o5,%o5
	cmp	%o5,32
	bg,a	LY46
	ld	dst32addr,%i4
	b	L77305
	ld	sdw_mask,%i0
LY46:					! [internal]
!	cmp	Rop,3
	ld	[%i4],%i4
!	sll	Rop,2,%o0
!	bgu	L77060
	andn	%i4,Rpreloop_mask,%i0
	sethi	%hi(L2000000),%o1
	or	%o1,%lo(L2000000),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	b	L77060
L2000000:
	.word	L77054
	.word	L77055
	.word	L77056
	.word	L77057
L77054:
	mov	%i5,%i4
L77055:
	and	%i4,%i5,%i4
L77056:
	or	%i4,%i5,%i4
L77057:
	xor	%i4,%i5,%i4
L77060:
	ld	dst32addr,%o7		! o7 = dst32addr down to ~ LY45
	and	%i4,Rpreloop_mask,%i4
	or	%i4,%i0,%i4
	st	%i4,[%o7]
!	ld	dst32addr,%o7
!	cmp	Rop,3
	inc	4,%o7
!	sll	Rop,2,%o0
!	bgu	L77079
!	st	%o7,dst32addr
	sethi	%hi(L2000001),%o1
	or	%o1,%lo(L2000001),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i4
L2000001:
	.word	L77061
	.word	L77065
	.word	L77069
	.word	L77073
L77061:
!	mov	%l7,%i4
L77062:
	deccc	%i4
	bneg,a	LY45
	cmp	Rdst32rbit,31
!	ld	dst32addr,%o0
	st	%i5,[%o7]
!	ld	dst32addr,%o1
!	inc	4,%o1
	b	L77062
	inc	4,%o7
!	st	%o1,dst32addr
L77065:
!	mov	%l7,%i4
L77066:
	deccc	%i4
	bneg,a	LY45
	cmp	Rdst32rbit,31
	ld	[%o7],%o3
	and	%o3,%i5,%o3
	st	%o3,[%o7]
!	ld	dst32addr,%o7
!	inc	4,%o7
	b	L77066
	inc	4,%o7
!	st	%o7,dst32addr
L77069:
!	mov	%l7,%i4
L77070:
	deccc	%i4
	bneg,a	LY45
	cmp	Rdst32rbit,31
!	ld	dst32addr,%o0
	ld	[%o7],%o1
	or	%o1,%i5,%o1
	st	%o1,[%o7]
!	ld	dst32addr,%o4
!	inc	4,%o4
	b	L77070
	inc	4,%o7
!	st	%o4,dst32addr
L77073:
!	sub	%l7,1,%i4
!	tst	%i4
	deccc	%i4
	bl,a	LY45
	cmp	Rdst32rbit,31
!LY9:					! [internal]
!	ld	dst32addr,%o5		
LY9a:	deccc	%i4
	ld	[%o7],%o1
	xor	%o1,%i5,%o1
	st	%o1,[%o7]
!	ld	dst32addr,%o2
!	inc	4,%o2
	bpos	LY9a
	inc	4,%o7
!	st	%o5,dst32addr		
L77079:
	cmp	Rdst32rbit,31
LY45:					! [internal]
!	be,a	LY31
	be	LY31
	st	%o7,dst32addr		! end of dst32addr in o7
	ld	gray,%o2
	b	L77305
	mov	Rpostloop_mask,%i0
LY47:					! [internal]
	tst	%o3 !fwd
	be,a	LY44
	ld	src32rbit,%o7
	cmp	Rsrc32lbit,Rdst32lbit
	ble,a	LY43
	cmp	Rsrc32lbit,Rdst32lbit
	ld	w,%o4
	add	Rsrc32lbit,%o4,%o4
	cmp	%o4,32
	ble,a	LY43
	cmp	Rsrc32lbit,Rdst32lbit
	ld	src32addr,%o1
	ld	[%o1],%i3
	inc	4,%o1
!	st	%o1,src32addr
	ld	[%o1],%o3
	sll	%i3,%i1,%i3
	srl	%o3,RsrcRshift,%i5
	and	%i5,RsrcRmask,%i5
	or	%i3,%i5,%i5
	sll	%o3,%i1,%i3
	inc	4,%o1
	andn	%i3,RsrcRmask,%i3
	b	LY19
	st	%o1,src32addr
LY43:					! [internal]
	bg	LY42
	ld	src32addr,%o7
	ld	[%o7],%i3
	inc	4,%o7
	srl	%i3,RsrcRshift,%i5
	sll	%i3,%i1,%i3
	st	%o7,src32addr
	and	%i5,RsrcRmask,%i5
	b	LY19
	andn	%i3,RsrcRmask,%i3
LY42:					! [internal]
	ld	[%o7],%i5
	inc	4,%o7
	st	%o7,src32addr
	sll	%i5,%i1,%i5
LY19:					! [internal]
	ld	src_comp,%o1
	tst	%o1
	be,a	LY41
	ld	w,%o3
	xor	%i5,-1,%i5
	ld	w,%o3
LY41:					! [internal]
	add	Rdst32lbit,%o3,%o3
	cmp	%o3,32
	bg,a	LY40
	ld	dst32addr,%i4
	b	L77305
	ld	sdw_mask,%i0
LY40:					! [internal]
!	cmp	Rop,3
	ld	[%i4],%i4
!	sll	Rop,2,%o0
!	bgu	L77101
	andn	%i4,Rpreloop_mask,%i0
	sethi	%hi(L2000002),%o1
	or	%o1,%lo(L2000002),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	b	L77101
L2000002:
	.word	L77095
	.word	L77096
	.word	L77097
	.word	L77098
L77095:
!	b	L77101
	mov	%i5,%i4
L77096:
!	b	L77101
	and	%i4,%i5,%i4
L77097:
!	b	L77101
	or	%i4,%i5,%i4
L77098:
	xor	%i4,%i5,%i4
L77101:
	ld	dst32addr,%o4		! o4 = dst32addr
	and	%i4,Rpreloop_mask,%i4
	or	%i4,%i0,%i4
	st	%i4,[%o4]
!	ld	dst32addr,%o4
	cmp	Rsrc32lbit,Rdst32lbit
	inc	4,%o4
	bne	L77144
	st	%o4,dst32addr
	ld	src_comp,%o5
	tst	%o5
	bne	LY39
!	cmp	Rop,3
!	bgu	L77186
!	sll	Rop,2,%o0
	ld	src32addr,%o5		! o5 = src32addr
	sethi	%hi(L2000004),%o1
	or	%o1,%lo(L2000004),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i5
L2000004:
	.word	L77124
	.word	L77128
	.word	L77132
	.word	L77136
L77104:
!	mov	%l7,%i5
L77105:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o7
!	ld	dst32addr,%o4
	ld	[%o5],%o7
	xor	%o7,-1,%o7
	st	%o7,[%o4]
!	ld	src32addr,%o3
!	ld	dst32addr,%o4
	inc	4,%o4
!	inc	4,%o5
!	st	%o3,src32addr
	b	L77105
	inc	4,%o5
!	st	%o4,dst32addr
L77108:
!	mov	%l7,%i5
L77109:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o5
!	ld	dst32addr,%o0
	ld	[%o5],%o7
	ld	[%o4],%o1
	andn	%o1,%o7,%o1
	st	%o1,[%o4]
!	ld	src32addr,%o4
!	ld	dst32addr,%o5
	inc	4,%o4
!	inc	4,%o5
!	st	%o4,src32addr
	b	L77109
	inc	4,%o5
!	st	%o5,dst32addr
L77112:
!	mov	%l7,%i5
L77113:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o7
!	ld	dst32addr,%o1
	ld	[%o5],%o7
	ld	[%o4],%o2
	orn	%o2,%o7,%o2
	st	%o2,[%o4]
!	ld	src32addr,%o5
!	ld	dst32addr,%o7
	inc	4,%o5
!	inc	4,%o4
!	st	%o5,src32addr
	b	L77113
	inc	4,%o4
!	st	%o7,dst32addr
L77116:
!	mov	%l7,%i5
L77117:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o0
!	ld	dst32addr,%o2
	ld	[%o5],%o0
	ld	[%o4],%o3
	xnor	%o3,%o0,%o3
	st	%o3,[%o4]
!	ld	src32addr,%o7
!	ld	dst32addr,%o0
	inc	4,%o5
!	inc	4,%o4
!	st	%o7,src32addr
	b	L77117
	inc	4,%o4
!	st	%o0,dst32addr
LY39:					! [internal] o4 = dst32addr getting here
!	bgu	L77186
!	sll	Rop,2,%o0
!	ld	src32addr,%o5		! o5 = src32addr when getting to LY39
	sethi	%hi(L2000003),%o1
	or	%o1,%lo(L2000003),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i5
L2000003:
	.word	L77104
	.word	L77108
	.word	L77112
	.word	L77116
L77124:
!	mov	%l7,%i5
L77125:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o2
!	ld	dst32addr,%o1
	ld	[%o5],%o2
	st	%o2,[%o4]
!	ld	src32addr,%o4
!	ld	dst32addr,%o5
	inc	4,%o4
!	inc	4,%o5
!	st	%o4,src32addr
	b	L77125
	inc	4,%o5
!	st	%o5,dst32addr
L77128:
!	mov	%l7,%i5
L77129:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o7
!	ld	dst32addr,%o1
	ld	[%o5],%o7
	ld	[%o4],%o2
	and	%o2,%o7,%o2
	st	%o2,[%o4]
!	ld	src32addr,%o5
!	ld	dst32addr,%o7
	inc	4,%o5
!	inc	4,%o4
!	st	%o5,src32addr
	b	L77129
	inc	4,%o4
!	st	%o7,dst32addr
L77132:
!	mov	%l7,%i5
L77133:
	deccc	%i5
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o0
!	ld	dst32addr,%o2
	ld	[%o5],%o0
	ld	[%o4],%o3
	or	%o3,%o0,%o3
	st	%o3,[%o4]
!	ld	src32addr,%o7
!	ld	dst32addr,%o0
	inc	4,%o4
!	inc	4,%o5
!	st	%o7,src32addr
	b	L77133
	inc	4,%o5
!	st	%o0,dst32addr
L77136:
!	sub	%l7,1,%i5
!	tst	%i5
	deccc	%i5
	bl,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
LY10:					! [internal]
!	ld	src32addr,%o1
!	ld	dst32addr,%o3
	ld	[%o5],%o1
	ld	[%o4],%o2
	deccc	%i5
	xor	%o2,%o1,%o2
	st	%o2,[%o4]
!	ld	src32addr,%o0
!	ld	dst32addr,%o1
	inc	4,%o4
!	inc	4,%o5
!	st	%o0,src32addr
	bpos	LY10
	inc	4,%o5
!	st	%o1,dst32addr
L77186:
!	cmp	Rdst32rbit,31
	st	%o4,dst32addr
LY37:					! [internal]
	st	%o5,src32addr
	cmp	Rdst32rbit,31
	be,a	LY31
	ld	gray,%o2
	ld	src32rbit,%o5
	cmp	%o5,Rdst32rbit
	bg,a	L77190
	mov	%i3,%i5
	ld	src32addr,%i5
	ld	[%i5],%i5
	srl	%i5,RsrcRshift,%i5
	and	%i5,RsrcRmask,%i5
	b	L77190
	or	%i3,%i5,%i5
L77144:				! o4 = dst32addr when getting here.
	ld	src_comp,%o2
	tst	%o2
	bne	LY38
!	cmp	Rop,3
!	bgu	L77186
!	sll	Rop,2,%o0
	ld	src32addr,%o5		! o5 = src322addr
	sethi	%hi(L2000006),%o1
	or	%o1,%lo(L2000006),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i4
L2000006:
	.word	L77166
	.word	L77170
	.word	L77174
	.word	L77178
L77146:
!	mov	%l7,%i4
L77147:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o5
!	ld	dst32addr,%o3
	ld	[%o5],%o1
	inc	4,%o5
	srl	%o1,RsrcRshift,%o7
	or	%i3,%o7,%o7
	xor	%o7,-1,%o7
!	st	%o5,src32addr
	st	%o7,[%o4]
	sll	%o1,%i1,%i3
!	ld	dst32addr,%o4
!	inc	4,%o4
	b	L77147
	inc	4,%o4
!	st	%o4,dst32addr
L77150:
!	mov	%l7,%i4
L77151:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o0
!	ld	dst32addr,%o5
	ld	[%o5],%o7
	inc	4,%o5
	srl	%o7,RsrcRshift,%o1
	or	%i3,%o1,%o1
	sll	%o7,%i1,%i3
!	st	%o0,src32addr
	ld	[%o4],%o7
	xor	%o1,-1,%o1
	and	%o7,%o1,%o7
	st	%o7,[%o4]
!	ld	dst32addr,%o2
!	inc	4,%o2
	b	L77151
	inc	4,%o4
!	st	%o2,dst32addr
L77154:
!	mov	%l7,%i4
L77155:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o5
!	ld	dst32addr,%o3
	ld	[%o5],%o1
	inc	4,%o5
	srl	%o1,RsrcRshift,%o7
	or	%i3,%o7,%o7
	sll	%o1,%i1,%i3
!	st	%o5,src32addr
	ld	[%o4],%o1
	xor	%o7,-1,%o7
	or	%o1,%o7,%o1
	st	%o1,[%o4]
!	ld	dst32addr,%o0
!	inc	4,%o4
	b	L77155
	inc	4,%o4
!	st	%o0,dst32addr
L77158:
!	mov	%l7,%i4
L77159:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o3
!	ld	dst32addr,%o1
	ld	[%o5],%o2
	inc	4,%o5
	srl	%o2,RsrcRshift,%o1
	or	%i3,%o1,%o1
	sll	%o2,%i1,%i3
!	st	%o3,src32addr
	ld	[%o4],%o2
	xor	%o1,-1,%o1
	xor	%o2,%o1,%o2
	st	%o2,[%o4]
!	ld	dst32addr,%o5
!	inc	4,%o4
	b	L77159
	inc	4,%o4
!	st	%o5,dst32addr
LY38:					! [internal]
!	bgu	L77186
!	sll	Rop,2,%o0
	ld	src32addr,%o5
	sethi	%hi(L2000005),%o1
	or	%o1,%lo(L2000005),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i4
L2000005:
	.word	L77146
	.word	L77150
	.word	L77154
	.word	L77158
!	b	LY37
!	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
L77166:
!	mov	%l7,%i4
L77167:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o1
!	ld	dst32addr,%o5
	ld	[%o5],%o0
	inc	4,%o5
	srl	%o0,RsrcRshift,%o2
	or	%i3,%o2,%o2
!	st	%o1,src32addr
	st	%o2,[%o4]
!	ld	dst32addr,%o7
	sll	%o0,%i1,%i3
!	inc	4,%o4
	b	L77167
	inc	4,%o4
!	st	%o7,dst32addr
L77170:
!	mov	%l7,%i4
L77171:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o2
!	ld	dst32addr,%o7
	ld	[%o5],%o1
	inc	4,%o5
	srl	%o1,RsrcRshift,%o3
	or	%i3,%o3,%o3
!	st	%o2,src32addr
	ld	[%o4],%o0
	sll	%o1,%i1,%i3
	and	%o0,%o3,%o0
	st	%o0,[%o4]
!	ld	dst32addr,%o3
!	inc	4,%o4
	b	L77171
	inc	4,%o4
!	st	%o3,dst32addr
L77174:
!	mov	%l7,%i4
L77175:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o7
!	ld	dst32addr,%o3
	ld	[%o5],%o7
	inc	4,%o5
	srl	%o7,RsrcRshift,%o0
	or	%i3,%o0,%o0
!	st	%o7,src32addr
	ld	[%o4],%o1
	sll	%o7,%i1,%i3
	or	%o1,%o0,%o1
	st	%o1,[%o4]
!	ld	dst32addr,%o0
!	inc	4,%o4
	b	L77175
	inc	4,%o4
!	st	%o0,dst32addr
L77178:
!	mov	%l7,%i4
L77179:
	deccc	%i4
	bneg,a	LY37
	st	%o4,dst32addr
!	cmp	Rdst32rbit,31
!	ld	src32addr,%o3
!	ld	dst32addr,%o0
	ld	[%o5],%o2
	inc	4,%o5
	srl	%o2,RsrcRshift,%o3
	or	%i3,%o3,%o3
!	st	%o3,src32addr
	ld	[%o4],%o1
	sll	%o2,%i1,%i3
	xor	%o1,%o3,%o1
	st	%o1,[%o4]
!	ld	dst32addr,%o4
!	inc	4,%o4
	b	L77179
	inc	4,%o4
!	st	%o4,dst32addr
L77190:
	ld	src_comp,%o4
	b	LY17
	tst	%o4
LY44:					! [internal]
	cmp	%o7,Rdst32rbit
	bge,a	LY36
	cmp	%o7,Rdst32rbit
	ld	w,%o1
	add	Rdst32lbit,1,%o0
	add	Rsrc32lbit,%o1,%o1
	cmp	%o1,%o0
	ble,a	LY36
	cmp	%o7,Rdst32rbit
	ld	src32addr,%o3
	ld	[%o3],%i3
	dec	4,%o3
	st	%o3,src32addr
	ld	[%o3],%o2
	srl	%i3,RsrcRshift,%i3
	and	%i3,RsrcRmask,%i3
	sll	%o2,%i1,%i5
	or	%i3,%i5,%i5
	srl	%o2,RsrcRshift,%i3
	dec	4,%o3
	and	%i3,RsrcRmask,%i3
	b	LY21
	st	%o3,src32addr
LY36:					! [internal]
	bl,a	LY35
	ld	src32addr,%o3
	ld	src32addr,%o4
	ld	[%o4],%i3
	dec	4,%o4
	sll	%i3,%i1,%i5
	srl	%i3,RsrcRshift,%i3
	st	%o4,src32addr
	b	LY21
	and	%i3,RsrcRmask,%i3
LY35:					! [internal]
	ld	[%o3],%i5
	dec	4,%o3
	srl	%i5,RsrcRshift,%i5
	and	%i5,RsrcRmask,%i5
	st	%o3,src32addr
LY21:					! [internal]
	ld	src_comp,%o7
	tst	%o7
	be,a	LY34
	ld	w,%o1
	xor	%i5,-1,%i5
	ld	w,%o1
LY34:					! [internal]
	add	Rdst32lbit,%o1,%o1
	cmp	%o1,32
	bg,a	LY33
	ld	dst32addr,%i4
	b	L77305
	ld	sdw_mask,%i0
LY33:					! [internal]
!	cmp	Rop,3
	ld	[%i4],%i4
!	sll	Rop,2,%o0
!	bgu	L77212
	andn	%i4,Rpreloop_mask,%i0
	sethi	%hi(L2000007),%o1
	or	%o1,%lo(L2000007),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	b	L77212
L2000007:
	.word	L77206
	.word	L77207
	.word	L77208
	.word	L77209
L77206:
!	b	L77212
	mov	%i5,%i4
L77207:
!	b	L77212
	and	%i4,%i5,%i4
L77208:
!	b	L77212
	or	%i4,%i5,%i4
L77209:
	xor	%i4,%i5,%i4
L77212:
	ld	dst32addr,%o4			! o4 = dst32addr
	ld	src32addr,%o5			! o5 = src32addr
	and	%i4,Rpreloop_mask,%i4
	or	%i4,%i0,%i4
	st	%i4,[%o4]
!	ld	dst32addr,%o4
	cmp	Rsrc32lbit,Rdst32lbit
	dec	4,%o4
	bne	L77255
	st	%o4,dst32addr
	ld	src_comp,%o3
	tst	%o3
	bne	LY32
!	cmp	Rop,3
!	bgu	L77297
!	sll	Rop,2,%o0
	sethi	%hi(L2000009),%o1
	or	%o1,%lo(L2000009),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i5
L2000009:
	.word	L77235
	.word	L77239
	.word	L77243
	.word	L77247
L77215:
!	mov	%l7,%i5
L77216:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o4
!	ld	dst32addr,%o0
	ld	[%o5],%o1
	xor	%o1,-1,%o1
	st	%o1,[%o4]
!	ld	src32addr,%o1
!	ld	dst32addr,%o2
	dec	4,%o5
!	st	%o1,src32addr
	b	L77216
	dec	4,%o4
!	st	%o2,dst32addr
L77219:
!	mov	%l7,%i5
L77220:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o3
!	ld	dst32addr,%o5
	ld	[%o5],%o3
	ld	[%o4],%o7
	andn	%o7,%o3,%o7
	st	%o7,[%o4]
!	ld	src32addr,%o2
!	ld	dst32addr,%o3
	dec	4,%o4
!	st	%o2,src32addr
	b	L77220
	dec	4,%o5
!	st	%o3,dst32addr
L77223:
!	mov	%l7,%i5
L77224:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o4
!	ld	dst32addr,%o7
	ld	[%o5],%o1
	ld	[%o4],%o0
	orn	%o0,%o1,%o0
	st	%o0,[%o4]
!	ld	src32addr,%o3
!	ld	dst32addr,%o4
	dec	4,%o5
!	st	%o3,src32addr
	b	L77224
	dec	4,%o4
!	st	%o4,dst32addr
L77227:
!	mov	%l7,%i5
L77228:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o5
!	ld	dst32addr,%o0
	ld	[%o5],%o0
	ld	[%o4],%o1
	xnor	%o1,%o0,%o1
	st	%o1,[%o4]
!	ld	src32addr,%o4
!	ld	dst32addr,%o5
	dec	4,%o4
!	st	%o4,src32addr
	b	L77228
	dec	4,%o5
!	st	%o5,dst32addr
LY32:					! [internal]
!	bgu	L77297
!	sll	Rop,2,%o0
	ld	dst32addr,%o4
	ld	src32addr,%o5
	sethi	%hi(L2000008),%o1
	or	%o1,%lo(L2000008),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i5
L2000008:
	.word	L77215
	.word	L77219
	.word	L77223
	.word	L77227
L77235:
!	mov	%l7,%i5
L77236:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o0
!	ld	dst32addr,%o7
	ld	[%o5],%o0
	st	%o0,[%o4]
!	ld	src32addr,%o2
!	ld	dst32addr,%o3
	dec	4,%o4
!	dec	4,%o5
!	st	%o2,src32addr
	b	L77236
	dec	4,%o5
!	st	%o3,dst32addr
L77239:
!	mov	%l7,%i5
L77240:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o4
!	ld	dst32addr,%o7
	ld	[%o5],%o2
	ld	[%o4],%o0
	and	%o0,%o2,%o0
	st	%o0,[%o4]
!	ld	src32addr,%o3
!	ld	dst32addr,%o4
	dec	4,%o5
!	dec	4,%o4
!	st	%o3,src32addr
	b	L77240
	dec	4,%o4
!	st	%o4,dst32addr
L77243:
!	mov	%l7,%i5
L77244:
	deccc	%i5
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o5
!	ld	dst32addr,%o0
	ld	[%o5],%o2
	ld	[%o4],%o1
	or	%o1,%o2,%o1
	st	%o1,[%o4]
!	ld	src32addr,%o4
!	ld	dst32addr,%o5
	dec	4,%o4
!	dec	4,%o5
!	st	%o4,src32addr
	b	L77244
	dec	4,%o5
!	st	%o5,dst32addr
L77247:
!	sub	%l7,1,%i5
!	tst	%i5
	deccc	%i5
	bl,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
LY11:					! [internal]
!	ld	src32addr,%o7
!	ld	dst32addr,%o1
	ld	[%o5],%o7
	ld	[%o4],%o2
	deccc	%i5
	xor	%o2,%o7,%o2
	st	%o2,[%o4]
!	ld	src32addr,%o5
!	ld	dst32addr,%o7
	dec	4,%o5
!	dec	4,%o4
!	st	%o5,src32addr
	bpos	LY11
	dec	4,%o4
!	st	%o7,dst32addr
!L77297:
!	tst	Rdst32lbit
	st	%o4,dst32addr
LY29:					! [internal]
	st	%o5,src32addr
	tst	Rdst32lbit
	be,a	LY31
	ld	gray,%o2
	cmp	Rsrc32lbit,Rdst32lbit
	bl,a	L77301
	mov	%i3,%i5
	ld	src32addr,%i5
	ld	[%i5],%i5
	sll	%i5,%i1,%i5
	b	L77301
	or	%i3,%i5,%i5
L77255:
	ld	src_comp,%o0
	tst	%o0
	bne	LY30
!	cmp	Rop,3
!	bgu	L77297
!	sll	Rop,2,%o0
	ld	dst32addr,%o4
	ld	src32addr,%o5
	sethi	%hi(L2000011),%o1
	or	%o1,%lo(L2000011),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i4
L2000011:
	.word	L77277
	.word	L77281
	.word	L77285
	.word	L77289
L77257:
!	mov	%l7,%i4
L77258:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o3
!	ld	dst32addr,%o1
	ld	[%o5],%o2
	dec	4,%o5
	sll	%o2,%i1,%o3
	or	%i3,%o3,%o3
	xor	%o3,-1,%o3
!	st	%o3,src32addr
	st	%o3,[%o1]
	srl	%o2,RsrcRshift,%i3
!	ld	dst32addr,%o2
	b	L77258
	dec	4,%o4
!	st	%o2,dst32addr
L77261:
!	mov	%l7,%i4
L77262:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o5
!	ld	dst32addr,%o3
	ld	[%o5],%o3
	dec	4,%o5
	sll	%o4,%i1,%o7
	or	%i3,%o7,%o7
	srl	%o3,RsrcRshift,%i3
!	st	%o5,src32addr
	ld	[%o4],%o3
	xor	%o7,-1,%o7
	and	%o3,%o7,%o3
	st	%o3,[%o4]
!	ld	dst32addr,%o0
	b	L77262
	dec	4,%o4
!	st	%o0,dst32addr
L77265:
!	mov	%l7,%i4
L77266:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o3
!	ld	dst32addr,%o1
	ld	[%o5],%o2
	dec	4,%o5
	sll	%o2,%i1,%o3
	or	%i3,%o4,%o3
	srl	%o2,RsrcRshift,%i3
!	st	%o3,src32addr
	ld	[%o4],%o2
	xor	%o3,-1,%o3
	or	%o2,%o3,%o2
	st	%o2,[%o4]
!	ld	dst32addr,%o5
	b	L77266
	dec	4,%o4
!	st	%o5,dst32addr
L77269:
!	mov	%l7,%i4
L77270:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o1
!	ld	dst32addr,%o7
	ld	[%o5],%o0
	dec	4,%o5
	sll	%o0,%i1,%o2
	or	%i3,%o2,%o2
	srl	%o0,RsrcRshift,%i3
!	st	%o1,src32addr
	ld	[%o4],%o0
	xor	%o2,-1,%o2
	xor	%o0,%o2,%o0
	st	%o0,[%o4]
!	ld	dst32addr,%o3
	b	L77270
	dec	4,%o4
!	st	%o3,dst32addr
LY30:					! [internal]
!	bgu	L77297
!	sll	Rop,2,%o0
	ld	dst32addr,%o4
	ld	src32addr,%o5
	sethi	%hi(L2000010),%o1
	or	%o1,%lo(L2000010),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	mov	%l7,%i4
L2000010:
	.word	L77257
	.word	L77261
	.word	L77265
	.word	L77269
!	b	LY29
!	tst	Rdst32lbit
L77277:
!	mov	%l7,%i4
L77278:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o7
!	ld	dst32addr,%o3
	ld	[%o5],%o7
	dec	4,%o5
	sll	%o7,%i1,%o0
	or	%i3,%o0,%o0
!	st	%o7,src32addr
	st	%o0,[%o4]
!	ld	dst32addr,%o4
	srl	%o7,RsrcRshift,%i3
	b	L77278
	dec	4,%o4
!	st	%o4,dst32addr
L77281:
!	mov	%l7,%i4
L77282:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o0
!	ld	dst32addr,%o4
	ld	[%o5],%o7
	dec	4,%o5
	sll	%o7,%i1,%o1
	or	%i3,%o1,%o1
	!st	%o0,src32addr
	ld	[%o4],%o2
	srl	%o7,RsrcRshift,%i3
	and	%o2,%o1,%o2
	st	%o2,[%o4]
!	ld	dst32addr,%o1
	b	L77282
	dec	4,%o4
!	st	%o1,dst32addr
L77285:
!	mov	%l7,%i4
L77286:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o4
!	ld	dst32addr,%o1
	ld	[%o5],%o3
	dec	4,%o5
	sll	%o3,%i1,%o1
	or	%i3,%o1,%o1
!	st	%o4,src32addr
	ld	[%o4],%o2
	srl	%o3,RsrcRshift,%i3
	or	%o2,%o1,%o2
	st	%o2,[%o4]
!	ld	dst32addr,%o1
	b	L77286
	dec	4,%o4
!	st	%o5,dst32addr
L77289:
!	mov	%l7,%i4
L77290:
	deccc	%i4
	bneg,a	LY29
	st	%o4,dst32addr
!	tst	Rdst32lbit
!	ld	src32addr,%o1
!	ld	dst32addr,%o5
	ld	[%o5],%o0
	dec	4,%o5
	sll	%o0,%i1,%o2
	or	%i3,%o2,%o2
!	st	%o1,src32addr
	ld	[%o4],%o7
	srl	%o0,RsrcRshift,%i3
	xor	%o7,%o2,%o7
	st	%o7,[%o4]
!	ld	dst32addr,%o2
	b	L77290
	dec	4,%o4
!	st	%o2,dst32addr
L77301:
	ld	src_comp,%o0
	tst	%o0
LY17:					! [internal]
	be,a	L77305
	mov	Rpostloop_mask,%i0
	xor	%i5,-1,%i5
	mov	Rpostloop_mask,%i0
L77305:
	ld	dst32addr,%i4
!	cmp	Rop,3
	ld	[%i4],%i4
!	sll	Rop,2,%o0
	andn	%i4,%i0,%o4
!	bgu	L77312
!	st	%o4,[%fp-40]
	sethi	%hi(L2000012),%o1
	or	%o1,%lo(L2000012),%o1	! [internal]
	ld	[Rop+%o1],%o0
	jmp	%o0
!	nop
	b	L77312
L2000012:
	.word	L77306
	.word	L77307
	.word	L77308
	.word	L77309
L77306:
!	b	L77312
	mov	%i5,%i4
L77307:
!	b	L77312
	and	%i4,%i5,%i4
L77308:
!	b	L77312
	or	%i4,%i5,%i4
L77309:
	xor	%i4,%i5,%i4
L77312:
!	ld	[%fp-40],%o7
	ld	dst32addr,%o1
	and	%i4,%i0,%i4
!	or	%i4,%o7,%i4
	or	%i4,%o4,%i4
	st	%i4,[%o1]
	ld	gray,%o2
LY31:					! [internal]
	tst	%o2
	be,a	LY28
	ld	fwd,%o7
	ld	bb_fast,%o3
	tst	%o3
	be,a	LY27
	ld	dstbpl,%o2
	ld	dstbpl_rshift_3,%o5
	ld	OrigDstAddr,%o4
	add	%o4,%o5,%o4
	st	%o4,dst32addr
	b	L77320
	st	%o4,OrigDstAddr
LY27:					! [internal]
	ld	OrigDstAddr,%o1
	ld	w,%l7
	add	Rdst32lbit,%o2,Rdst32lbit
	sra	Rdst32lbit,5,%o7
	and	Rdst32lbit,31,Rdst32lbit
	sll	%o7,2,%o7
	add	%o1,%o7,%o7
	add	Rdst32lbit,%l7,%l7
	sub	%l7,1,Rdst32rbit
	tst	Rdst32lbit
	st	%o1,dst32addr
	st	%o7,OrigDstAddr
	and	Rdst32rbit,31,Rdst32rbit
	be	L77318
	st	%o7,dst32addr
	mov	32,%o0
	sub	%o0,Rdst32lbit,%o0
	mov	-1,%i5
	sll	%i5,%o0,%i5
	b	L77319
	xor	%i5,-1,%i5
L77318:
	mov	-1,%i5
L77319:
	mov	31,%o7
	sub	%o7,Rdst32rbit,%o7
	mov	-1,Rpostloop_mask
	sll	Rpostloop_mask,%o7,Rpostloop_mask
	mov	%i5,Rpreloop_mask
	and	Rpreloop_mask,Rpostloop_mask,%o4
	sra	%l7,5,%l7
	dec	%l7
	st	%o4,sdw_mask
L77320:
	ld	curr_gray_line,%o0
	ld	num_gray,%o2
	inc	%o0
	cmp	%o0,%o2
	bl	L77322
	st	%o0,curr_gray_line
	ld	srcbase,%o4
	ld	num_gray_1,%o3
	st	%g0,curr_gray_line
	sub	%o4,%o3,%o4
	b	L77047
	st	%o4,srcbase
L77322:
	ld	srcbase,%o5
	inc	2,%o5
	b	L77047
	st	%o5,srcbase
LY28:					! [internal]
	tst	%o7
	be,a	LY26
	ld	bb_fast,%o2
	ld	bb_fast,%o0
	tst	%o0
	be,a	LY25
	ld	dstbpl,%o7
	ld	dstbpl_rshift_3,%o2
	ld	OrigDstAddr,%o1
	add	%o1,%o2,%o1
	st	%o1,dst32addr
	b	L77331
	st	%o1,OrigDstAddr
LY25:					! [internal]
	ld	OrigDstAddr,%o5
	ld	w,%l7
	add	Rdst32lbit,%o7,Rdst32lbit
	sra	Rdst32lbit,5,%o3
	and	Rdst32lbit,31,Rdst32lbit
	sll	%o3,2,%o3
	add	%o5,%o3,%o3
	add	Rdst32lbit,%l7,%l7
	sub	%l7,1,Rdst32rbit
	tst	Rdst32lbit
	st	%o5,dst32addr
	st	%o3,OrigDstAddr
	and	Rdst32rbit,31,Rdst32rbit
	be	L77329
	st	%o3,dst32addr
	mov	32,%o4
	sub	%o4,Rdst32lbit,%o4
	mov	-1,%i5
	sll	%i5,%o4,%i5
	b	L77330
	xor	%i5,-1,%i5
L77329:
	mov	-1,%i5
L77330:
	mov	31,%o3
	sub	%o3,Rdst32rbit,%o3
	mov	-1,Rpostloop_mask
	sll	Rpostloop_mask,%o3,Rpostloop_mask
	mov	%i5,Rpreloop_mask
	and	Rpreloop_mask,Rpostloop_mask,%o1
	sra	%l7,5,%l7
	dec	%l7
	st	%o1,sdw_mask
L77331:
	tst	%o0
	be,a	LY24
	ld	srcbpl,%o3
	ld	srcbpl_rshift_3,%o7
	ld	OrigSrcAddr,%o5
	add	%o5,%o7,%o5
	st	%o5,src32addr
	b	L77047
	st	%o5,OrigSrcAddr
LY24:					! [internal]
	ld	OrigSrcAddr,%o2
	add	Rsrc32lbit,%o3,Rsrc32lbit
	ld	w,%o3
	sra	Rsrc32lbit,5,%o0
	and	Rsrc32lbit,31,Rsrc32lbit
	add	Rsrc32lbit,%o3,%o3
	sub	Rsrc32lbit,Rdst32lbit,%i1
	sll	%o0,2,%o0
	add	%o2,%o0,%o0
	and	%i1,31,%i1
	dec	%o3
	and	%o3,31,%o3
	tst	%i1
	sub	Rdst32lbit,Rsrc32lbit,RsrcRshift
	st	%o2,src32addr
	st	%o0,OrigSrcAddr
	st	%o3,src32rbit
	and	RsrcRshift,31,RsrcRshift
	be	L77335
	st	%o0,src32addr
	mov	-1,%i5
	sll	%i5,%i1,%i5
	b	L77336
	xor	%i5,-1,%i5
L77335:
	mov	-1,%i5
L77336:
	b	L77047
	mov	%i5,RsrcRmask
LY26:					! [internal]
	tst	%o2
	be,a	LY23
	ld	dstbpl,%i5
	ld	dstbpl_rshift_3,%o4
	ld	OrigDstAddr,%o3
	add	%o3,%o4,%o3
	st	%o3,dst32addr
	b	L77347
	st	%o3,OrigDstAddr
LY23:					! [internal]
	ld	OrigDstAddr,%o0
	add	Rdst32rbit,%i5,%i5
	tst	%i5
	bge	L77342
	st	%o0,dst32addr
	mov	31,%l7
	sub	%l7,%i5,%l7
	sra	%l7,5,%l7
	sll	%l7,2,%l7
	sub	%o0,%l7,%o0
	b	L77343
	st	%o0,dst32addr
L77342:
	sra	Rdst32lbit,5,Rdst32lbit
	sll	Rdst32lbit,2,Rdst32lbit
	add	%o0,Rdst32lbit,Rdst32lbit
	st	Rdst32lbit,dst32addr
L77343:
	ld	w,Rdst32lbit
	ld	dst32addr,%o4
	and	%i5,31,Rdst32rbit
	ld	w,%i5
	sub	Rdst32rbit,Rdst32lbit,Rdst32lbit
	inc	Rdst32lbit
!	and	Rdst32lbit,31,Rdst32lbit
	andcc	Rdst32lbit,31,Rdst32lbit
	be	Ldst0b
	mov	-1,Rpostloop_mask
	mov	32,%o3
	sub	%o3,Rdst32lbit,%o3
	mov	31,%o5
	sub	%o5,Rdst32rbit,%o5
	sll	Rpostloop_mask,%o3,Rpostloop_mask
	xor	Rpostloop_mask,-1,Rpostloop_mask	! from below
Ldst0b:	sub	%i5,Rdst32rbit,%i5
	mov	-1,Rpreloop_mask
	sll	Rpreloop_mask,%o5,Rpreloop_mask
	dec	%i5
!	xor	Rpostloop_mask,-1,Rpostloop_mask
	and	Rpreloop_mask,Rpostloop_mask,%o2
	tst	%i5
	st	%o4,OrigDstAddr
	ble	L77345
	st	%o2,sdw_mask
	b	L77346
	sra	%i5,5,%i5
L77345:
	mov	0,%i5
L77346:
	mov	%i5,%l7
L77347:
	ld	bb_fast,%o0
	tst	%o0
	be,a	LY22
	ld	srcbpl,%i1
	ld	srcbpl_rshift_3,%o2
	ld	OrigSrcAddr,%o1
	add	%o1,%o2,%o1
	st	%o1,src32addr
	b	L77047
	st	%o1,OrigSrcAddr
LY22:					! [internal]
	ld	src32rbit,%o7 ! src32rbit loaded
	ld	OrigSrcAddr,%o5 
	add	%o7,%i1,%i1
	tst	%i1
	bge	L77351
	st	%o5,src32addr
	mov	31,%i2
	sub	%i2,%i1,%i2
	sra	%i2,5,%i2
	sll	%i2,2,%i2
	sub	%o5,%i2,%o5
	b	L77352
	st	%o5,src32addr
L77351:
	sra	%i1,5,%o3
	sll	%o3,2,%o3
	add	%o5,%o3,%o3
	st	%o3,src32addr
L77352:
	ld	w,Rsrc32lbit
	and	%i1,31,%i1
	sub	%i1,Rsrc32lbit,Rsrc32lbit
	inc	Rsrc32lbit
	and	Rsrc32lbit,31,Rsrc32lbit
	st	%i1,src32rbit
	sub	Rsrc32lbit,Rdst32lbit,%i1
	and	%i1,31,%i1
	tst	%i1
	sub	Rdst32lbit,Rsrc32lbit,RsrcRshift
	be	L77354
	and	RsrcRshift,31,RsrcRshift
	mov	-1,%i5
	sll	%i5,%i1,%i5
	b	L77355
	xor	%i5,-1,%i5
L77354:
	mov	0,%i5
L77355:
	ld	src32addr,%o5
	mov	%i5,RsrcRmask
	st	%o5,OrigSrcAddr
L77047:
	ld	num_lines_remaining,%i5
	ld	num_lines_remaining,%o3
	tst	%i5
	dec	%o3
	bg	LY8
	st	%o3,num_lines_remaining
LY1:					! [internal]
	ret
	restore	%g0,0,%o0
	.seg	"data"			! [internal]
