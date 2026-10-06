#ifndef CHARDEVDEFS_H
#define CHARDEVDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR CHAR_openfile(LispPTR *args);
LispPTR CHAR_closefile(LispPTR *args);
LispPTR CHAR_ioctl(LispPTR *args);
LispPTR CHAR_bin(int fd, LispPTR errn);
LispPTR CHAR_bout(int fd, LispPTR ch, LispPTR errn);
LispPTR CHAR_bins(LispPTR *args);
LispPTR CHAR_bouts(LispPTR *args);
#endif
