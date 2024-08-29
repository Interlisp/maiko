/* $Id: loopsops.c,v 1.3 1999/05/31 23:35:37 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
  LOOPS opcodes:

        FetchMethodOrHelp(object, selector) -> MethodFn
        LookupIV(object, iv) -> Index into iDescrs
        GetValue(object, iv)
        PutValue(object, iv, val)
*/

#include "adr68k.h"        // for NativeAligned2FromLAddr, NativeAligned4FromLAddr, LAddrFromNative
#include "car-cdrdefs.h"   // for car, cdr
#include "cell.h"          // for GetVALCELL68k, definition_cell, GetDEFCELL68k
#include "commondefs.h"    // for error
#include "gcarraydefs.h"   // for get_package_atom
#include "gcdata.h"        // for FRPLPTR
#include "gchtfinddefs.h"  // for htfind, rec_htfind
#include "lispemul.h"      // for LispPTR, state, CurrentStackPTR, NIL_PTR, NIL
#include "lispmap.h"       // for S_POSITIVE
#include "loopsopsdefs.h"  // for lcfuncall, LCFetchMethod, LCFetchMethodOrHelp
#include "lspglob.h"
#include "lsptypes.h"      // for GetDTD, GetTypeNumber, dtd, Listp, GETWORD
#include "stack.h"         // for frameex1, fnhead, BF_MARK, FX_MARK, STK_SAFE

struct LCIVCacheEntry;
struct LCInstance;

static const char il_string[] = "INTERLISP";
#define GET_IL_ATOM(string) get_package_atom((string), (sizeof(string) - 1), il_string, 9, NIL)

#define AtomValPtr(index) NativeAligned4FromLAddr(*(GetVALCELL68k(index)))

#ifdef BIGVM
#define DTD_FROM_LADDR(x) (((struct dtd *)GetDTD(GetTypeNumber((x))))->dtd_name)
#else
#define DTD_FROM_LADDR(x)                                   \
  (((struct dtd *)GetDTD(GetTypeNumber((x))))->dtd_namelo + \
   (((struct dtd *)GetDTD(GetTypeNumber((x))))->dtd_namehi << 16))
#endif /* BIGVM */

/* These assume 0 <= POSINT <= 65535 */
#define SMALLP_FROM_POSINT(x) ((x) | S_POSITIVE)
#define POSINT_FROM_SMALLP(x) ((x)&0xffff)

#define RETCALL(fn, argnum) return lcfuncall(fn, argnum, 3)

/* #define PUNT	(-1)*/

#define METH_CACHE_INDEX(CLASS, SELECTOR) (1023 & ((CLASS) ^ (SELECTOR)))
#define IV_CACHE_INDEX(VARLIST, IV) (1023 & ((VARLIST) ^ (IV)))

#define LC_TYPEP(obj, typeATOM) (DTD_FROM_LADDR((obj)) == (typeATOM))

#define INSTANCEP(obj) (LC_TYPEP((obj), atom_instance))
#define CLASSP(obj) (LC_TYPEP((obj), atom_class))

#define INSTANCE_OR_PUNT(obj, fn, argnum)                     \
  do {                                                        \
    if (!LC_TYPEP((obj), atom_instance)) RETCALL(fn, argnum); \
  } while (0)

#define INSTANCE_CLASS_OR_PUNT(obj, fn, argnum)                         \
  do {                                                                  \
    LispPTR tmp = DTD_FROM_LADDR(obj);                         \
    if (tmp != atom_instance && tmp != atom_class) RETCALL(fn, argnum); \
  } while (0)

#define LC_INIT \
  if (atom_instance == 0) LCinit()

#define GET_IV_INDEX(objptr, iv, dest, otherwise)                 \
  do {                                                            \
    struct LCIVCacheEntry *ce;                           \
    LispPTR iNames = (objptr)->iNames;                   \
                                                                  \
    ce = &(LCIVCache[IV_CACHE_INDEX(iNames, iv)]);                \
    if (ce->iNames == iNames && ce->iv == (iv)) {                 \
      (dest) = POSINT_FROM_SMALLP(ce->index);                     \
    } else {                                                      \
      if (!Listp(iNames)) {                                       \
        otherwise;                                                \
      } else {                                                    \
        int i = 0;                                       \
        while (1) {                                               \
          if (car(iNames) == (iv)) {                              \
            ce->iNames = (objptr)->iNames;                        \
            ce->iv = iv;                                          \
            ce->index = SMALLP_FROM_POSINT(i);                    \
            (dest) = i;                                           \
            break;                                                \
          } else {                                                \
            i++;                                                  \
            if ((iNames = cdr(iNames)) == NIL_PTR) { otherwise; } \
          }                                                       \
        }                                                         \
      }                                                           \
    }                                                             \
  } while (0)

