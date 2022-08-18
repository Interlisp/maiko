#ifndef LOOPSOPSDEFS_H
#define LOOPSOPSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR lcfuncall(unsigned int atom_index, int argnum, int bytenum);
LispPTR LCinit(void);
LispPTR LCFetchMethod(LispPTR class, LispPTR selector);
LispPTR LCFetchMethodOrHelp(LispPTR object, LispPTR selector);
LispPTR LCFindVarIndex(LispPTR iv, LispPTR object);
LispPTR LCGetIVValue(LispPTR object, LispPTR iv);
LispPTR LCPutIVValue(LispPTR object, LispPTR iv, LispPTR val);
LispPTR lcfuncall(unsigned int atom_index, int argnum, int bytenum);
#endif
