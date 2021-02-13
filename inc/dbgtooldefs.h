#ifndef DBGTOOLDEFS_H
#define DBGTOOLDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
#include "stack.h" /* frameex1, fnhead, FX */
LispPTR get_ivar_name(struct frameex1 *fx_addr68k, DLword offset, int *localivar);
LispPTR get_pvar_name(struct frameex1 *fx_addr68k, DLword offset);
LispPTR get_fn_fvar_name(struct fnhead *fnobj, DLword offset);
LispPTR get_fvar_name(struct frameex1 *fx_addr68k, DLword offset);
void bt(void);
void bt1(FX *startFX);
void btvv(void);
int sf(struct frameex1 *fx_addr68k);
void sff(LispPTR laddr);
void nt(LispPTR index);
void nt1(LispPTR *start, int size, char *str);
void ntheader(struct fnhead *fnobj);
void nts(struct frameex1 *fxp);
#endif
