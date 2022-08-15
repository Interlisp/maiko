#ifndef CAR_CDRDEFS_H
#define CAR_CDRDEFS_H 1
#include "cell.h" /* for ConsCell */
#include "lispemul.h" /* for LispPTR */
LispPTR car(LispPTR datum);
LispPTR cdr(LispPTR datum);
LispPTR rplaca(LispPTR x, LispPTR y);
LispPTR rplacd(LispPTR x, LispPTR y);
LispPTR N_OP_car(LispPTR tos);
LispPTR N_OP_cdr(LispPTR tos);
LispPTR N_OP_rplaca(LispPTR tosm1, LispPTR tos);
LispPTR N_OP_rplacd(LispPTR tosm1, LispPTR tos);
ConsCell *find_close_prior_cell(struct conspage *page, LispPTR oldcell);
#endif
