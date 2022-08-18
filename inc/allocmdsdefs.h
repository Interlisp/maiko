#ifndef ALLOCMDSDEFS_H
#define ALLOCMDSDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR initmdspage(LispPTR *base, DLword size, LispPTR prev);
LispPTR *alloc_mdspage(short int type);
#endif
