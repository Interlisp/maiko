#ifndef BINDSDEFS_H
#define BINDSDEFS_H 1
LispPTR *N_OP_bind(register LispPTR *stack_pointer, register LispPTR tos, int byte1, int byte2);
LispPTR *N_OP_unbind(register LispPTR *stack_pointer);
LispPTR *N_OP_dunbind(register LispPTR *stack_pointer, register LispPTR tos);
#endif
