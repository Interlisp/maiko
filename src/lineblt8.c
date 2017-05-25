/* $Id: lineblt8.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: lineblt8.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ Copyright (C) Venue";







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



#include <sys/types.h>
#include "lispemul.h"

#ifdef ISC
#include <sys/bsdtypes.h>
#endif /* ISC */


#define COLOR8ARRAYSIZE	16
#define BITSPERNIBBLE	4
#define BITSPERDLWORD 16

DLword BitMaskArray[]={32768,16384,8192,4096,2048,1024,
				512,256,128,64,32,16,8,4,2,1};
unsigned int ConvBM_tbl[] = { 
	         0 ,       0xff ,     0xff00 ,     0xffff , 
	  0xff0000 ,   0xff00ff ,   0xffff00 ,   0xffffff , 
	0xff000000 , 0xff0000ff , 0xff00ff00 , 0xff00ffff ,
	0xffff0000 , 0xffff00ff , 0xffffff00 , 0xffffffff };
unsigned int BMask_tbl[] = { 0xf , 7 , 3 , 1 };

#define noop 1

/*************************************************************** 
	Macro:WriteLongW
**************************************************************/
#define WriteLongW(srcpattern,destptr,op1,op2) {\
	register cnt;\
	register u_char *des,*src;\
	for(cnt=0,des=(u_char*)destptr,src=(u_char*)(&(srcpattern));\
	 cnt <4 ; cnt++,des++,src++)\
		(*des) op1 (*src);}


/*************************************************************** 
	Macro:LineBLT8
	srcWptr :		DLword ptr
	offset  :		Bits offset
	width	   :		Bits width to copy
	dstLptr :		Destination's LispPTR ptr
	op1,op2 :		operations in C(=,|=,^=,&=,^)
				ERASE only uses op2
**************************************************************/
 /* srcw points DLword which contains the nibble processed now */
 /* offset indicates MSB in the nibble */
 /* width:before copy,make sure the nibble is inside source bitmap */
 /* width:after copy,decremented by BITSPERNIBBLE.
	therefore,width indictes the rest bits in source bitmap */

#define LineBLT8(srcWptr,offset,width,dstLptr,op1,op2){  					\
  register DLword *srcw;										\
  register u_int temp1;										\
 for(srcw= (srcWptr) + (offset)/BITSPERDLWORD ;		\
      ((width) -BITSPERNIBBLE)>= 0;			\
      (width) -=BITSPERNIBBLE,				\
      (dstLptr) = (u_char *) ((u_int *)(dstLptr) + 1),	\
       (offset)+=BITSPERNIBBLE)				\
   {\
 switch((offset) % 16) {\
 case 0 :   WriteLongW(color_array[(*srcw )>> 12],dstLptr,op1,op2);\
	    break;\
 case 1 :   WriteLongW(color_array[(*srcw & 0x7800) >> 11],dstLptr,op1,op2);	\
	   break;\
 case 2 :   WriteLongW(color_array[(*srcw & 0x3c00) >> 10],dstLptr,op1,op2);	\
	   break;\
 case 3 :   WriteLongW(color_array[(*srcw & 0x1e00) >> 9],dstLptr,op1,op2);\
	   	break;\
 case 4 :   WriteLongW(color_array[(*srcw & 0x0f00) >> 8],dstLptr,op1,op2);\
	   	break;\
 case 5 :   WriteLongW(color_array[(*srcw & 0x0780) >> 7],dstLptr,op1,op2);\
	   	break;\
 case 6 :   WriteLongW(color_array[(*srcw & 0x03c0) >> 6],dstLptr,op1,op2);\
	   	break;\
 case 7 :   WriteLongW(color_array[(*srcw & 0x01e0) >> 5],dstLptr,op1,op2);\
	   	break;\
 case 8 :   WriteLongW(color_array[(*srcw & 0x00f0) >> 4],dstLptr,op1,op2);\
	   	break;\
 case 9 :   WriteLongW(color_array[(*srcw & 0x0078) >> 3],dstLptr,op1,op2);\
	     break;\
 case 10 :  WriteLongW(color_array[(*srcw & 0x003c) >> 2],dstLptr,op1,op2);\
	   	break;\
 case 11 :  WriteLongW(color_array[(*srcw & 0x001e) >> 1],dstLptr,op1,op2);\
	   	break;\
 case 12 :  WriteLongW(color_array[*srcw & 0xf],dstLptr,op1,op2);\
		 srcw++; /* move srcw pointer */\
		 break;\
 case 13 : temp1= (*srcw & 7) << 1 ;\
	   temp1 |= ((*(++srcw)) & 0x8000)>> 15 ;/** Compiler's BUG?*/\
		 WriteLongW(color_array[temp1],dstLptr,op1,op2);\
		 break;\
 case 14 : temp1= ((*srcw & 3) << 2) ;\
	   temp1 |= ((*(++srcw) & 0xc000)>> 14) ;\
		 WriteLongW(color_array[temp1],dstLptr,op1,op2);\
		 break;\
 case 15 : temp1= ((*srcw & 1) << 3);\
	   temp1 |= ((*(++srcw) & 0xe000)>> 13) ;\
		 WriteLongW(color_array[temp1],dstLptr,op1,op2);\
		 break;\
 } /* switch end */\
 } /* for end */\
