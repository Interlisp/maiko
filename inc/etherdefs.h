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
LispPTR check_sum(LispPTR *args);

void setNethubHost(char* host);
void setNethubPort(int port);
void setNethubMac(int m0, int m1, int m2, int m3, int m4, int m5);
void setNethubLogLevel(int ll);
void connectToHub(void);
#endif
