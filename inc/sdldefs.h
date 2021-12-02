#ifndef SDLDEFS_H
#define SDLDEFS_H 1

void sdl_notify_damage(int x, int y, int w, int h);
void sdl_setCursor(int hot_x, int hot_y);
void sdl_bitblt_to_screen(int x, int y, int w, int h);
void sdl_set_invert(int flag);
void sdl_setMousePosition(int x, int y);
void process_SDLevents();
int init_SDL(char *windowtitle, int w, int h, int s);
#endif
