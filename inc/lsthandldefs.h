#ifndef LSTHANDLDEFS_H
#define LSTHANDLDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_fmemb(register LispPTR item, register LispPTR tos);
LispPTR fmemb(register LispPTR item, register LispPTR list);
LispPTR N_OP_listget(register LispPTR plist, register LispPTR tos);
#endif
