#ifndef TESTTOOLDEFS_H
#define TESTTOOLDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword, DLbyte */
#include "cell.h" /* for conspage */
#include "stack.h" /* for frameex1, FX, Bframe, fnhead */
void print_package_name(int index);
void print_atomname(LispPTR index);
int find_package_from_name(const char *packname, int len);
void print_package_name(int index);
void dump_dtd(void);
void check_type_68k(int type, LispPTR *ptr);
int type_num(LispPTR lispptr);
void dump_conspage(struct conspage *base, int linking);
void trace_listpDTD(void);
void a68k(LispPTR lispptr);
void laddr(DLword *addr68k);
void dump_fnbody(LispPTR fnblockaddr);
void dump_fnobj(LispPTR index);
int print_opcode(int pc, DLbyte *addr, struct fnhead *fnobj);
void doko(void);
void dumpl(LispPTR laddr);
void dumps(LispPTR laddr);
void printPC(void);
void dump_bf(Bframe *bf);
void dump_fx(struct frameex1 *fx_addr68k);
void dump_stackframe(struct frameex1 *fx_addr68k);
void dump_CSTK(int before);
void btv(void);
int get_framename(struct frameex1 *fx_addr68k);
FX *get_nextFX(FX *fx);
LispPTR MAKEATOM(char *string);
LispPTR *MakeAtom68k(char *string);
void GETTOPVAL(char *string);
void all_stack_dump(DLword start, DLword end, DLword silent);
void dtd_chain(DLword type);
void check_dtd_chain(DLword type);
void Trace_FNCall(int numargs, int atomindex, int arg1, LispPTR *tos);
void Trace_APPLY(int atomindex);
#endif
