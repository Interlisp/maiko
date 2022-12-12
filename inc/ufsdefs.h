#ifndef UFSDEFS_H
#define UFSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR UFS_getfilename(LispPTR *args);
LispPTR UFS_deletefile(LispPTR *args);
LispPTR UFS_renamefile(LispPTR *args);
LispPTR UFS_directorynamep(LispPTR *args);
#ifdef DOS
int unixpathname(char *src, char *dst, int versionp, int genp, char *drive, int *extlenptr, char *rawname);
#else
int unixpathname(char *src, char *dst, int versionp, int genp);
#endif
int lisppathname(char *fullname, char *lispname, int dirp, int versionp);
int quote_fname(char *file);
int quote_fname_ufs(char *file);
int quote_dname(char *dir);
#ifdef DOS
init_host_filesystem(void);
exit_host_filesystem(void);
#endif
#endif
