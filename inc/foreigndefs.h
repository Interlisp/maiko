#ifndef FOREIGNDEFS_H
#define FOREIGNDEFS_H 1

LispPTR call_c_fn(LispPTR *args);
LispPTR smashing_c_fn(LispPTR *args);
int Mdld_link(LispPTR *args);
int Mdld_unlink_by_file(LispPTR *args);
int Mdld_unlink_by_symbol(LispPTR *args);
unsigned long Mdld_get_symbol(LispPTR *args);
unsigned long Mdld_get_func(LispPTR *args);
int Mdld_function_executable_p(LispPTR *args);
int Mdld_list_undefined_sym(void);
int c_malloc(LispPTR *args);
int c_free(LispPTR *args);
int put_c_basebyte(LispPTR *args);
int get_c_basebyte(LispPTR *args);

#endif /* FOREIGNDEFS_H */
