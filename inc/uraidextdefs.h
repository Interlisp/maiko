#ifndef URAIDEXTDEFS_H
#define URAIDEXTDEFS_H 1

#include "stack.h" /* for FX */
#include "lispemul.h" /* for LispPTR */

#define URMAXFXNUM 2000
#define URMAXCOMM 512
#define URSCAN_ALINK 0
#define URSCAN_CLINK 1

extern FX *URaid_FXarray[URMAXFXNUM];
extern char URaid_arg1[URMAXCOMM / 2];
extern char URaid_arg2[URMAXCOMM / 2];
extern char URaid_comm[2];
extern char URaid_inputstring[URMAXCOMM];
extern const char *URaid_errmess;
extern int URaid_ArrMAXIndex;
extern int URaid_argnum;
extern int URaid_currentFX;
extern int URaid_scanlink;
extern LispPTR Uraid_mess;
#endif
