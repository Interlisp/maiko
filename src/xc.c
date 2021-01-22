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

#include "tos1defs.h"
#include "tosret.h"
#include "tosfns.h"
#include "inlineC.h"

#include "xcdefs.h"
#include "arith2defs.h"
#include "arith3defs.h"
#include "arith4defs.h"
#include "arraydefs.h"
#include "array2defs.h"
#include "array3defs.h"
#include "array4defs.h"
#include "array5defs.h"
#include "array6defs.h"
#include "bitbltdefs.h"
#include "bltdefs.h"
#include "car-cdrdefs.h"
#include "commondefs.h"
#include "conspagedefs.h"
#include "drawdefs.h"
#include "eqfdefs.h"
#include "findkeydefs.h"
#include "fpdefs.h"
#include "fvardefs.h"
#include "gchtfinddefs.h"
#include "gcscandefs.h"
#include "gvar2defs.h"
#include "hardrtndefs.h"
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
#include "z2defs.h"

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

/* This used to be here for including optimized dispatch
 * for SPARC, but it has some other things in it, so we
 * are keeping it around for now until we sort that out. */
#ifdef SPARCDISP
#include "inlnSPARC.h"
#endif /* SPARCDISP */

#ifdef I386
#include "inln386i.h"
#endif

#include "fast_dsp.h"
#include "profile.h"

/* trick now is that pccache points one ahead... */
#define PCMAC (pccache - 1)
#define PCMACL pccache
#define CSTKPTR ((LispPTR *)cspcache)
#define PVAR ((LispPTR *)PVar)
#define IVAR ((LispPTR *)IVar)
#define BCE_CURRENTFX ((struct frameex2 *)((DLword *)PVAR - FRAMESIZE))

#define CSTKPTRL (cspcache)
#define PVARL PVar
#define IVARL IVar

#ifdef DOS
extern unsigned char inchar;
extern unsigned short kn;
#endif

#ifdef XWINDOW
extern int Event_Req; /* != 0 when it's time to check X events
                                                 on machines that don't get them reliably
                                                 (e.g. Suns running OpenWindows) */
#endif                /* XWINDOW */

#ifdef OPDISP
InstPtr optable[512];
#endif /* OPDISP */

#ifdef PCTRACE
/* For keeping a trace table (ring buffer) of 100 last PCs */
int pc_table[100],     /* The PC */
    op_table[100];     /* The opcode at that PC */
LispPTR fn_table[100]; /* The code block the PC is in (Lisp ptr) */
int pccounter = 0;     /* ring-buffer counter */
#endif                 /* PCTRACE */

int dbgflag = 0;

int extended_frame; /*indicates if soft stack overflow */

int n_mask_array[16] = {1,     3,     7,     0xf,   0x1f,   0x3f,   0x7f,   0xff,
                        0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff};

extern int TIMER_INTERVAL;

