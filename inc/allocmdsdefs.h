#ifndef ALLOCMDSDEFS_H
#define ALLOCMDSDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR initmdspage(register LispPTR *base, register DLword size, register LispPTR prev);
LispPTR *alloc_mdspage(register short int type);
#endif
