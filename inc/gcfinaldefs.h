#ifndef GCFINALDEFS_H
#define GCFINALDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
void printarrayblock(LispPTR base);
int integerlength(unsigned int n);
LispPTR findptrsbuffer(LispPTR ptr);
LispPTR releasingvmempage(LispPTR ptr);
LispPTR checkarrayblock(LispPTR base, LispPTR free, LispPTR onfreelist);
LispPTR deleteblock(LispPTR base);
LispPTR linkblock(LispPTR base);
LispPTR makefreearrayblock(LispPTR block, DLword length);
LispPTR arrayblockmerger(LispPTR base, LispPTR nbase);
LispPTR mergebackward(LispPTR base);
LispPTR mergeforward(LispPTR base);
LispPTR reclaimarrayblock(LispPTR ptr);
LispPTR reclaimstackp(LispPTR ptr);
#endif


