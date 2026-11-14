#ifndef BITBLTDEFS_H
#define BITBLTDEFS_H 1
#include <stdbool.h>  /* for bool */
#include "lispemul.h" /* for LispPTR, DLword */
LispPTR N_OP_pilotbitblt(LispPTR pilot_bt_tbl, LispPTR tos);
bool cursorin(DLword addrhi, DLword addrlo, int w, int h, int backward);
#endif

