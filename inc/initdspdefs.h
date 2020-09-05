#ifndef INITDSPDEFS_H
#define INITDSPDEFS_H 1
void init_cursor();
void set_cursor();
void clear_display();
void init_display2(DLword *display_addr, int display_max);
void display_before_exit();
void flush_display_buffer();
void flush_display_region(int x, int y, int w, int h);
void byte_swapped_displayregion(int x, int y, int w, int h);
void flush_display_lineregion(UNSIGNED x, DLword *ybase, UNSIGNED w, UNSIGNED h);
void flush_display_ptrregion(DLword *ybase, UNSIGNED bitoffset, UNSIGNED w, UNSIGNED h);
#endif
