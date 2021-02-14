#ifndef Z2DEFS_H
#define Z2DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_classoc(LispPTR key, LispPTR list);
LispPTR N_OP_clfmemb(register LispPTR item, register LispPTR list);
LispPTR N_OP_restlist(register LispPTR tail, register int last, register int skip);
#endif
