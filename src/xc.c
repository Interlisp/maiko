/* $Id: xc.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: xc.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ Copyright (C) Venue";




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
/*		  M A I N   D I S P A T C H   L O O P			*/
/*									*/
/*	This file contains the main dispatch loop for the emulator.	*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef OSF1
#include "time.h"
#endif

#include <sys/types.h>
#ifdef DOS
#include <i32.h>      /* Defines "#pragma interrupt"  */
#include <stk.h>      /* _XSTACK struct definition    */
#include <dos.h>      /* Defines REGS & other structs */
#else /* DOS */
#include <sys/time.h>
#endif /* DOS */
#include <stdio.h>


#include "lispemul.h"
#include "emlglob.h"
#include "address.h"
#include "adr68k.h"
#include "stack.h"
#include "dbprint.h"

#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "cell.h"
#include "initatms.h"
#include "gc.h"
#include "arith.h"
#include "stream.h"

#include "tos1defs.h" 
#include "tosret.h"
#include "tosfns.h"
#include "inlineC.h"

#ifdef DOS
#include "iopage.h"
extern IOPAGE *IOPage68K;
#include "devif.h"
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
extern MouseInterface currentmouse;
#endif /* DOS */

#ifdef SUN3_OS3_OR_OS4_IL
#include "inln68k.h"
#ifdef UNSAFE
#include "fastinln68k.h"
#endif
#endif

typedef struct conspage ConsPage;
typedef ByteCode *InstPtr;


#ifdef GCC386
    register InstPtr pccache asm("si");
	register LispPTR *cspcache asm("di");
	register LispPTR tscache asm("bx");
#include "inlnPS2.h"
#elif (DOS && OPDISP)
#include "inlndos.h"
    register InstPtr pccache asm("si");
	register LispPTR *cspcache asm("di");
	register LispPTR tscache asm("bx");
#endif /* DOS */


	/* Used to just be ifdef sparc, but want to be able to turn */
	/* off the inline code even on sparc machines.		    */
#ifdef SPARCDISP
#include "inlnSPARC.h"
#endif /* SPARCDISP */


#ifdef I386
#include "inln386i.h"
#endif

#include "fast_dsp.h"
#include "profile.h"

/* trick now is that pccache points one ahead... */
#define PCMAC 		(pccache-1)
#define PCMACL		pccache
#define CSTKPTR  	((LispPTR *) cspcache)
#define	PVAR		((LispPTR *) PVar)
#define IVAR		((LispPTR *) IVar)
#define BCE_CURRENTFX	((struct  frameex2 *)((DLword *) PVAR - FRAMESIZE))

#define CSTKPTRL	(cspcache)
#define PVARL		PVar
#define IVARL		IVar

extern DLword *createcell68k(unsigned int type);

#ifdef DOS
extern unsigned char  inchar;
extern unsigned short kn;
#endif

#ifdef XWINDOW
extern int Event_Req;	/* != 0 when it's time to check X events
						   on machines that don't get them reliably
						   (e.g. Suns running OpenWindows) */
#endif /* XWINDOW */

#ifndef ISC
#ifndef DOS
#ifdef OPDISP
InstPtr optable[512];
#endif /* OPDISP */
#endif /* DOS */
#endif /* ISC */



#ifdef PCTRACE
  /* For keeping a trace table (ring buffer) of 100 last PCs */
int pc_table[100],	/* The PC */
    op_table[100];	/* The opcode at that PC */
LispPTR fn_table[100];	/* The code block the PC is in (Lisp ptr) */
int pccounter = 0;	/* ring-buffer counter */
#endif /* PCTRACE */

int dbgflag = 0;

int	extended_frame;		/*indicates if soft stack overflow */


int n_mask_array[16] = {	1, 3, 7, 0xf, 
			0x1f, 0x3f, 0x7f, 0xff,
			0x1ff, 0x3ff, 0x7ff, 0xfff,
			0x1fff, 0x3fff, 0x7fff, 0xffff};

extern int TIMER_INTERVAL;


