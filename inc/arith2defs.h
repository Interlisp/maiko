#ifndef ARITH2DEFS_H
#define ARITH2DEFS_H 1
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
#endif
