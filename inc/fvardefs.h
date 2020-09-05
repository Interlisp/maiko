#ifndef FVARDEFS_H
#define FVARDEFS_H 1
LispPTR N_OP_fvarn(register int n);
LispPTR N_OP_stkscan(LispPTR tos);
LispPTR N_OP_fvar_(register LispPTR tos, register int n);
void nnewframe(register struct frameex1 *newpfra2, register DLword *achain, register int name);
void nfvlookup(struct frameex1 *apframe1, register DLword *achain, register struct fnhead *apfnhead1);
LispPTR native_newframe(int slot);
#endif
