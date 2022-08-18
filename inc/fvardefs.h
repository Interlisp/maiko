#ifndef FVARDEFS_H
#define FVARDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
#include "stack.h" /* for fnhead, frameex1 */
LispPTR N_OP_fvarn(int n);
LispPTR N_OP_stkscan(LispPTR tos);
LispPTR N_OP_fvar_(LispPTR tos, int n);
void nnewframe(struct frameex1 *newpfra2, DLword *achain, int name);
void nfvlookup(struct frameex1 *apframe1, DLword *achain, struct fnhead *apfnhead1);
LispPTR native_newframe(int slot);
#endif
