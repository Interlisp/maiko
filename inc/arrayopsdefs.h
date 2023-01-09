#ifndef ARRAYOPSDEFS_H
#define ARRAYOPSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR N_OP_misc3(LispPTR baseL, LispPTR typenumber, LispPTR inx, int alpha);
LispPTR N_OP_misc4(LispPTR data, LispPTR base, LispPTR typenumber, LispPTR inx, int alpha);
LispPTR N_OP_aref1(LispPTR arrayarg, LispPTR inx);
LispPTR N_OP_aset1(LispPTR data, LispPTR arrayarg, LispPTR inx);
LispPTR N_OP_aref2(LispPTR arrayarg, LispPTR inx0, LispPTR inx1);
LispPTR N_OP_aset2(LispPTR data, LispPTR arrayarg, LispPTR inx0, LispPTR inx1);
#endif
