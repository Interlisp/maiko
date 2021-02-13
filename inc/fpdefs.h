#ifndef FPDEFS_H
#define FPDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_fplus2(LispPTR parg1, LispPTR parg2);
LispPTR N_OP_fdifference(LispPTR parg1, LispPTR parg2);
LispPTR N_OP_ftimes2(LispPTR parg1, LispPTR parg2);
LispPTR N_OP_fquotient(LispPTR parg1, LispPTR parg2);
LispPTR N_OP_fgreaterp(LispPTR parg1, LispPTR parg2);
#endif
