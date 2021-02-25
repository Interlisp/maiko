/* $Id: testtool.c,v 1.4 2001/12/24 01:09:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***************************************************************/
/*
        file name :	 testtool.c

        For Debugging Aids

        Including :
                dump_check_atoms()
                print_atomname(index)
                dump_dtd()
                check_type_68k(type,ptr)
                type_num(LISPPTR)
                dump_conspage(base , linking )
                trace_listpDTD()
                a68k( lispptr)
                laddr(addr68k)
                dump_fnobj(index)
                dump_fnbody(lisp-codeaddr)
                doko()
                dumpl(laddr)
                ptintPC()
                all_stack_dump(start,end)

                date :   14 May 1987   takeshi
                         15 May 1987 take
                         1  June 1987 take
                         21  June 1987 NMitani
                          9  Sep. 1987 take


*/
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "emlglob.h"
#include "cell.h"
#include "ifpage.h"
#include "debug.h"
#include "dbprint.h"
#include "tosfns.h"

#include "testtooldefs.h"
#include "dbgtooldefs.h"
#include "gcarraydefs.h"
#include "kprintdefs.h"
#include "mkatomdefs.h"

#define URMAXFXNUM 100
#define URSCAN_ALINK 0
#define URSCAN_CLINK 1
extern int URaid_scanlink;
extern int URaid_currentFX;
extern FX *URaid_FXarray[];
extern int URaid_ArrMAXIndex;

/************************************************************************/
/*									*/
/*			P R I N T _ A T O M N A M E			*/
/*									*/
/*	Given the Atom # for an atom, print the atom's name.		*/
/*									*/
/************************************************************************/

void print_atomname(LispPTR index)
/* atomindex */
{
  char *pname;
  DLword length;
  PNCell *pnptr;

  pnptr = (PNCell *)GetPnameCell(index);
  print_package_name(pnptr->pkg_index);
  pname = (char *)Addr68k_from_LADDR(pnptr->pnamebase);

  length = (DLword)GETBYTE(pname++);

  while (length > 0) {
    putchar(GETBYTE(pname++));
    length--;
  }

} /* end print_atomname */

/************************************************************************/
/*									*/
/*		F I N D _ P A C K A G E _ F R O M _ N A M E		*/
/*									*/
/************************************************************************/

#define PACKAGES_LIMIT 255
/** GET PACKAGE INDEX from PACKAGE FULL NAME */
int find_package_from_name(const char *packname, int len) {
  int index;
  PACKAGE *package;
  NEWSTRINGP *namestring;
  DLword len2;
  char *pname;

  for (index = 1; index <= PACKAGES_LIMIT; index++) {
    package = (PACKAGE *)Addr68k_from_LADDR(aref1(*Package_from_Index_word, index));
    namestring = (NEWSTRINGP *)Addr68k_from_LADDR(package->NAME);
    pname = (char *)Addr68k_from_LADDR(namestring->base);
    if (namestring->offset != 0) { pname += namestring->offset; }

    len2 = (DLword)(namestring->fillpointer);
    if (len == len2) {
      if (compare_chars(pname, packname, len) == T) { return (index); }
    }
  } /* for end */
  return (-1);
}

/************************************************************************/
/*									*/
/*		    P R I N T _ P A C K A G E _ N A M E			*/
/*									*/
/************************************************************************/

void print_package_name(int index) {
  PACKAGE *package;
  NEWSTRINGP *namestring;
  DLword len;
  char *pname;

  if (index == 0) {
    printf("#:");
    return;
  }
  package = (PACKAGE *)Addr68k_from_LADDR(aref1(*Package_from_Index_word, index));
  namestring = (NEWSTRINGP *)Addr68k_from_LADDR(package->NAME);
  pname = (char *)Addr68k_from_LADDR(namestring->base);
  if (namestring->offset != 0) {
    pname += namestring->offset;
    printf("OFFSET:\n");
  }
  len = (DLword)(namestring->fillpointer);

  if (compare_chars(pname, "INTERLISP", len) == T) {
    printf("IL:");
    return;
  } else if (compare_chars(pname, "LISP", len) == T) {
    printf("CL:");
    return;
  } else if (compare_chars(pname, "XEROX-COMMON-LISP", len) == T) {
    printf("XCL:");
    return;
  } else if (compare_chars(pname, "SYSTEM", len) == T) {
    printf("SI:");
    return;
  } else if (compare_chars(pname, "KEYWORD", len) == T) {
    printf(":");
    return;
  } else if (compare_chars(pname, "COMPILER", len) == T) {
    printf("XCLC:");
    return;
  } else {
    while (len > 0) {
      putchar(GETBYTE(pname++));
      len--;
    }
    putchar(':');
    return;
  }

} /*print_package_name */

/************************************************************************/
/*									*/
/*				d u m p _ d t d				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void dump_dtd(void) {
  extern DLword *DTDspace;
  struct dtd *dtdp;
  DLword cnt;

  dtdp = (struct dtd *)DTDspace;
  dtdp++;

  for (cnt = 0; cnt < INIT_TYPENUM; cnt++) {
    printf("DTD[ %d ] for ", cnt + 1);
#ifdef BIGVM
    print_atomname(dtdp->dtd_name);
#else
    print_atomname(dtdp->dtd_namelo + (dtdp->dtd_namehi << 16));
#endif /* BIGVM */
    putchar('\n');

