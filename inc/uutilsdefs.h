#ifndef UUTILSDEFS_H
#define UUTILSDEFS_H 1
int lisp_string_to_c_string(LispPTR Lisp, char *C, size_t length);
int c_string_to_lisp_string(char *C, LispPTR Lisp);
LispPTR check_unix_password(LispPTR *args);
LispPTR unix_username(LispPTR *args);
LispPTR unix_getparm(LispPTR *args);
LispPTR unix_getenv(LispPTR *args);
LispPTR unix_fullname(LispPTR *args);
LispPTR suspend_lisp(LispPTR *args);
#endif
