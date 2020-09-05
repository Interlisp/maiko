#ifndef XSCROLLDEFS_H
#define XSCROLLDEFS_H 1
void Scroll(DspInterface dsp, int newX, int newY);
void JumpScrollVer(DspInterface dsp, int y);
void JumpScrollHor(DspInterface dsp, int x);
void ScrollLeft(DspInterface dsp);
void ScrollRight(DspInterface dsp);
void ScrollUp(DspInterface dsp);
void ScrollDown(DspInterface dsp);
#endif
