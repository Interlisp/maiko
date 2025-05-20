#ifndef DSKDEFS_H
#define DSKDEFS_H 1
#include "lispemul.h" /* for LispPTR */
#ifdef DOS
void separate_host(char *lfname, char *host, char *drive);
#else
void separate_host(char *lfname, char *host);
#endif
LispPTR COM_openfile(LispPTR *args);
LispPTR COM_closefile(LispPTR *args);
LispPTR DSK_getfilename(LispPTR *args);
LispPTR DSK_deletefile(LispPTR *args);
LispPTR DSK_renamefile(LispPTR *args);
LispPTR DSK_directorynamep(LispPTR *args);
LispPTR COM_getfileinfo(LispPTR *args);
LispPTR COM_setfileinfo(LispPTR *args);
LispPTR COM_readpage(LispPTR *args);
LispPTR COM_writepage(LispPTR *args);
LispPTR COM_truncatefile(LispPTR *args);
LispPTR COM_changedir(LispPTR *args);
LispPTR COM_getfreeblock(LispPTR *args);
void conc_dir_and_name(char *dir, char *name, char *fname, size_t fname_size);
void conc_name_and_version(char *name, char *ver, char *rname, size_t rname_size);
void separate_version(char *name, size_t namesize, char *ver, size_t versize, int checkp);
int unpack_filename(char *file, char *dir, char *name, char *ver, int checkp);
int true_name(char *path, size_t pathsize);
#endif
