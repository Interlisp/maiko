#ifndef BBTSUBDEFS_H
#define BBTSUBDEFS_H 1

/********************************************************/
/*                                                      */
/*      Don Charnley's bitblt code                      */
/*                                                      */
/********************************************************/
#include "lispemul.h" /* for LispPTR, DLword */

void bitbltsub(LispPTR *argv);
LispPTR n_new_cursorin(const DLword *baseaddr, int dx, int dy, int w, int h);
LispPTR bitblt_bitmap(LispPTR *args);
LispPTR bltshade_bitmap(LispPTR *args);
void bltchar(LispPTR *args);
void newbltchar(LispPTR *args);
void ccfuncall(unsigned int atom_index, int argnum, int bytenum);
void tedit_bltchar(const LispPTR *args);

#endif
