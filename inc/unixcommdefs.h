#ifndef UNIXCOMMDEFS_H
#define UNIXCOMMDEFS_H 1
#include "lispemul.h" /* for LispPTR */
int find_process_slot(int pid);
void wait_for_comm_processes(void);
char *build_socket_pathname(int desc);
void close_unix_descriptors(void);
int FindUnixPipes(void);
void WriteLispStringToPipe(LispPTR lispstr);
LispPTR Unix_handlecomm(LispPTR *args);
void WriteLispStringToPipe(LispPTR lispstr);
#endif
