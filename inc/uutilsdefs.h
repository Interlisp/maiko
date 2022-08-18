#ifndef UUTILSDEFS_H
#define UUTILSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR check_unix_password(LispPTR *args);
LispPTR unix_username(LispPTR *args);
LispPTR unix_getparm(LispPTR *args);
LispPTR unix_getenv(LispPTR *args);
LispPTR unix_fullname(LispPTR *args);
LispPTR suspend_lisp(LispPTR *args);
#endif
