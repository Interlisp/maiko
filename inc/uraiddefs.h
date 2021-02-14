#ifndef URAIDDEFS_H
#define URAIDDEFS_H 1
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR parse_atomstring(char *string);
void uraid_commclear(void);
void copy_region(const DLword *src, DLword *dst, int width, int h);
struct dtd *uGetDTD(unsigned int typenum);
unsigned int uGetTN(unsigned int address);
LispPTR uraid_commands(void);
int device_before_raid(void);
int device_after_raid(void);
#endif
