/* $Id: foreign.c,v 1.3 1999/05/31 23:35:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/* Foreign function call support relies upon DLD which
 * weren't supported in modern OSes and the GNU DLD
 * library hasn't been supported or maintained since
 * at least 2006. */
#ifdef MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE
#include <stdio.h>
#include <sys/param.h>

#include "byteswapdefs.h"
#include "dld.h"
#include "lispemul.h"
#include "lspglob.h"
#include "emlglob.h"
#include "adr68k.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "lisp2cdefs.h"
#include "locfile.h"
#include "medleyfp.h"
#include "mkcelldefs.h"
#include "arith.h"
#include "commondefs.h"
#include "stack.h"

#include "foreigndefs.h"

/***********************************************************/
/*	       L S t r i n g T o C S t r i n g		   */
/*							   */
/*  Convert a lisp string to a C string up to MaxLen long. */
/***********************************************************/

#define LStringToCString(Lisp, C, MaxLen, Len)                                                     \
  do {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int i;                                                                                         \
                                                                                                   \
    arrayp = (OneDArray *)(NativeAligned4FromLAddr((unsigned int)Lisp));                                \
    Len = min(MaxLen, arrayp->fillpointer);                                                        \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base =                                                                                     \
            ((char *)(NativeAligned2FromLAddr((unsigned int)arrayp->base))) + ((int)(arrayp->offset));  \
        for (i = 0; i < Len; i++) C[i] = base[i];                                                  \
        C[Len] = '\0';                                                                             \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase =                                                                                    \
            ((short *)(NativeAligned2FromLAddr((unsigned int)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (i = 0; i < Len * 2; i++) C[i] = base[i];                                              \
        C[Len * 2] = '\0';                                                                         \
        break;                                                                                     \
                                                                                                   \
      default: error("LStringToCString can not handle\n");                                         \
    }                                                                                              \
  } while (0)

/************************************************************************/
/*									*/
/*    F O R E I G N - F U N C T I O N   C A L L   I N T E R F A C E	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
/*     void ( *fn ) () = args[0];
    (*fn)();
    return(NIL);
 */

#define VOIDTYPE 0

typedef void (*PFV)();  /* Pointer to Function returning Void */
typedef int (*PFI)();   /* Pointer to Function returning Int */
typedef char (*PFC)();  /* Pointer to Function returning Char */
typedef float (*PFF)(); /* Pointer to Function returning Float */
typedef int (*PFP)();   /* Pointer to Function returning a Pointer */

/************************************************************************/
/*									*/
/*			C A L L _ C _ F N				*/
/*									*/
/*	args[0]: descriptor block for the function			*/
/*		0 is the function address if the function is callable	*/
/*			or 0 if the function is undefined or unresolved	*/
/*		1 is the result type					*/
/*		2 is the error return flag: 0=no errors. 1 if there's	*/
/*			some problem					*/
/*		3 is the length of the arglist passed			*/
/*		4 is the pointer to the smasher place or 0 if return	*/
/*		5-n the arglist types					*/
/*	args[1-<number-of-args+1>]: arguments				*/
/*									*/
/************************************************************************/
#define Max_Arg 32

LispPTR call_c_fn(LispPTR *args) {
  int intarg[Max_Arg], result, i, j;
  int fnaddr, resulttype, *errorflag, *smasher, arglistlength, *descriptorblock;
  PFI pickapart1, pickapart2, pickapart3, pickapart4;
  float fresult;

  DLword *fword, *createcell68k(unsigned int type);

  FX2 *caller;
  struct fnhead *fnhead;
  ByteCode *pc;

  /* Initialize the variables from the descriptorblock */
  descriptorblock = (int *)NativeAligned4FromLAddr(args[0]);
  fnaddr = *descriptorblock++;
  resulttype = *descriptorblock++;
  errorflag = descriptorblock++;
  arglistlength = *descriptorblock++;
  smasher = descriptorblock++;

  /* initialize the errorflag */
  *errorflag = 0;

  /* Initialize the argvector */
  for (i = 0; i < Max_Arg; i++) { intarg[i] = 0; }

  /* Test the function addr. If it is 0 we can not execute. */
  if (fnaddr == 0) {
    *errorflag = -1;
    return (NIL);
  }

#ifdef TRACE
  {
    int *tracedesc;

    printf("Start Foreign function call=====\n");
    tracedesc = (int *)NativeAligned4FromLAddr(args[0]);
    printf("fnaddr: %d\n", *tracedesc++);
    printf("resulttype: %d\n", *tracedesc++);
    printf("errorflag: %d\n", *tracedesc++);
    printf("arglistlength: %d\n", *tracedesc++);
    printf("smasher: %d\n", *tracedesc++);
    for (i = 0; i < arglistlength; i++) { printf("arg[%d]= %d\n", i, *tracedesc++); }
    printf("End Foreign function call=====\n");
  }
#endif /* TRACE */
  for (i = 1, j = 0; i < (arglistlength + 1); i++, j++) {
    int expectedtype;
    expectedtype = *descriptorblock++;
    switch (GetTypeNumber(args[i])) {
      case TYPE_ARRAYBLOCK:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_SMALLP:
        if (expectedtype <= TYPE_FIXP) {
          intarg[j] = LispIntToCInt(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_FIXP:
        if (expectedtype <= TYPE_FIXP) {
          intarg[j] = LispIntToCInt(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_FLOATP:
        if (expectedtype == TYPE_FLOATP) {
          float temp;
          temp = FLOATP_VALUE(args[i]);
          intarg[j++] = pickapart1(temp);
          intarg[j++] = pickapart2(temp);
          intarg[j++] = pickapart3(temp);
          intarg[j++] = pickapart4(temp);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_LITATOM:
      case TYPE_NEWATOM:
        if (expectedtype == TYPE_LITATOM) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_LISTP:
        if (expectedtype == TYPE_LISTP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_ARRAYP:
        if (expectedtype == TYPE_ARRAYP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_STRINGP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_STACKP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_CHARACTERP:
        if (expectedtype == TYPE_CHARACTERP) {
          intarg[j] = (0xFFFF & args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_VMEMPAGEP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_STREAM:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_BITMAP:
        if (expectedtype == TYPE_BITMAP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_COMPILED_CLOSURE: break;
      case TYPE_ONED_ARRAY:
        if (expectedtype == TYPE_ONED_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_TWOD_ARRAY:
        if (expectedtype == TYPE_TWOD_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_GENERAL_ARRAY:
        if (expectedtype == TYPE_GENERAL_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_BIGNUM:
        if (expectedtype == TYPE_BIGNUM) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_RATIO:
        if (expectedtype == TYPE_RATIO) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_COMPLEX:
        if (expectedtype == TYPE_COMPLEX) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_PATHNAME: break;
      default:
        *errorflag = i;
        return (NIL);
        break;
    }
  }

  switch (resulttype) {
    case VOIDTYPE:
      ((PFV)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5], intarg[6],
                    intarg[7], intarg[8], intarg[9], intarg[10], intarg[11], intarg[12], intarg[13],
                    intarg[14], intarg[15], intarg[16], intarg[17], intarg[18], intarg[19],
                    intarg[20], intarg[21], intarg[22], intarg[23], intarg[24], intarg[25],
                    intarg[26], intarg[27], intarg[28], intarg[29], intarg[30], intarg[31]);
      caller = (FX2 *)CURRENTFX; /* Don't return values, just continue. */
      fnhead = (struct fnhead *)NativeAligned4FromLAddr(POINTERMASK & swapx((int)caller->fnheader));
      pc = (ByteCode *)fnhead + (caller->pc);
      break;
    case TYPE_SMALLP:
    case TYPE_FIXP: {
      int tmp;
      tmp = ((PFI)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                          intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                          intarg[12], intarg[13], intarg[14], intarg[15], intarg[16], intarg[17],
                          intarg[18], intarg[19], intarg[20], intarg[21], intarg[22], intarg[23],
                          intarg[24], intarg[25], intarg[26], intarg[27], intarg[28], intarg[29],
                          intarg[30], intarg[31]);
      return (CIntToLispInt(tmp));
    } break;
    case TYPE_CHARACTERP: {
      int tmp;
      tmp = ((PFC)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                          intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                          intarg[12], intarg[13], intarg[14], intarg[15], intarg[16], intarg[17],
                          intarg[18], intarg[19], intarg[20], intarg[21], intarg[22], intarg[23],
                          intarg[24], intarg[25], intarg[26], intarg[27], intarg[28], intarg[29],
                          intarg[30], intarg[31]);
      return (S_CHARACTER | tmp);
    } break;
    case TYPE_FLOATP:
      fresult = ((PFF)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                              intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                              intarg[12], intarg[13], intarg[14], intarg[15], intarg[16],
                              intarg[17], intarg[18], intarg[19], intarg[20], intarg[21],
                              intarg[22], intarg[23], intarg[24], intarg[25], intarg[26],
                              intarg[27], intarg[28], intarg[29], intarg[30], intarg[31]);
      fword = createcell68k(TYPE_FLOATP);
      *((float *)fword) = fresult;
      return (LAddrFromNative(fword));
      break;
    default: *errorflag = -2; break;
  }
}

/* These functions are created so that you can split a float into */
/* four integers. The general idea behind these functions is to */
/* act as a caster between different entities on the stack */

/* These used to live in hacks.c, but were only used here. The code */
/* involved here is not valid ANSI C and will have to be fixed before */
/* this code can be used again. */
static int pickapart1(int i1, int i2, int i3, int i4) { return (i1); }
static int pickapart2(int i1, int i2, int i3, int i4) { return (i2); }
static int pickapart3(int i1, int i2, int i3, int i4) { return (i3); }
static int pickapart4(int i1, int i2, int i3, int i4) { return (i4); }

/************************************************************************/
/*									*/
/*			S M A S H I N G _ C _ F N			*/
/*									*/
/*	args[0]: descriptor block for the function			*/
/*		0 is the function address if the function is callable	*/
/*			or 0 if the function is undefined or unresolved	*/
/*		1 is the result type					*/
/*		2 is the error return flag: 0=no errors. 1 if there's	*/
/*			some problem					*/
/*		3 is the length of the arglist passed			*/
/*		4-n the arglist types					*/
/*	args[1]: Smashing place						*/
/*	args[2 - <number-of-args+1>]: arguments				*/
/*									*/
/*	This is an aberration. It is only implemented on the specific	*/
/*	request of an influential customer. The things we do for money!	*/
/*									*/
/*	The result of this functioncall will be smashed into what	*/
/*	arg[0] points to. If it is a cell of the right type we are ok.	*/
/*	If it is not we are on the road to hell.  /jarl nilsson		*/
/*									*/
/************************************************************************/
LispPTR smashing_c_fn(LispPTR *args) {
  int intarg[Max_Arg], result, i, j;
  int fnaddr, resulttype, *errorflag, arglistlength, *descriptorblock;
  float fresult;

  int *valueplace;
  DLword *fword, *createcell68k(unsigned int type);

  FX2 *caller;
  struct fnhead *fnhead;
  ByteCode *pc;

  /* Initialize the variables from the descriptorblock */
  descriptorblock = (int *)NativeAligned4FromLAddr(args[0]);
  fnaddr = *descriptorblock++;
  resulttype = *descriptorblock++;
  errorflag = descriptorblock++;
  arglistlength = *descriptorblock++;

  /* initialize the errorflag */
  *errorflag = 0;

  /* Initialize the valueplace */
  valueplace = (int *)NativeAligned4FromLAddr(args[1]);

  /* Initialize the argvector */
  for (i = 0; i < Max_Arg; i++) { intarg[i] = 0; }

  /* Test the function addr. If it is 0 we can not execute. */
  if (fnaddr == 0) {
    *errorflag = -1;
    return (NIL);
  }

  for (i = 2, j = 0; i < (arglistlength + 2); i++, j++) {
    int expectedtype;
    expectedtype = *descriptorblock++;
    switch (GetTypeNumber(args[i])) {
      case TYPE_ARRAYBLOCK:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_SMALLP:
        if (expectedtype <= TYPE_FIXP) {
          intarg[j] = LispIntToCInt(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_FIXP:
        if (expectedtype <= TYPE_FIXP) {
          intarg[j] = LispIntToCInt(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_FLOATP:
        if (expectedtype == TYPE_FLOATP) {
          float temp;
          temp = FLOATP_VALUE(args[i]);
          intarg[j++] = pickapart1(temp);
          intarg[j++] = pickapart2(temp);
          intarg[j++] = pickapart3(temp);
          intarg[j++] = pickapart4(temp);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_LITATOM:
      case TYPE_NEWATOM:
        if (expectedtype == TYPE_LITATOM) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_LISTP:
        if (expectedtype == TYPE_LISTP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_ARRAYP:
        if (expectedtype == TYPE_ARRAYP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_STRINGP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_STACKP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_CHARACTERP:
        if (expectedtype == TYPE_CHARACTERP) {
          intarg[j] = (char)(0xFF && args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_VMEMPAGEP:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_STREAM:
        *errorflag = i;
        return (NIL);
        break;
      case TYPE_BITMAP:
        if (expectedtype == TYPE_BITMAP) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_COMPILED_CLOSURE: break;
      case TYPE_ONED_ARRAY:
        if (expectedtype == TYPE_ONED_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_TWOD_ARRAY:
        if (expectedtype == TYPE_TWOD_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_GENERAL_ARRAY:
        if (expectedtype == TYPE_GENERAL_ARRAY) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_BIGNUM:
        if (expectedtype == TYPE_BIGNUM) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_RATIO:
        if (expectedtype == TYPE_RATIO) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_COMPLEX:
        if (expectedtype == TYPE_COMPLEX) {
          intarg[j] = *(int *)NativeAligned4FromLAddr(args[i]);
        } else {
          *errorflag = i;
          return (NIL);
        }
        break;
      case TYPE_PATHNAME: break;
      default:
        *errorflag = i;
        return (NIL);
        break;
    }
  }

  switch (resulttype) {
    case VOIDTYPE:
      ((PFV)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5], intarg[6],
                    intarg[7], intarg[8], intarg[9], intarg[10], intarg[11], intarg[12], intarg[13],
                    intarg[14], intarg[15], intarg[16], intarg[17], intarg[18], intarg[19],
                    intarg[20], intarg[21], intarg[22], intarg[23], intarg[24], intarg[25],
                    intarg[26], intarg[27], intarg[28], intarg[29], intarg[30], intarg[31]);
      caller = (FX2 *)CURRENTFX; /* Don't return values, just continue. */
      fnhead = (struct fnhead *)NativeAligned4FromLAddr(POINTERMASK & swapx((int)caller->fnheader));
      pc = (ByteCode *)fnhead + (caller->pc);
      break;
    case TYPE_SMALLP:
    case TYPE_FIXP: {
      int tmp;
      tmp = ((PFI)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                          intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                          intarg[12], intarg[13], intarg[14], intarg[15], intarg[16], intarg[17],
                          intarg[18], intarg[19], intarg[20], intarg[21], intarg[22], intarg[23],
                          intarg[24], intarg[25], intarg[26], intarg[27], intarg[28], intarg[29],
                          intarg[30], intarg[31]);
      *valueplace = tmp;
      return (NIL);
    } break;
    case TYPE_CHARACTERP:
      return (S_CHARACTER |
              (((PFC)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                             intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                             intarg[12], intarg[13], intarg[14], intarg[15], intarg[16], intarg[17],
                             intarg[18], intarg[19], intarg[20], intarg[21], intarg[22], intarg[23],
                             intarg[24], intarg[25], intarg[26], intarg[27], intarg[28], intarg[29],
                             intarg[30], intarg[31])));
      break;
    case TYPE_FLOATP:
      fresult = ((PFF)fnaddr)(intarg[0], intarg[1], intarg[2], intarg[3], intarg[4], intarg[5],
                              intarg[6], intarg[7], intarg[8], intarg[9], intarg[10], intarg[11],
                              intarg[12], intarg[13], intarg[14], intarg[15], intarg[16],
                              intarg[17], intarg[18], intarg[19], intarg[20], intarg[21],
                              intarg[22], intarg[23], intarg[24], intarg[25], intarg[26],
                              intarg[27], intarg[28], intarg[29], intarg[30], intarg[31]);
      *valueplace = fresult;
      return (NIL);
      break;
    default: *errorflag = -2; break;
  }
}

/************************************************************************/
/*									*/
/*			M d l d _ l i n k				*/
/*									*/
/*	args[0] - The lisp string name of the path to the filename	*/
/*	Return value: 0 -> ok.						*/
/*	   1 - 16 -> errorcode						*/
/*									*/
/************************************************************************/

int Mdld_link(LispPTR *args) {
  char filename[MAXPATHLEN];
  int result, leng;

#ifdef TRACE
  printf("TRACE: dld_link(");
#endif

  LStringToCString(args[0], filename, MAXPATHLEN, leng);

#ifdef TRACE
  printf("%s)\n", filename);
#endif

  result = dld_link(filename);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*            	M d l d _ u n l i n k _ b y _ f i l e			*/
/*									*/
/*	args[0] - The lisp string name of the path to the filename	*/
/*	args[1] - Force flag. If NonZero, force the unlinking, even	*/
/*		if there are references to this module.			*/
/*	Return value: 0 -> ok.						*/
/*	   1 - 16 -> errorcode						*/
/*									*/
/************************************************************************/

int Mdld_unlink_by_file(LispPTR *args) {
  char filename[MAXPATHLEN];
  int hard, result, leng;

#ifdef TRACE
  printf("TRACE: dld_unlink_by_file(");
#endif

  LStringToCString(args[0], filename, MAXPATHLEN, leng);
  hard = GetSmalldata(args[1]);

#ifdef TRACE
  printf("%s, %d)\n", filename, hard);
#endif

  result = dld_unlink_by_file(filename, hard);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*            	M d l d _ u n l i n k _ b y _ s y m b o l		*/
/*									*/
/*	args[0] - The lisp string name of the symbol in some module.	*/
/*	args[1] - Force flag. If NonZero, force the unlinking, even	*/
/*		if there are references to this module.			*/
/*	Return value: 0 -> ok.						*/
/*	   1 - 16 -> errorcode						*/
/*									*/
/************************************************************************/

int Mdld_unlink_by_symbol(LispPTR *args) {
  char symbolname[MAXPATHLEN];
  int hard, result, leng;

#ifdef TRACE
  printf("TRACE: dld_unlink_by_symbol(");
#endif

  LStringToCString(args[0], symbolname, MAXPATHLEN, leng);
  hard = GetSmalldata(args[1]);

#ifdef TRACE
  printf("%s, %d)\n", symbolname, hard);
#endif

  result = dld_unlink_by_symbol(symbolname, hard);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*		       	M d l d _ g e t _ s y m b o l			*/
/*									*/
/*	args[0] - The lisp string name of the symbol.			*/
/*	Return value - a pointer to the symbol or 0			*/
/*									*/
/************************************************************************/

unsigned long Mdld_get_symbol(LispPTR *args) {
  char symbolname[MAXPATHLEN];
  int result, leng;

#ifdef TRACE
  printf("TRACE: dld_get_symbol(");
#endif

  LStringToCString(args[0], symbolname, MAXPATHLEN, leng);

#ifdef TRACE
  printf("%s, %d)\n", symbolname);
#endif

  result = dld_get_symbol(symbolname);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*		       	M d l d _ g e t _ f u n c			*/
/*									*/
/*	args[0] - The lisp string name of the function.			*/
/*	Return value - a pointer to the function or 0.			*/
/*									*/
/************************************************************************/
unsigned long Mdld_get_func(LispPTR *args) {
  char funcname[MAXPATHLEN];
  int result, leng;

#ifdef TRACE
  printf("TRACE: dld_get_func(");
#endif

  LStringToCString(args[0], funcname, MAXPATHLEN, leng);

#ifdef TRACE
  printf("%s )\n", funcname);
#endif

  result = dld_get_func(funcname);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*     	M d l d _ f u n c t i o n _ e x e c u t a b l e _ p		*/
/*									*/
/*	args[0] - The lisp string name of the function.			*/
/*									*/
/************************************************************************/
int Mdld_function_executable_p(LispPTR *args) {
  char funcname[MAXPATHLEN];
  int result, leng;

#ifdef TRACE
  printf("TRACE: dld_function_executable_p(");
#endif

  LStringToCString(args[0], funcname, MAXPATHLEN, leng);

#ifdef TRACE
  printf("%s, %d)\n", funcname);
#endif

  result = dld_function_executable_p(funcname);
  N_ARITH_SWITCH(result);
};

/************************************************************************/
/*									*/
/*	M d l d _ l i s t _ u n d e f i n e d _ s y m			*/
/*									*/
/*									*/
/************************************************************************/
int Mdld_list_undefined_sym(void) {
  char **dld_list_undefined_sym();
  int temp;
  extern int dld_undefined_sym_count;

#ifdef TRACE
  printf("TRACE: dld_list_undefined_sym()\n");
#endif

  if (dld_undefined_sym_count == 0) { return (NIL); }

  temp = (int)dld_list_undefined_sym();
  N_ARITH_SWITCH(temp);
};

/************************************************************************/
/*									*/
/*			m a l l o c					*/
/*									*/
/*									*/
/************************************************************************/
int c_malloc(LispPTR *args) {
  printf("malloc!\n");
  return (NIL);
}

/************************************************************************/
/*									*/
/*				f r e e					*/
/*									*/
/*									*/
/************************************************************************/
int c_free(LispPTR *args) {
  printf("free!\n");
  return (NIL);
}

/************************************************************************/
/*									*/
/*			P U T B A S E B Y T E				*/
/*									*/
/*	arg[0] = the base address. 					*/
/*	arg[1] = the offset as calculated by the type			*/
/*	arg[2] = type of access.					*/
/*		0 = set the bit pointed out by address + offset		*/
/*		1 = set the byte pointed out by address + offset	*/
/*		2 = set the word (16bit) pointed to by address + offset	*/
/*		3 = set the integer pointed out by address + offset	*/
/*		4 = set the float pointed out by address + offset	*/
/*	arg[3] = the new value.						*/
/*	The offset changes depending on the type argument. If type is	*/
/*	0 the changed value will be the bit at ADDRESS + OFFSET bits.	*/
/*	If type is 1 the changedvalue will be the byte at ADDRESS + 	*/
/*	OFFSET bytes. If type is 2 the changed value will be the int	*/
/*	at ADDRESS + OFFSET.						*/
/*									*/
/*	This makes it easy to set values in arrays.			*/
/*									*/
/************************************************************************/
int put_c_basebyte(LispPTR *args) {
  int addr, offset, newval;

  addr = LispIntToCInt(args[0]);
  offset = LispIntToCInt(args[1]);
  newval = LispIntToCInt(args[3]);

  switch (LispIntToCInt(args[2])) {
    case 0: /* bit */
      if (newval == 0) {
        GETBYTE((char *)(addr + (offset >> 3))) &= (~(1 << (0x7 & offset)));
      } else {
        GETBYTE((char *)(addr + (offset >> 3))) |= (1 << (0x7 & offset));
      }
      break;
    case 1: /* byte */ GETBYTE((char *)(addr + offset)) = 0xFF & newval; break;
    case 2: /* word */
      newval &= 0xFFFF;
      (*((short *)((addr & 0xFFFFFFFE) + (offset << 1)))) = newval;
      break;
    case 3: /* int */ (*((int *)((addr & 0xFFFFFFFE) + (offset << 2)))) = newval; break;
    case 4: /* float */
      (*((float *)((addr & 0xFFFFFFFE) + (offset << 2)))) = FLOATP_VALUE(args[3]);
      break;
  }
  return (NIL);
}

/************************************************************************/
/*									*/
/*			G E T _ C _ B A S E B Y T E			*/
/*									*/
/*	arg[0] = the base address. 					*/
/*	arg[1] = the offset as calculated by the type			*/
/*	arg[2] = type of access.					*/
/*		0 = get the bit pointed out by address + offset		*/
/*		1 = get the byte pointed out by address + offset	*/
/*		2 = get the word (16bit) pointed to by address + offset	*/
/*		3 = get the integer pointed out by address + offset	*/
/*		4 = get the float pointed out by address + offset	*/
/*	The offset changes depending on the type argument. If type	*/
/*	is 0 the return value will be the bit at ADDRESS + OFFSET bits.	*/
/*	If type is 1 the return value will be the byte at ADDRESS + 	*/
/*	OFFSET bytes. If type is 2 the return value will be the integer	*/
/*	at ADDRESS + OFFSET.						*/
/*									*/
/*	This makes it easy to access arrays.				*/
/*									*/
/************************************************************************/
int get_c_basebyte(LispPTR *args) {
  int addr, offset, type;
  DLword *fword, *createcell68k(unsigned int type);

  addr = LispIntToCInt(args[0]);
  offset = LispIntToCInt(args[1]);

  switch (LispIntToCInt(args[2])) {
    case 0: /* bit */
      if ((GETBYTE((char *)(addr + (offset >> 3)))) & (1 << (0x7 & offset))) {
        /* ^get bitmask from offset    ^get the byte at the byteaddress */
        return (ATOM_T);
      } else {
        return (NIL);
      }
      break;
    case 1: /* byte */ return ((0xFF & (GETBYTE((char *)(addr + offset)))) | S_POSITIVE); break;
    case 2: /* word */
      return (CIntToLispInt(0xFFFF & (*((short *)((addr & 0xFFFFFFFE) + (offset << 1))))));
      break;
    case 3: /* int */
      return (CIntToLispInt(*((int *)((addr & 0xFFFFFFFE) + (offset << 2)))));
      break;
    case 4: /* float */
      fword = createcell68k(TYPE_FLOATP);
      *((float *)fword) = *(float *)((addr & 0xFFFFFFFE) + (offset << 2));
      return (LAddrFromNative(fword));
      break;
  }
}
#endif /* MAIKO_ENABLE_FOREIGN_FUNCTION_INTERFACE */
