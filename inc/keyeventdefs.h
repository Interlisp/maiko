#ifndef KEYEVENTDEFS_H
#define KEYEVENTDEFS_H 1
#include <sys/types.h>
void process_io_events(void);
void kb_trans(u_short keycode, u_short upflg);
void taking_mouse_down(void);
void taking_mouse_up(int newx, int newy);
void copy_cursor(int newx, int newy);
void cursor_hidden_bitmap(int x, int y);
#endif