#ifdef BIGVM
    printf("    dtd_name = %d\n", dtdp->dtd_name);
#else
    printf("    dtd_name = %d\n", dtdp->dtd_namelo + (dtdp->dtd_namehi << 16));
#endif /* BIGVM */
    printf("    dtd_size = %d\n", dtdp->dtd_size);
    printf("    dtd_free = %d\n", dtdp->dtd_free);
    printf("    dtd_obsolate = %d\n", dtdp->dtd_obsolate);
    printf("    dtd_finalizable = %d\n", dtdp->dtd_finalizable);
    printf("    dtd_lockedp = %d\n", dtdp->dtd_lockedp);
    printf("    dtd_hunkp = %d\n", dtdp->dtd_hunkp);
    printf("    dtd_gctype = %d\n", dtdp->dtd_gctype);
    printf("    dtd_descrs = %d\n", dtdp->dtd_descrs);
    printf("    dtd_typespecs = %d\n", dtdp->dtd_typespecs);
    printf("    dtd_ptrs = %d\n", dtdp->dtd_ptrs);
    printf("    dtd_oldcnt = %d\n", dtdp->dtd_oldcnt);
    printf("    dtd_cnt0 = %d\n", dtdp->dtd_cnt0);
    printf("    dtd_nextpage = %d\n", dtdp->dtd_nextpage);
    printf("    dtd_typeentry = 0x%x\n", dtdp->dtd_typeentry);
    printf("    dtd_supertype = %d\n", dtdp->dtd_supertype);

    dtdp++;
  }

} /* end dump dtd */

/************************************************************************/
/*									*/
/*			c h e c k _ t y p e _ 6 8 k			*/
/*									*/
/*	Check that the lisp pointer ptr is of type type, then		*/
/*	print a message showing the type number.			*/
/*									*/
/************************************************************************/

void check_type_68k(int type, LispPTR *ptr) {
  if (type != (GetTypeNumber(LADDR_from_68k(ptr)))) {
    printf("Mismatching occur !!! LispAddr 0x%x  type %d\n", LADDR_from_68k(ptr), type);
    exit(-1);
  }

  printf("LispPTR 0x%x is the datatype %d\n", LADDR_from_68k(ptr),
         GetTypeNumber(LADDR_from_68k(ptr)));
}

/************************************************************************/
/*									*/
/*				t y p e _ n u m				*/
/*									*/
/*	Given a lisp pointer, return its type number.			*/
/*									*/
/************************************************************************/

int type_num(LispPTR lispptr) {
  int type;
  type = GetTypeNumber(lispptr);

  printf("LispPTR 0x%x is datatype %dth\n", lispptr, type);
  return (type);
}

/************************************************************************/
/*									*/
/*			d u m p _ c o n s p a g e			*/
/*									*/
/*	Print information about a CONS page, and the cells in it.	*/
/*									*/
/************************************************************************/

void dump_conspage(struct conspage *base, int linking)
/* target conspage address */
/* look for chaining conspage ? T/NIL */
{
  ConsCell *cell;
  int i;

lp:
  printf(
      "conspage at 0x%x(lisp) has %d free cells , next available cell offset is %d ,and next page "
      "is 0x%x(lisp)\n",
      LADDR_from_68k(base), (0xff & base->count), (0xff & base->next_cell), base->next_page);

  for (i = 0, cell = (ConsCell *)base + 1; i < 127; i++, cell++) {
    printf(" LADDR : %d = Cell[ %d ]## cdr_code= %d ,car = %d\n", LADDR_from_68k(cell), i + 1,
           cell->cdr_code, cell->car_field);
  }

  if ((linking == T) && (base->next_page != NIL_PTR)) {
    base = (struct conspage *)Addr68k_from_LPAGE(base->next_page);
    goto lp;
  }

} /* end dump_conspage */

/*********************************/
/* trace the link in ListpDTD->dtd_nextpage */

void trace_listpDTD(void) {
  extern struct dtd *ListpDTD;
  printf("Dump conspages from ListpDTD chain\n");
  dump_conspage((struct conspage *)Addr68k_from_LPAGE(ListpDTD->dtd_nextpage), T);
}

/************************************************************************/
/*									*/
/*				a 6 8 k					*/
/*									*/
/*	Given a lisp pointer, print the corresponding native address.	*/
/*									*/
/************************************************************************/

void a68k(LispPTR lispptr) {
  DLword *val;
  val = Addr68k_from_LADDR(lispptr);
  printf("68k: %p (%"PRIuPTR")\n", (void *)val, (uintptr_t)val);
}

/************************************************************************/
/*									*/
/*			   l a d d r					*/
/*									*/
/*	Given a native address, print the corresponding lisp ptr.	*/
/*									*/
/************************************************************************/

