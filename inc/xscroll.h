#ifndef XSCROLL_H
#define XSCROLL_H 1
/************************************************************************/
/*									*/
/*			xscroll.h					*/
/*									*/
/* Scrolling functions implemented in xscroll.c				*/
/*									*/
/************************************************************************/
#include "devif.h" /* for DspInterface */
void Scroll(DspInterface dsp, int newX, int newY);
void JumpScrollVer(DspInterface dsp, int y);
void JumpScrollHor(DspInterface dsp, int x);
void ScrollLeft(DspInterface dsp);
void ScrollRight(DspInterface dsp);
void ScrollUp(DspInterface dsp);
void ScrollDown(DspInterface dsp);
#endif /* XSCROLL_H */
