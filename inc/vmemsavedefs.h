#ifndef VMEMSAVEDEFS_H
#define VMEMSAVEDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
int lispstringP(LispPTR Lisp);
LispPTR vmem_save(char *sysout_file_name);
LispPTR vmem_save0(LispPTR *args);
void lisp_finish(void);
#endif
