#ifndef ETHERDEFS_H
#define ETHERDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR ether_suspend(LispPTR args[]);
LispPTR ether_resume(LispPTR args[]);
LispPTR ether_ctrlr(LispPTR args[]);
LispPTR ether_reset(LispPTR args[]);
LispPTR get_packet(void);
LispPTR ether_get(LispPTR args[]);
LispPTR ether_send(LispPTR args[]);
LispPTR ether_setfilter(LispPTR args[]);
int *ether_debug(void);
LispPTR check_ether(void);
void init_ifpage_ether(void);
void init_ether(void);
LispPTR check_sum(register LispPTR *args);
#endif
