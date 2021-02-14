#ifndef GCRCELLDEFS_H
#define GCRCELLDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void freelistcell(LispPTR cell);
LispPTR gcreccell(LispPTR cell);
void freelistcell(LispPTR cell);
#endif
