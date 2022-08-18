#ifndef RETURNDEFS_H
#define RETURNDEFS_H 1
#include "lispemul.h" /* for DLword */
void OP_contextsw(void);
void contextsw(DLword fxnum, DLword bytenum, DLword flags);
#endif
