#ifndef UNWINDDEFS_H
#define UNWINDDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR *N_OP_unwind(LispPTR *cstkptr, LispPTR tos, int n, int keep);
#endif
