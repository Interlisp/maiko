/* @(#) dsp386.il Version 1.1 (4/21/92). copyright Venue & Fuji Xerox  */
/* @(#) disp386i.il Version 1.14 (5/3/90). copyright Venue & Fuji Xerox  */
//////////////////////////////////////////////////////////////////////////
//									//
//			d i s p 3 8 6 i . i l				//
//									//
//	INLINE-code definitions specific to the Sun 386i		//
//									//
//	Conventions (empirically discovered):				//
//									//
//		Caller pushes args on stack				//
//			0(%esp) = arg 1					//
//			4(%esp) = arg 2					//
//			etc.						//
//									//
//		Leave the result in %eax				//
//									//
//		YOU CAN'T USE LOCAL LABELS				//
//									//
//		jmp over code makes the code dead for -O???		//
//									//
//									//
//									//
//////////////////////////////////////////////////////////////////////////


/************************************************************************/
/*									*/
/*	Copyright 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.		*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/



/ swap the words in a long-word.
.inline swapx,1
	movl	0(%esp),%eax
	rol	$16,%eax
.end


/ swap the words in a long-word
.inline word_swap_longword,1
	movl	0(%esp),%eax
	rol	$16,%eax
.end

/swap the bytes in a 16-bit word
.inline byte_swap_word,1
	movl	0(%esp),%eax
	rolw	$8,%ax
.end


.inline word_swap_page,8
	movl	4(%esp),%ecx	/ word count into the loop counter
	movl	0(%esp),%edx	/ address of the block to swap
	movl	0(%edx),%eax
	rolw	$8,%ax
	roll	$16,%eax
	rolw	$8,%ax
	movl	%eax,0(%edx)
	add	$4,%edx
	loop	.-20
.end

.inline bit_reverse_region,16
/args: (top width height rasterwidth)
	.noopt
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	12(%esp),%edx	/top
	movl	%edx,%eax
	andl	$-2,%edx
	movl	%edx,12(%esp)
	andl	$1,%eax
	shll	$4,%eax
	addl	16(%esp),%eax	/width
	movl	20(%esp),%edx	/height
/		24(%esp)	/rasterwidth, in words
	leal	31(%eax),%eax
	sarl	$5,%eax
	leal	(%eax,%eax),%eax
	movl	%eax,16(%esp)	/ word wid now in width.
	cld			/ so we move up thru memory
	leal	reversedbits,%ebx / for xlateb
	movl	12(%esp),%edi	/ starting init of word ptr
	sub	$0,%edx
	jle	.+63		
/ ..4:
	movl	%edi,%esi	/so both edi & esi are at start of line to swap

	movl	16(%esp),%ecx	/ byte count to ecx
	leal	(%ecx,%ecx),%ecx
/ ..3:
	lodsb
	xlat
	stosb
	loop	.-3		/ ..3

	movl	16(%esp),%ecx
	incl	%ecx
	sarl	$1,%ecx
	movl	12(%esp),%esi	/ must be into esi, then edi, to
/	andl	$-2,%edi	/ defeat the peephole optimizer.
	movl	%esi,%edi
/ ..5:
	lodsl
	rolw	$8,%ax
	roll	$16,%eax
	rolw	$8,%ax
	stosl
	loop	.-13		/ ..5
	movl	12(%esp),%edi	/ word = word + rasterwidth
	addl	24(%esp),%edi
	addl	24(%esp),%edi
	movl	%edi,12(%esp)
	decl	%edx
/ .L60:	/ at this point, starting word addr is in %edi
	jg	.-59		/ ..4
	popl	%ebx
	popl	%esi
	popl	%edi
	.optim
.end

/////////////////////////////
//
//	Dispatch loop speedup functions for the 386i
//
//	Register assumptions:
//
//		pccache	  %edi
//		TOS	  %ebx
//		stk ptr	  %esi
//
//
/////////////////////////////


	///////////////////
	//  Get_BYTE_PCMAC1 -- fetch one byte at PCMAC+1 //
	//  (= pccache)
	///////////////////

.inline Get_BYTE_PCMAC0fn,0
	leal	-1(%edi),%eax
	xorl	$3,%eax
	movzbl	(%eax),%eax
.end

.inline Get_BYTE_PCMAC1fn,0
	movl	%edi,%eax
	xorl	$3,%eax
	movzbl	(%eax),%eax
.end

.inline Get_BYTE_PCMAC2fn,0
	leal	1(%edi),%eax
	xorl	$3,%eax
	movzbl	(%eax),%eax
.end

.inline Get_BYTE_PCMAC3fn,0
	leal	2(%edi),%eax
	xorl	$3,%eax
	movzbl	(%eax),%eax
.end


.inline Get_DLword_PCMAC0fn,0
/	.byte	0x33,0xdb	/ xorl %ebx,%ebx
	leal	0(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	-1(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
.end

.inline Get_DLword_PCMAC1fn,0
/	.byte	0x33,0xdb	/ xorl %ebx,%ebx
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
.end

.inline Get_DLword_PCMAC2fn,0
/	.byte	0x33,0xdb	/ xorl %ebx,%ebx
	leal	2(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	1(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
.end

.inline Get_DLword_PCMAC3fn,0
	.byte	0x33,0xdb	/ xorl %ebx,%ebx
	leal	3(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	2(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
.end


.inline fast0_dispatcher,0
	leal	-1(%edi),%eax
	xorb	$3,%al
	movzbl	(%eax),%eax
	jmp	*optable(,%eax,4)
.end

.inline fast1_dispatcher,0
	movl	%edi,%eax
	xorb	$3,%al
	movzbl	(%eax),%eax
	incl	%edi
	jmp	*optable(,%eax,4)
.end

.inline fast2_dispatcher,0
	leal	1(%edi),%eax
	xorb	$3,%al
	addl	$2,%edi
	movzbl	(%eax),%eax
	jmp	*optable(,%eax,4)
.end


	/////////////////////////////
	//
	//  Arithmetic code speedups
	//
	// Assume edi & esi are arguments
	//
	//	  ebx is result.
	//
	////////////////////////////////

.inline Xiplus32,0
	addl	%edi,%esi
	jo	iplus_err
	movl	%esi,%eax
.end

.inline Xiplus32n,0
	leal	(%edi),%eax
	addl	%esi,%eax
	jo	iplusn_err
.end


.inline Xplus32,0
	leal	(%edi),%eax
	addl	%esi,%eax
	jo	plus_err
.end

.inline Xsub32,0
	leal	(%edi),%eax
	subl	%esi,%eax
	jo	diff_err
.end

.inline Xisub32,0
	leal	(%edi),%eax
	subl	%esi,%eax
	jo	idiff_err
.end

.inline Xisub32n,0
	leal	(%edi),%eax
	subl	%esi,%eax
	jo	idiffn_err
.end

.inline	plus_err_label,0
plus_err:
.end

.inline	iplus_err_label,0
iplus_err:
.end

.inline	diff_err_label,0
diff_err:
.end

.inline	idiff_err_label,0
idiff_err:
.end

.inline	iplusn_err_label,0
iplusn_err:
.end

.inline	 idiffn_err_label,0
idiffn_err:
.end

.inline	fast_op_difference,4

	movl	0(%esp),%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	diff_err
	roll	$15,%eax
	subb	$7,%al
	jne	diff_err
	subl	%ebx,%eax
	jo	diff_err
	rorl	$15,%eax
	orl	$917504,%eax
	movl	%eax,%ebx
.end

.inline	fast_op_idifference,4

	movl	0(%esp),%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	idiff_err
	roll	$15,%eax
	subb	$7,%al
	jne	idiff_err
	subl	%ebx,%eax
	jo	idiff_err
	rorl	$15,%eax
	orl	$917504,%eax
	movl	%eax,%ebx
.end


.inline fast_op_idifferencen,4

	movl	0(%esp),%eax
	roll	$15,%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	idiffn_err
	subl	%eax,%ebx
	jo	idiffn_err
	rorl	$15,%ebx
	orl	$917504,%ebx
.end


/*
	***************************************************************
	PLUS VERSIONS sp@ + sp@(4)  i.e. (tos-1)  +  (tos)
	***************************************************************
*/


