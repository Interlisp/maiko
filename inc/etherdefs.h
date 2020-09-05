#ifndef ETHERDEFS_H
#define ETHERDEFS_H 1
LispPTR ether_suspend(LispPTR args[]);
LispPTR ether_resume(LispPTR args[]);
LispPTR ether_ctrlr(LispPTR args[]);
LispPTR ether_reset(LispPTR args[]);
LispPTR get_packet();
LispPTR ether_get(LispPTR args[]);
LispPTR ether_send(LispPTR args[]);
LispPTR ether_setfilter(LispPTR args[]);
int *ether_debug();
LispPTR check_ether();
void init_ifpage_ether();
void init_ether();
LispPTR check_sum(register LispPTR *args);
#endif
