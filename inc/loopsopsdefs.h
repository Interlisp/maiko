#ifndef LOOPSOPSDEFS_H
#define LOOPSOPSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR lcfuncall(register unsigned int atom_index, register int argnum, register int bytenum);
LispPTR LCinit(void);
LispPTR LCFetchMethod(register LispPTR class, register LispPTR selector);
LispPTR LCFetchMethodOrHelp(register LispPTR object, register LispPTR selector);
LispPTR LCFindVarIndex(register LispPTR iv, register LispPTR object);
LispPTR LCGetIVValue(register LispPTR object, register LispPTR iv);
LispPTR LCPutIVValue(register LispPTR object, register LispPTR iv, register LispPTR val);
LispPTR lcfuncall(register unsigned int atom_index, register int argnum, register int bytenum);
#endif