.inline	fast_op_plus,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	plus_err
	roll	$15,%eax
	subb	$7,%al
	jne	plus_err
	addl	%ebx,%eax
	jo	plus_err
	rorl	$15,%eax
	orl	$917504,%eax
	movl	%eax,%ebx
.end

.inline	fast_op_iplus,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	iplus_err
	roll	$15,%eax
	subb	$7,%al
	jne	iplus_err
	addl	%ebx,%eax
	jo	iplus_err
	rorl	$15,%eax
	orl	$917504,%eax
	movl	%eax,%ebx
.end

.inline	fast_op_iplusn,4

	movl	0(%esp),%eax
	roll	$15,%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	iplusn_err
	addl	%ebx,%eax
	jo	iplusn_err
	rorl	$15,%eax
	orl	$917504,%eax
	movl	%eax,%ebx
.end



.inline	fast_op_logor,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	cmpb	$7,%bl
	jne	logor_err
	roll	$15,%eax
	cmpb	$7,%al
	jne	logor_err
	orl	%eax,%ebx
	rorl	$15,%ebx
.end


.inline	fast_op_logand,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	cmpb	$7,%bl
	jne	logand_err
	roll	$15,%eax
	cmpb	$7,%al
	jne	logand_err
	andl	%eax,%ebx
	rorl	$15,%ebx