void laddr(DLword *addr68k) {
  int val;
  val = LADDR_from_68k(addr68k);
  printf("LADDR : 0x%x (%d)\n", val, val);
}

/************************************************************************/
/*									*/
/*			d u m p _ f n b o d y				*/
/*									*/
/*	Given the (Lisp) address of a function header, dump the		*/
/*	function's definition.						*/
/*									*/
/************************************************************************/

void dump_fnbody(LispPTR fnblockaddr)
/* atom index */
{
  struct fnhead *fnobj;
  DLbyte *scratch;
  int i;

  fnobj = (struct fnhead *)Addr68k_from_LADDR(fnblockaddr);

  printf("***DUMP Func Obj << ");
  printf("start at 0x%x lisp address(%p 68k)\n", LADDR_from_68k(fnobj), fnobj);

  print(fnobj->framename);
  putchar('\n');

  printf("stkmin    : %d\n", fnobj->stkmin);
  printf("na        : %d\n", fnobj->na);
  printf("pv        : %d\n", fnobj->pv);
  printf("startpc   : %d\n", fnobj->startpc);
  printf("argtype   : %d\n", fnobj->argtype);
  printf("framename : %d\n", fnobj->framename);
  printf("ntsize    : %d\n", fnobj->ntsize);
  printf("nlocals   : %d\n", fnobj->nlocals);
  printf("fvaroffset: %d\n", fnobj->fvaroffset);

  scratch = (DLbyte *)fnobj;
  for (i = 20; i < (fnobj->startpc); i += 2) {
    int word;
    word = (int)(0xffff & (GETWORD((DLword *)(scratch + i))));
    printf(" 0x%x(%p 68k): 0%6o  0x%4x\n", LADDR_from_68k(scratch + i), scratch + i, word, word);
  }

  scratch = (DLbyte *)fnobj + (fnobj->startpc);
  for (i = 0; i < 2000; i++) {
    int len = print_opcode(fnobj->startpc + i, scratch, fnobj);
    if (len < 1) return;
    scratch += len;
    i += (len - 1);
  }

} /*dump_fnbody end */

/************************************************************************/
/*									*/
/*			d u m p _ f n o b j				*/
/*									*/
/*	Given an atom number, dump that atom's definition.		*/
/*									*/
/************************************************************************/

#define DUMPSIZE 40

void dump_fnobj(LispPTR index)
/* atom index */
{
  LispPTR *defcell68k;

  defcell68k = GetDEFCELL68k(index);
  dump_fnbody(*defcell68k);

} /*dump_fnobj end */

/************************************************************************/
/*									*/
/*			p r i n t _ o p c o d e				*/
/*									*/
/*	Print a single opcode's worth of a function body.		*/
/*									*/
/************************************************************************/

