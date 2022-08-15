#ifndef LSTHANDLDEFS_H
#define LSTHANDLDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_fmemb(LispPTR item, LispPTR tos);
LispPTR fmemb(LispPTR item, LispPTR list);
LispPTR N_OP_listget(LispPTR plist, LispPTR tos);
#endif
