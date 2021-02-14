#ifndef GVAR2DEFS_H
#define GVAR2DEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_gvar_(register LispPTR tos, unsigned int atom_index);
LispPTR N_OP_rplptr(register LispPTR tos_m_1, register LispPTR tos, unsigned int alpha);
#endif
