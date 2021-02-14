#ifndef XBBTDEFS_H
#define XBBTDEFS_H 1
#include "devif.h" /* for DspInterface */
#include "lispemul.h" /* for DLword */
unsigned long clipping_Xbitblt(DspInterface dsp, DLword *dummy, int x, int y, int w, int h);
#endif
