#ifndef XINITDEFS_H
#define XINITDEFS_H 1
#include "devif.h" /* for DspInterface */
void init_Xevent(DspInterface dsp);
void lisp_Xexit(DspInterface dsp);
void Xevent_before_raid(DspInterface dsp);
void Xevent_after_raid(DspInterface dsp);
void Open_Display(DspInterface dsp);
DspInterface X_init(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint, int depth_hint);
#endif
