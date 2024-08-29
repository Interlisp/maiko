#ifndef GCHTFINDDEFS_H
#define GCHTFINDDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
#include "gcdata.h"   /* for GCENTRY */
void enter_big_reference_count(LispPTR ptr);
void modify_big_reference_count(GCENTRY *entry, DLword casep, LispPTR ptr);
LispPTR htfind(LispPTR ptr, int casep);
LispPTR rec_htfind(LispPTR ptr, int casep);
#endif
