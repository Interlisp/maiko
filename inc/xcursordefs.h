#ifndef XCURSORDEFS_H
#define XCURSORDEFS_H 1
#include <X11/Xlib.h>
#include "devif.h"
void Init_XCursor(void);
void Set_XCursor(int x, int y);
void init_Xcursor(Display *display, Window window);
void set_Xcursor(DspInterface dsp, unsigned char *bitmap, int hotspot_x, int hotspot_y, Cursor *return_cursor, int from_lisp);
#endif