.end


.inline	fast_op_logxor,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	cmpb	$7,%bl
	jne	logxor_err
	roll	$15,%eax
	subb	$7,%al
	jne	logxor_err
	xorl	%eax,%ebx
	rorl	$15,%ebx
.end

.inline	fast_op_lrsh8,0
	movl	%ebx,%eax
	roll	$16,%eax
	cmpw	$0xe,%ax
	jne	lrsh8_err
	shrw	$8,%bx
.end


.inline	fast_op_lrsh1,0
	movl	%ebx,%eax
	roll	$16,%eax
	cmpw	$0xe,%ax
	jne	lrsh1_err
	shrw	$1,%bx
.end


.inline	fast_op_llsh8,0
	cmpw	$0x0FF,%bx
	jg	llsh8_err
	movl	%ebx,%eax
	roll	$16,%eax
	cmpw	$0xe,%eax
	jne	llsh8_err
	shlw	$8,%bx
.end


.inline	fast_op_llsh1,0
	cmpw	$0x07FFF,%bx
	jg	llsh1_err
	movl	%ebx,%eax
	roll	$16,%eax
	cmpw	$0xe,%ax
	jne	llsh1_err
	shlw	$1,%bx
.end


.inline	fast_op_greaterp,4

	movl	0(%esp),%eax
	movl	%ebx,%edx
	roll	$15,%edx
	subb	$7,%dl
	jne	greaterp_err
	roll	$15,%eax
	subb	$7,%al
	jne	greaterp_err
	xorl	%ebx,%ebx
	cmpl	%edx,%eax
	jle	.+7
	movl	$76,%ebx
.end


.inline	fast_op_igreaterp,4

	movl	0(%esp),%eax
	movl	%ebx,%edx
	roll	$15,%edx
	subb	$7,%dl
	jne	igreaterp_err
	roll	$15,%eax
	subb	$7,%al
	jne	igreaterp_err
	xorl	%ebx,%ebx
	cmpl	%edx,%eax
	jle	.+7
	movl	$76,%ebx
.end


.inline fast_op_addbase,4
	movl	0(%esp),%eax
	roll	$15,%ebx
	subb	$7,%bl
	jne	addbase_err
	sarl	$15,%ebx
	andl	$0xFFFFFF,%eax
	addl	%eax,%ebx
.end


.inline fast_op_loloc,0
	andl	$0x0000FFFF,%ebx
	orl	$0x000E0000,%ebx
.end



.inline	fast_op_hiloc,0
	shrl	$16,%ebx
	andl	$0x0000FFFF,%ebx
	orl	$0x000E0000,%ebx
.end


/// Unused, because 386i as peephole optimizer removes the shll.
.inline	fast_op_vag2,4
	movl	0(%esp),%eax
	shll	$16,%ebx
	movw	%ax,%bx
	rorl	$16,%ebx
.end
	


.inline	_fast_op_listp,0
	movl	d7,d1
	lsrl	#8,d1
	andl	#0xFFFE,d1
	movl	_MDStypetbl,a0
	movw	a0@(0,d1:l:1),d1
	andw	#0x7FF,d1
	cmpw	#5,d1
	jeq	110$
	clrl	d7