struct LCClass { /* class datatype */
  LispPTR metaClass, ivNames, ivDescrs, classUnitRec, localIVs, cvNames, cvDescrs, className,
      supers, subClasses, otherClassDescription, selectors, methods, localSupers;
};

struct LCInstance { /* instance datatype */
  LispPTR class, iNames, iDescrs, instMiscField;
};

static struct LCMethodCacheEntry {
  LispPTR class, selector, method_fn, junk;
} * LCMethodCache;

static struct LCIVCacheEntry {
  LispPTR iNames, iv, index, junk;
} * LCIVCache;

static LispPTR atom_instance = 0, /* various atom indices */
    atom_class, atom_annotatedValue, atom_FetchMethodOrHelp_LCUFN, atom_FetchMethod_LCUFN,
        atom_FindVarIndex_LCUFN, atom_GetIVValue_LCUFN, atom_PutIVValue_LCUFN;

/* Called once to initialize the "constants" above */

LispPTR LCinit(void) {
  atom_instance = GET_IL_ATOM("instance");
  atom_class = GET_IL_ATOM("class");
  atom_annotatedValue = GET_IL_ATOM("annotatedValue");
  atom_FetchMethodOrHelp_LCUFN = GET_IL_ATOM("\\FetchMethodOrHelp-LCUFN");
  atom_FetchMethod_LCUFN = GET_IL_ATOM("\\FetchMethod-LCUFN");
  atom_FindVarIndex_LCUFN = GET_IL_ATOM("\\FindVarIndex-LCUFN");
  atom_GetIVValue_LCUFN = GET_IL_ATOM("\\GetIVValue-LCUFN");
  atom_PutIVValue_LCUFN = GET_IL_ATOM("\\PutIVValue-LCUFN");
  LCMethodCache = (struct LCMethodCacheEntry *)AtomValPtr(GET_IL_ATOM("*Global-Method-Cache*"));
  LCIVCache = (struct LCIVCacheEntry *)AtomValPtr(GET_IL_ATOM("*Global-IV-Cache-Block*"));
  return NIL_PTR; /* in case called from lisp */
}

/* Type check fn */
/* We only check for instance and class, neither of which has supertypes,
   so the loop is unnecessary.  */
/* * * NOT USED * * */
#ifdef NEVER
int LCTypeOf(LispPTR thing, LispPTR typename)
{
  struct dtd *dtd68k;
#ifdef BIGVM
  for (dtd68k = (struct dtd *)GetDTD(GetTypeNumber(thing)); typename != (dtd68k->dtd_name);
       dtd68k = (struct dtd *)GetDTD(dtd68k->dtd_supertype)) {
    if (dtd68k->dtd_supertype == 0) return 0;
  }
#else
  for (dtd68k = (struct dtd *)GetDTD(GetTypeNumber(thing));
       typename != dtd68k->dtd_namelo + (dtgd68k->dtd_namehi << 16);
       dtd68k = (struct dtd *)GetDTD(dtd68k->dtd_supertype)) {
    if (dtd68k->dtd_supertype == 0) return 0;
  }
#endif /* BIGVM */
  return 1;
}
#endif /* NEVER */

/* Method lookup using global cache */

