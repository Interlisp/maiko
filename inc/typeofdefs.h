#ifndef TYPEOFDEFS_H
#define TYPEOFDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_dtest(register LispPTR tos, register int atom_index);
LispPTR N_OP_instancep(register LispPTR tos, register int atom_index);
#endif