110$:
.end


.inline	_fast_op_ntypex,0
	lsrl	#8,d7
	andl	#0xFFFE,d7
	movl	_MDStypetbl,a0
	movw	a0@(0,d7:l:1),d7
	andw	#0x7FF,d0
	orl	#0x000E0000,d7
.end


.inline _fast_op_typep,0
	movl	d7,d0
	lsrl	#8,d0
	andl	#0xFFFE,d0
	movl	_MDStypetbl,a0
	movw	a0@(0,d0:l:1),d0
	andw	#0x7FF,d0
	moveq	#0,d1
	movb	a5@,d1
	cmpw	d1,d0
	beq	115$
	moveq	#0,d7
115$:
.end

/////////////////////////////////////////////////////////////////
///
///	F U N C T I O N   C A L L   S P E E D - U P S
///
/////////////////////////////////////////////////////////////////


.inline fast_FN0,0
	cld			// so we can use string instructions
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.LFN0ccode
	movl	%eax,-12(%ebp)
	movl	$0,-24(%ebp)
	movl	$3,-20(%ebp)
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.LFN0ccode:
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	leal	4(%edi),%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
/	movl	%ecx,%eax
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN0noarg1
	xorl	%eax,%eax
/	movl	%ecx,%edx
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax		// for storing NILs.
	jmp	.FN0argls1
.FN0arglp1:
/	movl	$0,(%edi)
/	addl	$4,%edi
	stosl
	incl	-48(%ebp)
.FN0argls1:
	cmpl	$0,-48(%ebp)
	jl	.FN0arglp1
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN0noarg1:
/	movl	%edi,%eax
	movl	-44(%ebp),%eax
/	addl	$4,%edi
	orl	$-2147483648,%eax
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
/	pushl	-40(%ebp)
	movl	-40(%ebp),%eax
	rol	$16,%eax
/	popl	%ecx
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
/	movl	%ecx,%eax
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN0disp
/	movl	$-1,-52(%ebp)
/	movl	%edi,%eax
	movl	$-1,%eax
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN0disp
.FNxxalp:
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	decl	-48(%ebp)
.FN0xalp:
	decl	-48(%ebp)
	jl	.FN0disp
/	movl	%edi,%eax
/	movl	-52(%ebp),%edx
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	jmp	.FN0xalp
.FN0disp:
	movzwl	4(%ecx),%eax
	movl	%ecx,%edx
	addl	$4,%edi
	addl	%eax,%edx
	incl	%edx
	movl	%edx,%esi
	movl	%ecx,MachineState+20
	leal	-1(%esi),%eax
	xorb	$3,%al
	xchg	%edi,%esi	/ done using edit to store fast with.
	movzbl	(%eax),%eax
	jmp	*optable(,%eax,4)
.end

.inline fast_FN1,0
	cld			// for string instruction use
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN1ccode
	movl	%eax,-12(%ebp)
	movl	$1,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN1ccode:
/	movl	-40(%ebp),%eax
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
/	movl	%eax,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
/	movl	%ecx,%eax
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
/	movl	%edi,%eax
	movl	%edi,MachineState
	movl	%edi,-48(%ebp)
	movl	MachineState+4,%eax
	movl	%edi,%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
/	movl	%ecx,%eax
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN1noarg
	movl	$1,%eax
/	movl	%ecx,%edx
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN1lps
.FN1lp1:
/	movl	$0,(%edi)
/	addl	$4,%edi
	stosl
	incl	-48(%ebp)
.FN1lps:
	cmpl	$0,-48(%ebp)
	jl	.FN1lp1
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN1noarg:
/	movl	%edi,%eax
	movl	-44(%ebp),%edx
/	addl	$4,%edi
	orl	$-2147483648,%edx
	movl	%edx,(%edi)
	addl	$4,%edi
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
/	movl	$-1,-52(%ebp)
/	movl	%edi,%eax
/	movl	-52(%ebp),%edx
	movl	$-1,%eax
/	movl	%eax,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	decl	-48(%ebp)
.FN1xlp:
	decl	-48(%ebp)
	jl	.FN1disp
/	movl	%edi,%eax
/	movl	-52(%ebp),%edx
/	movl	%edx,(%eax)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	jmp	.FN1xlp

	// can be pointed to FN0disp when we do the ecx change here.
