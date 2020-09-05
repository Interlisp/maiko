#ifndef CAR_CDRDEFS_H
#define CAR_CDRDEFS_H 1
#include "cell.h"
LispPTR car(register LispPTR datum);
LispPTR cdr(register LispPTR datum);
LispPTR rplaca(register LispPTR x, register LispPTR y);
LispPTR rplacd(LispPTR x, register LispPTR y);
LispPTR N_OP_car(register LispPTR tos);
LispPTR N_OP_cdr(register LispPTR tos);
LispPTR N_OP_rplaca(register LispPTR tosm1, register LispPTR tos);
LispPTR N_OP_rplacd(register LispPTR tosm1, register LispPTR tos);
ConsCell *find_close_prior_cell(struct conspage *page, LispPTR oldcell);
#endif
