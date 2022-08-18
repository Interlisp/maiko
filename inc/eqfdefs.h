#ifndef EQFDEFS_H
#define EQFDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_clequal(LispPTR arg1, LispPTR arg2);
LispPTR N_OP_eqlop(LispPTR arg1, LispPTR arg2);
LispPTR N_OP_equal(LispPTR arg1, LispPTR arg2);
LispPTR N_OP_eqq(LispPTR arg1, LispPTR arg2);
#endif
