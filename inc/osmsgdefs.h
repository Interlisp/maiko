#ifndef OSMSGDEFS_H
#define OSMSGDEFS_H 1
void mess_init();
void mess_reset();
LispPTR mess_read(LispPTR *args);
LispPTR mess_readp();
LispPTR flush_pty();
#endif
