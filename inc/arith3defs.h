#ifndef ARITH3DEFS_H
#define ARITH3DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_makenumber(int tosm1, int tos);
LispPTR N_OP_boxiplus(register int a, int tos);
LispPTR N_OP_boxidiff(register int a, int tos);
#endif
