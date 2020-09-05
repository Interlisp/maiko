#ifndef URAIDDEFS_H
#define URAIDDEFS_H 1
LispPTR parse_atomstring(char *string);
void uraid_commclear();
void copy_region(DLword *src, DLword *dst, int width, int h);
LispPTR uraid_commands();
int device_before_raid();
int device_after_raid();
#endif
