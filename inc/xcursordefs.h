#ifndef XCURSORDEFS_H
#define XCURSORDEFS_H 1
#include <X11/X.h>     /* for Cursor, Window */
#include <X11/Xlib.h>  /* for Display */
#include <stdint.h>    /* for uint8_t */
#include "devif.h"     /* for DspInterface */
void Init_XCursor(void);
void Set_XCursor(int x, int y);
void init_Xcursor(DspInterface dsp);
void set_Xcursor(DspInterface dsp, const uint8_t *bitmap, int hotspot_x, int hotspot_y, Cursor *return_cursor, int from_lisp);
#endif
