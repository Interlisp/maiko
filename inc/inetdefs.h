#ifndef INETDEFS_H
#define INETDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR subr_TCP_ops(int op, LispPTR nameConn, LispPTR proto, LispPTR length, LispPTR bufaddr, LispPTR maxlen);
#endif
