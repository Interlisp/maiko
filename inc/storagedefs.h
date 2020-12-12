#ifndef STORAGEDEFS_H
#define STORAGEDEFS_H 1
void checkfor_storagefull(register unsigned int npages);
LispPTR newpage(LispPTR base);
void init_storage(void);
#endif