/* Opcode names, by opcode */
static const char *opcode_table[256] = {
                           "-X-",
                           "CAR",
                           "CDR",
                           "LISTP",
                           "NTYPX",
                           "TYPEP",
                           "DTEST",
                           "UNWIND",
                           "FN0",
                           "FN1",
                           "FN2",
                           "FN3",
                           "FN4",
                           "FNX",
                           "APPLYFN",
                           "CHECKAPPLY*",
                           "RETURN",
                           "BIND",
                           "UNBIND",
                           "DUNBIND",
                           "RPLPTR.N",
                           "GCREF",
                           "ASSOC",
                           "GVAR_",
                           "RPLACA",
                           "RPLACD",
                           "CONS",
                           "CMLASSOC",
                           "FMEMB",
                           "CMLMEMBER",
                           "FINDKEY",
                           "CREATECELL",
                           "BIN",
                           "BOUT",
                           "PROLOG",
                           "RESTLIST",
                           "MISCN",
                           "<>",
                           "RPLCONS",
                           "LISTGET",
                           "ELT",
                           "NTHCHC",
                           "SETA",
                           "RPLCHARCODE",
                           "EVAL",
                           "ENVCALL",
                           "TYPECHECK.N",
                           "STKSCAN",
                           "BUSBLT",
                           "MISC8",
                           "UBFLOAT3",
                           "TYPEMASK.N",
                           "PROLOG",
                           "PROLOG",
                           "PROLOG",
                           "PROLOG",
                           "PSEUDOCOLOR",
                           "<>",
                           "EQL",
                           "DRAWLINE",
                           "STORE.N",
                           "COPY.N",
                           "RAID",
                           "\\RETURN",
                           "IVAR0",
                           "IVAR1",
                           "IVAR2",
                           "IVAR3",
                           "IVAR4",
                           "IVAR5",
                           "IVAR6",
                           "IVARX",
                           "PVAR0",
                           "PVAR1",
                           "PVAR2",
                           "PVAR3",
                           "PVAR4",
                           "PVAR5",
                           "PVAR6",
                           "PVARX",
                           "FVAR0",
                           "FVAR1",
                           "FVAR2",
                           "FVAR3",
                           "FVAR4",
                           "FVAR5",
                           "FVAR6",
                           "FVARX",
                           "PVAR_0",
                           "PVAR_1",
                           "PVAR_2",
                           "PVAR_3",
                           "PVAR_4",
                           "PVAR_5",
                           "PVAR_6",
                           "PVAR_X",
                           "GVAR",
                           "ARG0",
                           "IVARX_",
                           "FVARX_",
                           "COPY",
                           "MYARGCOUNT",
                           "MYALINK",
                           "ACONST",
                           "'NIL",
                           "'T",
                           "'0",
                           "'1",
                           "SIC",
                           "SNIC",
                           "SICX",
                           "GCONST",
                           "ATOMNUMBER",
                           "READFLAGS",
                           "READRP",
                           "WRITEMAP",
                           "RPPORT",
                           "WPRTPORT",
                           "PILOTBBT",
                           "RCLK",
                           "MISC1",
                           "MISC2",
                           "RECCELL",
                           "GCSCAN1",
                           "GCSCAN2",
                           "SUBRCALL",
                           "CONTEXT",
                           "<>",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "JUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "FJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "TJUMP",
                           "JUMPX",
                           "JUMPXX",
                           "FJUMPX",
                           "TJUMPX",
                           "NFJUMPX",
                           "NTJUMPX",
                           "AREF1",
                           "ASET1",
                           "PVAR_0^",
                           "PVAR_1^",
                           "PVAR_2^",
                           "PVAR_3^",
                           "PVAR_4^",
                           "PVAR_5^",
                           "PVAR_6^",
                           "POP",
                           "POP.N",
                           "ATOMCELL.N",
                           "GETBASEBYTE",
                           "INSTANCEP",
                           "BLT",
                           "MISC10",
                           "<>",
                           "PUTBASEBYTE",
                           "GETBASE.N",
                           "GETBASEPTR.N",
                           "GETBITS.N.FD",
                           "<>",
                           "CMLEQUAL",
                           "PUTBASE.N",
                           "PUTBASEPTR.N",
                           "PUTBITS.N.FD",
                           "ADDBASE",
                           "VAG2",
                           "HILOC",
                           "LOLOC",
                           "PLUS2",
                           "DIFFERENCE",
                           "TIMES2",
                           "QUOTIENT",
                           "IPLUS2",
                           "IDIFFERENCE",
                           "ITIMES2",
                           "IQUOTIENT",
                           "IREMAINDER",
                           "IPLUS.N",
                           "IDIFFERENCE.N",
                           "<>",
                           "LLSH1",
                           "LLSH8",
                           "LRSH1",
                           "LRSH8",
                           "LOGOR2",
                           "LOGAND2",
                           "LOGXOR2",
                           "LSH",
                           "FPLUS2",
                           "FIDFFERENCE",
                           "FTIMES2",
                           "FQUOTIENT",
                           "UBFLOAT2",
                           "UBFLOAT1",
                           "AREF2",
                           "ASET2",
                           "EQ",
                           "IGREATERP",
                           "FGREATERP",
                           "GREATERP",
                           "EQUAL",
                           "MAKENUMBER",
                           "BOXIPLUS",
                           "BOSIDIFFERENCE",
                           "FLOATBLT",
                           "FFTSTEP",
                           "MISC3",
                           "MISC4",
                           "UPCTRACE",
                           "SWAP",
                           "NOP",
                           "CL="

};

