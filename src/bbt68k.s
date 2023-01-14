# @(#) bbt68k.s Version 1.1 (4/21/92). copyright Venue & Fuji Xerox  #
 ########################################################################
 #									#
 #			b i t b l t 6 8 K				#
 #									#
 #	Hand-optimized bitblt for the MC68020.  This was created	#
 #	by taking Don Charnley's C version & creating an assembler	#
 #	intermediate file, then hand tuning.				#
 #									#
 #	bitblt68K(srcbase, dstbase, sx, dx, width, height,		#
 #		  srcbpl, dstbpl, backwardflg, src_comp, op,		#
 #		  gray, num_gray, curr_gray_line)			#
 #									#
 #									#
 #									#
 #									#
 ########################################################################
	.data
	.text
LL0:
	.proc
	LF101	=	128
	LS101	=	15608
	LFF101	=	92
	LSS101	=	0
	LV101	=	92
|
|   Argument offsets on the stack
|
#define	srcbase a6@(8)
#define dstbase a6@(12)
#define sx a6@(16)
#define dx a6@(20)
#define width a6@(24)
#define height a6@(28)
#define srcbpl a6@(32)
#define dstbpl a6@(36)
#define backwardflg a6@(40)
#define src_comp a6@(44)
#define op a6@(48)
#define gray a6@(52)
#define num_gray a6@(56)
#define curr_gray_line a6@(60)
|
|  Local variable offsets on the stack
|
|  -4  num_lines_remaining
|  -8  PRECOMPUTED FOR GRAY CASE:  (num_gray-1)<<1
| -12  dstnumL
| -16  src32lbit
| -20  srcLshift
| -24  dst32lbit
| -28  srcRmask
| -32  dstold
| -36  PRECOMPUTED  dstbpl << 3
| -40  mask
| -44  preloop_mask
| -48  postloop_mask
| -52  sdw_mask
| -56  
| -60  dst32rbit
| -64  src32rbit
| -68  
| -72  
| -76  
| -80  bb_fast
| -84  
| -88  
| -92   
#define num_lines_remaining a6@(-4)
|#define temp a6@(-8)	/* NEVER loaded, only stored into */
#define gray_line_length a6@(-8)
#define dstnumL a6@(-12)
#define src32lbit a6@(-16)
#define srcLshift a6@(-20)
#define dst32lbit a6@(-24)
#define srcRmask a6@(-28)
#define dstold a6@(-32)
|#define dstdata a6@(-36)	/* no longer used */
#define dstbpl3 a6@(-36)
#define mask a6@(-40)
|#define x32byta a6@(-44)	/* no longer used */
|#define x32nbyt a6@(-48)	/* no longer used */
|#define x32ia a6@(-52)		/* no longer used */
|#define abc a6@(-56)		/* no longer used */
#define dst32rbit a6@(-60)
#define src32rbit a6@(-64)
|#define fwd a6@(-68)		/* No longer used 10/30/89 JDS */
|#define OrigSrcAddr a6@(-72)
|#define OrigDstAddr a6@(-76)
#define OrigSrcAddr a3
#define OrigDstAddr a2
|#define bb_fast a6@(-80)
|#define preloop_mask a6@(-84)
|#define postloop_mask a6@(-88)
|#define sdw_mask a6@(-92)
#define bb_fast a6@(-80)
#define preloop_mask a6@(-44)
#define postloop_mask a6@(-48)
#define sdw_mask a6@(-52)
|
|  Register usage remaining from the compiler
|
|	a5 srcbase	[used 7 times in code]
|	a4 dstbase
|	a3 OrigSrcAddr (via #define)
|	a2 OrigDstAddr (via #define)
|	a0 srcaddr
|	a1 dstaddr
|
|	d7 ShS
|	d6 SavedS
|	d5 NewS
|	d4 srcRshift
|	d3 cnt (Loop counter in very inner loops)
|	d2 dst32lbit, part-time.
|	d1 [working temp]
|	d0 [working temp]
|

	.data
	.text
	.globl	_bitblt
_bitblt:
|#PROLOGUE# 0
	link	a6,#-128
	moveml	#0x3cfc,sp@
|#PROLOGUE# 1
	movl	srcbase,a5
	movl	dstbase,a4
 #	  num_lines_remaining = h;
	movl	height,num_lines_remaining
 #	  fwd = !backwardflg;	/* Removed as redundant */
 #	  bb_fast = !(31 & (srcbpl | dstbpl));
	movl	srcbpl,d0
	orl	dstbpl,d0
	moveq	#31,d7
	andl	d7,d0
	notl	d0
	movl	d0,bb_fast

	movl	dstbpl,d0
	asrl	#3,d0
	movl	d0,dstbpl3

	tstl	gray
	jeq	L103

	#########################################################
	# Precompute the texture -backup amount for loop bottom #
	#########################################################
	movl	num_gray,d0
	subql	#1,d0
	asll	#1,d0
	movl	d0,gray_line_length

	tstl	dx
	jge	L104
	movl	a4,d0
	moveq	#7,d1
	subl	dx,d1
	asrl	#3,d1
	subl	d1,d0
	jra	LY00000
L104:
	movl	a4,d0
	movl	dx,d1
	asrl	#3,d1
	addl	d1,d0
|
| d0 = x32byta
| d1 = x32nbyt
| a1 = x32ia = dst32addr
LY00000:
	movl	d0,d1
|	movb	a6@(-41),d0
	moveq	#3,d7
	andl	d7,d1
|	movl	d0,x32nbyt
	movl	d0,a1
	subl	d1,a1
|	movl	a1,x32ia
	lsll	d7,d1
	movb	a6@(23),d0	| dx
	moveq	#7,d7
	andl	d7,d0
	addl	d0,d1
	movl	d1,dst32lbit
	movl	d1,d2		| d2 = dst32lbit starts here
	addl	width,d1
	subql	#1,d1
	moveq	#31,d7
	andl	d7,d1
	movl	d1,dst32rbit
	movl	a1,OrigDstAddr
|	tstl	dst32lbit
	tstl	d2
	jeq	L2000004
	moveq	#32,d0
|	subl	dst32lbit,d0
	subl	d2,d0
	moveq	#-1,d1
	asll	d0,d1
	notl	d1
	jra	L2000005
L2000004:
	moveq	#-1,d1
L2000005:
	movl	d1,preloop_mask
	moveq	#31,d0
	subl	dst32rbit,d0
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
|	movl	dst32lbit,d0
	movl	d2,d0
	addl	width,d0
	asrl	#5,d0
	subql	#1,d0
	movl	d0,dstnumL
	movb	a6@(39),d0	|dstbpl
	moveq	#31,d7
	andl	d7,d0
	notl	d0
	movl	d0,bb_fast
	movb	a6@(19),d0	|sx
	moveq	#15,d7
	andl	d7,d0
	movl	d0,src32lbit
	jra	LY00009
 # else if (fwd)
L103:
	tstl	backwardflg
	jne	L107
 #  {if (dx < 0) 
	tstl	dx
	jge	L108
 #    { x32byta = (int)dstbase - ((7 - dx) >> 3); } 
 #   else { x32byta = (int)dstbase + (dx >> 3); }
 #   x32nbyt = x32byta & 3; 
 #   x32ia = x32byta - x32nbyt;    
 #   dst32addr = (int *)x32ia; 
	movl	a4,d0
	moveq	#7,d1
	subl	dx,d1
	asrl	#3,d1
	subl	d1,d0
	jra	LY00001
L108:
	movl	a4,d0
	movl	dx,d1
	asrl	#3,d1
	addl	d1,d0
LY00001:
|	movl	d0,x32byta
	movl	d0,d1
|	movb	a6@(-41),d1
	moveq	#3,d7
	andl	d7,d1
|	movl	d1,x32nbyt
|	movl	x32byta,a1
	movl	d0,a1
	subl	d1,a1
|	movl	a1,x32ia
	asll	d7,d1
	movb	a6@(23),d0	| dx
	moveq	#7,d7
	andl	d7,d0
	addl	d0,d1
	movl	d1,dst32lbit
	movl	d1,d2		| d2 = dst32lbit starting here
	addl	width,d1
	subql	#1,d1
	moveq	#31,d7
	andl	d7,d1
	movl	d1,dst32rbit
 #   OrigDstAddr = dst32addr; 
 #   preloop_mask = ((dst32lbit) ? (~(0xFFFFFFFF << (32 - dst32lbit))) : 0xFFFFFFFF); 
 #  postloop_mask = 0xFFFFFFFF << (31 - dst32rbit); 
 #   sdw_mask = preloop_mask & postloop_mask; 
 #   dstnumL = ((dst32lbit + w) >> 5) - 1; 
	movl	a1,OrigDstAddr
|	tstl	dst32lbit
	tstl	d2
	jeq	L2000008
	moveq	#32,d0
|	subl	dst32lbit,d0
	subl	d2, d0
	moveq	#-1,d1
	asll	d0,d1
	notl	d1
	jra	L2000009
L2000008:
	moveq	#-1,d1
L2000009:
	movl	d1,preloop_mask
	moveq	#31,d0
	subl	dst32rbit,d0
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
|	movl	dst32lbit,d0
	movl	d2,d0
	addl	width,d0
	asrl	#5,d0
	subql	#1,d0
	movl	d0,dstnumL
 #   if (sx < 0) 
 #     { x32byta = (int)srcbase - ((7 - sx) >> 3); } 
 #   else { x32byta = (int)srcbase + (sx >> 3); }
	tstl	sx
	jge	L110
	movl	a5,d0
	moveq	#7,d1
	subl	sx,d1
	asrl	#3,d1
	subl	d1,d0
	jra	LY00002
L110:
	movl	a5,d0
	movl	sx,d1
	asrl	#3,d1
	addl	d1,d0
LY00002:
|	movl	d0,x32byta
 #   x32nbyt = x32byta & 3;
 #   x32ia = x32byta - x32nbyt;
 #   src32addr = (int *)x32ia;
|	movb	a6@(-41),d1
	movl	d0,d1
	moveq	#3,d7
	andl	d7,d1
|	movl	d1,x32nbyt
|	movl	x32byta,a0
	movl	d0,a0
	subl	d1,a0
|	movl	a1,x32ia
	asll	d7,d1
	movb	a6@(19),d0	| sx
	moveq	#7,d7
	andl	d7,d0
	addl	d1,d0		| d0 = src32lbit starts here
	movl	d0,src32lbit
	movl	d0,d1
	addl	width,d1
	subql	#1,d1
	moveq	#31,d7
	andl	d7,d1
	movl	d1,src32rbit
|	movl	dst32lbit,d4
	movl	d2,d4
|	subl	src32lbit,d4
	subl	d0,d4
	andl	d7,d4
|	movl	src32lbit,d0	| d0 = src32lbit stops here
|	subl	dst32lbit,d0
	subl	d2,d0
	andl	d7,d0
	movl	d0,srcLshift
	jeq	L2000010
LY00006:			| srcLshift is in d0 every way you get here.
	moveq	#-1,d1
|	movw	a6@(-18),d1	| srcLshift
	asll	d0,d1
	notl	d1
	jra	L2000051
L2000010:
	moveq	#-1,d1
	jra	L2000051
L107:
	movl	dx,d2		| d2 = abc starts here
	addl	width,d2
	subql	#1,d2
|	movl	d2,abc
	jge	L113
	movl	a4,d0
	moveq	#7,d1
|	subl	abc,d1
	subl	d2,d1
	asrl	#3,d1
	subl	d1,d0
	jra	LY00003
L113:
	movl	a4,d0
|	movl	abc,d1
	movl	d2,d1
	asrl	#3,d1
	addl	d1,d0
LY00003:
|	movl	d0,x32byta
|	movb	a6@(-41),d1
	movl	d0,d1
	moveq	#3,d7
	andl	d7,d1
|	movl	d1,x32nbyt
|	movl	x32byta,a1
	movl	d0,a1
	subl	d1,a1
|	movl	a1,x32ia
	asll	d7,d1
|	movb	a6@(-53),d0	| abc
	movl	d2,d0		| d2 = abc ends here
	moveq	#7,d7
	andl	d7,d0
	addl	d0,d1
	movl	d1,dst32rbit	| d1 = dst32rbit starts here
	movl	d1,d2
	subl	width,d2	| d2 = dst32lbit starts here
	addql	#1,d2
	moveq	#31,d7
	andl	d7,d2
	movl	d2,dst32lbit
	movl	a1,OrigDstAddr
	moveq	#31,d0
|	subl	dst32rbit,d0
	subl	d1,d0		| d1 = dst32rbit stops here
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,preloop_mask
	moveq	#32,d0
	moveq	#-1,d1
	tstl	d2
	jeq	JDS2
|	subl	dst32lbit,d0
	subl	d2,d0		| d2 = dst32lbit stops here
|	moveq	#-1,d1
	asll	d0,d1
	notl	d1
JDS2:	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
	movl	width,d0
	subl	dst32rbit,d0
	subql	#1,d0
	tstl	d0
	jle	L2000012
	movl	width,d0
	subl	dst32rbit,d0
	subql	#1,d0
	asrl	#5,d0
	jra	L2000013
L2000012:
	moveq	#0,d0
L2000013:
	movl	d0,dstnumL
	movl	sx,d2		| d2 = abc starts here
	addl	width,d2
	subql	#1,d2
|	movl	d2,abc
	jge	L115
	movl	a5,d0
	moveq	#7,d1
|	subl	abc,d1
	subl	d2,d1
	asrl	#3,d1
	subl	d1,d0
	jra	LY00004
L115:
	movl	a5,d0
|	movl	abc,d1
	movl	d2,d1
	asrl	#3,d1
	addl	d1,d0
LY00004:
|	movl	d0,x32byta
|	movb	a6@(-41),d1
	movl	d0,d1
	moveq	#3,d7
	andl	d7,d1
|	movl	d1,x32nbyt
|	movl	x32byta,a0
	movl	d0,a0
	subl	d1,a0
|	movl	a0,x32ia
	asll	d7,d1
|	movb	a6@(-53),d0	|abc
	movl	d2,d0		| d2 = abc ends here
	moveq	#7,d7
	andl	d7,d0
	addl	d0,d1
	movl	d1,src32rbit
	jra	LY00016
LY00010:
	tstl	gray
	jeq	L119
	movl	src32lbit,d0
	subl	dst32lbit,d0
	moveq	#15,d7
	andl	d7,d0		| d0 = srcLshift
	movl	d0,srcLshift
	moveq	#0,d7
	movw	a5@,d7
	movl	d7,d1
|	moveq	#16,d1
|	lsll	d1,d0
	swap	d1
	orl	d1,d7
|	movw	a6@(-18),d0
|	lsll	d0,d7
|	movl	d7,d0
|	moveq	#16,d1
|	lsrl	d1,d0
|	andl	#65535,d0
|	orl	d0,d7
	roll	d0,d7
	tstl	src_comp
	jeq	L120
	notl	d7		| end oof d0 = srcLshift
L120:
	movl	dst32lbit,d0
	addl	width,d0
	moveq	#32,d5
	cmpl	d5,d0
	jle	LY00008
	movl	preloop_mask,mask
	movl	a1@,d5	|  d5 = dstdata thru here...
	movl	mask,d1	|  d1 = dstold thru here
	notl	d1
	andl	d5,d1
|	movl	d1,dstold
	movl	op,d0
|	moveq	#3,d5
|	cmpl	d5,d0
|	jhi	L123
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000017:
	.word	L125-L2000017
	.word	L126-L2000017
	.word	L127-L2000017
	.word	L128-L2000017
L125:
	movl	d7,d5
	jra	L123
L126:
	andl	d7,d5
	jra	L123
L127:
	orl	d7,d5
	jra	L123
L128:
	eorl	d7,d5
L123:
	movl	mask,d0
	andl	d0,d5
	orl	d1,d5	| wass dstold
|	orl	d0,dstdata
	movl	d5,a1@+
	movl	op,d0
	movl	dstnumL,d3	| from all loop headers below.
|	moveq	#3,d5
|	cmpl	d5,d0
|	jhi	L129
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000019:
	.word	L131-L2000019
	.word	L135-L2000019
	.word	L139-L2000019
	.word	L143-L2000019
L131:
|	movl	dstnumL,d3
L134:
	subql	#1,d3
	jmi	L129
	movl	d7,a1@+
	jra	L134
L135:
|	movl	dstnumL,d3
L138:
	subql	#1,d3
	jmi	L129
	andl	d7,a1@+
	jra	L138
L139:
|	movl	dstnumL,d3
L142:
	subql	#1,d3
	jmi	L129
	orl	d7,a1@+
	jra	L142
|L143:
|	movl	dstnumL,d3
|	jra	LY00013
LY00014:
	eorl	d7,a1@+
L143:
LY00013:
	subql	#1,d3
	jpl	LY00014
L129:
	cmpl	#31,dst32rbit
	jeq	L148
	jra	L337
L119:
	tstl	backwardflg
	jne	L149
	movw	a6@(-18),d1	| srcLshift in d1 down thru L151
	movl	src32lbit,d0
	cmpl	dst32lbit,d0
	jle	L150
	addl	width,d0
	moveq	#32,d7
	cmpl	d7,d0
	jle	L150
	movl	a0@+,d6
|	movw	a6@(-18),d1	| srcLshift
	lsll	d1,d6
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	andl	srcRmask,d7
	orl	d6,d7
	lsll	d1,d0
	movl	srcRmask,d1
	notl	d1
	andl	d1,d0
	movl	d0,d6
	jra	L151
L150:
	movl	src32lbit,d0
	cmpl	dst32lbit,d0
	jgt	L152
	movl	a0@+,d6
	movl	d6,d7
	lsrl	d4,d7
	andl	srcRmask,d7
|	movw	a6@(-18),d1	|srcLshift
	lsll	d1,d6
	movl	srcRmask,d1
	notl	d1
	andl	d1,d6
	jra	L151
L152:
	movl	a0@+,d7
|	movw	a6@(-18),d1	| srcLshift
	lsll	d1,d7
L151:
	tstl	src_comp
	jeq	L154
	notl	d7
L154:
	movl	dst32lbit,d0
	addl	width,d0
	moveq	#32,d5
	cmpl	d5,d0
	jle	LY00008
	movl	preloop_mask,mask
	movl	a1@,d5		| d5 = dstdata thru here
	movl	mask,d1		| d1 = dstold thru here.
	notl	d1
	andl	d5,d1
|	movl	d1,dstold
	movl	op,d0
|	moveq	#3,d5
|	cmpl	d5,d0
|	jhi	L156
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000021:
	.word	L158-L2000021
	.word	L159-L2000021
	.word	L160-L2000021
	.word	L161-L2000021
L158:
	movl	d7,d5
	jra	L156
L159:
	andl	d7,d5
	jra	L156
L160:
	orl	d7,d5
	jra	L156
L161:
	eorl	d7,d5
L156:
	movl	mask,d0
	andl	d0,d5
|	movl	dstold,d1
	orl	d1,d5
	movl	d5,a1@+
	movl	src32lbit,d0
	cmpl	dst32lbit,d0
	jne	L162
	tstl	src_comp
	jeq	L184
	movl	op,d0
	movl	dstnumL,d3	| from loop headers below
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L201
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000023:
	.word	L166-L2000023
	.word	L170-L2000023
	.word	L174-L2000023
	.word	L178-L2000023
L166:
|	movl	dstnumL,d3
L169:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	notl	d0
	movl	d0,a1@+
	jra	L169
L170:
|	movl	dstnumL,d3
L173:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	notl	d0
	andl	d0,a1@+
	jra	L173
L174:
|	movl	dstnumL,d3
L177:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	notl	d0
	orl	d0,a1@+
	jra	L177
L178:
|	movl	dstnumL,d3
L181:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	notl	d0
	eorl	d0,a1@+
	jra	L181
L185:
|	movl	dstnumL,d3
L188:
	subql	#1,d3
	jmi	L201
	movl	a0@+,a1@+
	jra	L188
L189:
|	movl	dstnumL,d3
L192:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	andl	d0,a1@+
	jra	L192
L193:
|	movl	dstnumL,d3
L196:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	orl	d0,a1@+
	jra	L196
L197:
|	movl	dstnumL,d3
L200:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	eorl	d0,a1@+
	jra	L200
L184:
	movl	op,d0
	movl	dstnumL,d3		|from loop headers
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L201
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000025:
	.word	L185-L2000025
	.word	L189-L2000025
	.word	L193-L2000025
	.word	L197-L2000025
	jra	L201
L162:
	tstl	src_comp
	jeq	L223
	movl	op,d0
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L201
	movw	a6@(-18),d1	| from within the loops below, srcLshift
	movl	dstnumL,d3	| from loop headers
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000027:
	.word	L205-L2000027
	.word	L209-L2000027
	.word	L213-L2000027
	.word	L217-L2000027
L205:
|	movl	dstnumL,d3
L208:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
	notl	d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	movl	d7,a1@+
	jra	L208
L209:
|	movl	dstnumL,d3
L212:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
	notl	d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	andl	d7,a1@+
	jra	L212
L213:
|	movl	dstnumL,d3
L216:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
	notl	d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	orl	d7,a1@+
	jra	L216
L217:
|	movl	dstnumL,d3
L220:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
	notl	d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	eorl	d7,a1@+
	jra	L220
L224:
|	movl	dstnumL,d3
L227:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	movl	d7,a1@+
	jra	L227
L228:
|	movl	dstnumL,d3
L231:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	andl	d7,a1@+
	jra	L231
L232:
|	movl	dstnumL,d3
L235:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	orl	d7,a1@+
	jra	L235
L236:
|	movl	dstnumL,d3
L239:
	subql	#1,d3
	jmi	L201
	movl	a0@+,d0
	movl	d0,d7
	lsrl	d4,d7
	orl	d6,d7
|	movw	a6@(-18),d1
	lsll	d1,d0
	movl	d0,d6
	eorl	d7,a1@+
	jra	L239
L223:
	movl	op,d0
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L201
	movw	a6@(-18),d1	| from loops after dispatch, srcLshift
	movl	dstnumL,d3	| from loop headers
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000029:
	.word	L224-L2000029
	.word	L228-L2000029
	.word	L232-L2000029
	.word	L236-L2000029
L201:
	cmpl	#31,dst32rbit
	jeq	L148
	movl	src32rbit,d0
	cmpl	dst32rbit,d0
	jgt	L335
	movl	a0@,d0
	lsrl	d4,d0
	andl	srcRmask,d0
	jra	LY00011
L149:
	movl	src32rbit,d0
	cmpl	dst32rbit,d0
	jge	L244
	movl	src32lbit,d0
	addl	width,d0
	movl	dst32lbit,d1
	addql	#1,d1
	cmpl	d1,d0
	jle	L244
	movl	a0@,d6
	lsrl	d4,d6
	andl	srcRmask,d6
	movl	a0@-,d0
	subqw	#4,a0
	movl	d0,d7
	movw	a6@(-18),d1	| srcLshift
	lsll	d1,d7
	orl	d6,d7
	lsrl	d4,d0
	andl	srcRmask,d0
	movl	d0,d6
	jra	L245
L244:
	movl	src32rbit,d0
	cmpl	dst32rbit,d0
	jlt	L246
	movl	a0@,d6
	subqw	#4,a0
	movl	d6,d7
	movw	a6@(-18),d1	| srcLshift
	lsll	d1,d7
	lsrl	d4,d6
	andl	srcRmask,d6
	jra	L245
L246:
	movl	a0@,d7
	subqw	#4,a0
	lsrl	d4,d7
	andl	srcRmask,d7
L245:
	tstl	src_comp
	jeq	L248
	notl	d7
L248:
	movl	dst32lbit,d0
	addl	width,d0
	moveq	#32,d5
	cmpl	d5,d0
	jgt	L249
LY00008:
	movl	sdw_mask,mask
	jra	L122
L249:
	movl	preloop_mask,mask
	movl	a1@,d5		| d5 = dstdata thru here
	movl	mask,d1		| d1 = dstold thru here.
	notl	d1
	andl	d5,d1
|	movl	d1,dstold
	movl	op,d0
|	moveq	#3,d5
|	cmpl	d5,d0
|	jhi	L250
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000031:
	.word	L252-L2000031
	.word	L253-L2000031
	.word	L254-L2000031
	.word	L255-L2000031
L252:
	movl	d7,d5
	jra	L250
L253:
	andl	d7,d5
	jra	L250
L254:
	orl	d7,d5
	jra	L250
L255:
	eorl	d7,d5
L250:
|	movl	mask,d0
|	andl	d0,d5
	andl	mask,d5
|	movl	dstold,d0
|	orl	d0,d5
	orl	d1,d5		| end of d1/dstold range
	movl	d5,a1@
	subqw	#4,a1
	movl	src32lbit,d0
	cmpl	dst32lbit,d0
	jne	L256
	tstl	src_comp
	jeq	L278
	movl	op,d0
	movl	dstnumL,d3	| from loop headers below
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L295
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000033:
	.word	L260-L2000033
	.word	L264-L2000033
	.word	L268-L2000033
	.word	L272-L2000033
L260:
|	movl	dstnumL,d3
L263:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	notl	d0
	movl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L263
L264:
|	movl	dstnumL,d3
L267:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	notl	d0
	andl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L267
L268:
|	movl	dstnumL,d3
L271:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	notl	d0
	orl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L271
L272:
|	movl	dstnumL,d3
L275:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	notl	d0
	eorl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L275
L279:
|	movl	dstnumL,d3
L282:
	subql	#1,d3
	jmi	L295
	movl	a0@,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L282
L283:
|	movl	dstnumL,d3
L286:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	andl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L286
L287:
|	movl	dstnumL,d3
L290:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	orl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L290
L291:
|	movl	dstnumL,d3
L294:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	eorl	d0,a1@
	subqw	#4,a0
	subqw	#4,a1
	jra	L294
L278:
	movl	op,d0
	movl	dstnumL,d3	| from loop headers above
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L295
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000035:
	.word	L279-L2000035
	.word	L283-L2000035
	.word	L287-L2000035
	.word	L291-L2000035
	jra	L295
L256:
	tstl	src_comp
	jeq	L317
	movw	a6@(-18),d1	| from loops after dispatch, below, srcLshift
	movl	dstnumL,d3	| from loop headers
	movl	op,d0
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L295
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000037:
	.word	L299-L2000037
	.word	L303-L2000037
	.word	L307-L2000037
	.word	L311-L2000037
L299:
|	movl	dstnumL,d3
L302:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	notl	d7
	lsrl	d4,d0
	movl	d0,d6
	movl	d7,a1@
	subqw	#4,a1
	jra	L302
L303:
|	movl	dstnumL,d3
L306:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	notl	d7
	lsrl	d4,d0
	movl	d0,d6
	andl	d7,a1@
	subqw	#4,a1
	jra	L306
L307:
|	movl	dstnumL,d3
L310:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	notl	d7
	lsrl	d4,d0
	movl	d0,d6
	orl	d7,a1@
	subqw	#4,a1
	jra	L310
L311:
|	movl	dstnumL,d3
L314:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	notl	d7
	lsrl	d4,d0
	movl	d0,d6
	eorl	d7,a1@
	subqw	#4,a1
	jra	L314
L318:
|	movl	dstnumL,d3
L321:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	lsrl	d4,d0
	movl	d0,d6
	movl	d7,a1@
	subqw	#4,a1
	jra	L321
L322:
|	movl	dstnumL,d3
L325:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	lsrl	d4,d0
	movl	d0,d6
	andl	d7,a1@
	subqw	#4,a1
	jra	L325
L326:
|	movl	dstnumL,d3
L329:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	lsrl	d4,d0
	movl	d0,d6
	orl	d7,a1@
	subqw	#4,a1
	jra	L329
L330:
|	movl	dstnumL,d3
L333:
	subql	#1,d3
	jmi	L295
	movl	a0@,d0
	subqw	#4,a0
	movl	d0,d7
|	movw	a6@(-18),d1
	lsll	d1,d7
	orl	d6,d7
	lsrl	d4,d0
	movl	d0,d6
	eorl	d7,a1@
	subqw	#4,a1
	jra	L333
L317:
	movl	op,d0
	movw	a6@(-18),d1	| from loops after dispatch (above), srcLshift
	movl	dstnumL,d3	| from loop headers
|	moveq	#3,d7
|	cmpl	d7,d0
|	jhi	L295
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000039:
	.word	L318-L2000039
	.word	L322-L2000039
	.word	L326-L2000039
	.word	L330-L2000039
L295:
	tstl	dst32lbit
	jeq	L148
	movl	src32lbit,d0
	cmpl	dst32lbit,d0
	jlt	L335
	movl	a0@,d0
	movw	a6@(-18),d1	| srcLshift
	lsll	d1,d0
LY00011:
	orl	d6,d0
	movl	d0,d7
	jra	L336
L335:
	movl	d6,d7
L336:
	tstl	src_comp
	jeq	L337
	notl	d7
L337:
	movl	postloop_mask,mask
L122:
	movl	a1@,d5		| d5 = dstdata thru here....
	movl	mask,d1		| d1 = dstold thru here.
	notl	d1
	andl	d5,d1
|	movl	d1,dstold
	movl	op,d0
|	moveq	#3,d5
|	cmpl	d5,d0
|	jhi	L338
	movw	pc@(6,d0:l:2),d0
	jmp	pc@(2,d0:w)
L2000041:
	.word	L340-L2000041
	.word	L341-L2000041
	.word	L342-L2000041
	.word	L343-L2000041
L340:
	movl	d7,d5
	jra	L338
L341:
	andl	d7,d5
	jra	L338
L342:
	orl	d7,d5
	jra	L338
L343:
	eorl	d7,d5
L338:
	movl	mask,d0
	andl	d0,d5
|	movl	dstold,d5	| transcription error??, replaced with:
	orl	d1, d5
	movl	d5,a1@
L148:
	tstl	gray
	jeq	L344
	tstl	bb_fast
	jeq	L345
|	movl	dstbpl,d0
|	asrl	#3,d0
|	addl	d0,OrigDstAddr
	addl	dstbpl3,OrigDstAddr
	movl	OrigDstAddr,a1
	jra	L346
L345:
	movl	OrigDstAddr,a1	| d2 = dst32lbit starts here
	movl	dst32lbit,d2
|	movl	dstbpl,d0
|	addl	d0,dst32lbit
	addl	dstbpl,d2
|	movl	dst32lbit,d0
	movl	d2,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	addl	d0,a1
	moveq	#31,d7
|	andl	d7,dst32lbit
	andl	d7,d2
|	movl	dst32lbit,d0
	movl	d2,d0
	addl	width,d0
	subql	#1,d0
	andl	d7,d0
	movl	d0,dst32rbit
	movl	a1,OrigDstAddr
|	tstl	dst32lbit
	tstl	d2
	jeq	L2000042
	moveq	#32,d0
|	subl	dst32lbit,d0
	subl	d2,d0
	moveq	#-1,d1
	lsll	d0,d1
	notl	d1
	jra	L2000043
L2000042:
	moveq	#-1,d1
L2000043:
	movl	d1,preloop_mask
	moveq	#31,d0
	subl	dst32rbit,d0
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
|	movl	dst32lbit,d0
	movl	d2,d0
	addl	width,d0
	asrl	#5,d0
	subql	#1,d0
	movl	d2,dst32lbit		| d2 = dst32lbit ends here
	movl	d0,dstnumL
	####################################
	#  Bottom of the texture-blt loop  #
	#
L346:
	addql	#1,curr_gray_line
	movl	curr_gray_line,d0
	cmpl	num_gray,d0
	jlt	L347
	clrl	curr_gray_line
|	movl	num_gray,d0
|	subql	#1,d0
|	asll	#1,d0
|	subl	d0,a5
	subl	gray_line_length,a5
	jra	LY00009
L347:
	addqw	#2,a5
	jra	LY00009
L344:
	tstl	backwardflg
	jne	L349
	tstl	bb_fast
	jeq	L350
|	movl	dstbpl,d0
|	asrl	#3,d0
|	addl	d0,OrigDstAddr
	addl	dstbpl3,OrigDstAddr
	movl	OrigDstAddr,a1
	jra	L351
L350:
	movl	OrigDstAddr,a1		| d2 = dst32lbit starts here
	movl	dst32lbit,d2
|	movl	dstbpl,d0
	addl	dstbpl,d2
|	addl	d0,dst32lbit
|	movl	dst32lbit,d0
	movl	d2,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	addl	d0,a1
	moveq	#31,d7
|	andl	d7,dst32lbit
|	movl	dst32lbit,d0
	andl	d7,d2
	movl	d2,d0
	addl	width,d0
	subql	#1,d0
	andl	d7,d0
	movl	d0,dst32rbit
	movl	a1,OrigDstAddr
|	tstl	dst32lbit
	tstl	d2
	jeq	L2000044
	moveq	#32,d0
|	subl	dst32lbit,d0
	subl	d2,d0
	moveq	#-1,d1
	asll	d0,d1
	notl	d1
	jra	L2000045
L2000044:
	moveq	#-1,d1
L2000045:
	movl	d1,preloop_mask
	moveq	#31,d0
	subl	dst32rbit,d0
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
|	movl	dst32lbit,d0
	movl	d2,d0
	addl	width,d0
	asrl	#5,d0
	subql	#1,d0
	movl	d0,dstnumL
	movl	d2,dst32lbit		| end of d2 = dst32lbit
L351:
	tstl	bb_fast
	jne	LY00007
	movl	OrigSrcAddr,a0
	movl	src32lbit,d2		| d2 = src32lbit starts here
|	movl	srcbpl,d0
|	addl	d0,src32lbit
|	movl	src32lbit,d0
	addl	srcbpl,d2
	movl	d2,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	addl	d0,a0
	moveq	#31,d7
|	andl	d7,src32lbit
	andl	d7,d2
|	movl	src32lbit,d0
	movl	d2,d0
	addl	width,d0
	subql	#1,d0
	andl	d7,d0
	movl	d0,src32rbit
	movl	a0,OrigSrcAddr
	movl	dst32lbit,d4
|	subl	src32lbit,d4
	subl	d2,d4
	andl	d7,d4
|	movl	src32lbit,d0
	movl	d2,d1
	subl	dst32lbit,d1
	andl	d7,d1
	movl	d1,srcLshift
	jeq	L2000046
	moveq	#-1,d0
|	movw	a6@(-18),d1	| srcLshift
	asll	d1,d0
	notl	d0
	jra	L2000047
L2000046:
	moveq	#-1,d0
L2000047:
	movl	d0,srcRmask
	movl	d2,src32lbit	| end of d2 = src32lbit
	jra	LY00009
L349:
	tstl	bb_fast
	jeq	L354
|	movl	dstbpl,d0
|	asrl	#3,d0
|	addl	d0,OrigDstAddr
	addl	dstbpl3,OrigDstAddr
	movl	OrigDstAddr,a1
	jra	L355
L354:
	movl	OrigDstAddr,a1
	movl	dstbpl,d0
	addl	d0,dst32rbit
	jpl	L356
	moveq	#31,d0
	subl	dst32rbit,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	subl	d0,a1
	jra	L357
L356:
	movl	dst32lbit,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	addl	d0,a1
L357:
	moveq	#31,d7
	andl	d7,dst32rbit
	movl	dst32rbit,d0
	subl	width,d0
	addql	#1,d0
	andl	d7,d0
	movl	d0,dst32lbit
	movl	a1,OrigDstAddr
	moveq	#31,d0
	subl	dst32rbit,d0
	moveq	#-1,d1
	asll	d0,d1
	movl	d1,preloop_mask
	moveq	#-1,d1
	tstl	dst32lbit
	jeq	JDS1
	moveq	#32,d0
	subl	dst32lbit,d0
	asll	d0,d1
	notl	d1
JDS1:	movl	d1,postloop_mask
	movl	preloop_mask,d0
	andl	d1,d0
	movl	d0,sdw_mask
	movl	width,d0
	subl	dst32rbit,d0
	subql	#1,d0
	tstl	d0
	jle	L2000048
	movl	width,d0
	subl	dst32rbit,d0
	subql	#1,d0
	asrl	#5,d0
	jra	L2000049
L2000048:
	moveq	#0,d0
L2000049:
	movl	d0,dstnumL
L355:
	tstl	bb_fast
	jeq	L358
LY00007:
	movl	srcbpl,d0
	asrl	#3,d0
	addl	d0,OrigSrcAddr
	movl	OrigSrcAddr,a0
	jra	LY00009
L358:
	movl	OrigSrcAddr,a0
	movl	srcbpl,d0
	addl	d0,src32rbit
	jpl	L360
	moveq	#31,d0
	subl	src32rbit,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	subl	d0,a0
	jra	L361
L360:
	movl	src32rbit,d0
	asrl	#5,d0
|	movl	d0,temp
	asll	#2,d0
|	movl	d0,temp
	addl	d0,a0
L361:
	moveq	#31,d7
	andl	d7,src32rbit
LY00016:
	movl	src32rbit,d0
	subl	width,d0
	addql	#1,d0
	moveq	#31,d7
	andl	d7,d0
	movl	d0,src32lbit
	movl	dst32lbit,d4
	subl	d0,d4
	andl	d7,d4
	subl	dst32lbit,d0
	andl	d7,d0
	movl	d0,srcLshift
	jne	LY00006
	moveq	#0,d1
L2000051:
	movl	d1,srcRmask
	movl	a0,OrigSrcAddr
LY00009:
	movl	num_lines_remaining,d0
	subql	#1,num_lines_remaining
	tstl	d0
	jgt	LY00010
	moveq	#0,d0
|#PROLOGUE# 2
	moveml	a6@(-128),#0x3cfc
	unlk	a6
|#PROLOGUE# 3
	rts
