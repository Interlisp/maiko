#ifndef LOWLEV1DEFS_H
#define LOWLEV1DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_putbitsnfd(register LispPTR base, register LispPTR data, int word_offset, register int beta);
LispPTR N_OP_getbitsnfd(int base_addr, register int word_offset, int beta);
LispPTR N_OP_putbasen(register LispPTR base, register LispPTR tos, int n);
LispPTR N_OP_putbaseptrn(register LispPTR base, register LispPTR tos, int n);
#endif
