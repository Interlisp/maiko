/* @(#) dspSPARC.il Version 1.1 (4/21/92). copyright Venue & Fuji Xerox  */
/*  @(#) dispSPARC.il Version 2.7 (7/25/90). copyright Xerox & Fuji Xerox  */


/*  change _name_scan for name table expanding '90/07/13 by osamu */
/*  Name table Expanded version */
/* _name_scan2(entry, limit, names)	inner loop of FVAR
	%o0	entry		even pointer to name table entry
	%o1	limit		oct_byte pointer beyond last name
	%o2	names		32 bit (actually 24bit) atom number
	%o4	low		contents of entry 
/**/ 

.inline _name_scan2,12		!(entry, limit, names) returns first+4 or 0
loop:	cmp	%o0,%o1		!loop:	if(entry<limit)
loop1:	blu	ph0		!		then continue{
	ld	[%o0],%o4	!	     low = *(entry);}	
	 ba	done		!		else entry=0; goto done;
	 sub	%o0,%o0,%o0	!  
ph0:	inc	4,%o0		!ph0:	entry += 4
	xorcc	%o4,%o2,%o4	!	if(cc = low ^ names) then continue;
	bne	loop1		!	if(cc=(last==first)) then loop
	cmp	%o0,%o1		!
	 ba	done		!		else done;
	 nop			!		
done:				!
.end

/* Old Nametable(entry size 16bit) version */
/* _name_scan(entry, limit, names)	inner loop of FVAR
	%o0	entry		even pointer to name table entry
	%o1	limit		oct_byte pointer beyond last name
	%o2	names		2 copies of 16 bit atom number
	%o3	mask		0xFFFF0000
	%o4	low or temp	low half of double word fetch
	%o5	high		high half of double word fetch
/**/ 

.inline _name_scan,12		!(entry, limit, names) returns first+2 or 0
	andcc	%o0,7,%o4	! switch (temp=entry & 7)
	be	loop		!	case0: goto loop;
	sethi	%hi(0xFFFF0000),%o3
	andn	%o0,0x7,%o0	!	entry &= ~7;
	cmp	%o4,0x4		! if (temp=entry&7 < 4)
	ldd	[%o0],%o4	!	low = *(entry); high = *(entry+4);
	inc	8,%o0		!	entry += 8
	xor	%o4,%o2,%o4	!	  {temp = low ^ names}
	bl,a	ph2		!	case2:
	andncc	%o4,%o3,%g0	!		cc=temp & ^0xFFFF0000; goto ph1;
	be	ph4		!	  {temp = high^ names}
	xor	%o5,%o2,%o4	!	case4:	 goto ph2;
	ba	ph6		!	case6:	
	andncc	%o4,%o3,%g0	!		cc=temp & ^0xFFFF0000; goto ph3;
loop:	cmp	%o0,%o1		!loop:	if(entry<limit)
loop1:	blu	ph0		!		then continue{
	ldd	[%o0],%o4	!	     low = *(entry); high = *(entry+4)}	
	 ba	done		!		else entry=0; goto done;
	 sub	%o0,%o0,%o0	!  
ph0:	inc	8,%o0		!ph0:	entry += 8
	xor	%o4,%o2,%o4	!	temp = low ^ names
	andcc	%o4,%o3,%g0	!	if (temp & 0xFFFF0000)
	bne	ph2		!		then continue{
	andncc	%o4,%o3,%g0	!			cc=temp &^ 0xFFFF0000}
	 ba	done		!		else {entry -= 6; goto done;}
	 dec	6,%o0		!		
ph2:	bne	ph4		!ph1: if (cc=temp &^ 0xFFFF0000) then continue{
	xor	%o5,%o2,%o4	!			temp = high ^ names}
	 ba	done		!		else {entry -=4; goto done;}
	 dec	4,%o0		!		
ph4:	andcc	%o4,%o3,%g0	!ph2:	if (temp & 0xFFFF0000)
	bne	ph6		!		then continue{
	andncc	%o4,%o3,%g0	!			cc=temp &^ 0xFFFF0000}
	 ba	done		!		else {entry -= 2; goto done;}
	 dec	2,%o0		!		
ph6:	bne	loop1		!ph3:	if (cc=temp &^ 0xFFFF0000) then loop{
	cmp	%o0,%o1		! 			cc=(last==first)}
done:				!		else done;
.end


/*
	***************************************************************
	Inline Assembly help for dispatcher.
	***************************************************************
*/


