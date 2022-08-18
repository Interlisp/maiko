#ifndef LISP2CDEFS_H
#define LISP2CDEFS_H 1
#include "lispemul.h" /* for LispPTR */
int LispStringP(LispPTR object);
int LispStringSimpleLength(LispPTR lispstring);
void LispStringToCStr(LispPTR lispstring, char *cstring);
int LispIntToCInt(LispPTR lispint);
LispPTR CIntToLispInt(int cint);
#endif
