#ifndef UFSDEFS_H
#define UFSDEFS_H 1
#include "lispemul.h" /* for LispPTR */
LispPTR UFS_getfilename(LispPTR *args);
LispPTR UFS_deletefile(LispPTR *args);
LispPTR UFS_renamefile(LispPTR *args);
LispPTR UFS_directorynamep(LispPTR *args);
void  UnixVersionToLispVersion(char *pathname, size_t pathsize, int vlessp);
#ifdef DOS
int unixpathname(char *src, char *dst, int dstlen, int versionp, int genp, char *drive, int *extlenptr, char *rawname);
#else
int unixpathname(char *src, char *dst, size_t dstlen, int versionp, int genp);
#endif
int lisppathname(char *fullname, char *lispname, size_t lispnamesize, int dirp, int versionp);
#ifdef DOS
init_host_filesystem(void);
exit_host_filesystem(void);
#endif
#endif
