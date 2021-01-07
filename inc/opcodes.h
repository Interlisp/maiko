#ifndef OPCODES_H
#define OPCODES_H 1
/* $Id: opcodes.h,v 1.2 1999/01/03 02:06:19 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*		    O P C O D E   D E F I N I T I O N S			*/
/*									*/
/*		   Symbolic equivalents for Lisp Opcodes		*/
/*									*/
/************************************************************************/


/************************************************************************/
/*									*/
/*	Copyright 1989, 1990 Venue, Fuji Xerox Co., Ltd, Xerox Corp.	*/
/*									*/
/*	This file is work-product resulting from the Xerox/Venue	*/
/*	Agreement dated 18-August-1989 for support of Medley.		*/
/*									*/
/************************************************************************/


#define	opc_X		000
#define opc_CAR		001
#define opc_CDR		002
#define opc_LISTP	003
#define opc_NTYPEX	004
#define opc_TYPEP	005
#define opc_UNWIND	007
#define opc_FN0		010
#define opc_FN1		011
#define opc_FN2		012
#define opc_FN3		013
#define opc_FN4		014
#define opc_FNX		015
#define opc_APPLY	016
#define opc_RETURN	020
#define opc_UNBIND	022
#define opc_DUNBIND	023
#define opc_SLRETURN	077
#define opc_JUMP	0200
#define opc_FJUMP	0220
#define opc_TJUMP	0240
#define opc_JUMPX	0260
#define opc_JUMPXX	0261
#endif /* OPCODES_H */
