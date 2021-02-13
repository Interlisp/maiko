#ifndef KBDSUBRSDEFS_H
#define KBDSUBRSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void KB_enable(LispPTR *args);
void KB_beep(LispPTR *args);
void KB_setmp(LispPTR *args);
void KB_setled(LispPTR *args);
#endif
