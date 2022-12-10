#ifndef INITDSPDEFS_H
#define INITDSPDEFS_H 1
#include "lispemul.h" /* for DLword */
#include "version.h" /* for UNSIGNED */
void init_cursor(void);
void set_cursor(void);
void clear_display(void);
void init_display2(DLword *display_addr, unsigned display_max);
void display_before_exit(void);
void flush_display_buffer(void);
void flush_display_region(int x, int y, int w, int h);
void byte_swapped_displayregion(int x, int y, int w, int h);
void flush_display_lineregion(UNSIGNED x, DLword *ybase, int w, int h);
void flush_display_ptrregion(DLword *ybase, UNSIGNED bitoffset, int w, int h);
#endif