dispatch(void)
{
	register InstPtr pccache;

#ifdef I386
	InstPtr *table;
#elif DOS
	InstPtr * table;
#else
#if defined(OPDISP) || defined(SPARCDISP)
#ifdef ISC
  InstPtr *table;
#else
register InstPtr *table;
#endif
#endif /* OPDISP */
#endif /* I386 */

#ifdef ISC
#elif (DOS && OPDISP)
#else
	register LispPTR *cspcache;
	register LispPTR tscache;
#endif /* ISC */

#ifdef sparc
	register struct state *stateptrcache = MState;
#undef MState
#define MState stateptrcache
#endif

#if (defined(I386) || defined(ISC))
	int SaveD6;
#else
#ifdef OPDISP
#ifndef DOS
	register int SaveD6;
#endif
#endif
#endif

#ifdef UNSAFE
	register int Save_D5_shift_amount;
#endif

/* OP_FN_COMMON arguments */

    DefCell *fn_defcell;
    LispPTR fn_atom_index;
    int fn_opcode_size;
    int fn_num_args;
    int fn_apply;
    LispPTR fn_loc_defcell;

	RET; 
	CLR_IRQ;

#ifndef ISC
#ifndef DOS
#ifdef OPDISP
	table = optable;
#endif
#endif
#endif /* ISC */


#ifdef SPARCDISP
	table = (InstPtr *) compute_dispatch_table();
#endif

#ifdef UNSAFE
	Save_D5_shift_amount = 15;
#endif

#ifdef I386
	goto setup_table;
#else
#ifdef OPDISP
#ifdef ISC
	asm("	leal optable,%%eax \n\
		movl %%eax,%0" : "=g" (table) : "0" (table));
	goto nextopcode;
#elif (DOS && OPDISP)
	asm("	lea eax,optable \n\
		mov %0,%%eax" : "=g" (table) : "0" (table));
    asm volatile("fldcw WORD PTR CODE32:FP_noint"); /* Turn off FP interrupts */
	goto nextopcode;
#else
	SaveD6 = 0;
	goto setup_table;
#endif /* ISC */

#else
	goto nextopcode;
#endif /* OPDISP */

#endif /* I386 */




/* INLINE OPCODE FAIL ENTRY POINTS, CALL EXTERNAL ROUTINES HERE */
	OPCODEFAIL;
/* OPCODE FAIL ENTRY POINTS, CALL UFNS HERE */

	UFN_CALLS;

op_ufn:	
{
#ifdef ISC
   UFN *entry68k;				
#else
   register UFN *entry68k;						
#endif
	entry68k = (UFN *)GetUFNEntry(Get_BYTE_PCMAC0);	
   fn_num_args = entry68k->arg_num;					
   fn_opcode_size = entry68k->byte_num+1;				
   fn_atom_index = entry68k->atom_name;					
   fn_defcell = (DefCell *) GetDEFCELL68k(fn_atom_index);		
   fn_apply = 2 + entry68k->byte_num; /* code for UFN entry */
   goto op_fn_common;							
};

/* FUNCTION CALL TAIL ROUTINE */

	OP_FN_COMMON;

/* DISPATCH "LOOP" */

nextopcode :
#ifdef MYOPTRACE
  if ((struct fnhead *)Addr68k_from_LADDR(0x2ed600) == FuncObj)
      {
	quick_stack_check();
#endif /* MYOPTRACE */

  OPTPRINT(("Dispatch, PC = 0x%x, op = 0%o. TOS = 0x%x.\n", (int)PCMAC, Get_BYTE_PCMAC0, TOPOFSTACK));
#ifdef MYOPTRACE
      }
#endif /* MYOPTRACE */


#ifdef PCTRACE
  /* Tracing PC/Function/Opcode in a ring buffer */
  pc_table[pccounter] = (int)PCMAC - (int)FuncObj;
  fn_table[pccounter] = (LispPTR) LADDR_from_68k(FuncObj);
  op_table[pccounter] = Get_BYTE_PCMAC0;
  if (99 == pccounter++) pccounter = 0;
#endif /* PCTRACE */

/* quick_stack_check();*/	/* JDS 2/12/98 */


switch (Get_BYTE_PCMAC0) {
 
 case 000 : CASE000: { goto op_ufn; } /* unused */
 case 001 : CASE001: OPCAR;
 case 002 : CASE002: OPCDR;
 case 003 : CASE003: LISTP;
 case 004 : CASE004: NTYPEX;
 case 005 : CASE005: TYPEP(Get_BYTE_PCMAC1);
 case 056 : CASE056: 
 case 006 : CASE006: DTEST(Get_AtomNo_PCMAC1);
 case 007 : CASE007: UNWIND(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
#ifdef NATIVETRAN
 	    ret_to_fn0: asm("_ret_to_fn0:"); 
			asm("	.globl _ret_to_fn0"); 
			RET_FROM_NATIVE;
#endif
 case 010 : CASE010: FN0;
#ifdef NATIVETRAN
	    ret_to_fn1: asm("_ret_to_fn1:"); 
			asm("	.globl _ret_to_fn1"); 
			RET_FROM_NATIVE;
#endif
 case 011 : CASE011: FN1;
#ifdef NATIVETRAN
	    ret_to_fn2: asm("_ret_to_fn2:"); 
			asm("	.globl _ret_to_fn2"); 
			RET_FROM_NATIVE;
#endif
 case 012 : CASE012: FN2;
#ifdef NATIVETRAN
	    ret_to_fn3: asm("_ret_to_fn3:"); 
			asm("	.globl _ret_to_fn3"); 
			RET_FROM_NATIVE;
#endif
 case 013 : CASE013: FN3;
#ifdef NATIVETRAN
	    ret_to_fn4: asm("_ret_to_fn4:"); 
			asm("	.globl _ret_to_fn4"); 
			RET_FROM_NATIVE;
#endif
 case 014 : CASE014: FN4;
#ifdef NATIVETRAN
	    ret_to_fnx: asm("_ret_to_fnx:"); 
			asm("	.globl _ret_to_fnx"); 
			RET_FROM_NATIVE;
#endif
 case 015 : CASE015: FNX;
#ifdef NATIVETRAN
	    ret_to_apply: asm("_ret_to_apply:"); 
			asm("	.globl _ret_to_apply"); 
			RET_FROM_NATIVE;
#endif
 case 016 : CASE016: APPLY;

 case 017 : CASE017: CHECKAPPLY;
 case 020 : CASE020: RETURN;

 case 021 : CASE021: BIND;
 case 022 : CASE022: UNBIND;
 case 023 : CASE023: DUNBIND;
 case 024 : CASE024: RPLPTR(Get_BYTE_PCMAC1);
 case 025 : CASE025: GCREF(Get_BYTE_PCMAC1);
 case 026 : CASE026: ASSOC;
 case 027 : CASE027:
		     GVAR_(Get_AtomNo_PCMAC1);
 case 030 : CASE030: RPLACA;
 case 031 : CASE031: RPLACD;
 case 032 : CASE032: CONS;
 case 033 : CASE033: CLASSOC;
 case 034 : CASE034: FMEMB;
 case 035 : CASE035: CLFMEMB;
 case 036 : CASE036: FINDKEY(Get_BYTE_PCMAC1);
 case 037 : CASE037: CREATECELL;
 case 040 : CASE040: BIN;
 case 041 : CASE041: { goto op_ufn; } /* BOUT */
 case 042 : CASE042: { goto op_ufn; } /* POPDISP - prolog only */
 case 043 : CASE043: RESTLIST(Get_BYTE_PCMAC1);
 case 044 : CASE044: MISCN(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
 case 045 : CASE045: { goto op_ufn; }  /* unused */
 case 046 : CASE046: RPLCONS;
 case 047 : CASE047: LISTGET;
 case 050 : CASE050: { goto op_ufn; }  /* unused */
 case 051 : CASE051: { goto op_ufn; }  /* unused */
 case 052 : CASE052: { goto op_ufn; }  /* unused */
 case 053 : CASE053: { goto op_ufn; }  /* unused */
 case 054 : CASE054: EVAL;
#ifdef NATIVETRAN
	    ret_to_envcall: asm("_ret_to_envcall:"); 
			asm("	.globl _ret_to_envcall"); 
			RET_FROM_NATIVE;
#endif
 case 055 : CASE055: ENVCALL;

/*  case 056 : CASE056: @ 006 */
 case 057 : CASE057: STKSCAN;
 case 060 : CASE060: { goto op_ufn; } /* BUSBLT - DLion only */
 case 061 : CASE061: { goto op_ufn; } /* MISC8 - no longer used */
 case 062 : CASE062: UBFLOAT3(Get_BYTE_PCMAC1);
 case 063 : CASE063: TYPEMASK(Get_BYTE_PCMAC1);
 case 064 : CASE064: { goto op_ufn; } /* rdprologptr */
 case 065 : CASE065: { goto op_ufn; } /* rdprologtag */
 case 066 : CASE066: { goto op_ufn; } /* writeptr&tag */
 case 067 : CASE067: { goto op_ufn; } /* writeptr&0tag */
 case 070 : CASE070: MISC7(Get_BYTE_PCMAC1);	   /* misc7 (pseudocolor, fbitmapbit) */
 case 071 : CASE071: { goto op_ufn; } /* dovemisc - dove only */
 case 072 : CASE072: EQLOP;
 case 073 : CASE073: DRAWLINE;
 case 074 : CASE074: STOREN(Get_BYTE_PCMAC1);
 case 075 : CASE075: COPYN(Get_BYTE_PCMAC1);
 case 076 : CASE076: { goto op_ufn; } /* RAID */
 case 077 : CASE077: { goto op_ufn; } /* \RETURN */



 case 0100 : CASE100:  IVARMACRO (0);  
 case 0101 : CASE101:  IVARMACRO (1);  
 case 0102 : CASE102:  IVARMACRO (2);  
 case 0103 : CASE103:  IVARMACRO (3);  
 case 0104 : CASE104:  IVARMACRO (4);  
 case 0105 : CASE105:  IVARMACRO (5);  
 case 0106 : CASE106:  IVARMACRO (6);   
 case 0107 : CASE107:  IVARX (Get_BYTE_PCMAC1);   

 case 0110 : CASE110:  PVARMACRO (0);  
 case 0111 : CASE111:  PVARMACRO (1); 
 case 0112 : CASE112:  PVARMACRO (2);  
 case 0113 : CASE113:  PVARMACRO (3);  
 case 0114 : CASE114:  PVARMACRO (4);  
 case 0115 : CASE115:  PVARMACRO (5);  
 case 0116 : CASE116:  PVARMACRO (6);    

 case 0117 : CASE117: PVARX(Get_BYTE_PCMAC1);

 case 0120 : CASE120: FVAR(0);
 case 0121 : CASE121: FVAR(2);
 case 0122 : CASE122: FVAR(4);
 case 0123 : CASE123: FVAR(6);
 case 0124 : CASE124: FVAR(8);
 case 0125 : CASE125: FVAR(10);
 case 0126 : CASE126: FVAR(12);
 case 0127 : CASE127: FVARX(Get_BYTE_PCMAC1);
		
 case 0130 : CASE130: PVARSETMACRO (0); 
 case 0131 : CASE131: PVARSETMACRO (1); 
 case 0132 : CASE132: PVARSETMACRO (2);
 case 0133 : CASE133: PVARSETMACRO (3); 
 case 0134 : CASE134: PVARSETMACRO (4); 
 case 0135 : CASE135: PVARSETMACRO (5); 
 case 0136 : CASE136: PVARSETMACRO (6); 

 case 0137 : CASE137: PVARX_(Get_BYTE_PCMAC1);
 
 case 0140 : CASE140: GVAR(Get_AtomNo_PCMAC1);
 case 0141 : CASE141: ARG0;
 case 0142 : CASE142: IVARX_(Get_BYTE_PCMAC1);
 case 0143 : CASE143: FVARX_(Get_BYTE_PCMAC1);
 case 0144 : CASE144: COPY;
 case 0145 : CASE145: MYARGCOUNT;
 case 0146 : CASE146: MYALINK;

/******** Aconst	********/
 case 0147 : CASE147: { PUSH(Get_AtomNo_PCMAC1); nextop_atom;}
 case 0150 : CASE150: { PUSHATOM(NIL_PTR     ); }
 case 0151 : CASE151: { PUSHATOM(ATOM_T      ); }
 case 0152 : CASE152: { PUSHATOM(S_POSITIVE  ); } /* '0 */
 case 0153 : CASE153: { PUSHATOM(0xE0001     ); } /* '1 */

/********* SIC		********/
 case 0154 : CASE154: {	
 		PUSH(S_POSITIVE | Get_BYTE_PCMAC1);
 		nextop2;
		}

/********* SNIC		********/
 case 0155 : CASE155: { 	
		PUSH(S_NEGATIVE | 0xff00 | Get_BYTE_PCMAC1);
 		nextop2;
		}

/********* SICX		********/
 case 0156 : CASE156:{ 	
 		PUSH(S_POSITIVE | Get_DLword_PCMAC1);
 		nextop3;
		}

/********* GCONST	********/
 case 0157 : CASE157: {
		PUSH(Get_Pointer_PCMAC1);
		nextop_ptr;
		}
 
 case 0160 : CASE160: { goto op_ufn; } /* unused */
 case 0161 : CASE161: { goto op_ufn; } /* readflags */
 case 0162 : CASE162: { goto op_ufn; } /* readrp */
 case 0163 : CASE163: { goto op_ufn; } /* writemap */
 case 0164 : CASE164: { goto op_ufn; } /* readprinterport */
 case 0165 : CASE165: { goto op_ufn; } /* writeprinterport */

 case 0166 : CASE166: PILOTBITBLT;
 case 0167 : CASE167: RCLK;
 case 0170 : CASE170: { goto op_ufn; } /* MISC1, dorado only */
 case 0171 : CASE171: { goto op_ufn; } /* MISC2, dorado only */
 case 0172 : CASE172: RECLAIMCELL;
 case 0173 : CASE173: GCSCAN1;
 case 0174 : CASE174: GCSCAN2;
 case 0175 : CASE175: { EXT; OP_subrcall(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2); RET; NATIVE_NEXTOP0; };
 case 0176 : CASE176: { CONTEXTSWITCH; }
 case 0177 : CASE177: { goto op_ufn; } /* RETCALL */

/* JUMP */

 case 0200 : CASE200: { JUMPMACRO(2); }
 case 0201 : CASE201: { JUMPMACRO(3); }
 case 0202 : CASE202: { JUMPMACRO(4); }
 case 0203 : CASE203: { JUMPMACRO(5); }
 case 0204 : CASE204: { JUMPMACRO(6); }
 case 0205 : CASE205: { JUMPMACRO(7); }
 case 0206 : CASE206: { JUMPMACRO(8); }
 case 0207 : CASE207: { JUMPMACRO(9); }
 case 0210 : CASE210: { JUMPMACRO(10); }
 case 0211 : CASE211: { JUMPMACRO(11); }
 case 0212 : CASE212: { JUMPMACRO(12); }
 case 0213 : CASE213: { JUMPMACRO(13); }
 case 0214 : CASE214: { JUMPMACRO(14); }
 case 0215 : CASE215: { JUMPMACRO(15); }
 case 0216 : CASE216: { JUMPMACRO(16); }
 case 0217 : CASE217: { JUMPMACRO(17); }


/* FJUMP */

 case 0220 : CASE220: { FJUMPMACRO(2); }
 case 0221 : CASE221: { FJUMPMACRO(3); }
 case 0222 : CASE222: { FJUMPMACRO(4); }
 case 0223 : CASE223: { FJUMPMACRO(5); }
 case 0224 : CASE224: { FJUMPMACRO(6); }
 case 0225 : CASE225: { FJUMPMACRO(7); }
 case 0226 : CASE226: { FJUMPMACRO(8); }
 case 0227 : CASE227: { FJUMPMACRO(9); }
 case 0230 : CASE230: { FJUMPMACRO(10); }
 case 0231 : CASE231: { FJUMPMACRO(11); }
 case 0232 : CASE232: { FJUMPMACRO(12); }
 case 0233 : CASE233: { FJUMPMACRO(13); }
 case 0234 : CASE234: { FJUMPMACRO(14); }
 case 0235 : CASE235: { FJUMPMACRO(15); }
 case 0236 : CASE236: { FJUMPMACRO(16); }
 case 0237 : CASE237: { FJUMPMACRO(17); }

/* TJUMP */

 case 0240 : CASE240: { TJUMPMACRO(2); }
 case 0241 : CASE241: { TJUMPMACRO(3); }
 case 0242 : CASE242: { TJUMPMACRO(4); }
 case 0243 : CASE243: { TJUMPMACRO(5); }
 case 0244 : CASE244: { TJUMPMACRO(6); }
 case 0245 : CASE245: { TJUMPMACRO(7); }
 case 0246 : CASE246: { TJUMPMACRO(8); }
 case 0247 : CASE247: { TJUMPMACRO(9); }
 case 0250 : CASE250: { TJUMPMACRO(10); }
 case 0251 : CASE251: { TJUMPMACRO(11); }
 case 0252 : CASE252: { TJUMPMACRO(12); }
 case 0253 : CASE253: { TJUMPMACRO(13); }
 case 0254 : CASE254: { TJUMPMACRO(14); }
 case 0255 : CASE255: { TJUMPMACRO(15); }
 case 0256 : CASE256: { TJUMPMACRO(16); }
 case 0257 : CASE257: { TJUMPMACRO(17); }

/******* JUMPX ********/
 case 0260 : CASE260: {
		CHECK_INTERRUPT;
		PCMACL += Get_SBYTE_PCMAC1; nextop0; 
		}

/******* JUMPXX ********/
 case 0261 : CASE261: {
		CHECK_INTERRUPT;
		PCMACL += (Get_SBYTE_PCMAC1<<8) | Get_BYTE_PCMAC2; nextop0; 
		}

/******* FJumpx *******/
 case 0262 : CASE262: {
    if(TOPOFSTACK != 0) {goto PopNextop2;}
	CHECK_INTERRUPT;
	POP;
	PCMACL += Get_SBYTE_PCMAC1;
	nextop0; 
	}

/******* TJumpx *******/

 case 0263 : CASE263: {
	if(TOPOFSTACK == 0) {goto PopNextop2;}
	CHECK_INTERRUPT;
	POP;
	PCMACL += Get_SBYTE_PCMAC1;
	nextop0; 
	}

/******* NFJumpx *******/

 case 0264 : CASE264: {
	if(TOPOFSTACK != 0) {goto PopNextop2;}
	CHECK_INTERRUPT;
	PCMACL += Get_SBYTE_PCMAC1;
	nextop0; 
	}

/******* NTJumpx *******/

 case 0265 : CASE265: {
	if(TOPOFSTACK == 0) {goto PopNextop2;}
	CHECK_INTERRUPT;
	PCMACL += Get_SBYTE_PCMAC1;
	nextop0; 
	}

 case 0266 : CASE266:	AREF1;
 case 0267 : CASE267:	ASET1;

 case 0270 : CASE270:	PVARSETPOPMACRO(0);
 case 0271 : CASE271:	PVARSETPOPMACRO(1);
 case 0272 : CASE272:	PVARSETPOPMACRO(2);
 case 0273 : CASE273:	PVARSETPOPMACRO(3);
 case 0274 : CASE274:	PVARSETPOPMACRO(4);
 case 0275 : CASE275:	PVARSETPOPMACRO(5);
 case 0276 : CASE276:	PVARSETPOPMACRO(6);

 case 0277 : CASE277: 	{ POP; nextop1; }

 case 0300 : CASE300:  POPN(Get_BYTE_PCMAC1);
 case 0301 : CASE301: 	ATOMCELL_N(Get_BYTE_PCMAC1);
 case 0302 : CASE302: 	GETBASEBYTE;
 case 0303 : CASE303:  INSTANCEP(Get_AtomNo_PCMAC1);
 case 0304 : CASE304:  BLT;
 case 0305 : CASE305:  {goto op_ufn; } /* MISC10 */
 case 0306 : CASE306:  {goto op_ufn; } /* P-MISC2 ??? */
 case 0307 : CASE307:	PUTBASEBYTE;
 case 0310 : CASE310:	GETBASE_N(Get_BYTE_PCMAC1);
 case 0311 : CASE311:	GETBASEPTR_N(Get_BYTE_PCMAC1);
 case 0312 : CASE312:	GETBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
 case 0313 : CASE313:  {goto op_ufn; } /* unused */
 case 0314 : CASE314:	CLEQUAL;
 case 0315 : CASE315:	PUTBASE_N(Get_BYTE_PCMAC1);
 case 0316 : CASE316:	PUTBASEPTR_N(Get_BYTE_PCMAC1);
 case 0317 : CASE317:	PUTBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);

 case 0320 : CASE320:	N_OP_ADDBASE;
 case 0321 : CASE321:	N_OP_VAG2;
 case 0322 : CASE322:	N_OP_HILOC;
 case 0323 : CASE323:	N_OP_LOLOC;
 case 0324 : CASE324: 	PLUS2; /* PLUS */
 case 0325 : CASE325:  DIFFERENCE;	/* DIFFERENCE */	
 case 0326 : CASE326:	TIMES2; /* TIMES2 */
 case 0327 : CASE327:  QUOTIENT /* QUOTIENT */
 case 0330 : CASE330:  IPLUS2; /* IPLUS2 only while PLUS has no float */
 case 0331 : CASE331:  IDIFFERENCE; /* IDIFFERENCE only while no float */
 case 0332 : CASE332:	ITIMES2; /* ITIMES2 only while no float */
 case 0333 : CASE333:	IQUOTIENT; /* IQUOTIENT */
 case 0334 : CASE334:	IREMAINDER;
 case 0335 : CASE335:	IPLUS_N(Get_BYTE_PCMAC1);
 case 0336 : CASE336:	IDIFFERENCE_N(Get_BYTE_PCMAC1);
 case 0337 : CASE337:  { goto op_ufn; } /* BASE-< */
 case 0340 : CASE340:  LLSH1;
 case 0341 : CASE341:  LLSH8;
 case 0342 : CASE342:  LRSH1;
 case 0343 : CASE343:  LRSH8;
 case 0344 : CASE344:  LOGOR;
 case 0345 : CASE345:  LOGAND;
 case 0346 : CASE346:  LOGXOR;
 case 0347 : CASE347:  LSH;
 case 0350 : CASE350:  FPLUS2;
 case 0351 : CASE351:  FDIFFERENCE;
 case 0352 : CASE352:  FTIMES2;
 case 0353 : CASE353:  FQUOTIENT;
 case 0354 : CASE354:  UBFLOAT2(Get_BYTE_PCMAC1);
 case 0355 : CASE355:  UBFLOAT1(Get_BYTE_PCMAC1);
 case 0356 : CASE356:  AREF2;
 case 0357 : CASE357:  ASET2;

 case 0360 : CASE360: {
		if(TOPOFSTACK == POP_TOS_1)
			TOPOFSTACK = ATOM_T;
		else	TOPOFSTACK = NIL_PTR;
		nextop1;
		}

 case 0361 : CASE361:  IGREATERP; /* IGREATERP if no float */	
 case 0362 : CASE362:  FGREATERP;
 case 0363 : CASE363:	GREATERP;
 case 0364 : CASE364:  ILEQUAL;
 case 0365 : CASE365:	MAKENUMBER;
 case 0366 : CASE366:	BOXIPLUS;
 case 0367 : CASE367:	BOXIDIFFERENCE;
 case 0370 : CASE370:  { goto op_ufn; } /* FLOATBLT */
 case 0371 : CASE371:  { goto op_ufn; } /* FFTSTEP */
 case 0372 : CASE372: 	MISC3(Get_BYTE_PCMAC1);
 case 0373 : CASE373: 	MISC4(Get_BYTE_PCMAC1);
 case 0374 : CASE374:  { goto op_ufn; } /* upctrace */
 case 0375 : CASE375: 	SWAP;
 case 0376 : CASE376: 	NOP;
 case 0377 : CASE377: 	CLARITHEQUAL;
#ifdef OPDISP
#ifdef ISC
case 0400 : goto setup_table; /* to defeat optimizer, so optable exists */
#elif (DOS && OPDISP)
case 0400: goto setup_table;	
#endif /* ISC */

#endif /* OPDISP */



#ifdef I386
	/* to defeat the damn optimizer, make it look like */
	/* we might branch to the error labels. */
 case 0400 : goto plus_err;
 case 0401 : goto iplus_err;
 case 0402 : goto iplusn_err;
 case 0403 : goto idiff_err;
 case 0404 : goto diff_err;
 case 0405 : goto idiffn_err;
 case 0406 : goto greaterp_err;
 case 0411 : goto igreaterp_err;
 case 0407 : goto llsh8_err;
 case 0410 : goto lrsh1_err;
 case 0414 : goto lrsh8_err;
 case 0417 : goto llsh1_err;
 case 0413 : goto logor_err;
 case 0412 : goto logand_err;
 case 0416 : goto logxor_err;
 case 0415 : goto addbase_err;
#endif
 	
 default: 	error("should not default");

	 }	/* switch */

#ifdef NATIVETRAN
/************************************************************************/
/*	 	NATIVE CODE INTERFACE 					*/
/************************************************************************/


/*		FORIEGN -> DISPATCH 					*/
/*		Return to current frame ext				*/

c_ret_to_dispatch:
	asm("	.globl	_c_ret_to_dispatch");
	asm("_c_ret_to_dispatch:");
	PCMACL = (ByteCode *) FuncObj + BCE_CURRENTFX->pc;
	goto ret_to_dispatch;		/* assume optimizer will remove */

/*		NATIVE -> DISPATCH 					*/
/*		Return to current frame ext				*/

ret_to_dispatch:
	asm("	.globl	_ret_to_dispatch");
	asm("_ret_to_dispatch:");
	RET_FROM_NATIVE;
	nextop0;

/*		NATIVE -> DISPATCH 					*/
/*		Execute opcode in current frame ext			*/

ret_to_unimpl:
	asm("	.globl	_ret_to_unimpl");
	asm("_ret_to_unimpl:");
	SaveD6 = 0x100;
			/* HACK.  Reg. d6 is set to dispatch to native_check */
			/* so need to do switch instead of dispatch! */
	RET_FROM_NATIVE;
	goto nextopcode;

/*		NATIVE -> UFN(PC) 					*/

ret_to_ufn:
	asm("	.globl	_ret_to_ufn");
	asm("_ret_to_ufn:");
	RET_FROM_NATIVE;
	goto op_ufn;

/*		DISPATCH -> NATIVE? 					*/
/*		Return to current frame ext?				*/
	
native_check:
	SaveD6 = 0;
	NATIVE_NEXTOP0;

/*		NATIVE -> TIMER						*/
/*		Return to Execute timer interrupt			*/

ret_to_timer:
	asm("_ret_to_timer:");
	asm("	.globl	_ret_to_timer");
	SaveD6 = 0x100;
	RET_FROM_NATIVE;
	goto check_interrupt;	/* assume optimizer will remove */

#else

native_check: 

#ifndef DOS
#ifdef OPDISP
		SaveD6 = 0x000;
#endif
#endif /* DOS */
		goto nextopcode;
#endif

/************************************************************************/
/*		TIMER INTERRUPT CHECK ROUTINE				*/
/************************************************************************/
check_interrupt:
#if	(defined(NATIVETRAN) || defined(SUN3_OS3_OR_OS4_IL) || defined(I386) || defined(ISC))
	asm_label_check_interrupt();
#endif

	if ( (UNSIGNED)CSTKPTR > (UNSIGNED)EndSTKP )
		{EXT;
		 error("Unrecoverable Stack Overflow");
	         RET;
		}

	/* Check for an IRQ request */

{register int need_irq;
 static int period_cnt=60;
extern int KBDEventFlg;
extern int ETHEREventCount;
extern LispPTR DOBUFFEREDTRANSITION_index;
extern LispPTR INTERRUPTFRAME_index;
extern LispPTR *KEYBUFFERING68k;
extern LispPTR *PENDINGINTERRUPT68k;
extern LispPTR ATOM_STARTED;
extern LispPTR *PERIODIC_INTERRUPT68k;
extern LispPTR *PERIODIC_INTERRUPT_FREQUENCY68k;
extern LispPTR PERIODIC_INTERRUPTFRAME_index;
extern LispPTR *Reclaim_cnt_word;
extern LispPTR DORECLAIM_index;
extern int URaid_req;

	/* Check for an Stack Overflow */
/* JDS 22 May 96 -- >= below used to be just >, changed because we got
   stack oflows with last frame right at end of stack, leading to loops,
   odd bugs, ... */
/**** Changed back to > 31 July 97 ****/
re_check_stack:
	need_irq = 0;
	if ( 	((UNSIGNED)(CSTKPTR+1) > Irq_Stk_Check) && 
		(Irq_Stk_End > 0) && 
		(Irq_Stk_Check > 0) )
		{
	 	 HARD_PUSH(TOPOFSTACK);
		 EXT;
		 extended_frame = NIL;
		 if (do_stackoverflow(NIL)) { 
stackoverflow_help:
			period_cnt=60; need_irq = T;
			error("Stack Overflow, MUST HARDRESET!"); 
			RET; TOPOFSTACK = NIL_PTR; 
			}
		 else { RET; POP; }
		 Irq_Stk_Check = (UNSIGNED)EndSTKP-STK_MIN(FuncObj);
		 need_irq = (Irq_Stk_End == 0)  || extended_frame;
		 *PENDINGINTERRUPT68k |= extended_frame; 
		 Irq_Stk_End = (UNSIGNED) EndSTKP;
		}

	/* Check for an IRQ request */

	if ((Irq_Stk_End <= 0) || (Irq_Stk_Check <= 0) || need_irq) {
		if (StkOffset_from_68K(CSTKPTR) > InterfacePage->stackbase) {

			/* Interrupts not Disabled */
#ifndef KBINT
			getsignaldata();
#endif
#ifdef OS4
			getsignaldata();
#endif
			EXT; 
			update_timer(); 

		/*** If SPY is running, check to see if it ***/
		/*** needs an interrupt; give it one, if so. ***/
		if(*PERIODIC_INTERRUPT68k!=NIL)
		  {
		    if(period_cnt>0)  period_cnt--;
	   	    else
		      {
			cause_interruptcall(PERIODIC_INTERRUPTFRAME_index);
			if(*PERIODIC_INTERRUPT_FREQUENCY68k==NIL)
				period_cnt=0;
			else
			  period_cnt=(*PERIODIC_INTERRUPT_FREQUENCY68k & 0xffff)
				     *(1000000/60) /TIMER_INTERVAL;
			/* number of 1/60 second periods between interrupts.
	  		TIMER_INTERVAL is the number of microseconds between
	  		timer interrupts. The calculation here avoids some
	  		overflow errors although there is some roundoff
	  		if the interrupt frequency number is too low,
	 		it will bottom out and just set period_cnt to 0 */
		      }
		  }


#ifdef DOS
         if(currentkbd->URaid == TRUE){
            currentkbd->URaid = NIL;
            (currentkbd->device.exit)(currentkbd); /* Install the original handler */
		    error("Call URaid by User Interrupt");
		}
       else if(currentmouse->Cursor.Moved){
             union REGS regs;

             currentdsp->device.locked++;

	     /* Remove the mouse from the old place on the screen */
	     (currentdsp->mouse_invissible)(currentdsp, IOPage68K);

             /* Find the new delta */
             regs.w.eax = 0x000B; /* Function 0xB = get delta mickeys */
             int86(0x33, &regs, &regs);
             currentmouse->Cursor.New.x += (short)regs.w.ecx;
             currentmouse->Cursor.New.y += (short)regs.w.edx;

             if(currentmouse->Cursor.New.x < 0)
               currentmouse->Cursor.New.x = 0;
             else if(currentmouse->Cursor.New.x > (currentdsp->Display.width - 1))
             currentmouse->Cursor.New.x = currentdsp->Display.width - 1;

           if(currentmouse->Cursor.New.y < 0)
             currentmouse->Cursor.New.y = 0;
           else if(currentmouse->Cursor.New.y > (currentdsp->Display.height - 1))
             currentmouse->Cursor.New.y = currentdsp->Display.height - 1;

           IOPage68K->dlmousex =
	     IOPage68K->dlcursorx =
               currentmouse->Cursor.New.x;
           IOPage68K->dlmousey =
	     IOPage68K->dlcursory =
               currentmouse->Cursor.New.y;

           /* Paint the mouse back up on the screen on the new place */
           (currentdsp->mouse_vissible)( currentmouse->Cursor.New.x,
		                                 currentmouse->Cursor.New.y);
           currentmouse->Cursor.Moved = FALSE;
           currentdsp->device.locked--;
         }

#else
			if(URaid_req ==T){
				URaid_req=NIL;
				error("Call URaid by User Interrupt");
			}
#endif /* DOS */
			else if((KBDEventFlg>0)&&(*KEYBUFFERING68k==ATOM_T)) {
				*KEYBUFFERING68k= ATOM_STARTED;
				cause_interruptcall(DOBUFFEREDTRANSITION_index);
				KBDEventFlg --;
				}
			else if(*Reclaim_cnt_word == S_POSITIVE) {  
				*Reclaim_cnt_word=NIL;
				cause_interruptcall(DORECLAIM_index);
				}
			else if (*PENDINGINTERRUPT68k!=NIL)
			  { INTSTAT2 * intstate = ((INTSTAT2 *)Addr68k_from_LADDR(*INTERRUPTSTATE_word));
			    unsigned char newints = (intstate->pendingmask) & ~(intstate->handledmask);
			 /*   if (newints) */
			      {
				intstate->handledmask |= intstate->pendingmask;
				*PENDINGINTERRUPT68k=NIL;
				cause_interruptcall(INTERRUPTFRAME_index);
			      }
			  }
			else if (ETHEREventCount>0)
			  { INTSTAT * intstate = ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word));
			    if (!(intstate->ETHERInterrupt) &&
				!(((INTSTAT2 *)intstate)->handledmask & 0x40))
			      {
				intstate->ETHERInterrupt=1;
				((INTSTAT2 *)intstate)->handledmask |=
				    ((INTSTAT2 *)intstate)->pendingmask;
				cause_interruptcall(INTERRUPTFRAME_index);
				ETHEREventCount--;
			      }
			    else *PENDINGINTERRUPT68k = ATOM_T;
			  }
			RET;
			CLR_IRQ;
			} /* Interrupts not Disabled */
		else {
			/* Clear out IRQ (loses pending interrupt request 
			   if interrupts are disabled) */
			CLR_IRQ;
			goto re_check_stack;
			}
		}

}

	nextop0;


