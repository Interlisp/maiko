#ifndef GCFINALDEFS_H
#define GCFINALDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
void printarrayblock(LispPTR base);
void printfreeblockchainn(int arlen);
LispPTR releasingvmempage(LispPTR ptr);
LispPTR checkarrayblock(LispPTR base, LispPTR free, LispPTR onfreelist);
LispPTR makefreearrayblock(LispPTR block, DLword length);
LispPTR mergebackward(LispPTR base);
LispPTR mergeforward(LispPTR base);
LispPTR reclaimarrayblock(LispPTR ptr);
LispPTR reclaimstackp(LispPTR ptr);
#endif