int print_opcode(int pc, DLbyte *addr, struct fnhead *fnobj) {
  /* Print the opcode at addr, including args, and return length */
  /* if this opcode is the last, return -1			   */
  int op = (int)(0xFF & GETBYTE(addr));
  int i;
  extern unsigned int oplength[256];
  int len = oplength[op] + 1;

  printf(" 0%o (0x%x)	", pc, pc);
  for (i = 0; i < len; i++) printf("%o ", 0xFF & GETBYTE(addr + i));
  printf("	%s", opcode_table[op]);

  switch (op) {
    case 0:
      putchar('\n'); /* End of function */
      return (-1);
      break;
    case 015:
      printf("(%d)", (unsigned char)GETBYTE(addr + 1));
      addr += 1; /* FNX uses an extra byte */
                 /* Fall thru to the name print */
    case 006:    /* DTEST */
    case 010:    /* FN0-4 */
    case 011:
    case 012:
    case 013:
    case 014:
    case 027:  /* GVAR_ */
    case 0140: /* GVAR */
    case 0303: /* INSTANCEP */
    case 056:  /* TYPENAMEP */
    case 0147: putchar(' ');
#ifdef BIGVM
      print_atomname(((unsigned char)GETBYTE(addr + 1) << 24) +
                     ((unsigned char)GETBYTE(addr + 2) << 16) +
                     ((unsigned char)GETBYTE(addr + 3) << 8) + (unsigned char)GETBYTE(addr + 4));
#elif defined(BIGATOMS)
      print_atomname(((unsigned char)GETBYTE(addr + 1) << 16) +
                     ((unsigned char)GETBYTE(addr + 2) << 8) + (unsigned char)GETBYTE(addr + 3));
#else
      print_atomname(((unsigned char)GETBYTE(addr + 1) << 8) + (unsigned char)GETBYTE(addr + 2));
#endif /* BIGATOMS */

      putchar('\n');
      break;
    case 0200: /* Jump opcodes */
    case 0201:
    case 0202:
    case 0203:
    case 0204:
    case 0205:
    case 0206:
    case 0207:
    case 0210:
    case 0211:
    case 0212:
    case 0213:
    case 0214:
    case 0215:
    case 0216:
    case 0217: printf(" 0%o (0x%x)\n", pc + 2 + op - 0200, pc + 2 + op - 0200); break;
    case 0220: /* FJUMP opcodes */
    case 0221:
    case 0222:
    case 0223:
    case 0224:
    case 0225:
    case 0226:
    case 0227:
    case 0230:
    case 0231:
    case 0232:
    case 0233:
    case 0234:
    case 0235:
    case 0236:
    case 0237: printf(" 0%o (0x%x)\n", pc + 2 + op - 0220, pc + 2 + op - 0220); break;
    case 0240: /* TJUMP opcodes */
    case 0241:
    case 0242:
    case 0243:
    case 0244:
    case 0245:
    case 0246:
    case 0247:
    case 0250:
    case 0251:
    case 0252:
    case 0253:
    case 0254:
    case 0255:
    case 0256:
    case 0257: printf(" 0%o (0x%x)\n", pc + 2 + op - 0240, pc + 2 + op - 0240); break;
    case 0260: /* JUMPX */
    case 0262: /* FJUMPX */
    case 0263: /* TJUMPX */
    case 0264: /* NFJUMPX */
    case 0265: /* NTJUMPX */
      printf(" 0%o (0x%x)\n", pc + (int8_t)GETBYTE(addr + 1), pc + (int8_t)GETBYTE(addr + 1));
      break;
    case 0261: /* JUMPXX */
      printf(" 0%o (0x%x)\n",
             pc + ((int8_t)GETBYTE(addr + 1) << 8) + (uint8_t)GETBYTE(addr + 2),
             pc + ((int8_t)GETBYTE(addr + 1) << 8) + (uint8_t)GETBYTE(addr + 2));
      break;
    case 0120: /* FVAR opcodes */
    case 0121:
    case 0122:
    case 0123:
    case 0124:
    case 0125:
    case 0126:
      putchar(' ');
      print_atomname(get_fn_fvar_name(fnobj, op - 0120));
      putchar('\n');
      break;
    default: putchar('\n');
  }
  fflush(stdout); /* Make sure each line is really printed as we go. */
  return (len);
}

/************************************************************************/
/*									*/
/*					d o k o				*/
/*									*/
/*	For URAID:  Display the current function name & PC.		*/
/*									*/
/************************************************************************/

void doko(void) {
  printf(" At ");
  print_atomname(FuncObj->framename);
  putchar('\n');
  printf("   PC cnt = 0%"PRIoPTR"\n", ((UNSIGNED)(PC) - (UNSIGNED)FuncObj));
}

/**** dump specified area (in 32 bit width) ***/
void dumpl(LispPTR laddr) {
  int i;
  LispPTR *ptr;

  ptr = (LispPTR *)Addr68k_from_LADDR(laddr);

  for (i = 0; i < 40; i++, ptr++) printf("LADDR 0x%x : %d\n", LADDR_from_68k(ptr), *ptr);
}

/**** dump specified area (in 16 bit width) ***/

void dumps(LispPTR laddr) {
  int i;
  DLword *ptr;

  ptr = (DLword *)Addr68k_from_LADDR(laddr);

  for (i = 0; i < 40; i++, ptr++)
    printf("LADDR 0x%x : %d\n", LADDR_from_68k(ptr), (GETWORD(ptr) & 0xffff));
}

/***********************/
void printPC(void) {
  unsigned short pc;

  pc = (UNSIGNED)PC - (UNSIGNED)FuncObj;

  printf("PC: O%o ", pc);
}

/***************************/
int countchar(char *string) {
  int cnt = 0;

  while (*string != '\0') {
    string++;
    cnt++;
  }

  return (cnt);
}

void dump_bf(Bframe *bf) {
  DLword *ptr;
  printf("\n*** Basic Frame");
  if (BFRAMEPTR(bf)->flags != 4) {
    printf("\nInvalid basic frame");
    return;
  };

  if (BFRAMEPTR(bf)->residual) { goto printflags; }

  ptr = Addr68k_from_LADDR(STK_OFFSET + bf->ivar);
  if ((((DLword *)bf - ptr) > 512) || (((UNSIGNED)ptr & 1) != 0)) {
    printf("\nInvalid basic frame");
    return;
  }
  while (ptr < (DLword *)bf) {
    printf("\n %x : %x %x", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));
    print(*ptr);
    ptr += 2;
  }

printflags:
  printf("\n %x : %x %x ", LADDR_from_68k(bf), *(DLword *)bf, *((DLword *)bf + 1));
  putchar('[');
  if (BFRAMEPTR(bf)->residual) printf("Residual, ");
  if (BFRAMEPTR(bf)->padding) printf("Padded, ");
  printf("usecnt=%d ] ", BFRAMEPTR(bf)->usecnt);
  printf("ivar : 0x%x", BFRAMEPTR(bf)->ivar);
}

