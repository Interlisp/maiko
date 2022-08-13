#ifndef LOWLEV2DEFS_H
#define LOWLEV2DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_addbase(LispPTR base, LispPTR offset);
LispPTR N_OP_getbasebyte(LispPTR base_addr, LispPTR byteoffset);
LispPTR N_OP_putbasebyte(LispPTR base_addr, LispPTR byteoffset, LispPTR tos);
#endif
