#ifndef STORAGEDEFS_H
#define STORAGEDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void checkfor_storagefull(unsigned int npages);
LispPTR newpage(LispPTR base);
void init_storage(void);
#endif