.FN1disp:
	movzwl	4(%ecx),%eax
	movl	%ecx,%esi
	addl	$4,%edi
	addl	%eax,%esi
	incl	%esi
/	movl	%edx,%esi
	movl	%ecx,MachineState+20
	leal	-1(%esi),%eax
	xchg	%edi,%esi	/ done using edi to store fast with.
	xorb	$3,%al
	movzbl	(%eax),%eax
	jmp	*optable(,%eax,4)

.end

.inline fast_FN2,0
	cld			// for string ops, so they push upward.
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN2ccode
	movl	%eax,-12(%ebp)
	movl	$2,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN2ccode:
/	movl	-40(%ebp),%eax
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
/	movl	%eax,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
/	movl	%ecx,%eax
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$4,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
/	movl	%ecx,%eax
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN2noarg
	movl	$2,%eax
/	movl	%ecx,%edx
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN2lps
.FN2lp:
/	movl	$0,(%edi)
/	addl	$4,%edi
	stosl
	incl	-48(%ebp)
.FN2lps:
	cmpl	$0,-48(%ebp)
	jl	.FN2lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN2noarg:
/	movl	%edi,%eax
	movl	-44(%ebp),%eax
/	addl	$4,%edi
	orl	$-2147483648,%eax
/	movl	%edx,(%eax)
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
/	movl	$-1,-52(%ebp)
/	movl	%edi,%eax
	movl	$-1,%eax
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	decl	-48(%ebp)
.FN2xlp:
	decl	-48(%ebp)
	jl	.FN1disp
/	movl	%edi,%eax
/	movl	-52(%ebp),%edx
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	jmp	.FN2xlp
.end


.inline fast_FN3,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN3ccode
	movl	%eax,-12(%ebp)
	movl	$3,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN3ccode:
	cld			// so string ops work upward.
/	movl	-40(%ebp),%eax
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
/	movl	%eax,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
/	movl	%ecx,%eax
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$8,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
/	movl	%ecx,%eax
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN3noarg
	movl	$3,%eax
/	movl	%ecx,%edx
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN3lps
.FN3lp:
/	movl	$0,(%edi)
/	addl	$4,%edi
	stosl
	incl	-48(%ebp)
.FN3lps:
	cmpl	$0,-48(%ebp)
	jl	.FN3lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN3noarg:
/	movl	%edi,%eax
	movl	-44(%ebp),%eax
/	addl	$4,%edi
	orl	$-2147483648,%eax
/	movl	%edx,(%eax)
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
/	movl	$-1,-52(%ebp)
/	movl	%edi,%eax
	movl	$-1,%eax
/	movl	%edx,(%eax)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	decl	-48(%ebp)
.FN3xlp:
	decl	-48(%ebp)
	jl	.FN1disp
/	movl	%edi,%eax
/	movl	-52(%ebp),%edx
/	movl	%edx,(%edi)
/	addl	$4,%edi
/	movl	%edx,(%edi)
/	addl	$4,%edi
	stosl
	stosl
	jmp	.FN3xlp
.end


.inline fast_FN4,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN4ccode
	movl	%eax,-12(%ebp)
	movl	$4,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN4ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$12,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN4noarg
	movl	$4,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN4lps
.FN4lp:
	stosl
	incl	-48(%ebp)
.FN4lps:
	cmpl	$0,-48(%ebp)
	jl	.FN4lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN4noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN4xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN4xlp
.end



.inline fast_FN5,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN5ccode
	movl	%eax,-12(%ebp)
	movl	$5,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN5ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$16,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN5noarg
	movl	$5,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN5lps
.FN5lp:
	stosl
	incl	-48(%ebp)
.FN5lps:
	cmpl	$0,-48(%ebp)
	jl	.FN5lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN5noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN5xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN5xlp
.end



.inline fast_FN6,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN6ccode
	movl	%eax,-12(%ebp)
	movl	$6,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN6ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$20,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN6noarg
	movl	$6,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN6lps
.FN6lp:
	stosl
	incl	-48(%ebp)
.FN6lps:
	cmpl	$0,-48(%ebp)
	jl	.FN6lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN6noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN6xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN6xlp
.end



