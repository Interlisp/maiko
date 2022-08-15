#ifndef DSPSUBRSDEFS_H
#define DSPSUBRSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void DSP_dspbout(LispPTR *args);
void DSP_showdisplay(LispPTR *args);
LispPTR DSP_VideoColor(LispPTR *args);
void DSP_Cursor(LispPTR *args, int argnum);
void DSP_SetMousePos(LispPTR *args);
LispPTR DSP_ScreenWidth(LispPTR *args);
LispPTR DSP_ScreenHight(LispPTR *args);
void flip_cursor(void);
#endif
