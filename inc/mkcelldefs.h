#ifndef MKCELLDEFS_H
#define MKCELLDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR N_OP_createcell(LispPTR tos);
void *createcell68k(unsigned int type);
#endif
