#ifndef MKCELLDEFS_H
#define MKCELLDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR N_OP_createcell(register LispPTR tos);
DLword *createcell68k(unsigned int type);
LispPTR Create_n_Set_Cell(unsigned int type, LispPTR value);
#endif
