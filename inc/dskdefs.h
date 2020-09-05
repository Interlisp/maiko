#ifndef DSKDEFS_H
#define DSKDEFS_H 1
#ifdef DOS
void separate_host(char *lfname, char *host, char *drive);
#else
void separate_host(char *lfname, char *host);
#endif
LispPTR COM_openfile(register LispPTR *args);
LispPTR COM_closefile(register LispPTR *args);
LispPTR DSK_getfilename(register LispPTR *args);
LispPTR DSK_deletefile(register LispPTR *args);
LispPTR DSK_renamefile(register LispPTR *args);
LispPTR DSK_directorynamep(register LispPTR *args);
LispPTR COM_getfileinfo(register LispPTR *args);
LispPTR COM_setfileinfo(register LispPTR *args);
LispPTR COM_readpage(register LispPTR *args);
LispPTR COM_writepage(register LispPTR *args);
LispPTR COM_truncatefile(register LispPTR *args);
LispPTR COM_changedir(register LispPTR *args);
LispPTR COM_getfreeblock(register LispPTR *args);
void separate_version(char *name, char *ver, int checkp);
int unpack_filename(char *file, char *dir, char *name, char *ver, int checkp);
int true_name(register char *path);
#endif
