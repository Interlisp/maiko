#ifndef BBTSUBDEFS_H
#define BBTSUBDEFS_H 1
void bitbltsub(LispPTR *argv);
LispPTR n_new_cursorin(DLword *baseaddr, int dx, int dy, int w, int h);
LispPTR bitblt_bitmap(LispPTR *args);
LispPTR bitshade_bitmap(LispPTR *args);
#ifndef prropstyle
void bltchar(LispPTR *args);
void newbltchar(LispPTR *args);
#else
LispPTR bltchar(LispPTR *args);
LispPTR newbltchar(LispPTR *args);
#endif
void ccfuncall(register unsigned int atom_index, register int argnum, register int bytenum);
void tedit_bltchar(LispPTR *args);
#endif