/************************************************************************/
/* Common Jump Tails (they have to jump anyway, so use common Tail) */
/************************************************************************/
PopNextop1:
	POP; 
	nextop1;

PopNextop2:
	POP; 
	nextop2;

/************************************************************************/
/*									*/
/*	Set up the dispatch table for use when we do assembler		*/
/*	optimization of the dispatch-jump sequence.			*/
/*									*/
/*									*/
/************************************************************************/

#ifdef OPDISP
 setup_table:
#ifndef ISC
	SaveD6 = 0;

#ifdef UNSAFE
	Save_D5_shift_amount = 15;
#endif


	{int i; for (i = 0; i < 256; i++) { table[i] = (InstPtr) op_ufn; };}
	{int i; for (i = 256; i < 512; i++) 
		{ table[i] = (InstPtr) native_check; };
	}
	table[001] = (InstPtr) case001;
	table[002] = (InstPtr) case002;
	table[003] = (InstPtr) case003;
	table[004] = (InstPtr) case004;
	table[005] = (InstPtr) case005;
	table[006] = (InstPtr) case006;
	table[007] = (InstPtr) case007;
	table[010] = (InstPtr) case010;
	table[011] = (InstPtr) case011;
	table[012] = (InstPtr) case012;
	table[013] = (InstPtr) case013;
	table[014] = (InstPtr) case014;
	table[015] = (InstPtr) case015;
	table[016] = (InstPtr) case016;
	table[017] = (InstPtr) case017;
	table[020] = (InstPtr) case020;
	table[021] = (InstPtr) case021;
	table[022] = (InstPtr) case022;
	table[023] = (InstPtr) case023;
	table[024] = (InstPtr) case024;
	table[025] = (InstPtr) case025;
	table[026] = (InstPtr) case026;
	table[027] = (InstPtr) case027;
	table[030] = (InstPtr) case030;
	table[031] = (InstPtr) case031;
	table[032] = (InstPtr) case032;
	table[033] = (InstPtr) case033;
	table[034] = (InstPtr) case034;
	table[035] = (InstPtr) case035;
	table[036] = (InstPtr) case036;
	table[037] = (InstPtr) case037;
	table[040] = (InstPtr) case040;
	table[041] = (InstPtr) case041;
	table[042] = (InstPtr) case042;
	table[043] = (InstPtr) case043;
	table[044] = (InstPtr) case044;
	table[045] = (InstPtr) case045;
	table[046] = (InstPtr) case046;
	table[047] = (InstPtr) case047;

	table[054] = (InstPtr) case054;
	table[055] = (InstPtr) case055;
	table[056] = (InstPtr) case056;
	table[057] = (InstPtr) case057;

	table[062] = (InstPtr) case062;
	table[063] = (InstPtr) case063;

	table[070] = (InstPtr) case070;

	table[072] = (InstPtr) case072;
	table[073] = (InstPtr) case073;
	table[074] = (InstPtr) case074;
	table[075] = (InstPtr) case075;

	table[0100] = (InstPtr) case100;
	table[0101] = (InstPtr) case101;
	table[0102] = (InstPtr) case102;
	table[0103] = (InstPtr) case103;
	table[0104] = (InstPtr) case104;
	table[0105] = (InstPtr) case105;
	table[0106] = (InstPtr) case106;
	table[0107] = (InstPtr) case107;
	table[0110] = (InstPtr) case110;
	table[0111] = (InstPtr) case111;
	table[0112] = (InstPtr) case112;
	table[0113] = (InstPtr) case113;
	table[0114] = (InstPtr) case114;
	table[0115] = (InstPtr) case115;
	table[0116] = (InstPtr) case116;
	table[0117] = (InstPtr) case117;
	table[0120] = (InstPtr) case120;
	table[0121] = (InstPtr) case121;
	table[0122] = (InstPtr) case122;
	table[0123] = (InstPtr) case123;
	table[0124] = (InstPtr) case124;
	table[0125] = (InstPtr) case125;
	table[0126] = (InstPtr) case126;
	table[0127] = (InstPtr) case127;
	table[0130] = (InstPtr) case130;
	table[0131] = (InstPtr) case131;
	table[0132] = (InstPtr) case132;
	table[0133] = (InstPtr) case133;
	table[0134] = (InstPtr) case134;
	table[0135] = (InstPtr) case135;
	table[0136] = (InstPtr) case136;
	table[0137] = (InstPtr) case137;
	table[0140] = (InstPtr) case140;
	table[0141] = (InstPtr) case141;
	table[0142] = (InstPtr) case142;
	table[0143] = (InstPtr) case143;
	table[0144] = (InstPtr) case144;
	table[0145] = (InstPtr) case145;
	table[0146] = (InstPtr) case146;
	table[0147] = (InstPtr) case147;
	table[0150] = (InstPtr) case150;
	table[0151] = (InstPtr) case151;
	table[0152] = (InstPtr) case152;
	table[0153] = (InstPtr) case153;
	table[0154] = (InstPtr) case154;
	table[0155] = (InstPtr) case155;
	table[0156] = (InstPtr) case156;
	table[0157] = (InstPtr) case157;
	table[0160] = (InstPtr) case160;
	table[0161] = (InstPtr) case161;
	table[0162] = (InstPtr) case162;
	table[0163] = (InstPtr) case163;
	table[0164] = (InstPtr) case164;
	table[0165] = (InstPtr) case165;
	table[0166] = (InstPtr) case166;
	table[0167] = (InstPtr) case167;
	table[0170] = (InstPtr) case170;
	table[0171] = (InstPtr) case171;
	table[0172] = (InstPtr) case172;
	table[0173] = (InstPtr) case173;
	table[0174] = (InstPtr) case174;
	table[0175] = (InstPtr) case175;
	table[0176] = (InstPtr) case176;
	table[0177] = (InstPtr) case177;
	table[0200] = (InstPtr) case200;
	table[0201] = (InstPtr) case201;
	table[0202] = (InstPtr) case202;
	table[0203] = (InstPtr) case203;
	table[0204] = (InstPtr) case204;
	table[0205] = (InstPtr) case205;
	table[0206] = (InstPtr) case206;
	table[0207] = (InstPtr) case207;
	table[0210] = (InstPtr) case210;
	table[0211] = (InstPtr) case211;
	table[0212] = (InstPtr) case212;
	table[0213] = (InstPtr) case213;
	table[0214] = (InstPtr) case214;
	table[0215] = (InstPtr) case215;
	table[0216] = (InstPtr) case216;
	table[0217] = (InstPtr) case217;
	table[0220] = (InstPtr) case220;
	table[0221] = (InstPtr) case221;
	table[0222] = (InstPtr) case222;
	table[0223] = (InstPtr) case223;
	table[0224] = (InstPtr) case224;
	table[0225] = (InstPtr) case225;
	table[0226] = (InstPtr) case226;
	table[0227] = (InstPtr) case227;
	table[0230] = (InstPtr) case230;
	table[0231] = (InstPtr) case231;
	table[0232] = (InstPtr) case232;
	table[0233] = (InstPtr) case233;
	table[0234] = (InstPtr) case234;
	table[0235] = (InstPtr) case235;
	table[0236] = (InstPtr) case236;
	table[0237] = (InstPtr) case237;
	table[0240] = (InstPtr) case240;
	table[0241] = (InstPtr) case241;
	table[0242] = (InstPtr) case242;
	table[0243] = (InstPtr) case243;
	table[0244] = (InstPtr) case244;
	table[0245] = (InstPtr) case245;
	table[0246] = (InstPtr) case246;
	table[0247] = (InstPtr) case247;
	table[0250] = (InstPtr) case250;
	table[0251] = (InstPtr) case251;
	table[0252] = (InstPtr) case252;
	table[0253] = (InstPtr) case253;
	table[0254] = (InstPtr) case254;
	table[0255] = (InstPtr) case255;
	table[0256] = (InstPtr) case256;
	table[0257] = (InstPtr) case257;
	table[0260] = (InstPtr) case260;
	table[0261] = (InstPtr) case261;
	table[0262] = (InstPtr) case262;
	table[0263] = (InstPtr) case263;
	table[0264] = (InstPtr) case264;
	table[0265] = (InstPtr) case265;
	table[0266] = (InstPtr) case266;
	table[0267] = (InstPtr) case267;
	table[0270] = (InstPtr) case270;
	table[0271] = (InstPtr) case271;
	table[0272] = (InstPtr) case272;
	table[0273] = (InstPtr) case273;
	table[0274] = (InstPtr) case274;
	table[0275] = (InstPtr) case275;
	table[0276] = (InstPtr) case276;
	table[0277] = (InstPtr) case277;
	table[0300] = (InstPtr) case300;
	table[0301] = (InstPtr) case301;
	table[0302] = (InstPtr) case302;
	table[0303] = (InstPtr) case303;
	table[0304] = (InstPtr) case304;
	table[0305] = (InstPtr) case305;
	table[0306] = (InstPtr) case306;
	table[0307] = (InstPtr) case307;
	table[0310] = (InstPtr) case310;
	table[0311] = (InstPtr) case311;
	table[0312] = (InstPtr) case312;
	table[0313] = (InstPtr) case313;
	table[0314] = (InstPtr) case314;
	table[0315] = (InstPtr) case315;
	table[0316] = (InstPtr) case316;
	table[0317] = (InstPtr) case317;
	table[0320] = (InstPtr) case320;
	table[0321] = (InstPtr) case321;
	table[0322] = (InstPtr) case322;
	table[0323] = (InstPtr) case323;
	table[0324] = (InstPtr) case324;
	table[0325] = (InstPtr) case325;
	table[0326] = (InstPtr) case326;
	table[0327] = (InstPtr) case327;
	table[0330] = (InstPtr) case330;
	table[0331] = (InstPtr) case331;
	table[0332] = (InstPtr) case332;
	table[0333] = (InstPtr) case333;
	table[0334] = (InstPtr) case334;
	table[0335] = (InstPtr) case335;
	table[0336] = (InstPtr) case336;
	table[0337] = (InstPtr) case337;
	table[0340] = (InstPtr) case340;
	table[0341] = (InstPtr) case341;
	table[0342] = (InstPtr) case342;
	table[0343] = (InstPtr) case343;
	table[0344] = (InstPtr) case344;
	table[0345] = (InstPtr) case345;
	table[0346] = (InstPtr) case346;
	table[0347] = (InstPtr) case347;
	table[0350] = (InstPtr) case350;
	table[0351] = (InstPtr) case351;
	table[0352] = (InstPtr) case352;
	table[0353] = (InstPtr) case353;
	table[0354] = (InstPtr) case354;
	table[0355] = (InstPtr) case355;
	table[0356] = (InstPtr) case356;
	table[0357] = (InstPtr) case357;
	table[0360] = (InstPtr) case360;
	table[0361] = (InstPtr) case361;
	table[0362] = (InstPtr) case362;
	table[0363] = (InstPtr) case363;
	table[0364] = (InstPtr) case364;
	table[0365] = (InstPtr) case365;
	table[0366] = (InstPtr) case366;
	table[0367] = (InstPtr) case367;
	table[0370] = (InstPtr) case370;
	table[0371] = (InstPtr) case371;
	table[0372] = (InstPtr) case372;
	table[0373] = (InstPtr) case373;
	table[0374] = (InstPtr) case374;
	table[0375] = (InstPtr) case375;
	table[0376] = (InstPtr) case376;
	table[0377] = (InstPtr) case377;
     goto nextopcode;
