/* $Id: xc.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
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

#ifdef MAIKO_OS_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef DOS
#include <i32.h> /* Defines "#pragma interrupt"  */
#include <stk.h> /* _XSTACK struct definition    */
#include <dos.h> /* Defines REGS & other structs */
#else            /* DOS */
#include <sys/time.h>
#endif /* DOS */

#include "lispemul.h"
#include "emlglob.h"
#include "address.h"
#include "adr68k.h"
#include "stack.h"
#include "return.h"
#include "dbprint.h"

#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "cell.h"
#include "initatms.h"
#include "gcdata.h"
#include "arith.h"
#include "stream.h"

#include "testtooldefs.h"
#include "tos1defs.h"
#include "tosret.h"
#include "tosfns.h"
#include "inlineC.h"

#include "xcdefs.h"
#include "arithopsdefs.h"
#include "arrayopsdefs.h"
#include "bitbltdefs.h"
#include "bltdefs.h"
#include "byteswapdefs.h"
#include "car-cdrdefs.h"
#include "commondefs.h"
#include "conspagedefs.h"
#include "drawdefs.h"
#include "eqfdefs.h"
#include "devif.h"
#include "findkeydefs.h"
#include "fpdefs.h"
#include "fvardefs.h"
#include "gchtfinddefs.h"
#include "gcscandefs.h"
#include "gvar2defs.h"
#include "hardrtndefs.h"
#include "ifpage.h"
#include "intcalldefs.h"
#include "keyeventdefs.h"
#include "llstkdefs.h"
#include "lowlev2defs.h"
#include "lsthandldefs.h"
#include "misc7defs.h"
#include "miscndefs.h"
#include "mkcelldefs.h"
#include "returndefs.h"
#include "rplconsdefs.h"
#include "shiftdefs.h"
#include "subrdefs.h"
#include "timerdefs.h"
#include "typeofdefs.h"
#include "ubf1defs.h"
#include "ubf2defs.h"
#include "ubf3defs.h"
#include "unwinddefs.h"
#include "vars3defs.h"
#ifdef XWINDOW
#include "xwinmandefs.h"
#endif
#include "z2defs.h"

#ifdef DOS
#include "devif.h"
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
extern MouseInterface currentmouse;
#elif defined(XWINDOW)
extern DspInterface currentdsp;
#endif /* DOS */

#ifdef SDL
extern void process_SDLevents();
#endif

typedef struct conspage ConsPage;
typedef ByteCode *InstPtr;

#if defined(DOS) && defined(OPDISP)
#include "inlndos.h"
InstPtr pccache asm("si");
LispPTR *cspcache asm("di");
LispPTR tscache asm("bx");
#endif /* DOS */

/* This used to be here for including optimized dispatch
 * for SPARC, but it has some other things in it, so we
 * are keeping it around for now until we sort that out. */
#ifdef SPARCDISP
#include "inlnSPARC.h"
#endif /* SPARCDISP */

#include "fast_dsp.h"

/* trick now is that pccache points one ahead... */
#define PCMAC (pccache - 1)
#define PCMACL pccache
#define CSTKPTR ((LispPTR *)cspcache)
#define PVAR ((LispPTR *)PVar)
#define IVAR ((LispPTR *)IVar)
#define BCE_CURRENTFX ((struct frameex2 *)((DLword *)PVAR - FRAMESIZE))

/* Define alternative macros for CSTKPTR, PVAR, and IVAR that can be used
 * in an lvalue context, since CSTKPTR = ...; would generate
 * error: assignment to cast is illegal, lvalue casts are not supported
 */

#define CSTKPTRL (cspcache)
#define PVARL PVar
#define IVARL IVar

/* used by SIGIO signal handler to indicate I/O may be possible */
extern volatile sig_atomic_t IO_Signalled;

#ifdef PCTRACE
/* For keeping a trace table (ring buffer) of 100 last PCs */
int pc_table[100],     /* The PC */
    op_table[100];     /* The opcode at that PC */
LispPTR fn_table[100]; /* The code block the PC is in (Lisp ptr) */
int pccounter = 0;     /* ring-buffer counter */
#endif                 /* PCTRACE */

extern int extended_frame;
int extended_frame; /*indicates if soft stack overflow */

