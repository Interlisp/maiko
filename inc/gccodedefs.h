#ifndef GCCODEDEFS_H
#define GCCODEDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR reclaimcodeblock(LispPTR codebase);
int code_block_size(long unsigned int codeblock68k);
#endif
