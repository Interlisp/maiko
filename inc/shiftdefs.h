#ifndef SHIFTDEFS_H
#define SHIFTDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_llsh1(LispPTR a);
LispPTR N_OP_llsh8(LispPTR a);
LispPTR N_OP_lrsh1(LispPTR a);
LispPTR N_OP_lrsh8(LispPTR a);
LispPTR N_OP_lsh(LispPTR a, LispPTR b);
#endif