#elif GCC385
	/* This is the optable for 386's under gcc */

asm volatile("	.data \n\
	.align 4");
asm volatile("optable:	\n\
	.long	_op000 \n\
	.long	_op001 \n\
	.long	_op002 \n\
	.long	_op003 \n\
	.long	_op004 \n\
	.long	_op005 \n\
	.long	_op006 \n\
	.long	_op007 \n\
	.long	_op010 \n\
	.long	_op011 \n\
	.long	_op012 \n\
	.long	_op013 \n\
	.long	_op014 \n\
	.long	_op015 \n\
	.long	_op016 \n\
	.long	_op017 \n\
	.long	_op020 \n\
	.long	_op021 \n\
	.long	_op022 \n\
	.long	_op023 \n\
	.long	_op024 \n\
	.long	_op025 \n\
	.long	_op026 \n\
	.long	_op027 \n\
	.long	_op030 \n\
	.long	_op031 \n\
	.long	_op032 \n\
	.long	_op033 \n\
");

asm volatile("\n\
	.long	_op034 \n\
	.long	_op035 \n\
	.long	_op036 \n\
	.long	_op037 \n\
	.long	_op040 \n\
	.long	_op041 \n\
	.long	_op042 \n\
	.long	_op043 \n\
	.long	_op044 \n\
	.long	_op045 \n\
	.long	_op046 \n\
	.long	_op047 \n\
	.long	_op050 \n\
	.long	_op051 \n\
	.long	_op052 \n\
	.long	_op053 \n\
	.long	_op054 \n\
	.long	_op055 \n\
	.long	_op056 \n\
	.long	_op057 \n\
	.long	_op060 \n\
	.long	_op061 \n\
	.long	_op062 \n\
	.long	_op063 \n\
	.long	_op064 \n\
	.long	_op065 \n\
	.long	_op066 \n\
	.long	_op067 \n\
	.long	_op070 \n\
	.long	_op071 \n\
	.long	_op072 \n\
	.long	_op073 \n\
	.long	_op074 \n\
	.long	_op075 \n\
	.long	_op076 \n\
	.long	_op077 \n\
	.long	_op100 \n\
	.long	_op101 \n\
	.long	_op102 \n\
	.long	_op103 \n\
	.long	_op104 \n\
	.long	_op105 \n\
	.long	_op106 \n\
	.long	_op107 \n\
	.long	_op110 \n\
	.long	_op111 \n\
	.long	_op112 \n\
	.long	_op113 \n\
	.long	_op114 \n\
	.long	_op115 \n\
	.long	_op116 \n\
	.long	_op117 \n\
	.long	_op120 \n\
	.long	_op121 \n\
	.long	_op122 \n\
	.long	_op123 \n\
	.long	_op124 \n\
	.long	_op125 \n\
	.long	_op126 \n\
	.long	_op127 \n\
	.long	_op130 \n\
	.long	_op131 \n\
	.long	_op132 \n\
	.long	_op133 \n\
	.long	_op134 \n\
	.long	_op135 \n\
	.long	_op136 \n\
	.long	_op137 \n\
	.long	_op140 \n\
	.long	_op141 \n\
	.long	_op142 \n\
	.long	_op143 \n\
	.long	_op144 \n\
	.long	_op145 \n\
	.long	_op146 \n\
	.long	_op147 \n\
	.long	_op150 \n\
	.long	_op151 \n\
	.long	_op152 \n\
	.long	_op153 \n\
	.long	_op154 \n\
	.long	_op155 \n\
	.long	_op156 \n\
	.long	_op157 \n\
	.long	_op160 \n\
	.long	_op161 \n\
	.long	_op162 \n\
	.long	_op163 \n\
	.long	_op164 \n\
	.long	_op165 \n\
	.long	_op166 \n\
	.long	_op167 \n\
	.long	_op170 \n\
	.long	_op171 \n\
	.long	_op172 \n\
	.long	_op173 \n\
	.long	_op174 \n\
	.long	_op175 \n\
	.long	_op176 \n\
	.long	_op177 \n\
	.long	_op200 \n\
	.long	_op201 \n\
	.long	_op202 \n\
	.long	_op203 \n\
	.long	_op204 \n\
	.long	_op205 \n\
	.long	_op206 \n\
	.long	_op207 \n\
	.long	_op210 \n\
	.long	_op211 \n\
	.long	_op212 \n\
	.long	_op213 \n\
	.long	_op214 \n\
	.long	_op215 \n\
	.long	_op216 \n\
	.long	_op217 \n\
	.long	_op220 \n\
	.long	_op221 \n\
	.long	_op222 \n\
	.long	_op223 \n\
	.long	_op224 \n\
	.long	_op225 \n\
	.long	_op226 \n\
	.long	_op227 \n\
	.long	_op230 \n\
	.long	_op231 \n\
	.long	_op232 \n\
	.long	_op233 \n\
	.long	_op234 \n\
	.long	_op235 \n\
	.long	_op236 \n\
	.long	_op237 \n\
	.long	_op240 \n\
	.long	_op241 \n\
	.long	_op242 \n\
	.long	_op243 \n\
	.long	_op244 \n\
	.long	_op245 \n\
	.long	_op246 \n\
	.long	_op247 \n\
	.long	_op250 \n\
	.long	_op251 \n\
	.long	_op252 \n\
	.long	_op253 \n\
	.long	_op254 \n\
	.long	_op255 \n\
	.long	_op256 \n\
	.long	_op257 \n\
	.long	_op260 \n\
	.long	_op261 \n\
	.long	_op262 \n\
	.long	_op263 \n\
	.long	_op264 \n\
	.long	_op265 \n\
	.long	_op266 \n\
	.long	_op267 \n\
	.long	_op270 \n\
	.long	_op271 \n\
	.long	_op272 \n\
	.long	_op273 \n\
	.long	_op274 \n\
	.long	_op275 \n\
	.long	_op276 \n\
	.long	_op277 \n\
	.long	_op300 \n\
	.long	_op301 \n\
	.long	_op302 \n\
	.long	_op303 \n\
	.long	_op304 \n\
	.long	_op305 \n\
	.long	_op306 \n\
	.long	_op307 \n\
	.long	_op310 \n\
	.long	_op311 \n\
	.long	_op312 \n\
	.long	_op313 \n\
	.long	_op314 \n\
	.long	_op315 \n\
	.long	_op316 \n\
	.long	_op317 \n\
	.long	_op320 \n\
	.long	_op321 \n\
	.long	_op322 \n\
	.long	_op323 \n\
	.long	_op324 \n\
	.long	_op325 \n\
	.long	_op326 \n\
	.long	_op327 \n\
	.long	_op330 \n\
	.long	_op331 \n\
	.long	_op332 \n\
	.long	_op333 \n\
	.long	_op334 \n\
	.long	_op335 \n\
	.long	_op336 \n\
	.long	_op337 \n\
	.long	_op340 \n\
	.long	_op341 \n\
	.long	_op342 \n\
	.long	_op343 \n\
	.long	_op344 \n\
	.long	_op345 \n\
	.long	_op346 \n\
	.long	_op347 \n\
	.long	_op350 \n\
	.long	_op351 \n\
	.long	_op352 \n\
	.long	_op353 \n\
	.long	_op354 \n\
	.long	_op355 \n\
	.long	_op356 \n\
	.long	_op357 \n\
	.long	_op360 \n\
	.long	_op361 \n\
	.long	_op362 \n\
	.long	_op363 \n\
	.long	_op364 \n\
	.long	_op365 \n\
	.long	_op366 \n\
	.long	_op367 \n\
	.long	_op370 \n\
	.long	_op371 \n\
	.long	_op372 \n\
	.long	_op373 \n\
	.long	_op374 \n\
	.long	_op375 \n\
	.long	_op376 \n\
	.long	_op377 \n\
	 .text");