static const int n_mask_array[16] = {
    1,     3,     7,     0xf,   0x1f,   0x3f,   0x7f,   0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

extern int TIMER_INTERVAL;

#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)

#  if !defined(MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN)
#    define MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN 20000
#  endif

int insnsCountdownForTimerAsyncEmulation = MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN;
static int pseudoTimerAsyncCountdown = MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN;

#endif

void dispatch(void) {
  InstPtr pccache;

#if defined(OPDISP)
  static const void* optable[256] = {
    &&op_ufn,  &&case001, &&case002, &&case003, &&case004, &&case005, &&case006, &&case007,
    &&case010, &&case011, &&case012, &&case013, &&case014, &&case015, &&case016, &&case017,
    &&case020, &&case021, &&case022, &&case023, &&case024, &&case025, &&case026, &&case027,
    &&case030, &&case031, &&case032, &&case033, &&case034, &&case035, &&case036, &&case037,
    &&case040, &&case041, &&case042, &&case043, &&case044, &&case045, &&case046, &&case047,
    &&op_ufn,  &&op_ufn,  &&op_ufn,  &&op_ufn,  &&case054, &&case055, &&case056, &&case057,
    &&op_ufn,  &&op_ufn,  &&case062, &&case063, &&op_ufn,  &&op_ufn,  &&op_ufn,  &&op_ufn,
    &&case070, &&op_ufn,  &&case072, &&case073, &&case074, &&case075, &&op_ufn,  &&op_ufn,
    &&case100, &&case101, &&case102, &&case103, &&case104, &&case105, &&case106, &&case107,
    &&case110, &&case111, &&case112, &&case113, &&case114, &&case115, &&case116, &&case117,
    &&case120, &&case121, &&case122, &&case123, &&case124, &&case125, &&case126, &&case127,
    &&case130, &&case131, &&case132, &&case133, &&case134, &&case135, &&case136, &&case137,
    &&case140, &&case141, &&case142, &&case143, &&case144, &&case145, &&case146, &&case147,
    &&case150, &&case151, &&case152, &&case153, &&case154, &&case155, &&case156, &&case157,
    &&case160, &&case161, &&case162, &&case163, &&case164, &&case165, &&case166, &&case167,
    &&case170, &&case171, &&case172, &&case173, &&case174, &&case175, &&case176, &&case177,
    &&case200, &&case201, &&case202, &&case203, &&case204, &&case205, &&case206, &&case207,
    &&case210, &&case211, &&case212, &&case213, &&case214, &&case215, &&case216, &&case217,
    &&case220, &&case221, &&case222, &&case223, &&case224, &&case225, &&case226, &&case227,
    &&case230, &&case231, &&case232, &&case233, &&case234, &&case235, &&case236, &&case237,
    &&case240, &&case241, &&case242, &&case243, &&case244, &&case245, &&case246, &&case247,
    &&case250, &&case251, &&case252, &&case253, &&case254, &&case255, &&case256, &&case257,
    &&case260, &&case261, &&case262, &&case263, &&case264, &&case265, &&case266, &&case267,
    &&case270, &&case271, &&case272, &&case273, &&case274, &&case275, &&case276, &&case277,
    &&case300, &&case301, &&case302, &&case303, &&case304, &&case305, &&case306, &&case307,
    &&case310, &&case311, &&case312, &&case313, &&case314, &&case315, &&case316, &&case317,
    &&case320, &&case321, &&case322, &&case323, &&case324, &&case325, &&case326, &&case327,
    &&case330, &&case331, &&case332, &&case333, &&case334, &&case335, &&case336, &&case337,
    &&case340, &&case341, &&case342, &&case343, &&case344, &&case345, &&case346, &&case347,
    &&case350, &&case351, &&case352, &&case353, &&case354, &&case355, &&case356, &&case357,
    &&case360, &&case361, &&case362, &&case363, &&case364, &&case365, &&case366, &&case367,
    &&case370, &&case371, &&case372, &&case373, &&case374, &&case375, &&case376, &&case377,
  };
#endif

#if defined(DOS) && defined(OPDISP)
#else
  LispPTR *cspcache;
  LispPTR tscache;
#endif

  /* OP_FN_COMMON arguments */

  DefCell *fn_defcell;
  LispPTR fn_atom_index;
  int fn_opcode_size;
  int fn_num_args;
  int fn_apply;

  RET;
  CLR_IRQ;

  goto nextopcode;

  /* OPCODE FAIL ENTRY POINTS, CALL UFNS HERE */
  UFN_CALLS

op_ufn : {
  UFN *entry68k;
  entry68k = (UFN *)GetUFNEntry(Get_BYTE_PCMAC0);
  fn_num_args = entry68k->arg_num;
  fn_opcode_size = entry68k->byte_num + 1;
  fn_atom_index = entry68k->atom_name;
  fn_defcell = (DefCell *)GetDEFCELL68k(fn_atom_index);
  fn_apply = 2 + entry68k->byte_num; /* code for UFN entry */
  goto op_fn_common;
}

  /* FUNCTION CALL TAIL ROUTINE */

  OP_FN_COMMON

/* DISPATCH "LOOP" */

nextopcode:
#ifdef MYOPTRACE
  if ((struct fnhead *)NativeAligned4FromLAddr(0x2ed600) == FuncObj) {
    quick_stack_check();
#endif /* MYOPTRACE */
    OPTPRINT(("PC= %p (fn+%td) op= %02x TOS= 0x%x\n", (void *)PCMAC, PCMAC - (char *)FuncObj, Get_BYTE_PCMAC0, TOPOFSTACK));
#ifdef MYOPTRACE
  }
#endif /* MYOPTRACE */

#ifdef PCTRACE
  /* Tracing PC/Function/Opcode in a ring buffer */
  pc_table[pccounter] = (int)PCMAC - (int)FuncObj;
  fn_table[pccounter] = (LispPTR)LAddrFromNative(FuncObj);
  op_table[pccounter] = Get_BYTE_PCMAC0;
  if (99 == pccounter++) pccounter = 0;
#endif /* PCTRACE */

  /* quick_stack_check();*/ /* JDS 2/12/98 */
  
#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
  if (--pseudoTimerAsyncCountdown <= 0) {
	  Irq_Stk_Check = 0;
	  Irq_Stk_End = 0;
#if defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
	  IO_Signalled = TRUE;
#endif
#ifdef MAIKO_OS_EMSCRIPTEN
	  emscripten_sleep(1);
#endif
	  pseudoTimerAsyncCountdown = insnsCountdownForTimerAsyncEmulation;
  }
#endif

  switch (Get_BYTE_PCMAC0) {
    case 000:
    case000 : { goto op_ufn; } /* unused */
    case 001:
    case001:
      OPCAR;
    case 002:
    case002:
      OPCDR;
    case 003:
    case003:
      LISTP;
    case 004:
    case004:
      NTYPEX;
    case 005:
    case005:
      TYPEP(Get_BYTE_PCMAC1);
    case 056:
    case056:
    case 006:
    case006:
      DTEST(Get_AtomNo_PCMAC1);
    case 007:
    case007:
      UNWIND(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 010:
    case010:
      FN0;
    case 011:
    case011:
      FN1;
    case 012:
    case012:
      FN2;
    case 013:
    case013:
      FN3;
    case 014:
    case014:
      FN4;
    case 015:
    case015:
      FNX;
    case 016:
    case016:
      APPLY;

    case 017:
    case017:
      CHECKAPPLY;
    case 020:
    case020:
      RETURN;

    case 021:
    case021:
      /* UB: left shift of negative value -4 */
      BIND;
    case 022:
    case022:
      UNBIND;
    case 023:
    case023:
      DUNBIND;
    case 024:
    case024:
      RPLPTR(Get_BYTE_PCMAC1);
    case 025:
    case025:
      GCREF(Get_BYTE_PCMAC1);
    case 026:
    case026:
      ASSOC;
    case 027:
    case027:
      GVAR_(Get_AtomNo_PCMAC1);
    case 030:
    case030:
      RPLACA;
    case 031:
    case031:
      RPLACD;
    case 032:
    case032:
      CONS;
    case 033:
    case033:
      CLASSOC;
    case 034:
    case034:
      FMEMB;
    case 035:
    case035:
      CLFMEMB;
    case 036:
    case036:
      FINDKEY(Get_BYTE_PCMAC1);
    case 037:
    case037:
      CREATECELL;
    case 040:
    case040:
      BIN;
    case 041:
    case041 : { goto op_ufn; } /* BOUT */
    case 042:
    case042 : { goto op_ufn; } /* POPDISP - prolog only */
    case 043:
    case043:
      RESTLIST(Get_BYTE_PCMAC1);
    case 044:
    case044:
      MISCN(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 045:
    case045 : { goto op_ufn; } /* unused */
    case 046:
    case046:
      RPLCONS;
    case 047:
    case047:
      LISTGET;
    case 050:
    case050 : { goto op_ufn; } /* unused */
    case 051:
    case051 : { goto op_ufn; } /* unused */
    case 052:
    case052 : { goto op_ufn; } /* unused */
    case 053:
    case053 : { goto op_ufn; } /* unused */
    case 054:
    case054:
      EVAL;
    case 055:
    case055:
      ENVCALL;

    /*  case 056 : case056: @ 006 */
    case 057:
    case057:
      STKSCAN;
    case 060:
    case060 : { goto op_ufn; } /* BUSBLT - DLion only */
    case 061:
    case061 : { goto op_ufn; } /* MISC8 - no longer used */
    case 062:
    case062:
      UBFLOAT3(Get_BYTE_PCMAC1);
    case 063:
    case063:
      TYPEMASK(Get_BYTE_PCMAC1);
    case 064:
    case064 : { goto op_ufn; } /* rdprologptr */
    case 065:
    case065 : { goto op_ufn; } /* rdprologtag */
    case 066:
    case066 : { goto op_ufn; } /* writeptr&tag */
    case 067:
    case067 : { goto op_ufn; } /* writeptr&0tag */
    case 070:
    case070:
      MISC7(Get_BYTE_PCMAC1); /* misc7 (pseudocolor, fbitmapbit) */
    case 071:
    case071 : { goto op_ufn; } /* dovemisc - dove only */
    case 072:
    case072:
      EQLOP;
    case 073:
    case073:
      DRAWLINE;
    case 074:
    case074:
      STOREN(Get_BYTE_PCMAC1);
    case 075:
    case075:
      COPYN(Get_BYTE_PCMAC1);
    case 076:
    case076 : { goto op_ufn; } /* RAID */
    case 077:
    case077 : { goto op_ufn; } /* \RETURN */

    case 0100:
    case100:
      IVARMACRO(0);
    case 0101:
    case101:
      IVARMACRO(1);
    case 0102:
    case102:
      IVARMACRO(2);
    case 0103:
    case103:
      IVARMACRO(3);
    case 0104:
    case104:
      IVARMACRO(4);
    case 0105:
    case105:
      IVARMACRO(5);
    case 0106:
    case106:
      IVARMACRO(6);
    case 0107:
    case107:
      IVARX(Get_BYTE_PCMAC1);

    case 0110:
    case110:
      PVARMACRO(0);
    case 0111:
    case111:
      PVARMACRO(1);
    case 0112:
    case112:
      PVARMACRO(2);
    case 0113:
    case113:
      PVARMACRO(3);
    case 0114:
    case114:
      PVARMACRO(4);
    case 0115:
    case115:
      PVARMACRO(5);
    case 0116:
    case116:
      PVARMACRO(6);

    case 0117:
    case117:
      PVARX(Get_BYTE_PCMAC1);

    case 0120:
    case120:
      FVAR(0);
    case 0121:
    case121:
      FVAR(2);
    case 0122:
    case122:
      FVAR(4);
    case 0123:
    case123:
      FVAR(6);
    case 0124:
    case124:
      FVAR(8);
    case 0125:
    case125:
      FVAR(10);
    case 0126:
    case126:
      FVAR(12);
    case 0127:
    case127:
      FVARX(Get_BYTE_PCMAC1);

    case 0130:
    case130:
      PVARSETMACRO(0);
    case 0131:
    case131:
      PVARSETMACRO(1);
    case 0132:
    case132:
      PVARSETMACRO(2);
    case 0133:
    case133:
      PVARSETMACRO(3);
    case 0134:
    case134:
      PVARSETMACRO(4);
    case 0135:
    case135:
      PVARSETMACRO(5);
    case 0136:
    case136:
      PVARSETMACRO(6);

    case 0137:
    case137:
      PVARX_(Get_BYTE_PCMAC1);

    case 0140:
    case140:
      GVAR(Get_AtomNo_PCMAC1);
    case 0141:
    case141:
      ARG0;
    case 0142:
    case142:
      IVARX_(Get_BYTE_PCMAC1);
    case 0143:
    case143:
      FVARX_(Get_BYTE_PCMAC1);
    case 0144:
    case144:
      COPY;
    case 0145:
    case145:
      MYARGCOUNT;
    case 0146:
    case146:
      MYALINK;

    /******** Aconst	********/
    case 0147:
    case147 : {
      PUSH(Get_AtomNo_PCMAC1);
      nextop_atom;
    }
    case 0150:
    case150 : { PUSHATOM(NIL_PTR); }
    case 0151:
    case151 : { PUSHATOM(ATOM_T); }
    case 0152:
    case152 : { PUSHATOM(S_POSITIVE); } /* '0 */
    case 0153:
    case153 : { PUSHATOM(0xE0001); } /* '1 */

    /********* SIC		********/
    case 0154:
    case154 : {
      PUSH(S_POSITIVE | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SNIC		********/
    case 0155:
    case155 : {
      PUSH(S_NEGATIVE | 0xff00 | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SICX		********/
    case 0156:
    case156 : {
      PUSH(S_POSITIVE | Get_DLword_PCMAC1);
      nextop3;
    }

    /********* GCONST	********/
    case 0157:
    case157 : {
      PUSH(Get_Pointer_PCMAC1);
      nextop_ptr;
    }

    case 0160:
    case160 : { goto op_ufn; } /* unused */
    case 0161:
    case161 : { goto op_ufn; } /* readflags */
    case 0162:
    case162 : { goto op_ufn; } /* readrp */
    case 0163:
    case163 : { goto op_ufn; } /* writemap */
    case 0164:
    case164 : { goto op_ufn; } /* readprinterport */
    case 0165:
    case165 : { goto op_ufn; } /* writeprinterport */

    case 0166:
    case166:
      PILOTBITBLT;
    case 0167:
    case167:
      RCLK;
    case 0170:
    case170 : { goto op_ufn; } /* MISC1, dorado only */
    case 0171:
    case171 : { goto op_ufn; } /* MISC2, dorado only */
    case 0172:
    case172:
      RECLAIMCELL;
    case 0173:
    case173:
      GCSCAN1;
    case 0174:
    case174:
      GCSCAN2;
    case 0175:
    case175 : {
      EXT;
      OP_subrcall(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
      RET;
      nextop0;
    }
    case 0176:
    case176 : { CONTEXTSWITCH; }
    case 0177:
    case177 : { goto op_ufn; } /* RETCALL */

    /* JUMP */

    case 0200:
    case200 : { JUMPMACRO(2); }
    case 0201:
    case201 : { JUMPMACRO(3); }
    case 0202:
    case202 : { JUMPMACRO(4); }
    case 0203:
    case203 : { JUMPMACRO(5); }
    case 0204:
    case204 : { JUMPMACRO(6); }
    case 0205:
    case205 : { JUMPMACRO(7); }
    case 0206:
    case206 : { JUMPMACRO(8); }
    case 0207:
    case207 : { JUMPMACRO(9); }
    case 0210:
    case210 : { JUMPMACRO(10); }
    case 0211:
    case211 : { JUMPMACRO(11); }
    case 0212:
    case212 : { JUMPMACRO(12); }
    case 0213:
    case213 : { JUMPMACRO(13); }
    case 0214:
    case214 : { JUMPMACRO(14); }
    case 0215:
    case215 : { JUMPMACRO(15); }
    case 0216:
    case216 : { JUMPMACRO(16); }
    case 0217:
    case217 : { JUMPMACRO(17); }

    /* FJUMP */

    case 0220:
    case220 : { FJUMPMACRO(2); }
    case 0221:
    case221 : { FJUMPMACRO(3); }
    case 0222:
    case222 : { FJUMPMACRO(4); }
    case 0223:
    case223 : { FJUMPMACRO(5); }
    case 0224:
    case224 : { FJUMPMACRO(6); }
    case 0225:
    case225 : { FJUMPMACRO(7); }
    case 0226:
    case226 : { FJUMPMACRO(8); }
    case 0227:
    case227 : { FJUMPMACRO(9); }
    case 0230:
    case230 : { FJUMPMACRO(10); }
    case 0231:
    case231 : { FJUMPMACRO(11); }
    case 0232:
    case232 : { FJUMPMACRO(12); }
    case 0233:
    case233 : { FJUMPMACRO(13); }
    case 0234:
    case234 : { FJUMPMACRO(14); }
    case 0235:
    case235 : { FJUMPMACRO(15); }
    case 0236:
    case236 : { FJUMPMACRO(16); }
    case 0237:
    case237 : { FJUMPMACRO(17); }

    /* TJUMP */

    case 0240:
    case240 : { TJUMPMACRO(2); }
    case 0241:
    case241 : { TJUMPMACRO(3); }
    case 0242:
    case242 : { TJUMPMACRO(4); }
    case 0243:
    case243 : { TJUMPMACRO(5); }
    case 0244:
    case244 : { TJUMPMACRO(6); }
    case 0245:
    case245 : { TJUMPMACRO(7); }
    case 0246:
    case246 : { TJUMPMACRO(8); }
    case 0247:
    case247 : { TJUMPMACRO(9); }
    case 0250:
    case250 : { TJUMPMACRO(10); }
    case 0251:
    case251 : { TJUMPMACRO(11); }
    case 0252:
    case252 : { TJUMPMACRO(12); }
    case 0253:
    case253 : { TJUMPMACRO(13); }
    case 0254:
    case254 : { TJUMPMACRO(14); }
    case 0255:
    case255 : { TJUMPMACRO(15); }
    case 0256:
    case256 : { TJUMPMACRO(16); }
    case 0257:
    case257 : { TJUMPMACRO(17); }

    /******* JUMPX ********/
    case 0260:
    case260 : {
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* JUMPXX ********/
    case 0261:
    case261 : {
      CHECK_INTERRUPT;
      /* UB: left shift of negative value -1 */
      PCMACL += (Get_SBYTE_PCMAC1 << 8) | Get_BYTE_PCMAC2;
      nextop0;
    }

    /******* FJumpx *******/
    case 0262:
    case262 : {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* TJumpx *******/

    case 0263:
    case263 : {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NFJumpx *******/

    case 0264:
    case264 : {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NTJumpx *******/

    case 0265:
    case265 : {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    case 0266:
    case266:
      AREF1;
    case 0267:
    case267:
      ASET1;

    case 0270:
    case270:
      PVARSETPOPMACRO(0);
    case 0271:
    case271:
      PVARSETPOPMACRO(1);
    case 0272:
    case272:
      PVARSETPOPMACRO(2);
    case 0273:
    case273:
      PVARSETPOPMACRO(3);
    case 0274:
    case274:
      PVARSETPOPMACRO(4);
    case 0275:
    case275:
      PVARSETPOPMACRO(5);
    case 0276:
    case276:
      PVARSETPOPMACRO(6);

    case 0277:
    case277 : {
      POP;
      nextop1;
    }

    case 0300:
    case300:
      POPN(Get_BYTE_PCMAC1);
    case 0301:
    case301:
      ATOMCELL_N(Get_BYTE_PCMAC1);
    case 0302:
    case302:
      GETBASEBYTE;
    case 0303:
    case303:
      INSTANCEP(Get_AtomNo_PCMAC1);
    case 0304:
    case304:
      BLT;
    case 0305:
    case305 : { goto op_ufn; } /* MISC10 */
    case 0306:
    case306 : { goto op_ufn; } /* P-MISC2 ??? */
    case 0307:
    case307:
      PUTBASEBYTE;
    case 0310:
    case310:
      GETBASE_N(Get_BYTE_PCMAC1);
    case 0311:
    case311:
      GETBASEPTR_N(Get_BYTE_PCMAC1);
    case 0312:
    case312:
      GETBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 0313:
    case313 : { goto op_ufn; } /* unused */
    case 0314:
    case314:
      CLEQUAL;
    case 0315:
    case315:
      PUTBASE_N(Get_BYTE_PCMAC1);
    case 0316:
    case316:
      PUTBASEPTR_N(Get_BYTE_PCMAC1);
    case 0317:
    case317:
      PUTBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);

    case 0320:
    case320:
      N_OP_ADDBASE;
    case 0321:
    case321:
      N_OP_VAG2;
    case 0322:
    case322:
      N_OP_HILOC;
    case 0323:
    case323:
      N_OP_LOLOC;
    case 0324:
    case324:
      PLUS2; /* PLUS */
    case 0325:
    case325:
      DIFFERENCE; /* DIFFERENCE */
    case 0326:
    case326:
      TIMES2; /* TIMES2 */
    case 0327:
    case327:
      QUOTIENT;                          /* QUOTIENT */
    case 0330:
    case330:
      IPLUS2; /* IPLUS2 only while PLUS has no float */
    case 0331:
    case331:
      IDIFFERENCE; /* IDIFFERENCE only while no float */
    case 0332:
    case332:
      ITIMES2; /* ITIMES2 only while no float */
    case 0333:
    case333:
      IQUOTIENT; /* IQUOTIENT */
    case 0334:
    case334:
      IREMAINDER;
    case 0335:
    case335:
      IPLUS_N(Get_BYTE_PCMAC1);
    case 0336:
    case336:
      IDIFFERENCE_N(Get_BYTE_PCMAC1);
    case 0337:
    case337 : { goto op_ufn; } /* BASE-< */
    case 0340:
    case340:
      LLSH1;
    case 0341:
    case341:
      LLSH8;
    case 0342:
    case342:
      LRSH1;
    case 0343:
    case343:
      LRSH8;
    case 0344:
    case344:
      LOGOR;
    case 0345:
    case345:
      LOGAND;
    case 0346:
    case346:
      LOGXOR;
    case 0347:
    case347:
      LSH;
    case 0350:
    case350:
      FPLUS2;
    case 0351:
    case351:
      FDIFFERENCE;
    case 0352:
    case352:
      FTIMES2;
    case 0353:
    case353:
      FQUOTIENT;
    case 0354:
    case354:
      UBFLOAT2(Get_BYTE_PCMAC1);
    case 0355:
    case355:
      UBFLOAT1(Get_BYTE_PCMAC1);
    case 0356:
    case356:
      AREF2;
    case 0357:
    case357:
      ASET2;

    case 0360:
    case360 : {
      if (TOPOFSTACK == POP_TOS_1)
        TOPOFSTACK = ATOM_T;
      else
        TOPOFSTACK = NIL_PTR;
      nextop1;
    }

    case 0361:
    case361:
      IGREATERP; /* IGREATERP if no float */
    case 0362:
    case362:
      FGREATERP;
    case 0363:
    case363:
      GREATERP;
    case 0364:
    case364:
      ILEQUAL;
    case 0365:
    case365:
      MAKENUMBER;
    case 0366:
    case366:
      BOXIPLUS;
    case 0367:
    case367:
      BOXIDIFFERENCE;
    case 0370:
    case370 : { goto op_ufn; } /* FLOATBLT */
    case 0371:
    case371 : { goto op_ufn; } /* FFTSTEP */
    case 0372:
    case372:
      MISC3(Get_BYTE_PCMAC1);
    case 0373:
    case373:
      MISC4(Get_BYTE_PCMAC1);
    case 0374:
    case374 : { goto op_ufn; } /* upctrace */
    case 0375:
    case375:
      SWAP;
    case 0376:
    case376:
      NOP;
    case 0377:
    case377:
      CLARITHEQUAL;

    default: error("should not default");

  } /* switch */

/************************************************************************/
/*		TIMER INTERRUPT CHECK ROUTINE				*/
/************************************************************************/
check_interrupt:
  if ((UNSIGNED)CSTKPTR > (UNSIGNED)EndSTKP) {
    EXT;
    error("Unrecoverable Stack Overflow");
    RET;
  }

  /* Check for an IRQ request */

  {
    int need_irq;
    static int period_cnt = 60;
    extern int KBDEventFlg;
    extern int ETHEREventCount;
    extern LispPTR *KEYBUFFERING68k;
    extern LispPTR *PENDINGINTERRUPT68k;
    extern LispPTR ATOM_STARTED;
    extern LispPTR *PERIODIC_INTERRUPT68k;
    extern LispPTR *PERIODIC_INTERRUPT_FREQUENCY68k;
    extern int URaid_req;

  /* Check for an Stack Overflow */
  /* JDS 22 May 96 -- >= below used to be just >, changed because we got
     stack oflows with last frame right at end of stack, leading to loops,
     odd bugs, ... */
  /**** Changed back to > 31 July 97 ****/
  re_check_stack:
    need_irq = 0;
    if (((UNSIGNED)(CSTKPTR + 1) > Irq_Stk_Check) && (Irq_Stk_End > 0) && (Irq_Stk_Check > 0)) {
      HARD_PUSH(TOPOFSTACK);
      EXT;
      extended_frame = NIL;
      if (do_stackoverflow(NIL)) {
      stackoverflow_help:
        period_cnt = 60;
        need_irq = T;
        error("Stack Overflow, MUST HARDRESET!");
        RET;
        TOPOFSTACK = NIL_PTR;
      } else {
        RET;
        POP;
      }
      Irq_Stk_Check = (UNSIGNED)EndSTKP - STK_MIN(FuncObj);
      need_irq = (Irq_Stk_End == 0) || extended_frame;
      *PENDINGINTERRUPT68k |= extended_frame;
      Irq_Stk_End = (UNSIGNED)EndSTKP;
    }

    /* This is a good time to process keyboard/mouse and ethernet I/O
     * X events are not managed in the async/SIGIO code while
     * raw ethernet, serial port, and socket connections are.
     * If the system is configured with SIGIO handling we have a hint
     * that allows us to cheaply skip if there's nothing to do
     */
#ifdef XWINDOW
    process_Xevents(currentdsp);
#endif
#ifdef SDL
    process_SDLevents();
#endif
    if (IO_Signalled) {
      IO_Signalled = FALSE;
      process_io_events();
    }

    if ((Irq_Stk_End <= 0) || (Irq_Stk_Check <= 0) || need_irq) {
      if (StackOffsetFromNative(CSTKPTR) > InterfacePage->stackbase) {
        /* Interrupts not Disabled */
        EXT;
        update_timer();

        /*** If SPY is running, check to see if it ***/
        /*** needs an interrupt; give it one, if so. ***/
        if (*PERIODIC_INTERRUPT68k != NIL) {
          if (period_cnt > 0)
            period_cnt--;
          else {
            cause_interruptcall(PERIODIC_INTERRUPTFRAME_index);
            if (*PERIODIC_INTERRUPT_FREQUENCY68k == NIL)
              period_cnt = 0;
            else
              period_cnt =
                  (*PERIODIC_INTERRUPT_FREQUENCY68k & 0xffff) * (1000000 / 60) / TIMER_INTERVAL;
            /* number of 1/60 second periods between interrupts.
            TIMER_INTERVAL is the number of microseconds between
            timer interrupts. The calculation here avoids some
            overflow errors although there is some roundoff
            if the interrupt frequency number is too low,
            it will bottom out and just set period_cnt to 0 */
          }
        }

#ifdef DOS
        if (currentkbd->URaid == TRUE) {
          currentkbd->URaid = NIL;
          (currentkbd->device.exit)(currentkbd); /* Install the original handler */
          error("Call URaid by User Interrupt");
        } else if (currentmouse->Cursor.Moved) {
          union REGS regs;

          currentdsp->device.locked++;

          /* Remove the mouse from the old place on the screen */
          (currentdsp->mouse_invisible)(currentdsp, IOPage);

          /* Find the new delta */
          regs.w.eax = 0x000B; /* Function 0xB = get delta mickeys */
          int86(0x33, &regs, &regs);
          currentmouse->Cursor.New.x += (short)regs.w.ecx;
          currentmouse->Cursor.New.y += (short)regs.w.edx;

          if (currentmouse->Cursor.New.x < 0)
            currentmouse->Cursor.New.x = 0;
          else if (currentmouse->Cursor.New.x > (currentdsp->Display.width - 1))
            currentmouse->Cursor.New.x = currentdsp->Display.width - 1;

          if (currentmouse->Cursor.New.y < 0)
            currentmouse->Cursor.New.y = 0;
          else if (currentmouse->Cursor.New.y > (currentdsp->Display.height - 1))
            currentmouse->Cursor.New.y = currentdsp->Display.height - 1;

          IOPage->dlmousex = IOPage->dlcursorx = currentmouse->Cursor.New.x;
          IOPage->dlmousey = IOPage->dlcursory = currentmouse->Cursor.New.y;

          /* Paint the mouse back up on the screen on the new place */
          (currentdsp->mouse_visible)(currentmouse->Cursor.New.x, currentmouse->Cursor.New.y);
          currentmouse->Cursor.Moved = FALSE;
          currentdsp->device.locked--;
        }
#else
        if (URaid_req == T) {
          URaid_req = NIL;
          error("Call URaid by User Interrupt");
        }
#endif /* DOS */
        else if ((KBDEventFlg > 0) && (*KEYBUFFERING68k == ATOM_T)) {
          *KEYBUFFERING68k = ATOM_STARTED;
          cause_interruptcall(DOBUFFEREDTRANSITION_index);
          KBDEventFlg--;
        } else if (*Reclaim_cnt_word == S_POSITIVE) {
          *Reclaim_cnt_word = NIL;
          cause_interruptcall(DORECLAIM_index);
        } else if (*PENDINGINTERRUPT68k != NIL) {
          INTSTAT2 *intstate = ((INTSTAT2 *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
          /*unsigned char newints = (intstate->pendingmask) & ~(intstate->handledmask);
          if (newints) */
          {
            intstate->handledmask |= intstate->pendingmask;
            *PENDINGINTERRUPT68k = NIL;
            cause_interruptcall(INTERRUPTFRAME_index);
          }
        } else if (ETHEREventCount > 0) {
          INTSTAT *intstate = ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
          if (!(intstate->ETHERInterrupt) && !(((INTSTAT2 *)intstate)->handledmask & 0x40)) {
            intstate->ETHERInterrupt = 1;
            ((INTSTAT2 *)intstate)->handledmask |= ((INTSTAT2 *)intstate)->pendingmask;
            cause_interruptcall(INTERRUPTFRAME_index);
            ETHEREventCount--;
          } else
            *PENDINGINTERRUPT68k = ATOM_T;
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
}

void do_brk(void) {}
