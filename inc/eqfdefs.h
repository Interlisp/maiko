#ifndef EQFDEFS_H
#define EQFDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_clequal(int arg1, int arg2);
LispPTR N_OP_eqlop(int arg1, int arg2);
LispPTR N_OP_equal(int arg1, int arg2);
LispPTR N_OP_eqq(int arg1, int arg2);
#endif