#elif (DOS && OPDISP)
        /* This is the optable for 386's under gcc & Turbo Assembler */

asm volatile("	align 4");
asm volatile("optable:  \n\
        DD   _op000 \n\
        DD   _op001 \n\
        DD   _op002 \n\
        DD   _op003 \n\
        DD   _op004 \n\
        DD   _op005 \n\
        DD   _op006 \n\
        DD   _op007 \n\
        DD   _op010 \n\
        DD   _op011 \n\
        DD   _op012 \n\
        DD   _op013 \n\
        DD   _op014 \n\
        DD   _op015 \n\
        DD   _op016 \n\
        DD   _op017 \n\
        DD   _op020 \n\
        DD   _op021 \n\
        DD   _op022 \n\
        DD   _op023 \n\
        DD   _op024 \n\
        DD   _op025 \n\
        DD   _op026 \n\
        DD   _op027 \n\
        DD   _op030 \n\
        DD   _op031 \n\
        DD   _op032 \n\
        DD   _op033 \n\
");

asm volatile("\n\
        DD   _op034 \n\
        DD   _op035 \n\
        DD   _op036 \n\
        DD   _op037 \n\
        DD   _op040 \n\
        DD   _op041 \n\
        DD   _op042 \n\
        DD   _op043 \n\
        DD   _op044 \n\
        DD   _op045 \n\
        DD   _op046 \n\
        DD   _op047 \n\
        DD   _op050 \n\
        DD   _op051 \n\
        DD   _op052 \n\
        DD   _op053 \n\
        DD   _op054 \n\
        DD   _op055 \n\
        DD   _op056 \n\
        DD   _op057 \n\
        DD   _op060 \n\
        DD   _op061 \n\
        DD   _op062 \n\
        DD   _op063 \n\
        DD   _op064 \n\
        DD   _op065 \n\
        DD   _op066 \n\
        DD   _op067 \n\
        DD   _op070 \n\
        DD   _op071 \n\
        DD   _op072 \n\
        DD   _op073 \n\
        DD   _op074 \n\
        DD   _op075 \n\
        DD   _op076 \n\
        DD   _op077 \n\
        DD   _op100 \n\
        DD   _op101 \n\
        DD   _op102 \n\
        DD   _op103 \n\
        DD   _op104 \n\
        DD   _op105 \n\
        DD   _op106 \n\
        DD   _op107 \n\
        DD   _op110 \n\
        DD   _op111 \n\
        DD   _op112 \n\
        DD   _op113 \n\
        DD   _op114 \n\
        DD   _op115 \n\
        DD   _op116 \n\
        DD   _op117 \n\
        DD   _op120 \n\
        DD   _op121 \n\
        DD   _op122 \n\
        DD   _op123 \n\
        DD   _op124 \n\
        DD   _op125 \n\
        DD   _op126 \n\
        DD   _op127 \n\
        DD   _op130 \n\
        DD   _op131 \n\
        DD   _op132 \n\
        DD   _op133 \n\
        DD   _op134 \n\
        DD   _op135 \n\
        DD   _op136 \n\
        DD   _op137 \n\
        DD   _op140 \n\
        DD   _op141 \n\
        DD   _op142 \n\
        DD   _op143 \n\
        DD   _op144 \n\
        DD   _op145 \n\
        DD   _op146 \n\
        DD   _op147 \n\
        DD   _op150 \n\
        DD   _op151 \n\
        DD   _op152 \n\
        DD   _op153 \n\
        DD   _op154 \n\
        DD   _op155 \n\
        DD   _op156 \n\
        DD   _op157 \n\
        DD   _op160 \n\
        DD   _op161 \n\
        DD   _op162 \n\
        DD   _op163 \n\
        DD   _op164 \n\
        DD   _op165 \n\
        DD   _op166 \n\
        DD   _op167 \n\
        DD   _op170 \n\
        DD   _op171 \n\
        DD   _op172 \n\
        DD   _op173 \n\
        DD   _op174 \n\
        DD   _op175 \n\
        DD   _op176 \n\
        DD   _op177 \n\
        DD   _op200 \n\
        DD   _op201 \n\
        DD   _op202 \n\
        DD   _op203 \n\
        DD   _op204 \n\
        DD   _op205 \n\
        DD   _op206 \n\
        DD   _op207 \n\
        DD   _op210 \n\
        DD   _op211 \n\
        DD   _op212 \n\
        DD   _op213 \n\
        DD   _op214 \n\
        DD   _op215 \n\
        DD   _op216 \n\
        DD   _op217 \n\
        DD   _op220 \n\
        DD   _op221 \n\
        DD   _op222 \n\
        DD   _op223 \n\
        DD   _op224 \n\
        DD   _op225 \n\
        DD   _op226 \n\
        DD   _op227 \n\
        DD   _op230 \n\
        DD   _op231 \n\
        DD   _op232 \n\
        DD   _op233 \n\
        DD   _op234 \n\
        DD   _op235 \n\
        DD   _op236 \n\
        DD   _op237 \n\
        DD   _op240 \n\
        DD   _op241 \n\
        DD   _op242 \n\
        DD   _op243 \n\
        DD   _op244 \n\
        DD   _op245 \n\
        DD   _op246 \n\
        DD   _op247 \n\
        DD   _op250 \n\
        DD   _op251 \n\
        DD   _op252 \n\
        DD   _op253 \n\
        DD   _op254 \n\
        DD   _op255 \n\
        DD   _op256 \n\
        DD   _op257 \n\
        DD   _op260 \n\
        DD   _op261 \n\
        DD   _op262 \n\
        DD   _op263 \n\
        DD   _op264 \n\
        DD   _op265 \n\
        DD   _op266 \n\
        DD   _op267 \n\
        DD   _op270 \n\
        DD   _op271 \n\
        DD   _op272 \n\
        DD   _op273 \n\
        DD   _op274 \n\
        DD   _op275 \n\
        DD   _op276 \n\
        DD   _op277 \n\
        DD   _op300 \n\
        DD   _op301 \n\
        DD   _op302 \n\
        DD   _op303 \n\
        DD   _op304 \n\
        DD   _op305 \n\
        DD   _op306 \n\
        DD   _op307 \n\
        DD   _op310 \n\
        DD   _op311 \n\
        DD   _op312 \n\
        DD   _op313 \n\
        DD   _op314 \n\
        DD   _op315 \n\
        DD   _op316 \n\
        DD   _op317 \n\
        DD   _op320 \n\
        DD   _op321 \n\
        DD   _op322 \n\
        DD   _op323 \n\
        DD   _op324 \n\
        DD   _op325 \n\
        DD   _op326 \n\
        DD   _op327 \n\
        DD   _op330 \n\
        DD   _op331 \n\
        DD   _op332 \n\
        DD   _op333 \n\
        DD   _op334 \n\
        DD   _op335 \n\
        DD   _op336 \n\
        DD   _op337 \n\
        DD   _op340 \n\
        DD   _op341 \n\
        DD   _op342 \n\
        DD   _op343 \n\
        DD   _op344 \n\
        DD   _op345 \n\
        DD   _op346 \n\
        DD   _op347 \n\
        DD   _op350 \n\
        DD   _op351 \n\
        DD   _op352 \n\
        DD   _op353 \n\
        DD   _op354 \n\
        DD   _op355 \n\
        DD   _op356 \n\
        DD   _op357 \n\
        DD   _op360 \n\
        DD   _op361 \n\
        DD   _op362 \n\
        DD   _op363 \n\
        DD   _op364 \n\
        DD   _op365 \n\
        DD   _op366 \n\
        DD   _op367 \n\
        DD   _op370 \n\
        DD   _op371 \n\
        DD   _op372 \n\
        DD   _op373 \n\
        DD   _op374 \n\
        DD   _op375 \n\
        DD   _op376 \n\
        DD   _op377");
    asm volatile("\n\
FP_noint:	DW	003fh	;; No interrupts, round to closest, 24bit precision");


#endif /* ISC */


#endif /* OPDISP */

 
    }


int retfun(void) {return(0);}
do_brk(void) {}

