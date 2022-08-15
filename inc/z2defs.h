#ifndef Z2DEFS_H
#define Z2DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_classoc(LispPTR key, LispPTR list);
LispPTR N_OP_clfmemb(LispPTR item, LispPTR list);
LispPTR N_OP_restlist(LispPTR tail, int last, int skip);
#endif
