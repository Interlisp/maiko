/***** Don't use PixRect code on 386i for now *****/
/***** -or on any machine that doesn't support it (HP, e.g.) *****/

/********************************************************/
/*                                                      */
/*      prropstyle is DEFINED when we want to use       */
/*      pixrect versions of the operations in this      */
/*      file, and UNDEFINED, when we want to use        */
/*      Don Charnley's bitblt code to do them.          */
/*                                                      */
/********************************************************/
#if (!(defined(NOPIXRECT)) && !(defined(NEWBITBLT)) && !(defined(I386)))
#define prropstyle 1
#endif /* NOPIXRECT */

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
void ccfuncall(unsigned int atom_index, int argnum, int bytenum);
void tedit_bltchar(LispPTR *args);
