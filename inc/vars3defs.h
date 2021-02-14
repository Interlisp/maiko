#ifndef VARS3DEFS_H
#define VARS3DEFS_H 1
#include "cell.h" /* for cadr_cell */
#include "lispemul.h" /* for LispPTR */
struct cadr_cell cadr(LispPTR cell_adr);
LispPTR N_OP_arg0(register LispPTR tos);
LispPTR N_OP_assoc(register LispPTR key, register LispPTR list);
#endif
