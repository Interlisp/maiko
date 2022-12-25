#ifndef BINDSDEFS_H
#define BINDSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR *N_OP_bind(LispPTR *stack_pointer, LispPTR tos, unsigned byte1, unsigned byte2);
LispPTR *N_OP_unbind(LispPTR *stack_pointer);
LispPTR *N_OP_dunbind(LispPTR *stack_pointer, LispPTR tos);
#endif