void dump_fx(struct frameex1 *fx_addr68k) {
  DLword *next68k;
  DLword *ptr;
  LispPTR atomindex;

  ptr = (DLword *)fx_addr68k;

  if (fx_addr68k->flags != 6) {
    printf("\nInvalid frame,NOT FX");
    return;
  };

  atomindex = get_framename((struct frameex1 *)fx_addr68k);
  printf("\n*** Frame Extension for ");
  print(atomindex);
  printf("\n %x : %x %x ", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));

  putchar('[');
  if (fx_addr68k->fast) printf("F,");
  if (fx_addr68k->incall) printf("incall, ");
  if (fx_addr68k->validnametable) printf("V, ");
  printf("usecnt = %d]; alink", fx_addr68k->usecount);
  if (fx_addr68k->alink & 1) printf("[SLOWP]");

  ptr += 2;
  printf("\n %x : %x %x fnheadlo, fnheadhi\n", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));

  ptr += 2;
  printf("\n %x : %x %x next,     pc\n", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));

  ptr += 2;
  printf("\n %x : %x %x LoNmTbl,  HiNmTbl\n", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));

  ptr += 2;
  printf("\n %x : %x %x #blink,   #clink\n", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));

  /* should pay attention to the name table like RAID does */

  next68k = (DLword *)Addr68k_from_LADDR((fx_addr68k->nextblock + STK_OFFSET));
  if (fx_addr68k == CURRENTFX) { next68k = CurrentStackPTR + 2; }

  if ((next68k < ptr) || (((UNSIGNED)next68k & 1) != 0)) {
    printf("\nNext block invalid");
    return;
  }

  while (next68k > ptr) {
    ptr += 2;
    printf("\n %x : %x %x", LADDR_from_68k(ptr), GETWORD(ptr), GETWORD(ptr + 1));
  }
} /* end dump_fx */

/***************************************************************/
/*
        Func Name :	dump_stackframe
        Desc :		For Debugging Aids
        Changed		8 JUN 1987 TAKE

*/
/***************************************************************/
void dump_stackframe(struct frameex1 *fx_addr68k) {
  Bframe *bf;
  if ((fx_addr68k->alink & 1) == 0) { /* FAST */
    bf = (Bframe *)(((DLword *)fx_addr68k) - 2);
  } else { /* SLOW */
    bf = (Bframe *)Addr68k_from_LADDR((fx_addr68k->blink + STK_OFFSET));
  }
  dump_bf(bf);
  dump_fx((struct frameex1 *)fx_addr68k);
}

void dump_CSTK(int before) {
  DLword *ptr;
  ptr = CurrentStackPTR - before;
  while (ptr != CurrentStackPTR) {
    printf("\n%x : %x ", LADDR_from_68k(ptr), GETWORD(ptr));
    ptr++;
  }
  printf("\nCurrentSTKP : %x  ", LADDR_from_68k(CurrentStackPTR));
  printf("\ncontents :  %x ", *((LispPTR *)(CurrentStackPTR - 1)));
} /* dump_CSTK end */

/******************************************/
/* BTV */

void btv(void) {
  struct frameex1 *fx_addr68k;
  struct frameex1 *get_nextFX(FX * fx);

  fx_addr68k = CURRENTFX;

loop:
  dump_stackframe(fx_addr68k);
  if (fx_addr68k->alink == 0) {
    printf("\n BTV end");
    return;
  };

  fx_addr68k = get_nextFX(fx_addr68k);
  goto loop;
} /*end btv*/

int get_framename(struct frameex1 *fx_addr68k) {
  struct fnhead *fnheader;
  LispPTR scratch;

/* Get FNHEAD */
#ifdef BIGVM
  if (fx_addr68k->validnametable == 0) {
    scratch = (unsigned int)(fx_addr68k->fnheader);
  } else {
    scratch = (unsigned int)(fx_addr68k->nametable);
  }
#else
  if (fx_addr68k->validnametable == 0) {
    scratch = (unsigned int)(fx_addr68k->hi2fnheader << 16);
    scratch |= (unsigned int)(fx_addr68k->lofnheader);
  } else {
    scratch = (unsigned int)(fx_addr68k->hi2nametable << 16);
    scratch |= (unsigned int)(fx_addr68k->lonametable);
  }
#endif /* BIGVM */
  fnheader = (struct fnhead *)Addr68k_from_LADDR(scratch);
  return (fnheader->framename);
} /* get_framename end */

FX *get_nextFX(FX *fx) {

  if (URaid_scanlink == URSCAN_ALINK)
    return ((FX *)Addr68k_from_StkOffset(GETALINK(fx)));
  else
    return ((FX *)Addr68k_from_StkOffset(GETCLINK(fx)));

} /* get_nextFX end */

int MAKEATOM(char *string) {
  int length;
  length = countchar(string);
  return (make_atom(string, 0, length, 0));
}

/************************************************************************/
/*									*/
/*			M a k e A t o m 6 8 k				*/
/*									*/
/*	Given a LITATOM that exists before the package system was	*/
/*	turned on, return a pointer to that atom's value cell.		*/
/*									*/
/************************************************************************/

