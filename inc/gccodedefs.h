#ifndef GCCODEDEFS_H
#define GCCODEDEFS_H 1
#include <stdbool.h>  /* for bool */
#include "lispemul.h" /* for LispPTR */
bool reclaimcodeblock(LispPTR codebase);
int code_block_size(long unsigned int codeblock68k);
#endif
