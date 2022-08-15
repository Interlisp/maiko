#ifndef MKATOMDEFS_H
#define MKATOMDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
DLword compute_hash(const char *char_base, DLword offset, DLword length);
DLword compute_lisp_hash(const char *char_base, DLword offset, DLword length, DLword fatp);
LispPTR compare_chars(const char *char1, const char *char2, DLword length);
LispPTR compare_lisp_chars(const char *char1, const char *char2, DLword length, DLword fat1, DLword fat2);
LispPTR make_atom(const char *char_base, DLword offset, DLword length);
#endif