LispPTR LCFetchMethodOrHelp(LispPTR object, LispPTR selector) {
  struct LCInstance *objptr;
  struct LCMethodCacheEntry *ce;
  LispPTR cur_class;

  LC_INIT;

  INSTANCE_CLASS_OR_PUNT(object, atom_FetchMethodOrHelp_LCUFN, 2);

  objptr = (struct LCInstance *)NativeAligned4FromLAddr(object);
  ce = &(LCMethodCache[METH_CACHE_INDEX((cur_class = objptr->class), selector)]);
  if (ce->class == cur_class && ce->selector == selector) return ce->method_fn;

  /* not in cache, search class then supers */
  {
    LispPTR supers = ((struct LCClass *)NativeAligned4FromLAddr(cur_class))->supers;

    for (;;) {
      int i = 0;
      LispPTR val;
      LispPTR *selectorptr;
      struct LCClass *classptr;

      classptr = (struct LCClass *)NativeAligned4FromLAddr(cur_class);
      if (classptr->selectors == NIL_PTR) {
        goto next_class;
      } else {
        selectorptr = (LispPTR *)NativeAligned4FromLAddr(classptr->selectors);
      }

      while ((val = selectorptr[i++]) != NIL_PTR) {
        if (val == selector) {
          ce->class = objptr->class;
          ce->selector = selector;
          return (ce->method_fn = ((LispPTR *)NativeAligned4FromLAddr(classptr->methods))[i - 1]);
        }
      }

    next_class:
      if ((cur_class = car(supers)) == NIL_PTR) break;
      supers = cdr(supers);
    }
  }

  /* we didn't find it at all; punt */
  RETCALL(atom_FetchMethodOrHelp_LCUFN, 2);

  /*  return lcfuncall(atom_FetchMethodOrHelp_LCUFN,2,3); */
  /*  return PUNT;*/
}

LispPTR LCFetchMethod(LispPTR class, LispPTR selector) {
  struct LCMethodCacheEntry *ce;

  LC_INIT;

  /* Check cache before doing type check */
  ce = &(LCMethodCache[METH_CACHE_INDEX(class, selector)]);
  if (ce->class == class && ce->selector == selector) return ce->method_fn;

  /* it wasn't there, go search class then supers */

  if (!LC_TYPEP(class, atom_class)) RETCALL(atom_FetchMethod_LCUFN, 2);
  {
    LispPTR cur_class = class;
    LispPTR supers = ((struct LCClass *)NativeAligned4FromLAddr(cur_class))->supers;

    for (;;) {
      int i = 0;
      LispPTR val;
      struct LCClass *classptr;
      LispPTR *selectorptr;

      classptr = (struct LCClass *)NativeAligned4FromLAddr(cur_class);
      if (classptr->selectors == NIL_PTR)
        goto next_class;
      else
        selectorptr = (LispPTR *)NativeAligned4FromLAddr(classptr->selectors);

      while ((val = selectorptr[i++]) != NIL_PTR) {
        if (val == selector) {
          ce->class = class;
          ce->selector = selector;
          return (ce->method_fn = ((LispPTR *)NativeAligned4FromLAddr(classptr->methods))[i - 1]);
        }
      }

    next_class:
      if ((cur_class = car(supers)) == NIL_PTR) break;
      supers = cdr(supers);
    }
  }

  /* we didn't find it at all; return NIL */
  return NIL_PTR;
}

LispPTR LCFindVarIndex(LispPTR iv, LispPTR object) {
  struct LCInstance *objptr;
  struct LCIVCacheEntry *ce;
  LispPTR iNames;

  LC_INIT;

  INSTANCE_CLASS_OR_PUNT(object, atom_FindVarIndex_LCUFN, 2);

  objptr = (struct LCInstance *)NativeAligned4FromLAddr(object);
  ce = &(LCIVCache[IV_CACHE_INDEX((iNames = objptr->iNames), iv)]);
  if (ce->iNames == iNames && ce->iv == iv) return ce->index;

  if (!Listp(iNames)) return NIL_PTR; /* FastFindIndex lisp macro (& others?) */
                                      /* needs this check too ! */
  {
    int i;

    for (i = 0; (iNames = cdr(iNames)) != NIL_PTR; i++) {
      if (car(iNames) == iv) {
        ce->iNames = objptr->iNames;
        ce->iv = iv;
        return (ce->index = SMALLP_FROM_POSINT(i));
      }
    }
    return NIL_PTR;
  }
}

#if 01

LispPTR LCGetIVValue(LispPTR object, LispPTR iv) {
  struct LCInstance *objptr;
  LispPTR val;
  int index;

  LC_INIT;
  INSTANCE_OR_PUNT(object, atom_GetIVValue_LCUFN, 2);

  objptr = (struct LCInstance *)NativeAligned4FromLAddr(object);
  GET_IV_INDEX(objptr, iv, index, goto pnut);
  val = ((LispPTR *)NativeAligned4FromLAddr(objptr->iDescrs))[index];
  if (!LC_TYPEP(val, atom_annotatedValue)) return val;
pnut:
  RETCALL(atom_GetIVValue_LCUFN, 2);
  /*
    return LCGetActiveValue(object,val);
  */
}

