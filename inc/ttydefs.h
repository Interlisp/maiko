#ifndef TTYDEFS_H
#define TTYDEFS_H 1
void tty_init(void);
void tty_open(void);
void tty_close(void);
void tty_get(void);
void tty_put(void);
void tty_setbaudrate(void);
void tty_setparam(void);
void tty_breakon(void);
void tty_breakoff(void);
void tty_cmd(void);
void tty_debug(const char *name);
#endif