LispPTR *MakeAtom68k(char *string) {
  int index;
  index = make_atom(string, 0, countchar(string), 0);
#ifdef BIGVM
  index = (ATOMS_HI << 16) + (index * 10) + NEWATOM_VALUE_OFFSET;
#else
  index = VALS_OFFSET + (index << 1);
#endif /* BIGVM */
  return ((LispPTR *) Addr68k_from_LADDR(index));
}

/************************************************************************/
/*									*/
/*		G E T T O P V A L					*/
/*									*/
/*	Print the top-level value of a given atom; for use in dbx.			*/
/*									*/
/************************************************************************/
void GETTOPVAL(char *string) {
  int index;
  LispPTR *cell68k;

  index = MAKEATOM(string);
  if (index != -1) /* Only print if there's such an atom */
  {
    cell68k = (LispPTR *)GetVALCELL68k(index);
    print(*cell68k);
  } else
    printf("'%s': no such symbol.\n", string);
}

/************************************************************************/
/*									*/
/*				S _ T O P V A L				*/
/*									*/
/*	Given a string that's an atom name minus the initial \,		*/
/*	print the atom's top-level value.  This is here because		*/
/*	DBX won't put \'s in strings you type.				*/
/*									*/
/************************************************************************/

void S_TOPVAL(char *string) {
  int index;
  LispPTR *cell68k;
  int length;
  char dummy[256];

  dummy[0] = '\\';
  for (length = 1; *string != '\0'; length++, string++) { dummy[length] = *string; }

  index = make_atom(dummy, 0, length, 0);
  cell68k = (LispPTR *)GetVALCELL68k(index);
  print(*cell68k);
}

/***************/
int S_MAKEATOM(char *string) {
  int index = 0;
  int length;
  char dummy[256];

  dummy[0] = '\\';
  for (length = 1; *string != '\0'; length++, string++) { dummy[length] = *string; }

  index = make_atom(dummy, 0, length, 0);
  printf("#Atomindex : %d\n", index);
  return (index);
}

/****************************************************************************/
/*     all_stack_dump(start,end)
*/

jmp_buf SD_jumpbuf;

#define SDMAXLINE 30
#define SD_morep                                     \
  if (++sdlines > SDMAXLINE) {                       \
    int temp;                                        \
    printf("\nPress Return:(to quit Esc and Ret):"); \
    temp = getchar();                                \
    fflush(stdin);                                   \
    sdlines = 0;                                     \
    if (temp == 27) longjmp(SD_jumpbuf, 1);          \
  }

#ifndef BYTESWAP
typedef struct stack_header {
  unsigned flags1 : 3;
  unsigned flags2 : 5;
  unsigned usecount : 8;
} STKH;

#define STKHPTR(ptr) (ptr)

#else
typedef struct stack_header {
  unsigned usecount : 8;
  unsigned flags2 : 5;
  unsigned flags1 : 3;
} STKH;

#define STKHPTR(ptr) ((STKH *)(2 ^ (UNSIGNED)(ptr)))

#endif /* BYTESWAP */

