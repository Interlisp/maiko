#ifndef LOWLEV2DEFS_H
#define LOWLEV2DEFS_H 1
LispPTR N_OP_addbase(register int base, register int offset);
LispPTR N_OP_getbasebyte(register LispPTR base_addr, register int byteoffset);
LispPTR N_OP_putbasebyte(register LispPTR base_addr, register int byteoffset, register int tos);
#endif
