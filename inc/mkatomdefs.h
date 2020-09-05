#ifndef MKATOMDEFS_H
#define MKATOMDEFS_H 1
DLword compute_hash(char *char_base, DLword offset, DLword length);
DLword compute_lisp_hash(char *char_base, DLword offset, DLword length, DLword fatp);
LispPTR compare_chars(register char *char1, register char *char2, register DLword length);
int bytecmp(char *char1, char *char2, int len);
LispPTR compare_lisp_chars(register char *char1, register char *char2, register DLword length, DLword fat1, DLword fat2);
int lispcmp(DLword *char1, unsigned char *char2, int len);
LispPTR make_atom(char *char_base, DLword offset, DLword length, short int non_numericp);
LispPTR parse_number(char *char_base, short int length);
#endif
