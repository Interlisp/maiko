#ifndef DSKDEFS_H
#define DSKDEFS_H 1
#include "lispemul.h" /* for LispPTR */
#ifdef DOS
void separate_host(char *lfname, char *host, char *drive);
#else
void separate_host(char *lfname, char *host);
#endif
LispPTR COM_openfile(const LispPTR *args);
LispPTR COM_closefile(const LispPTR *args);
LispPTR DSK_getfilename(const LispPTR *args);
LispPTR DSK_deletefile(const LispPTR *args);
LispPTR DSK_renamefile(const LispPTR *args);
LispPTR DSK_directorynamep(const LispPTR *args);
LispPTR COM_getfileinfo(const LispPTR *args);
LispPTR COM_setfileinfo(const LispPTR *args);
LispPTR COM_readpage(const LispPTR *args);
LispPTR COM_writepage(const LispPTR *args);
LispPTR COM_truncatefile(const LispPTR *args);
LispPTR COM_changedir(const LispPTR *args);
LispPTR COM_getfreeblock(const LispPTR *args);
void conc_dir_and_name(const char *dir, const char *name, char *fname, size_t fname_size);
void conc_name_and_version(const char *name, const char *ver, char *rname, size_t rname_size);
void separate_version(const char *name, size_t namesize, char *ver, size_t versize, int checkp);
int unpack_filename(const char *file, char *dir, char *name, char *ver, int checkp);
int true_name(char *path, size_t pathsize);
#endif
