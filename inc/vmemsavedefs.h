#ifndef VMEMSAVEDEFS_H
#define VMEMSAVEDEFS_H 1
int lispstringP(LispPTR Lisp);
LispPTR vmem_save(char *sysout_file_name);
LispPTR vmem_save0(LispPTR *args);
int twowords(const void *i, const void *j);
void sort_fptovp(DLword *fptovp, int size);
void lisp_finish(void);
#endif
