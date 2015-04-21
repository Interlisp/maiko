/* $Id: mvs.c,v 1.3 1999/05/31 23:35:40 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: mvs.c,v 1.3 1999/05/31 23:35:40 sybalsky Exp $ Copyright (C) Venue";






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



/************************************************************************/
/*									*/
/*		M U L T I P L E - V A L U E   S U P P O R T		*/
/*			    F U N C T I O N S				*/
/*									*/
/*	Contains: values, values_list					*/
/*									*/
/************************************************************************/

#include <stdio.h>


#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "emlglob.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "stack.h"
#include "opcodes.h"

#ifdef AIXPS2
#include "inlnPS2.h"
#endif /* AIXPS2 */

 /* to optionally swap the fnhead field of a frame */
#ifdef BIGVM
#define SWA_FNHEAD
#else
#define SWA_FNHEAD swapx
#endif /* BIGVM */



LispPTR MVLIST_index;


/****************************************************************/
/*                                                              */
/*                            VALUES                            */
/*								*/
/*         C-coded version of the function CL:VALUES		*/
/*								*/
/****************************************************************/

LispPTR values (int arg_count, register LispPTR *args)
{
    FX2 *caller, *prevcaller=0, *immediate_caller=0;
    ByteCode *pc;
    int unbind_count=0;
    struct fnhead *fnhead;
    int byteswapped;  /* T if on 386 & reswapped code block */
	short opcode;

    caller = (FX2 *) CURRENTFX;
    immediate_caller = caller;

newframe:
    if (caller == immediate_caller)
      {
	fnhead = (struct fnhead *) FuncObj;
	pc = (ByteCode *) PC+3; /* to skip the miscn opcode we're in now */
      }
    else
      {
	fnhead = (struct fnhead *)
		    Addr68k_from_LADDR(POINTERMASK & SWA_FNHEAD((int)caller->fnheader));
	pc = (ByteCode *)fnhead+(caller->pc);
      }
#ifdef ISC
	if(!fnhead->byteswapped)
	  {
		byte_swap_code_block(fnhead);
		fnhead->byteswapped=1;
      }
#endif /* ISC */



	  
newpc:
#ifdef ISC
    opcode = (short)((unsigned char) *((char *)pc));
#else
	opcode = (short)((unsigned char)GETBYTE((char *)pc));
#endif
	switch (opcode)
      {
	case opc_RETURN:
	case opc_SLRETURN:  prevcaller = caller;
			    caller = (FX2 *) (Stackspace+(unsigned)(GETCLINK(caller)));
			    goto newframe;

	case opc_FN1:   if (MVLIST_index == Get_code_AtomNo(pc+1))
			 {
			   if (unbind_count > 0)
			     simulate_unbind(caller, unbind_count, prevcaller);
#ifndef BIGATOMS
			    /* would add 3 to  PC, but miscn return code does.*/
			   if (caller == immediate_caller) PC = pc;
#else
			   /* BUT 3's not enough for big atoms, so add diff between FN op size & MISCN op size */
			   if (caller == immediate_caller) PC = pc + (FN_OPCODE_SIZE-3);
#endif /* BIGATOMS */

			   else caller->pc = (UNSIGNED)pc+ FN_OPCODE_SIZE-(UNSIGNED)fnhead;
			   return(make_value_list(arg_count, args));
			 }
			break;

	case opc_UNBIND:  pc += 1;
			  unbind_count += 1;
			  goto newpc;

	case opc_JUMPX:  {
			   register short displacement;
#ifdef ISC
			   displacement = (short) (*((char *)pc+1));
#else
			   displacement = (short) (GETBYTE((char *)pc+1));
#endif
			   if (displacement >= 128) displacement -= 256;
			   pc += displacement;
			   goto newpc;
			 }

	case opc_JUMPXX:  {
			    register int displacement;
			    displacement = (int) Get_code_DLword(pc+1);
			    if (displacement >= 32768) displacement -= 65536;
			    pc += displacement;
			    goto newpc;
			  }
	default:  if ((opcode >= opc_JUMP) && (opcode < opc_FJUMP))
		    {
		      pc += 2 + opcode - opc_JUMP;
		      goto newpc;
		    }
      }

	/*****************************************/
	/* Default case:  Return a single value. */
	/*****************************************/

    if (arg_count>0) return(args[0]);
    else return(NIL_PTR);
  }




/****************************************************************/
/*                                                              */
/*                            VALUES_LIST         		*/
/*								*/
/*         C-coded version of the function CL:VALUES-LIST 	*/
/*								*/
/****************************************************************/