/* process for the rest bits (0~3)*/\
switch(width)\
{ register u_char *destc;\
  register int mod;\
 case 0: /* already finished */\
		break;\
 case 1: \
 case 2:\
 case 3:	destc=(u_char *)dstLptr;\
		while(width--){\
			if(BitMaskArray[mod =(offset %16)] & *srcw) \
				(*destc++) op1 (color1);\
			else    (*destc++) op1 (color0);\
			if(mod == 15) srcw++;\
			offset++;\
		} /* WHILE END */\
		break;\
 default: ;/* error */\
}/* switch end */\
} /* MACRO END */


/************************************************************************/
/*									*/
/*			l i n e b l t 8					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/* destination is always ColorFontCache's line */

/* I don't care sourcetype & operation NOW */

void lineBlt8(DLword *srcbase, register int offset, register u_char *destl, register int width, u_char color0, u_char color1, LispPTR sourcetype, LispPTR operation)
                  
                      
  /* register u_int *destl;*/
                          /* for SPARC */
                     
                 /* Background color */
                 /* foreground color */
                     
                     /*operation type PAINT,REPLACE or INVERT */

  {
    static unsigned char beforecolor0=0;
    static unsigned char beforecolor1=0;
    static color_array[COLOR8ARRAYSIZE];

    extern DLword  REPLACE_atom;
    extern DLword  INPUT_atom;
    extern DLword  INVERT_atom;
    extern DLword  PAINT_atom;
    extern DLword  ERASE_atom;



    DLword *baseword; /* Assume dest word always quad aligned */


    if(sourcetype == INVERT_atom)
      {
	register tempcol;
	tempcol = color0;
	color0 = color1;
	color1 = tempcol;
      }
    if((beforecolor0 != color0) || (beforecolor1 != color1))
      {
	/* making color-mapped array */
	register i;
	register u_int longcol0,longcol1;

	beforecolor0 = color0;
	beforecolor1 = color1;
	longcol0 = (color0 << 24) | (color0 << 16) | (color0 << 8) | color0;
	longcol1 = (color1 << 24) | (color1 << 16) | (color1 << 8) | color1;
 
	for(i=0;i < COLOR8ARRAYSIZE;i++)
	  {
	    color_array[i] = ConvBM_tbl[i] & longcol1;
	    color_array[i] |= (~(ConvBM_tbl[i])) & longcol0;
	  }
      } /* otherwise previous colors are same as current one. use cached one */

    if(operation == REPLACE_atom)
      {	
	LineBLT8(srcbase,offset,width,destl,=,noop);
      }
    else if (operation ==  INVERT_atom)
      {	
	LineBLT8(srcbase,offset,width,destl,^=,noop);
      }
    else if (operation ==  PAINT_atom)
      {	
	LineBLT8(srcbase,offset,width,destl,|=,noop);
      }
    else if (operation ==  ERASE_atom)
      { /*erase */
	LineBLT8(srcbase,offset,width,destl,&=~,noop);
      }
    else error("lineBlt8:Illegal operation specified");
 
  }
/* lineBlt8 end */

