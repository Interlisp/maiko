#ifndef CONSPAGEDEFS_H
#define CONSPAGEDEFS_H 1
#include "lispemul.h" /* for LispPTR */
struct conspage *next_conspage(void);
LispPTR N_OP_cons(register int cons_car, register int cons_cdr);
LispPTR cons(LispPTR cons_car, LispPTR cons_cdr);
#endif
