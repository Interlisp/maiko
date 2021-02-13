#ifndef KPRINTDEFS_H
#define KPRINTDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void prindatum(LispPTR x);
LispPTR print(LispPTR x);
void print_NEWstring(LispPTR x);
void print_fixp(LispPTR x);
void print_floatp(LispPTR x);
void print_string(LispPTR x);
#endif