LispPTR values_list (int arg_count, register LispPTR *args)
{
    FX2 *caller, *prevcaller=0, *immediate_caller=0;
    ByteCode *pc;
    int unbind_count=0;
    struct fnhead *fnhead;
    short opcode;

    caller = (FX2 *) CURRENTFX;
    immediate_caller = caller;

newframe:
    if (caller == immediate_caller)
      {
	fnhead = (struct fnhead *) FuncObj;
	pc = (ByteCode *) PC+3; /* Skip over the miscn opcode we're in now */
      }
    else
      {
	fnhead = (struct fnhead *)
		    Addr68k_from_LADDR(POINTERMASK & SWA_FNHEAD((int)caller->fnheader));
	pc = (ByteCode *)fnhead+(caller->pc);
      }


#ifdef ISC
	if(!fnhead->byteswapped)
	  {
		byte_swap_code_block(fnhead);
		fnhead->byteswapped=1;
      }
#endif /* ISC */



	  
newpc:
#ifdef ISC
    opcode = (short)((unsigned char) *((char *)pc));
#else
	opcode = (short)((unsigned char)GETBYTE((char *)pc));
#endif
    switch (opcode)
      {
	case opc_RETURN:
	case opc_SLRETURN:  prevcaller = caller;
			    caller = (FX2 *) (Stackspace+(int)(GETCLINK(caller)));
			    goto newframe;

	case opc_FN1:   if (MVLIST_index == Get_code_AtomNo(pc+1))
			 {
			   if (unbind_count > 0) 
			     simulate_unbind(caller, unbind_count, prevcaller);
			    /* would add 3 to PC, but miscn ret code does. */
#ifndef BIGATOMS
			   if (caller == immediate_caller) PC = pc;
#else
			   /* BUT 3's not enough for big atoms, so add 1 */
			   if (caller == immediate_caller) PC = pc + (FN_OPCODE_SIZE-3);
#endif /* BIGATOMS */

			   else caller->pc = (UNSIGNED)pc+ FN_OPCODE_SIZE-(UNSIGNED)fnhead;
			   return(args[0]);
			 }
			break;

	case opc_UNBIND:  pc += 1;
			  unbind_count += 1;
			  goto newpc;

	case opc_JUMPX:  {
			   register short displacement;
#ifdef ISC
			 displacement = (short) (*((char *)pc+1));
#else
			 displacement = (short) (GETBYTE((char *)pc+1));
#endif
			 if (displacement >= 128) displacement -= 256;
			   pc += displacement;
			   goto newpc;
			 }

	case opc_JUMPXX:  {
			    register int displacement;
			    displacement = (int) Get_code_DLword(pc+1);
			    if (displacement >= 32768) displacement -= 65536;
			    pc += displacement;
			    goto newpc;
			  }
	default:  if ((opcode >= opc_JUMP) && (opcode < opc_FJUMP))
		    {
		      pc += 2 + opcode - opc_JUMP;
		      goto newpc;
		    }
      }

	/*****************************************/
	/* Default case:  Return a single value. */
	/*****************************************/

    if (Listp(args[0])) return(car(args[0]));
    else return(args[0]);
  }


/************************************************************************/
/*									*/
/*			m a k e _ v a l u e _ l i s t			*/
/*									*/
/*	Given a count of values to return, and a pointer to an		*/
/*	array containing the values, CONS up a list that contains	*/
/*	the values.  This is because MVs are really returned on		*/
/*	the stack as a list -- SHOULD BE CHANGED!			*/
/*									*/
/************************************************************************/

make_value_list(int argcount, LispPTR *argarray)
{
    register LispPTR result = NIL_PTR;
    register int i;
    if (argcount == 0) return(NIL_PTR);
    for (i = argcount-1; i>=0; i--)
      {
	result = cons(argarray[i], result);
      }
    return(result);
  }



/************************************************************************/
/*									*/
/*		     s i m u l a t e _ u n b i n d			*/
/*									*/
/*	Simulate the effect of UNBIND on a frame, to back us up		*/
/*	to where we ought to be when we return multiple values.		*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

simulate_unbind(FX2 *frame, int unbind_count, FX2 *returner)
{
    int unbind;
    LispPTR *stackptr;
    DLword *nextblock;
    stackptr = (LispPTR *) (Stackspace+frame->nextblock);
    nextblock = (DLword *) stackptr;
    for (unbind = 0; unbind<unbind_count; unbind++)
      {
	register int value;
	register LispPTR *lastpvar;
	int bindnvalues;
	for (;((int)*--stackptr>=0);); /* find the binding mark */
	value = (int)*stackptr;
	lastpvar = (LispPTR *) ((DLword *)frame + FRAMESIZE + 2 + GetLoWord(value));;
	bindnvalues = (~value)>>16;
	for(value=bindnvalues; --value >= 0;){*--lastpvar = 0xffffffff;}
	/* This line caused \NSMAIL.READ.HEADING to smash memory, */
	/* so I removed it 21 Jul 91 --JDS.  This was the only	  */
	/* difference between this function and the UNWIND code   */
	/* in inlineC.h						  */
/*	MAKEFREEBLOCK(stackptr, (DLword *)stackptr-nextblock); */
      }
    if (returner) returner->fast = 0;	/* since we've destroyed congituity
					/* in the stack, but that only
					   matters if there's a return. */
  }


