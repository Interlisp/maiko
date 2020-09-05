#ifndef DSPSUBRSDEFS_H
#define DSPSUBRSDEFS_H 1
void DSP_dspbout(LispPTR *args);
void DSP_showdisplay(LispPTR *args);
LispPTR DSP_VideoColor(LispPTR *args);
void DSP_Cursor(LispPTR *args, int argnum);
void DSP_SetMousePos(register LispPTR *args);
LispPTR DSP_ScreenWidth(LispPTR *args);
LispPTR DSP_ScreenHight(LispPTR *args);
void flip_cursor();
#endif
