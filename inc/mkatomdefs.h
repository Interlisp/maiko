#ifndef MKATOMDEFS_H
#define MKATOMDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
#include <stdbool.h>  /* for bool */
DLword compute_hash(const char *char_base, DLword offset, DLword length);
DLword compute_lisp_hash(const char *char_base, DLword offset, DLword length, DLword fatp);
bool compare_chars(const char *lisp_string, const char *native_string, DLword length);
bool compare_lisp_chars(const char *char1, const char *char2, DLword length, DLword fat1, DLword fat2);
LispPTR make_atom(const char *char_base, DLword offset, DLword length);
#endif
