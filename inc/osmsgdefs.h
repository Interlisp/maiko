#ifndef OSMSGDEFS_H
#define OSMSGDEFS_H 1
void mess_init(void);
void mess_reset(void);
LispPTR mess_read(LispPTR *args);
LispPTR mess_readp(void);
LispPTR flush_pty(void);
#endif
