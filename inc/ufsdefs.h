#ifndef UFSDEFS_H
#define UFSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR UFS_getfilename(LispPTR *args);
LispPTR UFS_deletefile(LispPTR *args);
LispPTR UFS_renamefile(LispPTR *args);
LispPTR UFS_directorynamep(LispPTR *args);
int unixpathname(char *src, char *dst, int versionp, int genp);
int lisppathname(char *fullname, char *lispname, int dirp, int versionp);
int quote_fname(char *file);
int quote_fname_ufs(char *file);
int quote_dname(char *dir);
#endif
