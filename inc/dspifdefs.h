#ifndef DSPIFDEFS_H
#define DSPIFDEFS_H 1
#include "devif.h"
void make_dsp_instance(DspInterface dsp, char *lispbitmap, int width_hint, int height_hint, int depth_hint);
unsigned long GenericReturnT(void);
void GenericPanic(DspInterface dsp);
LispPTR SwitchDisplay(LispPTR display);
#endif
