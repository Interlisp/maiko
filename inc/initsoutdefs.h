#ifndef INITSOUTDEFS_H
#define INITSOUTDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR *fixp_value(LispPTR *ptr);
void init_ifpage(unsigned sysout_size);
void init_iopage(void);
void build_lisp_map(void);
void init_for_keyhandle(void);
void init_for_bltchar(void);
void init_for_bitblt(void);
#endif
