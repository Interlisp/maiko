#ifndef VMEMSAVEDEFS_H
#define VMEMSAVEDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR vmem_save(char *sysout_file_name);
LispPTR vmem_save0(LispPTR *args);
void lisp_finish(int exit_status);
#endif
