#ifndef SUBRDEFS_H
#define SUBRDEFS_H 1
#include "lispemul.h" /* for LispPTR */
char *atom_to_str(LispPTR atom_index);
void OP_subrcall(int subr_no, int argnum);
#endif
