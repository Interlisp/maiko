#ifndef GCARRAYDEFS_H
#define GCARRAYDEFS_H 1
LispPTR aref1(LispPTR array, int index);
LispPTR find_symbol(char *char_base, DLword offset, DLword length, LispPTR hashtbl, DLword fatp, DLword lispp);
LispPTR get_package_atom(char *char_base, DLword charlen, char *packname, DLword packlen, int externalp);
LispPTR with_symbol(LispPTR char_base, LispPTR offset, LispPTR charlen, LispPTR fatp, LispPTR hashtbl, LispPTR result);
#endif
