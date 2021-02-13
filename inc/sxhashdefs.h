#ifndef SXHASHDEFS_H
#define SXHASHDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR SX_hash(LispPTR object);
LispPTR STRING_EQUAL_HASHBITS(LispPTR object);
LispPTR STRING_HASHBITS(LispPTR object);
#endif
