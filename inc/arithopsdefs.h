#ifndef ARITHOPSDEFS_H
#define ARITHOPSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_plus2(int tosm1, int tos);
LispPTR N_OP_iplus2(int tosm1, int tos);
LispPTR N_OP_difference(int tosm1, int tos);
LispPTR N_OP_idifference(int tosm1, int tos);
LispPTR N_OP_logxor(int tosm1, int tos);
LispPTR N_OP_logand(int tosm1, int tos);
LispPTR N_OP_logor(int tosm1, int tos);
LispPTR N_OP_greaterp(int tosm1, int tos);
LispPTR N_OP_igreaterp(int tosm1, int tos);
LispPTR N_OP_iplusn(int tos, int n);
LispPTR N_OP_idifferencen(int tos, int n);
LispPTR N_OP_makenumber(int tosm1, int tos);
LispPTR N_OP_boxiplus(int a, int tos);
LispPTR N_OP_boxidiff(int a, int tos);
LispPTR N_OP_times2(int tosm1, int tos);
LispPTR N_OP_itimes2(int tosm1, int tos);
LispPTR N_OP_quot(int tosm1, int tos);
LispPTR N_OP_iquot(int tosm1, int tos);
LispPTR N_OP_iremainder(int tosm1, int tos);
#endif
