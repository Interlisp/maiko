#ifndef LINEBLT8DEFS_H
#define LINEBLT8DEFS_H 1
#include <sys/types.h> /* for u_char */
#include "lispemul.h" /* for LispPTR, DLword */
void lineBlt8(DLword *srcbase, int offset, u_char *destl, int width,
              u_char color0, u_char color1, LispPTR sourcetype, LispPTR operation);
#endif
