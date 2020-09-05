#ifndef LISP2CDEFS_H
#define LISP2CDEFS_H 1
int LispStringP(LispPTR object);
int LispStringLength(LispPTR lispstring);
void LispStringToCStr(LispPTR lispstring, char *cstring);
int LispIntToCInt(LispPTR lispint);
LispPTR CIntToLispInt(int cint);
#endif