.inline fast_FN7,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN7ccode
	movl	%eax,-12(%ebp)
	movl	$7,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN7ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$24,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN7noarg
	movl	$7,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN7lps
.FN7lp:
	stosl
	incl	-48(%ebp)
.FN7lps:
	cmpl	$0,-48(%ebp)
	jl	.FN7lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN7noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN7xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN7xlp
.end



.inline fast_FN8,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN8ccode
	movl	%eax,-12(%ebp)
	movl	$8,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN8ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$28,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN8noarg
	movl	$8,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN8lps
.FN8lp:
	stosl
	incl	-48(%ebp)
.FN8lps:
	cmpl	$0,-48(%ebp)
	jl	.FN8lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN8noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN8xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN8xlp
.end



.inline fast_FN9,0
	leal	1(%edi),%edx
	xorl	$3,%edx
	movzbl	(%edx),%eax
	leal	0(%edi),%edx
	xorl	$3,%edx
	movb	(%edx),%ah
	movl	%eax,%ecx
	leal	(,%eax,4),%eax
	addl	Defspace,%eax
	movl	(%eax),%edx
	testl	%edx,%edx
	jl	.FN9ccode
	movl	%eax,-12(%ebp)
	movl	$9,-24(%ebp)
	movl	$3,-20(%ebp)
	movl	%ecx,-16(%ebp)
	movl	$0,-28(%ebp)
	jmp	fn_common
.FN9ccode:
	cld			// so string ops work upward.
	andl	$16777215,%edx
	movl	%edx,-40(%ebp)
	leal	(%edx,%edx),%ecx
	addl	Lisp_world,%ecx
	movl	MachineState+4,%eax
	leal	-1(%edi),%edx
	subl	MachineState+20,%edx
	leal	3(%edx),%edx
	movw	%dx,-12(%eax)
	movzwl	2(%ecx),%eax
	leal	(%eax,%eax),%eax
	movl	MachineState+32,%edx
	subl	%eax,%edx
	movl	%edx,MachineState+28
	cmpl	%edx,%esi
	jg	check_interrupt
	xchg	%edi,%esi
	movl	%edi,%eax
	subl	$32,%eax
	movl	%eax,MachineState
	movl	%eax,-48(%ebp)
	movl	MachineState+4,%eax
	movl	-48(%ebp),%edx
	subl	Stackspace,%edx
	shrl	$1,%edx
	movl	%edx,-44(%ebp)
	movw	%dx,-10(%eax)
	movl	%ebx,(%edi)
	movw	(%ecx),%dx
	addl	$4,%edi
	testw	%dx,%dx
	jl	.FN9noarg
	movl	$9,%eax
	movswl	(%ecx),%edx
	subl	%edx,%eax
	movl	%eax,-48(%ebp)
	xorl	%eax,%eax
	jmp	.FN9lps
.FN9lp:
	stosl
	incl	-48(%ebp)
.FN9lps:
	cmpl	$0,-48(%ebp)
	jl	.FN9lp
	movl	-48(%ebp),%eax
	leal	(,%eax,4),%eax
	subl	%eax,%edi
.FN9noarg:
	movl	-44(%ebp),%eax
	orl	$-2147483648,%eax
	stosl
	movl	MachineState+4,%eax
	subl	Stackspace,%eax
	shrl	$1,%eax
	orl	$-1073741824,%eax
	movl	%eax,(%edi)
	movl	-40(%ebp),%eax
	rol	$16,%eax
	movl	%eax,4(%edi)
	addl	$20,%edi
	movl	%edi,MachineState+4
	movswl	6(%ecx),%eax
	movl	%eax,-48(%ebp)
	cmpl	$0,-48(%ebp)
	jl	.FN1disp
	movl	$-1,%eax
	stosl
	stosl
	cmpl	$0,-48(%ebp)
	jle	.FN1disp
	stosl
	stosl
	decl	-48(%ebp)
.FN9xlp:
	decl	-48(%ebp)
	jl	.FN1disp
	stosl
	stosl
	jmp	.FN9xlp
.end






//////////////////////////////////////////////////
///
///		assembler label definitions
///
/////////////////////////////////////////////////

.inline	asm_label_check_interrupt,0
check_interrupt:
.end
.inline	asm_label_op_fn_common,0
op_fn_common:
.end