LispPTR LCPutIVValue(LispPTR object, LispPTR iv, LispPTR val) {
  struct LCInstance *objptr;
  LispPTR *valptr;
  int index;

  LC_INIT;
  INSTANCE_OR_PUNT(object, atom_PutIVValue_LCUFN, 3);

  objptr = (struct LCInstance *)NativeAligned4FromLAddr(object);
  GET_IV_INDEX(objptr, iv, index, goto pnut);
  valptr = &(((LispPTR *)NativeAligned4FromLAddr(objptr->iDescrs))[index]);
  if (!LC_TYPEP(*valptr, atom_annotatedValue)) {
    FRPLPTR((*valptr), val);
    return val;
  }
pnut:
  RETCALL(atom_PutIVValue_LCUFN, 3);
  /*
    return LCPutActiveValue(object,*valptr,val);
  */
}

#endif

LispPTR lcfuncall(unsigned int atom_index, int argnum, int bytenum)
/* Atomindex for Function to invoke */
/* Number of ARGS on TOS and STK */
/* Number of bytes of Caller's
   OPCODE(including multi-byte) */
{
  struct definition_cell *defcell68k; /* Definition Cell PTR */
  short pv_num;                       /* scratch for pv */
  struct fnhead *tmp_fn;
  int rest; /* use for alignments */

  if (atom_index == 0xffffffff) error("Loops punt to nonexistent fn");

  /* Get Next Block offset from argnum */
  CURRENTFX->nextblock = (LAddrFromNative(CurrentStackPTR) & 0x0ffff) - (argnum << 1) + 4 /* +3  */;

  /* Setup IVar */
  IVar = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

  /* Set PC to the Next Instruction and save into FX */
  CURRENTFX->pc = ((UNSIGNED)PC - (UNSIGNED)FuncObj) + bytenum;

  PushCStack; /* save TOS */

  /* Get DEFCELL 68k address */
  defcell68k = (struct definition_cell *)GetDEFCELL68k(atom_index);

  tmp_fn = (struct fnhead *)NativeAligned4FromLAddr(defcell68k->defpointer);

  if ((UNSIGNED)(CurrentStackPTR + tmp_fn->stkmin + STK_SAFE) >= (UNSIGNED)EndSTKP) {
    LispPTR test;
    test = *((LispPTR *)CurrentStackPTR);
    /* DOSTACKOVERFLOW(argnum,bytenum-1); XXX until we figure out what should be happening */
    S_CHECK(test == *((LispPTR *)CurrentStackPTR), "overflow in ccfuncall");
  }
  FuncObj = tmp_fn;

  if (FuncObj->na >= 0) {
    /* This Function is Spread Type */
    /* Arguments on Stack Adjustment  */
    rest = argnum - FuncObj->na;

    while (rest < 0) {
      PushStack(NIL_PTR);
      rest++;
    }
    CurrentStackPTR -= (rest << 1);
  } /* if end */

  /* Set up BF */
  CurrentStackPTR += 2;
  GETWORD(CurrentStackPTR) = BF_MARK;
  GETWORD(CurrentStackPTR + 1) = CURRENTFX->nextblock;
  CurrentStackPTR += 2;

  /* Set up FX */
  GETWORD(CurrentStackPTR) = FX_MARK;

  /* Now SET new FX */
  ((struct frameex1 *)CurrentStackPTR)->alink = LAddrFromNative(PVar);
  PVar = (DLword *)CurrentStackPTR + FRAMESIZE;
#ifdef BIGVM
  ((struct frameex1 *)CurrentStackPTR)->fnheader = (defcell68k->defpointer);
#else
  ((struct frameex1 *)CurrentStackPTR)->lofnheader = (defcell68k->defpointer) & 0x0ffff;
  ((struct frameex1 *)CurrentStackPTR)->hi2fnheader = ((defcell68k->defpointer) & SEGMASK) >> 16;
  ((struct frameex1 *)CurrentStackPTR)->hi1fnheader = 0;
#endif /* BIGVM */

  CurrentStackPTR = PVar;

  /* Set up PVar area */
  pv_num = FuncObj->pv + 1; /* Changed Apr.27 */

  while (pv_num > 0) {
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    *((LispPTR *)CurrentStackPTR) = 0x0ffff0000;
    CurrentStackPTR += DLWORDSPER_CELL;
    pv_num--;
  }

  /* Set PC points New Function's first OPCODE */
  PC = (ByteCode *)FuncObj + FuncObj->startpc;

  return -2; /* signal to OP_miscn to leave stack & pc alone */
} /* end lcfuncall */
