#ifndef STORAGEDEFS_H
#define STORAGEDEFS_H 1
void checkfor_storagefull(register unsigned int npages);
LispPTR dremove(LispPTR x, LispPTR l);
LispPTR newpage(LispPTR base);
void init_storage(void);
#endif
