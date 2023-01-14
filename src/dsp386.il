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