/* Note: that the error exit of these routines may or may not need to
	fix the tos pointer.  The C code should decide when tos must
	be fixed through the ifdef flats.
*/

/*
	***************************************************************
	DIFFERENCE VERSIONS sp@ - sp@(4)  i.e. (tos-1)  -  (tos)
	***************************************************************
*/

.inline	_op_difference,8	! working
	srl	%o1,17,%o2	! r2 = type(arg1)
	cmp	%o2,7		!   ==smallp ?
	bne	diff_err	!   else diff_err
	srl	%o0,17,%o2	! r2 = type(arg0)
	cmp	%o2,7
	bne	diff_err
	sll	%o0,15,%o2	! preserve arg0 
	sll	%o1,15,%o1
	subcc	%o2,%o1,%o0
	bvs	diff_err
	srl	%o0,15,%o0
	sethi	%hi(0x000E0000),%o2
	or	%o2, %o0,%o0
.end

/*
	***************************************************************
	PLUS VERSIONS sp@ + sp@(4)  i.e. (tos-1)  +  (tos)
	***************************************************************
*/


.inline	_op_plus,8
	srl	%o1,17,%o2	! r2 = type(arg1)
	cmp	%o2,7		!   ==smallp ?
	bne	plus_err	!   else diff_err
	srl	%o0,17,%o2	! r2 = type(arg0)
	cmp	%o2,7
	bne	plus_err
	sll	%o0,15,%o2	! preserve arg0 
	sll	%o1,15,%o1
	addcc	%o2,%o1,%o0
	bvs	plus_err
	srl	%o0,15,%o0
	sethi	%hi(0x000E0000),%o2
	or	%o2, %o0,%o0
.end

/*
	***************************************************************
	LOGAND VERSIONS sp@ & sp@(4)  i.e. (tos-1)  &  (tos)
	***************************************************************
*/


.inline	_op_logand,8
	srl	%o1,17,%o2	! r2 = type(arg1)
	cmp	%o2,7		!   ==smallp ?
	bne	logand_err	!   else diff_err
	srl	%o0,17,%o2	! r2 = type(arg0)
	cmp	%o2,7
	bne	logand_err
	and	%o0,%o1,%o0
.end

/*
	***************************************************************
	LOGOR VERSIONS sp@ | sp@(4)  i.e. (tos-1)  |  (tos)
	***************************************************************
*/

.inline	_op_logor,8
	srl	%o1,17,%o2	! r2 = type(arg1)
	cmp	%o2,7		!   ==smallp ?
	bne	logor_err	!   else diff_err
	srl	%o0,17,%o2	! r2 = type(arg0)
	cmp	%o2,7
	bne	logor_err
	or	%o0,%o1,%o0
.end

/*
	***************************************************************
	LOGXOR VERSIONS sp@ | sp@(4)  i.e. (tos-1)  |  (tos)
	***************************************************************
*/

.inline	_op_logxor,8
	srl	%o1,17,%o2	! r2 = type(arg1)
	cmp	%o2,7		!   ==smallp ?
	bne	logxor_err	!   else diff_err
	srl	%o0,17,%o2	! r2 = type(arg0)
	cmp	%o2,7
	bne	logxor_err
	xor	%o0,%o1,%o0
	sethi	%hi(0x000E0000),%o1
	or	%o0,%o1,%o0
.end

/*
	***************************************************************
	SHIFT OPCODE VERSIONS
	***************************************************************
*/


.inline	_op_lrsh8,4
	srl	%o0,16,%o2	! r2 = type(arg0)
	cmp	%o2,0xE
	bne	lrsh8_err
	sethi	%hi(0xE0E0000),%o1	!E0000 xor E000000
	xor	%o0,%o1,%o0
	srl	%o0,8,%o0
.end


/* inline LRSH1 */
/* 	sp@ >> 1
*/

.inline	_op_lrsh1,4
	srl	%o0,16,%o2
	cmp	%o2,0xE
	bne	lrsh1_err
	srl	%o0,11,%o0
	sethi	%hi(0x00090000),%o1	!E0000 xor 70000
	or	%o0,%o1,%o0
.end


.inline	_op_llsh8,4
	srl	%o0,8,%o2	! (arg0>>8)==0xE00?
	cmp	%o2,0xE00
	bne	llsh8_err
	sll	%o0,8,%o0
	sethi	%hi(0xE0E0000),%o1	!E0000 xor E000000
	xor	%o0,%o1,%o0
.end


