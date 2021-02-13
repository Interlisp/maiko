#ifndef TIMERDEFS_H
#define TIMERDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void update_miscstats(void);
void init_miscstats(void);
LispPTR subr_gettime(LispPTR args[]);
void subr_settime(LispPTR args[]);
void subr_copytimestats(LispPTR args[]);
LispPTR N_OP_rclk(LispPTR tos);
void update_timer(void);
void int_io_open(int fd);
void int_io_close(int fd);
void int_block(void);
void int_unblock(void);
void int_init(void);
#endif
