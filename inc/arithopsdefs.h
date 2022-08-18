#ifndef ARITHOPSDEFS_H
#define ARITHOPSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_plus2(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_iplus2(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_difference(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_idifference(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_logxor(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_logand(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_logor(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_greaterp(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_igreaterp(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_iplusn(LispPTR tos, int n);
LispPTR N_OP_idifferencen(LispPTR tos, int n);
LispPTR N_OP_makenumber(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_boxiplus(LispPTR a, LispPTR tos);
LispPTR N_OP_boxidiff(LispPTR a, LispPTR tos);
LispPTR N_OP_times2(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_itimes2(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_quot(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_iquot(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_iremainder(LispPTR tosm1, LispPTR tos);
#endif
