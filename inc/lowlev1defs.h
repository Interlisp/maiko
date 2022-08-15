#ifndef LOWLEV1DEFS_H
#define LOWLEV1DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_putbitsnfd(LispPTR base, LispPTR data, int word_offset, int beta);
LispPTR N_OP_getbitsnfd(int base_addr, int word_offset, int beta);
LispPTR N_OP_putbasen(LispPTR base, LispPTR tos, int n);
LispPTR N_OP_putbaseptrn(LispPTR base, LispPTR tos, int n);
#endif