void all_stack_dump(DLword start, DLword end, DLword silent)
/* Stack offset in DLword */
{
  STKH *stkptr;
  DLword *start68k, *end68k, *orig68k;
  DLword size;
  int sdlines = 0;
  extern IFPAGE *InterfacePage;

  if (start == 0)
    start68k = Stackspace + InterfacePage->stackbase;
  else
    start68k = Addr68k_from_LADDR(STK_OFFSET | start);

  if (end == 0)
    end68k = Stackspace + InterfacePage->endofstack;
  else
    end68k = Addr68k_from_LADDR(STK_OFFSET | end);

  stkptr = (STKH *)start68k;

  while (((DLword *)stkptr) < end68k) {
    switch (STKHPTR(stkptr)->flags1) {
      case STK_GUARD:
      case STK_FSB:
        if ((STKHPTR(stkptr)->flags2 != 0) || (STKHPTR(stkptr)->usecount != 0)) { goto badblock; };
        size = GETWORD(((DLword *)stkptr) + 1);
        if (STKHPTR(stkptr)->flags1 == STK_GUARD)
          printf("\n0x%x GUARD, size : 0x%x", LADDR_from_68k(stkptr), size);
        else
          printf("\n0x%x FSB,   size : 0x%x", LADDR_from_68k(stkptr), size);

        if (size <= 0 || size > ((DLword *)end68k - (DLword *)stkptr)) { goto badblock; };

        SD_morep;
        size = GETWORD(((DLword *)stkptr) + 1);
      checksize:
        if (size <= 0 || size > ((DLword *)end68k - (DLword *)stkptr)) { goto badblock; };
        stkptr = (STKH *)(((DLword *)stkptr) + size);
        break;

      case STK_FX:
        /*if((((FX *)stkptr)->pc < 24) ||
                (((FX *)stkptr)->alink==0)  ||
                 (STKHPTR(stkptr)->usecount > 31))
         {goto badblock;};*/
        if (silent) {
          SD_morep;
          printf("\n0x%x: FX for ", LADDR_from_68k(stkptr));
          print(get_framename((struct frameex1 *)stkptr));
          printf(" [");
          if (((FX *)stkptr)->fast) printf("fast,");
          if (((FX *)stkptr)->incall) printf("incall,");
          if (((FX *)stkptr)->validnametable) printf("V,");
          if (((FX *)stkptr)->nopush) printf("nopush,");
          printf("]");
        } else {
          dump_fx((struct frameex1 *)stkptr);
        }

        if ((FX *)stkptr == CURRENTFX) {
          printf(" <-***current***");
          size = EndSTKP - (DLword *)stkptr;
        } else {
          size = Addr68k_from_LADDR(STK_OFFSET | ((FX *)stkptr)->nextblock) - (DLword *)stkptr;
        };
        goto checksize;
      default:
        orig68k = (DLword *)stkptr;

        while (STKHPTR(stkptr)->flags1 != STK_BF) {
          if (STKHPTR(stkptr)->flags1 != STK_NOTFLG) { goto badblock; };
          stkptr = (STKH *)(((DLword *)stkptr) + DLWORDSPER_CELL);
        };

        if ((BFRAMEPTR(stkptr))->residual) {
          if ((DLword *)stkptr != orig68k) {
            printf("\n$$$Bad BF(res):0x%x", LADDR_from_68k(stkptr));
            goto incptr;
          }
        } else {
          if (BFRAMEPTR(stkptr)->ivar != StkOffset_from_68K(orig68k)) {
            printf("\n$$$BF doesn't point TopIVAR:0x%x\n", LADDR_from_68k(stkptr));
            goto incptr;
          }
        }

        if (silent) {
          SD_morep;
          printf("\n0x%x BF,   ", LADDR_from_68k(stkptr));
          putchar('[');
          if (BFRAMEPTR(stkptr)->residual) printf("Res,");
          if (BFRAMEPTR(stkptr)->padding) printf("Pad,");
          printf("ivar : 0x%x]", BFRAMEPTR(stkptr)->ivar);
        } else
          dump_bf((Bframe *)stkptr);

        stkptr = (STKH *)(((DLword *)stkptr) + 2);
        break;

      badblock:
        SD_morep;
        printf("\n0x%x: Invalid, %x %x", LADDR_from_68k(stkptr), GETWORD(stkptr),
               GETWORD(stkptr + 1));
      incptr:
        stkptr = (STKH *)(((DLword *)stkptr) + 2);
        break;

    } /* case end */

  } /* while end */
  printf("\n<< That's All , last stack :0x%x >>\n", InterfacePage->endofstack);
}

/************************************************************/
void dtd_chain(DLword type) {
  struct dtd *dtdp;
  LispPTR next;
  LispPTR *next68k;

  dtdp = (struct dtd *)GetDTD(type);

  next = dtdp->dtd_free;
  next68k = (LispPTR *)Addr68k_from_LADDR(next);

  while ((*next68k) != 0) {
    if (type != GetTypeNumber(next)) {
      printf("BAD cell in next dtdfree\n");
      return;
    }
    print(next);
    putchar('\n');

    next = *next68k;
    next68k = (LispPTR *)Addr68k_from_LADDR(next);
  }
  printf("That's All !\n");

} /*  dtd_chain end **/

#ifdef DTDDEBUG

void check_dtd_chain(DLword type)
{
  register LispPTR next, onext;
  LispPTR before;

  onext = 0;
  next = ((struct dtd *)GetDTD(type))->dtd_free;
  next &= POINTERMASK;

  while (next != NIL) {
    if (next & 1) {
      error("Free pointer is ODD!");
      return;
    }

    if (next & 0x8000000) error("impossibly-big free pointer!");

    if (type != GetTypeNumber(next)) {
      error("BAD cell in next dtdfree ");
      return;
    }
    onext = next;
    next = *((LispPTR *)Addr68k_from_LADDR(next));
    next &= POINTERMASK;
  }
}

#endif

/************************************************************************/
/*									*/
/*			T R A C E _ F N C A L L				*/
/*			T R A C E _ A P P L Y				*/
/*									*/
/*	Functions for tracing function call and apply.  Trace_FNCall	*/
/*	takes 2 arguments:  The # of args the fn is bing called with	*/
/*	and the atom index of the function's name.			*/
/*									*/
/*	Trace_APPLY takes one argument:  The atom number of the		*/
/*	atom being applied.						*/
/*									*/
/************************************************************************/

void Trace_FNCall(int numargs, int atomindex, int arg1, LispPTR *tos) {
  printf("Calling a %d-arg FN:  ", numargs);
  print_atomname(atomindex);
  printf("(");
  if (numargs > 1) {
    int i;
    for (i = -(numargs - 2); i < 1; i++) {
      prindatum(*(tos + i));
      printf(", ");
    }
  }
  if (numargs) prindatum(arg1);
  printf(").\n");
  fflush(stdout);
}

void Trace_APPLY(int atomindex) {
  printf("APPLYing an atom:  ");
  print_atomname(atomindex);
  printf(".\n");
  fflush(stdout);
}