.inline	_op_llsh1,4
	srl	%o0,15,%o2	! (arg0>>15)==0x1C
	cmp	%o2,0x1C
	bne	llsh1_err
	sll	%o0,1,%o0
	sethi	%hi(0x00120000),%o1	!E0000 xor 1C0000
	xor	%o0,%o1,%o0
.end

/*
	***************************************************************
	GREATERP OPCODE VERSIONS
	***************************************************************
*/

.inline	_op_greaterp,8
	srl	%o1,17,%o2	! \
	cmp	%o2,7		!  > smallp(arg1) ?
	bne	greaterp_err	! /
	sll	%o0,15,%o2	! preserve arg0 
	srl	%o0,17,%o0	! return NIL
	cmp	%o2,7		!  > smallp(arg0) ?
	bne	greaterp_err	! /
	cmp	%o2,%o1		! arg0 - arg1  ??
	bg,a	9f		! if greater, return T
	or	%o0,76,%o0
9:
.end

/*
	***************************************************************
	POINTER OPCODE VERSIONS
	***************************************************************
*/

.inline _addbase,8
	srl	%o1,17,%o2	! \
	cmp	%o2,7		!  > smallp(arg1) ?
	bne	addbase_err	! /
	sll	%o1,15,%o1
	sra	%o1,15,%o1
	add	%o1,%o0,%o0
	sethi	%hi(0xFFFFFF),%o1	!is this necessary ??
	and	%o0,%o1,%o0
.end


.inline _loloc,4
	sll	%o0,16,%o0
	srl	%o0,16,%o0
	sethi	%hi(0xE0000),%o1
	or	%o0,%o1,%o0
.end


.inline	_hiloc,4
	srl	%o0,16,%o0
	sethi	%hi(0xE0000),%o1
	or	%o0,%o1,%o0
.end

/* this really doesn't need to check */
.inline	_vag2,8
	sll	%o0,16,%o0
	sll	%o1,16,%o1
	srl	%o1,16,%o1
	or	%o0,%o1,%o0
.end
	
/*
	***************************************************************
	TYPE OPCODE VERSIONS
	***************************************************************
*/
/* TYPE INLINE OPCODES */

.inline	_listp,4
	movl	a7@+,d0
	movl	d0,d1
	lsrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movw	a0@(0,d1:l:1),d1
	andw	#0x7FF,d1
	cmpw	#5,d1
	jeq	_xwzq5
	clrl	d0
_xwzq5:
.end

.inline	_fast_op_listp,0
	movl	d7,d1
	lsrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movw	a0@(0,d1:l:1),d1
	andw	#0x7FF,d1
	cmpw	#5,d1
	jeq	_xwzq6
	clrl	d7
_xwzq6:
.end


.inline	_ntypex,4
	movl	a7@+,d1
	lsrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movl	#0x000E0000,d0
	movw	a0@(0,d1:l:1),d0
	andw	#0x7FF,d0
.end

.inline	_fast_op_ntypex,0
	lsrl	#8,d7
	andl	#0xFFFE,d7
	movl	_MDStypetbl,a0
	movw	a0@(0,d7:l:1),d7
	andw	#0x7FF,d0
	orl	#0x000E0000,d7
.end


.inline _typep,8
	movl	a7@+,d1
	movl	a7@+,d2
	movl	d1,d0
	lsrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movw	a0@(0,d1:l:1),d1
	andw	#0x7FF,d1
	cmpl	d2,d1
	beq	115$
	moveq	#0,d0
115$:
.end

.inline _fast_op_typep,0
	movl	d7,d0
	lsrl	#8,d0
	andl	#0xFFFE,d0
	movl	_MDStypetbl,a0
	movw	a0@(0,d0:l:1),d0
	andw	#0x7FF,d0
	moveq	#0,d1
	movb	a5@(1),d1
	cmpw	d1,d0
	beq	115$
	moveq	#0,d7
115$:
.end


/* TYPE INLINE FUNCTIONS */

.inline _GetTypeNumber,4
	movl	a7@+,d1
	asrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	moveq	#00,d0
	movw	a0@(0,d1:l:1),d0
	andw	#0x7FF,d0
.end

.inline _GetTypeEntry,4
	movl	a7@+,d1
	asrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movw	a0@(0,d1:l:1),d0
.end

|.inline _min,8
|	cmp	%o0,%o1
|	ble,a	$10
|	mov	%o1,%o0
|$10
|.end

|.inline _max,8
|	cmp	%o0,%o1
|	bge,a	$10
|	mov	%o1,%o0
|$10
|.end
