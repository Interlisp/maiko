#ifndef MKATOMDEFS_H
#define MKATOMDEFS_H 1
DLword compute_hash(const char *char_base, DLword offset, DLword length);
DLword compute_lisp_hash(const char *char_base, DLword offset, DLword length, DLword fatp);
LispPTR compare_chars(register const char *char1, register const char *char2, register DLword length);
int bytecmp(const char *char1, const char *char2, int len);
LispPTR compare_lisp_chars(register const char *char1, register const char *char2, register DLword length, DLword fat1, DLword fat2);
int lispcmp(const DLword *char1, const char *char2, int len);
LispPTR make_atom(const char *char_base, DLword offset, DLword length, short int non_numericp);
LispPTR parse_number(const char *char_base, short int length);
#endif