void dispatch(void) {
  register InstPtr pccache;

#if defined(OPDISP)
  InstPtr *table;
#endif

#if (DOS && OPDISP)
#else
  register LispPTR *cspcache;
  register LispPTR tscache;
#endif

  /* OP_FN_COMMON arguments */

  DefCell *fn_defcell;
  LispPTR fn_atom_index;
  int fn_opcode_size;
  int fn_num_args;
  int fn_apply;

  RET;
  CLR_IRQ;

#ifdef OPDISP
  table = optable;
#endif

#ifdef OPDISP
  goto setup_table;
#else
  goto nextopcode;
#endif /* OPDISP */

  /* INLINE OPCODE FAIL ENTRY POINTS, CALL EXTERNAL ROUTINES HERE */
  OPCODEFAIL;
  /* OPCODE FAIL ENTRY POINTS, CALL UFNS HERE */

  UFN_CALLS;

op_ufn : {
  UFN *entry68k;
  entry68k = (UFN *)GetUFNEntry(Get_BYTE_PCMAC0);
  fn_num_args = entry68k->arg_num;
  fn_opcode_size = entry68k->byte_num + 1;
  fn_atom_index = entry68k->atom_name;
  fn_defcell = (DefCell *)GetDEFCELL68k(fn_atom_index);
  fn_apply = 2 + entry68k->byte_num; /* code for UFN entry */
  goto op_fn_common;
};

  /* FUNCTION CALL TAIL ROUTINE */

  OP_FN_COMMON;

/* DISPATCH "LOOP" */

nextopcode:
#ifdef MYOPTRACE
  if ((struct fnhead *)Addr68k_from_LADDR(0x2ed600) == FuncObj) {
    quick_stack_check();
#endif /* MYOPTRACE */

    OPTPRINT(
             ("PC= 0x%x (fn+%d) op= 0%o TOS= 0x%x\n", (int)PCMAC, (int)PCMAC - (int)FuncObj, Get_BYTE_PCMAC0, TOPOFSTACK));
#ifdef MYOPTRACE
  }
#endif /* MYOPTRACE */

#ifdef PCTRACE
  /* Tracing PC/Function/Opcode in a ring buffer */
  pc_table[pccounter] = (int)PCMAC - (int)FuncObj;
  fn_table[pccounter] = (LispPTR)LADDR_from_68k(FuncObj);
  op_table[pccounter] = Get_BYTE_PCMAC0;
  if (99 == pccounter++) pccounter = 0;
#endif /* PCTRACE */

  /* quick_stack_check();*/ /* JDS 2/12/98 */

  switch (Get_BYTE_PCMAC0) {
    case 000:
    CASE000 : { goto op_ufn; } /* unused */
    case 001:
    CASE001:
      OPCAR;
    case 002:
    CASE002:
      OPCDR;
    case 003:
    CASE003:
      LISTP;
    case 004:
    CASE004:
      NTYPEX;
    case 005:
    CASE005:
      TYPEP(Get_BYTE_PCMAC1);
    case 056:
    CASE056:
    case 006:
    CASE006:
      DTEST(Get_AtomNo_PCMAC1);
    case 007:
    CASE007:
      UNWIND(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 010:
    CASE010:
      FN0;
    case 011:
    CASE011:
      FN1;
    case 012:
    CASE012:
      FN2;
    case 013:
    CASE013:
      FN3;
    case 014:
    CASE014:
      FN4;
    case 015:
    CASE015:
      FNX;
    case 016:
    CASE016:
      APPLY;

    case 017:
    CASE017:
      CHECKAPPLY;
    case 020:
    CASE020:
      RETURN;

    case 021:
    CASE021:
      /* UB: left shift of negative value -4 */
      BIND;
    case 022:
    CASE022:
      UNBIND;
    case 023:
    CASE023:
      DUNBIND;
    case 024:
    CASE024:
      RPLPTR(Get_BYTE_PCMAC1);
    case 025:
    CASE025:
      GCREF(Get_BYTE_PCMAC1);
    case 026:
    CASE026:
      ASSOC;
    case 027:
    CASE027:
      GVAR_(Get_AtomNo_PCMAC1);
    case 030:
    CASE030:
      RPLACA;
    case 031:
    CASE031:
      RPLACD;
    case 032:
    CASE032:
      CONS;
    case 033:
    CASE033:
      CLASSOC;
    case 034:
    CASE034:
      FMEMB;
    case 035:
    CASE035:
      CLFMEMB;
    case 036:
    CASE036:
      FINDKEY(Get_BYTE_PCMAC1);
    case 037:
    CASE037:
      CREATECELL;
    case 040:
    CASE040:
      BIN;
    case 041:
    CASE041 : { goto op_ufn; } /* BOUT */
    case 042:
    CASE042 : { goto op_ufn; } /* POPDISP - prolog only */
    case 043:
    CASE043:
      RESTLIST(Get_BYTE_PCMAC1);
    case 044:
    CASE044:
      MISCN(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 045:
    CASE045 : { goto op_ufn; } /* unused */
    case 046:
    CASE046:
      RPLCONS;
    case 047:
    CASE047:
      LISTGET;
    case 050:
    CASE050 : { goto op_ufn; } /* unused */
    case 051:
    CASE051 : { goto op_ufn; } /* unused */
    case 052:
    CASE052 : { goto op_ufn; } /* unused */
    case 053:
    CASE053 : { goto op_ufn; } /* unused */
    case 054:
    CASE054:
      EVAL;
    case 055:
    CASE055:
      ENVCALL;

    /*  case 056 : CASE056: @ 006 */
    case 057:
    CASE057:
      STKSCAN;
    case 060:
    CASE060 : { goto op_ufn; } /* BUSBLT - DLion only */
    case 061:
    CASE061 : { goto op_ufn; } /* MISC8 - no longer used */
    case 062:
    CASE062:
      UBFLOAT3(Get_BYTE_PCMAC1);
    case 063:
    CASE063:
      TYPEMASK(Get_BYTE_PCMAC1);
    case 064:
    CASE064 : { goto op_ufn; } /* rdprologptr */
    case 065:
    CASE065 : { goto op_ufn; } /* rdprologtag */
    case 066:
    CASE066 : { goto op_ufn; } /* writeptr&tag */
    case 067:
    CASE067 : { goto op_ufn; } /* writeptr&0tag */
    case 070:
    CASE070:
      MISC7(Get_BYTE_PCMAC1); /* misc7 (pseudocolor, fbitmapbit) */
    case 071:
    CASE071 : { goto op_ufn; } /* dovemisc - dove only */
    case 072:
    CASE072:
      EQLOP;
    case 073:
    CASE073:
      DRAWLINE;
    case 074:
    CASE074:
      STOREN(Get_BYTE_PCMAC1);
    case 075:
    CASE075:
      COPYN(Get_BYTE_PCMAC1);
    case 076:
    CASE076 : { goto op_ufn; } /* RAID */
    case 077:
    CASE077 : { goto op_ufn; } /* \RETURN */

    case 0100:
    CASE100:
      IVARMACRO(0);
    case 0101:
    CASE101:
      IVARMACRO(1);
    case 0102:
    CASE102:
      IVARMACRO(2);
    case 0103:
    CASE103:
      IVARMACRO(3);
    case 0104:
    CASE104:
      IVARMACRO(4);
    case 0105:
    CASE105:
      IVARMACRO(5);
    case 0106:
    CASE106:
      IVARMACRO(6);
    case 0107:
    CASE107:
      IVARX(Get_BYTE_PCMAC1);

    case 0110:
    CASE110:
      PVARMACRO(0);
    case 0111:
    CASE111:
      PVARMACRO(1);
    case 0112:
    CASE112:
      PVARMACRO(2);
    case 0113:
    CASE113:
      PVARMACRO(3);
    case 0114:
    CASE114:
      PVARMACRO(4);
    case 0115:
    CASE115:
      PVARMACRO(5);
    case 0116:
    CASE116:
      PVARMACRO(6);

    case 0117:
    CASE117:
      PVARX(Get_BYTE_PCMAC1);

    case 0120:
    CASE120:
      FVAR(0);
    case 0121:
    CASE121:
      FVAR(2);
    case 0122:
    CASE122:
      FVAR(4);
    case 0123:
    CASE123:
      FVAR(6);
    case 0124:
    CASE124:
      FVAR(8);
    case 0125:
    CASE125:
      FVAR(10);
    case 0126:
    CASE126:
      FVAR(12);
    case 0127:
    CASE127:
      FVARX(Get_BYTE_PCMAC1);

    case 0130:
    CASE130:
      PVARSETMACRO(0);
    case 0131:
    CASE131:
      PVARSETMACRO(1);
    case 0132:
    CASE132:
      PVARSETMACRO(2);
    case 0133:
    CASE133:
      PVARSETMACRO(3);
    case 0134:
    CASE134:
      PVARSETMACRO(4);
    case 0135:
    CASE135:
      PVARSETMACRO(5);
    case 0136:
    CASE136:
      PVARSETMACRO(6);

    case 0137:
    CASE137:
      PVARX_(Get_BYTE_PCMAC1);

    case 0140:
    CASE140:
      GVAR(Get_AtomNo_PCMAC1);
    case 0141:
    CASE141:
      ARG0;
    case 0142:
    CASE142:
      IVARX_(Get_BYTE_PCMAC1);
    case 0143:
    CASE143:
      FVARX_(Get_BYTE_PCMAC1);
    case 0144:
    CASE144:
      COPY;
    case 0145:
    CASE145:
      MYARGCOUNT;
    case 0146:
    CASE146:
      MYALINK;

    /******** Aconst	********/
    case 0147:
    CASE147 : {
      PUSH(Get_AtomNo_PCMAC1);
      nextop_atom;
    }
    case 0150:
    CASE150 : { PUSHATOM(NIL_PTR); }
    case 0151:
    CASE151 : { PUSHATOM(ATOM_T); }
    case 0152:
    CASE152 : { PUSHATOM(S_POSITIVE); } /* '0 */
    case 0153:
    CASE153 : { PUSHATOM(0xE0001); } /* '1 */

    /********* SIC		********/
    case 0154:
    CASE154 : {
      PUSH(S_POSITIVE | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SNIC		********/
    case 0155:
    CASE155 : {
      PUSH(S_NEGATIVE | 0xff00 | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SICX		********/
    case 0156:
    CASE156 : {
      PUSH(S_POSITIVE | Get_DLword_PCMAC1);
      nextop3;
    }

    /********* GCONST	********/
    case 0157:
    CASE157 : {
      PUSH(Get_Pointer_PCMAC1);
      nextop_ptr;
    }

    case 0160:
    CASE160 : { goto op_ufn; } /* unused */
    case 0161:
    CASE161 : { goto op_ufn; } /* readflags */
    case 0162:
    CASE162 : { goto op_ufn; } /* readrp */
    case 0163:
    CASE163 : { goto op_ufn; } /* writemap */
    case 0164:
    CASE164 : { goto op_ufn; } /* readprinterport */
    case 0165:
    CASE165 : { goto op_ufn; } /* writeprinterport */

    case 0166:
    CASE166:
      PILOTBITBLT;
    case 0167:
    CASE167:
      RCLK;
    case 0170:
    CASE170 : { goto op_ufn; } /* MISC1, dorado only */
    case 0171:
    CASE171 : { goto op_ufn; } /* MISC2, dorado only */
    case 0172:
    CASE172:
      RECLAIMCELL;
    case 0173:
    CASE173:
      GCSCAN1;
    case 0174:
    CASE174:
      GCSCAN2;
    case 0175:
    CASE175 : {
      EXT;
      OP_subrcall(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
      RET;
      nextop0;
    };
    case 0176:
    CASE176 : { CONTEXTSWITCH; }
    case 0177:
    CASE177 : { goto op_ufn; } /* RETCALL */

    /* JUMP */

    case 0200:
    CASE200 : { JUMPMACRO(2); }
    case 0201:
    CASE201 : { JUMPMACRO(3); }
    case 0202:
    CASE202 : { JUMPMACRO(4); }
    case 0203:
    CASE203 : { JUMPMACRO(5); }
    case 0204:
    CASE204 : { JUMPMACRO(6); }
    case 0205:
    CASE205 : { JUMPMACRO(7); }
    case 0206:
    CASE206 : { JUMPMACRO(8); }
    case 0207:
    CASE207 : { JUMPMACRO(9); }
    case 0210:
    CASE210 : { JUMPMACRO(10); }
    case 0211:
    CASE211 : { JUMPMACRO(11); }
    case 0212:
    CASE212 : { JUMPMACRO(12); }
    case 0213:
    CASE213 : { JUMPMACRO(13); }
    case 0214:
    CASE214 : { JUMPMACRO(14); }
    case 0215:
    CASE215 : { JUMPMACRO(15); }
    case 0216:
    CASE216 : { JUMPMACRO(16); }
    case 0217:
    CASE217 : { JUMPMACRO(17); }

    /* FJUMP */

    case 0220:
    CASE220 : { FJUMPMACRO(2); }
    case 0221:
    CASE221 : { FJUMPMACRO(3); }
    case 0222:
    CASE222 : { FJUMPMACRO(4); }
    case 0223:
    CASE223 : { FJUMPMACRO(5); }
    case 0224:
    CASE224 : { FJUMPMACRO(6); }
    case 0225:
    CASE225 : { FJUMPMACRO(7); }
    case 0226:
    CASE226 : { FJUMPMACRO(8); }
    case 0227:
    CASE227 : { FJUMPMACRO(9); }
    case 0230:
    CASE230 : { FJUMPMACRO(10); }
    case 0231:
    CASE231 : { FJUMPMACRO(11); }
    case 0232:
    CASE232 : { FJUMPMACRO(12); }
    case 0233:
    CASE233 : { FJUMPMACRO(13); }
    case 0234:
    CASE234 : { FJUMPMACRO(14); }
    case 0235:
    CASE235 : { FJUMPMACRO(15); }
    case 0236:
    CASE236 : { FJUMPMACRO(16); }
    case 0237:
    CASE237 : { FJUMPMACRO(17); }

    /* TJUMP */

    case 0240:
    CASE240 : { TJUMPMACRO(2); }
    case 0241:
    CASE241 : { TJUMPMACRO(3); }
    case 0242:
    CASE242 : { TJUMPMACRO(4); }
    case 0243:
    CASE243 : { TJUMPMACRO(5); }
    case 0244:
    CASE244 : { TJUMPMACRO(6); }
    case 0245:
    CASE245 : { TJUMPMACRO(7); }
    case 0246:
    CASE246 : { TJUMPMACRO(8); }
    case 0247:
    CASE247 : { TJUMPMACRO(9); }
    case 0250:
    CASE250 : { TJUMPMACRO(10); }
    case 0251:
    CASE251 : { TJUMPMACRO(11); }
    case 0252:
    CASE252 : { TJUMPMACRO(12); }
    case 0253:
    CASE253 : { TJUMPMACRO(13); }
    case 0254:
    CASE254 : { TJUMPMACRO(14); }
    case 0255:
    CASE255 : { TJUMPMACRO(15); }
    case 0256:
    CASE256 : { TJUMPMACRO(16); }
    case 0257:
    CASE257 : { TJUMPMACRO(17); }

    /******* JUMPX ********/
    case 0260:
    CASE260 : {
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* JUMPXX ********/
    case 0261:
    CASE261 : {
      CHECK_INTERRUPT;
      /* UB: left shift of negative value -1 */
      PCMACL += (Get_SBYTE_PCMAC1 << 8) | Get_BYTE_PCMAC2;
      nextop0;
    }

    /******* FJumpx *******/
    case 0262:
    CASE262 : {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* TJumpx *******/

    case 0263:
    CASE263 : {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NFJumpx *******/

    case 0264:
    CASE264 : {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NTJumpx *******/

    case 0265:
    CASE265 : {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    case 0266:
    CASE266:
      AREF1;
    case 0267:
    CASE267:
      ASET1;

    case 0270:
    CASE270:
      PVARSETPOPMACRO(0);
    case 0271:
    CASE271:
      PVARSETPOPMACRO(1);
    case 0272:
    CASE272:
      PVARSETPOPMACRO(2);
    case 0273:
    CASE273:
      PVARSETPOPMACRO(3);
    case 0274:
    CASE274:
      PVARSETPOPMACRO(4);
    case 0275:
    CASE275:
      PVARSETPOPMACRO(5);
    case 0276:
    CASE276:
      PVARSETPOPMACRO(6);

    case 0277:
    CASE277 : {
      POP;
      nextop1;
    }

    case 0300:
    CASE300:
      POPN(Get_BYTE_PCMAC1);
    case 0301:
    CASE301:
      ATOMCELL_N(Get_BYTE_PCMAC1);
    case 0302:
    CASE302:
      GETBASEBYTE;
    case 0303:
    CASE303:
      INSTANCEP(Get_AtomNo_PCMAC1);
    case 0304:
    CASE304:
      BLT;
    case 0305:
    CASE305 : { goto op_ufn; } /* MISC10 */
    case 0306:
    CASE306 : { goto op_ufn; } /* P-MISC2 ??? */
    case 0307:
    CASE307:
      PUTBASEBYTE;
    case 0310:
    CASE310:
      GETBASE_N(Get_BYTE_PCMAC1);
    case 0311:
    CASE311:
      GETBASEPTR_N(Get_BYTE_PCMAC1);
    case 0312:
    CASE312:
      GETBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 0313:
    CASE313 : { goto op_ufn; } /* unused */
    case 0314:
    CASE314:
      CLEQUAL;
    case 0315:
    CASE315:
      PUTBASE_N(Get_BYTE_PCMAC1);
    case 0316:
    CASE316:
      PUTBASEPTR_N(Get_BYTE_PCMAC1);
    case 0317:
    CASE317:
      PUTBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);

    case 0320:
    CASE320:
      N_OP_ADDBASE;
    case 0321:
    CASE321:
      N_OP_VAG2;
    case 0322:
    CASE322:
      N_OP_HILOC;
    case 0323:
    CASE323:
      N_OP_LOLOC;
    case 0324:
    CASE324:
      PLUS2; /* PLUS */
    case 0325:
    CASE325:
      DIFFERENCE; /* DIFFERENCE */
    case 0326:
    CASE326:
      TIMES2; /* TIMES2 */
    case 0327:
    CASE327:
      QUOTIENT                          /* QUOTIENT */
    case 0330:
    CASE330:
      IPLUS2; /* IPLUS2 only while PLUS has no float */
    case 0331:
    CASE331:
      IDIFFERENCE; /* IDIFFERENCE only while no float */
    case 0332:
    CASE332:
      ITIMES2; /* ITIMES2 only while no float */
    case 0333:
    CASE333:
      IQUOTIENT; /* IQUOTIENT */
    case 0334:
    CASE334:
      IREMAINDER;
    case 0335:
    CASE335:
      IPLUS_N(Get_BYTE_PCMAC1);
    case 0336:
    CASE336:
      IDIFFERENCE_N(Get_BYTE_PCMAC1);
    case 0337:
    CASE337 : { goto op_ufn; } /* BASE-< */
    case 0340:
    CASE340:
      LLSH1;
    case 0341:
    CASE341:
      LLSH8;
    case 0342:
    CASE342:
      LRSH1;
    case 0343:
    CASE343:
      LRSH8;
    case 0344:
    CASE344:
      LOGOR;
    case 0345:
    CASE345:
      LOGAND;
    case 0346:
    CASE346:
      LOGXOR;
    case 0347:
    CASE347:
      LSH;
    case 0350:
    CASE350:
      FPLUS2;
    case 0351:
    CASE351:
      FDIFFERENCE;
    case 0352:
    CASE352:
      FTIMES2;
    case 0353:
    CASE353:
      FQUOTIENT;
    case 0354:
    CASE354:
      UBFLOAT2(Get_BYTE_PCMAC1);
    case 0355:
    CASE355:
      UBFLOAT1(Get_BYTE_PCMAC1);
    case 0356:
    CASE356:
      AREF2;
    case 0357:
    CASE357:
      ASET2;

    case 0360:
    CASE360 : {
      if (TOPOFSTACK == POP_TOS_1)
        TOPOFSTACK = ATOM_T;
      else
        TOPOFSTACK = NIL_PTR;
      nextop1;
    }

    case 0361:
    CASE361:
      IGREATERP; /* IGREATERP if no float */
    case 0362:
    CASE362:
      FGREATERP;
    case 0363:
    CASE363:
      GREATERP;
    case 0364:
    CASE364:
      ILEQUAL;
    case 0365:
    CASE365:
      MAKENUMBER;
    case 0366:
    CASE366:
      BOXIPLUS;
    case 0367:
    CASE367:
      BOXIDIFFERENCE;
    case 0370:
    CASE370 : { goto op_ufn; } /* FLOATBLT */
    case 0371:
    CASE371 : { goto op_ufn; } /* FFTSTEP */
    case 0372:
    CASE372:
      MISC3(Get_BYTE_PCMAC1);
    case 0373:
    CASE373:
      MISC4(Get_BYTE_PCMAC1);
    case 0374:
    CASE374 : { goto op_ufn; } /* upctrace */
    case 0375:
    CASE375:
      SWAP;
    case 0376:
    CASE376:
      NOP;
    case 0377:
    CASE377:
      CLARITHEQUAL;
#ifdef OPDISP
#ifdef ISC
    case 0400:
      goto setup_table; /* to defeat optimizer, so optable exists */
#elif (DOS && OPDISP)
    case 0400: goto setup_table;
#endif /* ISC */

#endif /* OPDISP */

#ifdef I386
    /* to defeat the damn optimizer, make it look like */
    /* we might branch to the error labels. */
    case 0400: goto plus_err;
    case 0401: goto iplus_err;
    case 0402: goto iplusn_err;
    case 0403: goto idiff_err;
    case 0404: goto diff_err;
    case 0405: goto idiffn_err;
    case 0406: goto greaterp_err;
    case 0411: goto igreaterp_err;
    case 0407: goto llsh8_err;
    case 0410: goto lrsh1_err;
    case 0414: goto lrsh8_err;
    case 0417: goto llsh1_err;
    case 0413: goto logor_err;
    case 0412: goto logand_err;
    case 0416: goto logxor_err;
    case 0415: goto addbase_err;
#endif

    default: error("should not default");

  } /* switch */

native_check:
  goto nextopcode;

/************************************************************************/
/*		TIMER INTERRUPT CHECK ROUTINE				*/
/************************************************************************/
check_interrupt:
#if (defined(SUN3_OS3_OR_OS4_IL) || defined(I386) || defined(ISC))
  asm_label_check_interrupt();
#endif

  if ((UNSIGNED)CSTKPTR > (UNSIGNED)EndSTKP) {
    EXT;
    error("Unrecoverable Stack Overflow");
    RET;
  }

  /* Check for an IRQ request */

  {
    register int need_irq;
    static int period_cnt = 60;
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

    /* Check for an IRQ request */

    if ((Irq_Stk_End <= 0) || (Irq_Stk_Check <= 0) || need_irq) {
      if (StkOffset_from_68K(CSTKPTR) > InterfacePage->stackbase) {
        /* Interrupts not Disabled */
        /* XXX: what on earth is this code trying to accomplish by calling
           getsignaldata
        */
#if !defined(KBINT) || defined(OS4)
        getsignaldata(0);
#endif
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
          (currentdsp->mouse_invisible)(currentdsp, IOPage68K);

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

          IOPage68K->dlmousex = IOPage68K->dlcursorx = currentmouse->Cursor.New.x;
          IOPage68K->dlmousey = IOPage68K->dlcursory = currentmouse->Cursor.New.y;

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
          INTSTAT2 *intstate = ((INTSTAT2 *)Addr68k_from_LADDR(*INTERRUPTSTATE_word));
          /*unsigned char newints = (intstate->pendingmask) & ~(intstate->handledmask);
          if (newints) */
          {
            intstate->handledmask |= intstate->pendingmask;
            *PENDINGINTERRUPT68k = NIL;
            cause_interruptcall(INTERRUPTFRAME_index);
          }
        } else if (ETHEREventCount > 0) {
          INTSTAT *intstate = ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word));
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

/************************************************************************/
/*									*/
/*	Set up the dispatch table for use when we do assembler		*/
/*	optimization of the dispatch-jump sequence.			*/
/*									*/
/*									*/
/************************************************************************/

#ifdef OPDISP
setup_table:
  {
    int i;
    for (i = 0; i < 256; i++) { table[i] = &&op_ufn; };
  }
  {
    int i;
    for (i = 256; i < 512; i++) { table[i] = &&native_check; };
  }
  table[001] = &&case001;
  table[002] = &&case002;
  table[003] = &&case003;
  table[004] = &&case004;
  table[005] = &&case005;
  table[006] = &&case006;
  table[007] = &&case007;
  table[010] = &&case010;
  table[011] = &&case011;
  table[012] = &&case012;
  table[013] = &&case013;
  table[014] = &&case014;
  table[015] = &&case015;
  table[016] = &&case016;
  table[017] = &&case017;
  table[020] = &&case020;
  table[021] = &&case021;
  table[022] = &&case022;
  table[023] = &&case023;
  table[024] = &&case024;
  table[025] = &&case025;
  table[026] = &&case026;
  table[027] = &&case027;
  table[030] = &&case030;
  table[031] = &&case031;
  table[032] = &&case032;
  table[033] = &&case033;
  table[034] = &&case034;
  table[035] = &&case035;
  table[036] = &&case036;
  table[037] = &&case037;
  table[040] = &&case040;
  table[041] = &&case041;
  table[042] = &&case042;
  table[043] = &&case043;
  table[044] = &&case044;
  table[045] = &&case045;
  table[046] = &&case046;
  table[047] = &&case047;

  table[054] = &&case054;
  table[055] = &&case055;
  table[056] = &&case056;
  table[057] = &&case057;

  table[062] = &&case062;
  table[063] = &&case063;

  table[070] = &&case070;

  table[072] = &&case072;
  table[073] = &&case073;
  table[074] = &&case074;
  table[075] = &&case075;

  table[0100] = &&case100;
  table[0101] = &&case101;
  table[0102] = &&case102;
  table[0103] = &&case103;
  table[0104] = &&case104;
  table[0105] = &&case105;
  table[0106] = &&case106;
  table[0107] = &&case107;
  table[0110] = &&case110;
  table[0111] = &&case111;
  table[0112] = &&case112;
  table[0113] = &&case113;
  table[0114] = &&case114;
  table[0115] = &&case115;
  table[0116] = &&case116;
  table[0117] = &&case117;
  table[0120] = &&case120;
  table[0121] = &&case121;
  table[0122] = &&case122;
  table[0123] = &&case123;
  table[0124] = &&case124;
  table[0125] = &&case125;
  table[0126] = &&case126;
  table[0127] = &&case127;
  table[0130] = &&case130;
  table[0131] = &&case131;
  table[0132] = &&case132;
  table[0133] = &&case133;
  table[0134] = &&case134;
  table[0135] = &&case135;
  table[0136] = &&case136;
  table[0137] = &&case137;
  table[0140] = &&case140;
  table[0141] = &&case141;
  table[0142] = &&case142;
  table[0143] = &&case143;
  table[0144] = &&case144;
  table[0145] = &&case145;
  table[0146] = &&case146;
  table[0147] = &&case147;
  table[0150] = &&case150;
  table[0151] = &&case151;
  table[0152] = &&case152;
  table[0153] = &&case153;
  table[0154] = &&case154;
  table[0155] = &&case155;
  table[0156] = &&case156;
  table[0157] = &&case157;
  table[0160] = &&case160;
  table[0161] = &&case161;
  table[0162] = &&case162;
  table[0163] = &&case163;
  table[0164] = &&case164;
  table[0165] = &&case165;
  table[0166] = &&case166;
  table[0167] = &&case167;
  table[0170] = &&case170;
  table[0171] = &&case171;
  table[0172] = &&case172;
  table[0173] = &&case173;
  table[0174] = &&case174;
  table[0175] = &&case175;
  table[0176] = &&case176;
  table[0177] = &&case177;
  table[0200] = &&case200;
  table[0201] = &&case201;
  table[0202] = &&case202;
  table[0203] = &&case203;
  table[0204] = &&case204;
  table[0205] = &&case205;
  table[0206] = &&case206;
  table[0207] = &&case207;
  table[0210] = &&case210;
  table[0211] = &&case211;
  table[0212] = &&case212;
  table[0213] = &&case213;
  table[0214] = &&case214;
  table[0215] = &&case215;
  table[0216] = &&case216;
  table[0217] = &&case217;
  table[0220] = &&case220;
  table[0221] = &&case221;
  table[0222] = &&case222;
  table[0223] = &&case223;
  table[0224] = &&case224;
  table[0225] = &&case225;
  table[0226] = &&case226;
  table[0227] = &&case227;
  table[0230] = &&case230;
  table[0231] = &&case231;
  table[0232] = &&case232;
  table[0233] = &&case233;
  table[0234] = &&case234;
  table[0235] = &&case235;
  table[0236] = &&case236;
  table[0237] = &&case237;
  table[0240] = &&case240;
  table[0241] = &&case241;
  table[0242] = &&case242;
  table[0243] = &&case243;
  table[0244] = &&case244;
  table[0245] = &&case245;
  table[0246] = &&case246;
  table[0247] = &&case247;
  table[0250] = &&case250;
  table[0251] = &&case251;
  table[0252] = &&case252;
  table[0253] = &&case253;
  table[0254] = &&case254;
  table[0255] = &&case255;
  table[0256] = &&case256;
  table[0257] = &&case257;
  table[0260] = &&case260;
  table[0261] = &&case261;
  table[0262] = &&case262;
  table[0263] = &&case263;
  table[0264] = &&case264;
  table[0265] = &&case265;
  table[0266] = &&case266;
  table[0267] = &&case267;
  table[0270] = &&case270;
  table[0271] = &&case271;
  table[0272] = &&case272;
  table[0273] = &&case273;
  table[0274] = &&case274;
  table[0275] = &&case275;
  table[0276] = &&case276;
  table[0277] = &&case277;
  table[0300] = &&case300;
  table[0301] = &&case301;
  table[0302] = &&case302;
  table[0303] = &&case303;
  table[0304] = &&case304;
  table[0305] = &&case305;
  table[0306] = &&case306;
  table[0307] = &&case307;
  table[0310] = &&case310;
  table[0311] = &&case311;
  table[0312] = &&case312;
  table[0313] = &&case313;
  table[0314] = &&case314;
  table[0315] = &&case315;
  table[0316] = &&case316;
  table[0317] = &&case317;
  table[0320] = &&case320;
  table[0321] = &&case321;
  table[0322] = &&case322;
  table[0323] = &&case323;
  table[0324] = &&case324;
  table[0325] = &&case325;
  table[0326] = &&case326;
  table[0327] = &&case327;
  table[0330] = &&case330;
  table[0331] = &&case331;
  table[0332] = &&case332;
  table[0333] = &&case333;
  table[0334] = &&case334;
  table[0335] = &&case335;
  table[0336] = &&case336;
  table[0337] = &&case337;
  table[0340] = &&case340;
  table[0341] = &&case341;
  table[0342] = &&case342;
  table[0343] = &&case343;
  table[0344] = &&case344;
  table[0345] = &&case345;
  table[0346] = &&case346;
  table[0347] = &&case347;
  table[0350] = &&case350;
  table[0351] = &&case351;
  table[0352] = &&case352;
  table[0353] = &&case353;
  table[0354] = &&case354;
  table[0355] = &&case355;
  table[0356] = &&case356;
  table[0357] = &&case357;
  table[0360] = &&case360;
  table[0361] = &&case361;
  table[0362] = &&case362;
  table[0363] = &&case363;
  table[0364] = &&case364;
  table[0365] = &&case365;
  table[0366] = &&case366;
  table[0367] = &&case367;
  table[0370] = &&case370;
  table[0371] = &&case371;
  table[0372] = &&case372;
  table[0373] = &&case373;
  table[0374] = &&case374;
  table[0375] = &&case375;
  table[0376] = &&case376;
  table[0377] = &&case377;
  goto nextopcode;
#endif /* OPDISP */
}

void do_brk(void) {}
