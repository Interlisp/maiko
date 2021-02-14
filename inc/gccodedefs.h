#ifndef GCCODEDEFS_H
#define GCCODEDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR map_code_pointers(LispPTR codeblock, short int casep);
LispPTR remimplicitkeyhash(LispPTR item, LispPTR ik_hash_table);
LispPTR reclaimcodeblock(LispPTR codebase);
int code_block_size(long unsigned int codeblock68k);
#endif
