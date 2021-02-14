#ifndef LLCOLORDEFS_H
#define LLCOLORDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR cgfour_init_color_display(LispPTR color_bitmapbase);
LispPTR cgfour_change_screen_mode(LispPTR which_screen);
LispPTR cgfour_set_colormap(LispPTR args[]);
#endif
