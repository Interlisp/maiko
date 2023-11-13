/* $Id: dir.c,v 1.4 2001/12/26 22:17:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#ifndef DOS
#include <dirent.h>         // for closedir, MAXNAMLEN, dirent, readdir, ope...
#include <pwd.h>            // for getpwuid, passwd
#include <sys/param.h>      // for MAXPATHLEN
#else /* DOS, now */
#include <dos.h>
#define MAXPATHLEN _MAX_PATH
#define MAXNAMLEN _MAX_PATH
#define alarm(x) 1
#endif /* DOS */
#include <errno.h>          // for errno, EINTR, ENOENT
#include <stdio.h>          // for NULL, sprintf, size_t
#include <stdlib.h>         // for calloc, free, strtoul, malloc, qsort
#include <string.h>         // for strcpy, strcmp, strlen, strrchr, strcat
#include <sys/stat.h>       // for stat, S_ISDIR, st_atime, st_mtime
#include <sys/time.h>       // for timespec_t
#include "adr68k.h"         // for NativeAligned4FromLAddr
#include "arith.h"          // for GetSmallp
#include "dirdefs.h"        // for COM_finish_finfo, COM_gen_files, COM_next...
#include "dskdefs.h"        // for separate_version, separate_host, true_name
#include "lispemul.h"       // for LispPTR, NIL, ATOM_T
#include "locfile.h"        // for VERSIONLEN, DOWNCASE, ToLispTime, STRING_...
#include "lspglob.h"
#include "lsptypes.h"
#include "timeout.h"        // for S_TOUT, TIMEOUT0, TIMEOUT, ERRSETJMP
#include "ufsdefs.h"        // for quote_dname, quote_fname, quote_fname_ufs

extern int *Lisp_errno;
extern int Dummy_errno;

#define DIRCHAR '>'

/************************************************************************
        SUBROUTINES
        For pattern matching check
************************************************************************/

/*
 * Name:	SetupMatch
 *
 * Argument:	char	*tname	Target name including name ,extension and version.
 *		char	*pname	Pattern including name and extension.
 *		char	*text	The place where separated target extension will be
 *				stored.
 *		char	*pext	The place where separated pattern extension will be
 *				stored.
 *		char	*tver	The place where separated target version will be
 *				stored.
 *
 * Value:	N/A
 *
 * Side Effect:	The string tname and pname points are destructively modified.
 *
 * Description:
 *
 * Split target and pattern to each component for the convenience of match_pattern
 * routine.
 */

#define SetupMatch(tname, pname, text, pext, tver)   \
  do {                                                  \
    char *pp;                               \
                                                     \
    separate_version(tname, tver, 0);                \
                                                     \
    if ((pp = (char *)strrchr(tname, '.')) == NULL) { \
      *(text) = '\0';                                  \
    } else {                                         \
      *pp = '\0';                                    \
      strcpy(text, pp + 1);                          \
    }                                                \
                                                     \
    if ((pp = (char *)strrchr(pname, '.')) == NULL) { \
      *(pext) = '\0';                                  \
    } else {                                         \
      *pp = '\0';                                    \
      strcpy(pext, pp + 1);                          \
    }                                                \
  } while (0)

#define MatchP(target, name, ver, matchtag, unmatchtag)                                       \
  do {                                                                                           \
    char tname[MAXNAMLEN], text[MAXNAMLEN], tver[VERSIONLEN];                                 \
    char pname[MAXNAMLEN], pext[MAXNAMLEN];                                                   \
                                                                                              \
    strcpy(tname, target);                                                                    \
    DOWNCASE(tname);                                                                          \
    strcpy(pname, name);                                                                      \
    DOWNCASE(pname);                                                                          \
                                                                                              \
    SetupMatch(tname, pname, text, pext, tver);                                               \
                                                                                              \
    if (match_pattern(tname, pname) && match_pattern(text, pext) && match_pattern(tver, ver)) \
      goto matchtag; /* NOLINT(bugprone-macro-parentheses) */                                 \
    else                                                                                      \
      goto unmatchtag; /* NOLINT(bugprone-macro-parentheses) */                               \
  } while (0)

#define MatchP_Case(target, name, ver, matchtag, unmatchtag)                                  \
  do {                                                                                           \
    char tname[MAXNAMLEN], text[MAXNAMLEN], tver[VERSIONLEN];                                 \
    char pname[MAXNAMLEN], pext[MAXNAMLEN];                                                   \
                                                                                              \
    strcpy(tname, target);                                                                    \
    strcpy(pname, name);                                                                      \
                                                                                              \
    SetupMatch(tname, pname, text, pext, tver);                                               \
                                                                                              \
    if (match_pattern(tname, pname) && match_pattern(text, pext) && match_pattern(tver, ver)) \
      goto matchtag; /* NOLINT(bugprone-macro-parentheses) */                                 \
    else                                                                                      \
      goto unmatchtag; /* NOLINT(bugprone-macro-parentheses) */                               \
  } while (0)

/*
 * Name:	match_pattern
 *
 * Argument:	char	*tp	String which is matched against pattern.
 *		char	*pp	String represents a pattern.
 *
 * Value:	If target is regarded to match with pattern, returns 1, otherwise 0.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Matches string against pattern.  Wild character is '*', it matches to arbitrary
 * number of characters.
 */

static int match_pattern(char *tp, char *pp)
{
  char *tsp, *psp;
  int inastr;

#ifdef DOS
  /* % is not allowed in DOS names for Medley. */
  if (strchr(tp, '%')) return 0;

#endif /* DOS */

  for (tsp = tp, psp = pp, inastr = 0;; tp++, pp++) {
    switch (*pp) {
      case '\0': return ((*tp == '\0') ? 1 : 0);

      case '*':
        while (*pp == '*') pp++; /* Skip successive '*'s. */
        if (*pp == '\0') return (1);

        psp = pp;
        while (*tp != *pp && *tp != '\0') tp++;

        if (*tp == '\0') return (0);

        tsp = tp;
        inastr = 1;

        continue;

      default:
        if (*tp == *pp) continue;

        if (inastr) {
          /*
           * Try to find a character which match to
           * a character psp points from a character
           * next to tsp.  If found retry from there.
           */
          for (tp = tsp + 1; *tp != '\0' && *tp != *psp; tp++) {}
          if (*tp == '\0') return (0);
          pp = psp;
          tsp = tp;
          continue;
        } else {
          return (0);
        }
    }
  }
}

#ifdef DOS

int make_old_version(char *old, char *file)
{
  int len = (int)strlen(file) - 1;
  if (file[len] == DIRCHAR) return 0;
  /* look up old versions of files for version # 0's */
  strcpy(old, file);

  if (old[len] == '.')
    strcat(old, "%");
  else if ((len > 0) && old[len - 1] == '.')
    strcat(old, "%");
  else if ((len > 1) && old[len - 2] == '.')
    strcat(old, "%");
  else if ((len > 2) && old[len - 3] == '.')
    old[len] = '%';
  else
    strcat(old, ".%");
  return 1;
}
#endif /* DOS */

/************************************************************************/
/******** E N D   O F   P A T T E R N - M A T C H I N G   C O D E *******/
/************************************************************************/

/************************************************************************/
/************ B E G I N  O F   F I L E - I N F O   C O D E **************/
/************************************************************************/


static FINFO *FreeFinfoList;
#define INITFINFONUM 1024

static DFINFO *FinfoArray;
#define INITFINFOARRAY 32

static int MAXFINFO;

#define FINFOARRAYRSIZE 16

#define AllocFinfo(fp)                                                   \
  do {                                                                      \
    if (FreeFinfoList != (FINFO *)NULL) {                                \
      (fp) = FreeFinfoList;                                                \
      FreeFinfoList = (fp)->next;                                          \
    } else if (((fp) = (FINFO *)calloc(1, sizeof(FINFO))) == NULL) {       \
      (fp) = (FINFO *)NULL;                                                \
    } else if (((fp)->prop = (FPROP *)calloc(1, sizeof(FPROP))) == NULL) { \
      free(fp);                                                          \
      (fp) = (FINFO *)NULL;                                                \
    }                                                                    \
  } while (0)

#define FreeFinfo(fp)                                                      \
  do {                                                                        \
    FINFO *lastp;                                                 \
    for (lastp = fp; lastp->next != (FINFO *)NULL; lastp = lastp->next) {} \
    lastp->next = FreeFinfoList;                                           \
    FreeFinfoList = fp;                                                    \
  } while (0)


/*
 * For debug aid.
 */

#ifdef FSDEBUG
void print_finfo(FINFO *fp)
{
  FINFO *sp;
  sp = fp;

  if (fp != (FINFO *)NULL) {
    do {
      printf("%s -> ", fp->lname);
      printf("%u\n", fp->version);
      fp = fp->next;
    } while (fp != (FINFO *)NULL && fp != sp);

    if (fp == sp) printf("Circular detected!\n");
  }
}
#endif /* FSDEBUG */

/*
 * Name:	init_finfo
 *
 * Argument:	None.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	FreeFinfoList will point to the alloced area.  MAXFINFO will hold
 *		the total number of allocated instances of FINFO structure.
 *
 * Description:
 *
 * Allocates the storage for the instances of FINFO and FPROP structure and arrange
 * them to build a linked list.
 * This routine is invoked at very first stage of emulator start up.
 */

int init_finfo(void) {
  FINFO *cp;
  int n;

  if ((FreeFinfoList = (FINFO *)calloc(sizeof(FINFO) + sizeof(FPROP), INITFINFONUM)) ==
      (FINFO *)NULL) {
    *Lisp_errno = errno;
    return (0);
  }
  for (cp = FreeFinfoList, n = INITFINFONUM; n > 1; n--) {
    cp->prop = (FPROP *)(cp + 1);
    cp->next = (FINFO *)((char *)cp + sizeof(FINFO) + sizeof(FPROP));
    cp = cp->next;
  }
  cp->prop = (FPROP *)(cp + 1);
  cp->next = (FINFO *)NULL;

  if ((FinfoArray = (DFINFO *)calloc(sizeof(DFINFO), INITFINFOARRAY)) == (DFINFO *)NULL) {
    *Lisp_errno = errno;
    return (0);
  }
  MAXFINFO = INITFINFOARRAY;

  return (1);
}

/*
 * Name:	get_finfo_id
 *
 * Argument:	None.
 *
 * Value:	If succeed, returns the id of linked list of FINFO structures,
 *		otherwise -1.
 *
 * Side Effect:	If needed, FinfoArray will be extended according to the value of
 *		FINFOARRAYRSIZE.
 *
 * Description:
 *
 * Get an ID which can be used to name a linked list of FINFO structure.  ID is
 * represented as an integer, and it is actually an index of an array FinfoArray.
 *
 * If all entries of FinfoArray is occupied by linked lists of FINFO structures,
 * extended storage is allocated and old contents of the array are copied to the
 * new area.  The size of the extended part is decided by the value of
 * FINFOARRAYRSIZE.
 */

static int get_finfo_id(void) {
  int i;
  DFINFO *dfap;

  for (i = 0; i < MAXFINFO; i++)
    if (FinfoArray[i].head == (FINFO *)0) return (i);

  if ((dfap = (DFINFO *)calloc(sizeof(DFINFO), MAXFINFO + FINFOARRAYRSIZE)) == (DFINFO *)NULL) {
    *Lisp_errno = errno;
    return (-1);
  }
  for (i = 0; i < MAXFINFO; i++) {
    dfap[i].head = FinfoArray[i].head;
    dfap[i].next = FinfoArray[i].next;
  }
  free(FinfoArray);
  MAXFINFO += FINFOARRAYRSIZE;
  FinfoArray = dfap;
  return (i);
}

/*
 * Name:	enum_dsk_prop
 *
 * Argument:	char	*dir	Absolute path of directory in UNIX format.
 *		char	*name	Pattern specify the files to be enumerated.
 *		char	*ver    String representation of version should be
 *				enumerated.
 *		FINFO	**finfo_buf
 *				The place where linked list of FINFO structures
 *				result of the enumeration will be stored.
 *
 * Value:	If succeed, returns the number of enumerated files, otherwise -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Enumerates files and directories matching to the pattern on the specified
 * directory.  The pattern matching is done in case insensitive manner.
 * File properties Lisp will need later are also stored in the result linked list
 * of FINFO structures.
 */

#ifdef DOS
static int enum_dsk_prop(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct direct *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, len, rval, res, isslash = 0, drive = 0;
  struct find_t dirp;
  struct passwd *pwd;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];
  char fver[VERSIONLEN];
  char old[MAXNAMLEN];

  /* The null directory has to be special cased */
  /* because adjacent \'s in the pathname don't match anything */
  if (dir[1] == DRIVESEP) drive = dir[0];

  if (strcmp(dir, "\\") == 0)
    isslash = 1;
  else if (drive && (strcmp(dir + 2, "\\") == 0))
    isslash = 1;

  if (!isslash)
    strcpy(namebuf, dir); /* Only add the dir if it's real */
  else if (drive) {
    namebuf[0] = drive;
    namebuf[1] = DRIVESEP;
    namebuf[2] = '\0';
  } else
    *namebuf = '\0';

  strcat(namebuf, DIRSEPSTR);
  strcat(namebuf, name);

  TIMEOUT(res = _dos_findfirst(namebuf, _A_NORMAL | _A_SUBDIR, &dirp));
  if (res < 0) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (nextp = prevp = (FINFO *)NULL, n = 0; res == 0;
       S_TOUT(res = _dos_findnext(&dirp)), prevp = nextp) {
    if (strcmp(dirp.name, ".") == 0 || strcmp(dirp.name, "..") == 0) continue;
    MatchP(dirp.name, name, ver, match, unmatch);
  unmatch:
    continue;
  match:
    AllocFinfo(nextp);
    if (nextp == (FINFO *)NULL) {
      FreeFinfo(prevp);
      *Lisp_errno = errno;
      return (-1);
    }
    nextp->next = prevp;
    if (isslash) {
      if (drive)
        sprintf(namebuf, "%c:\\%s", drive, dirp.name);
      else
        sprintf(namebuf, "\\%s", dirp.name);
    } else
      sprintf(namebuf, "%s\\%s", dir, dirp.name);

    TIMEOUT(rval = stat(namebuf, &sbuf));
    if (rval == -1 && errno != ENOENT) {
      /*
       * ENOENT error might be caused by missing symbolic
       * link. We should ignore such error here.
       */
      FreeFinfo(nextp);
      *Lisp_errno = errno;
      return (-1);
    }

    strcpy(namebuf, dirp.name);
    if (S_ISDIR(sbuf.st_mode)) {
      nextp->dirp = 1;
      quote_dname(namebuf);
      strcpy(nextp->lname, namebuf);
      len = strlen(namebuf);
      *(nextp->lname + len) = DIRCHAR;
      *(nextp->lname + len + 1) = '\0';
      nextp->lname_len = len + 1;
    } else {
      /* All other types than directory. */
      nextp->dirp = 0;
      strcat(namebuf, ".~1~");
      quote_fname(namebuf);
      len = strlen(namebuf);
      strcpy(nextp->lname, namebuf);
      *(nextp->lname + len) = '\0';
      nextp->lname_len = len;
    }

    strcpy(namebuf, dirp.name);
    len = strlen(namebuf);
    DOWNCASE(namebuf);
    strcpy(nextp->no_ver_name, namebuf);
    nextp->version = 1;
    nextp->ino = sbuf.st_ino;
    nextp->prop->length = (unsigned)sbuf.st_size;
    nextp->prop->wdate = (unsigned)ToLispTime(sbuf.st_mtime);
    nextp->prop->rdate = (unsigned)ToLispTime(sbuf.st_atime);
    nextp->prop->protect = (unsigned)sbuf.st_mode;
    /*	TIMEOUT(pwd = getpwuid(sbuf.st_uid));
            if (pwd == (struct passwd *)NULL) {
                    nextp->prop->au_len = 0;
            } else {
                    len = strlen(pwd->pw_name);
                    strcpy(nextp->prop->author, pwd->pw_name);
                    *(nextp->prop->author + len) = '\0';
                    nextp->prop->au_len = len;
            } */
    n++;
  }

  /***********************/
  /* Now go looking for version-0 entries */
  /***********************/

  for (nextp = prevp; nextp; nextp = nextp->next) {
    FINFO *newp;

    if (!make_old_version(old, nextp->no_ver_name)) continue;

    if (isslash) {
      if (drive)
        sprintf(namebuf, "%c:\\%s", drive, old);
      else
        sprintf(namebuf, "\\%s", old);
    } else
      sprintf(namebuf, "%s\\%s", dir, old);
    TIMEOUT(rval = stat(namebuf, &sbuf));

    if (rval == -1) continue;

    AllocFinfo(newp);
    newp->next = prevp;
    /* All other types than directory. */
    newp->dirp = 0;
    sprintf(namebuf, "%s.~00~", nextp->no_ver_name);
    quote_fname(namebuf);
    len = strlen(namebuf);
    strcpy(newp->lname, namebuf);
    *(newp->lname + len) = '\0';
    newp->lname_len = len;

    strcpy(newp->no_ver_name, old);
    newp->version = 0;
    newp->ino = sbuf.st_ino;
    newp->prop->length = (unsigned)sbuf.st_size;
    newp->prop->wdate = (unsigned)ToLispTime(sbuf.st_mtime);
    newp->prop->rdate = (unsigned)ToLispTime(sbuf.st_atime);
    newp->prop->protect = (unsigned)sbuf.st_mode;
    n++;
    prevp = newp;
  }
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#else  /* DOS */
static int enum_dsk_prop(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct dirent *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, rval;
  size_t len;
  DIR *dirp;
  struct passwd *pwd;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];
  char fver[VERSIONLEN];

  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (S_TOUT(dp = readdir(dirp)), nextp = prevp = (FINFO *)NULL, n = 0;
       dp != (struct dirent *)NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)), prevp = nextp)
    if (dp) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_ino == 0) continue;
      MatchP((char *)dp->d_name, name, ver, match, unmatch);
    unmatch:
      continue;
    match:
      AllocFinfo(nextp);
      if (nextp == (FINFO *)NULL) {
        FreeFinfo(prevp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }
      nextp->next = prevp;
      sprintf(namebuf, "%s/%s", dir, dp->d_name);
      TIMEOUT(rval = stat(namebuf, &sbuf));
      if (rval == -1 && errno != ENOENT) {
        /*
         * ENOENT error might be caused by missing symbolic
         * link. We should ignore such error here.
         */
        FreeFinfo(nextp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }

      strcpy(namebuf, dp->d_name);
      if (S_ISDIR(sbuf.st_mode)) {
        nextp->dirp = 1;
        quote_dname(namebuf);
        strcpy(nextp->lname, namebuf);
        len = strlen(namebuf);
        *(nextp->lname + len) = DIRCHAR;
        *(nextp->lname + len + 1) = '\0';
        nextp->lname_len = len + 1;
      } else {
        /* All other types than directory. */
        nextp->dirp = 0;
        quote_fname(namebuf);
        len = strlen(namebuf);
        strcpy(nextp->lname, namebuf);
        *(nextp->lname + len) = '\0';
        nextp->lname_len = len;
      }

      strcpy(namebuf, dp->d_name);
      len = strlen(namebuf);
      separate_version(namebuf, fver, 1);
      DOWNCASE(namebuf);
      strcpy(nextp->no_ver_name, namebuf);
      if (*fver == '\0')
        nextp->version = 0;
      else
        nextp->version = strtoul(fver, (char **)NULL, 10);
      nextp->ino = sbuf.st_ino;
      nextp->prop->length = (unsigned)sbuf.st_size;
      nextp->prop->wdate = (unsigned)ToLispTime(sbuf.st_mtime);
      nextp->prop->rdate = (unsigned)ToLispTime(sbuf.st_atime);
      nextp->prop->protect = (unsigned)sbuf.st_mode;
      TIMEOUT0(pwd = getpwuid(sbuf.st_uid));
      if (pwd == (struct passwd *)NULL) {
        nextp->prop->au_len = 0;
      } else {
        len = strlen(pwd->pw_name);
        strcpy(nextp->prop->author, pwd->pw_name);
        *(nextp->prop->author + len) = '\0';
        nextp->prop->au_len = len;
      }
      n++;
    }
  closedir(dirp);
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#endif /* DOS */

/*
 * Name:	enum_dsk
 *
 * Argument:	char	*dir	Absolute path of directory in UNIX format.
 *		char	*name	Pattern specify the files to be enumerated.
 *		char	*ver    String representation of version should be
 *				enumerated.
 *		FINFO	**finfo_buf
 *				The place where linked list of FINFO structures
 *				result of the enumeration will be stored.
 *
 * Value:	If succeed, returns the number of enumerated files, otherwise -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Similar to enum_dsk_prop, but file properties are not stored.
 */
#ifdef DOS
static int enum_dsk(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct direct *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, len, rval, isslash = 0, drive = 0;
  struct find_t dirp;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];
  char fver[VERSIONLEN];
  char old[MAXPATHLEN];

  /* The null directory has to be special cased */
  /* because adjacent \'s in the pathname don't match anything */
  if (dir[1] == DRIVESEP) drive = dir[0];

  if (strcmp(dir, "\\") == 0)
    isslash = 1;
  else if (drive && (strcmp(dir + 2, "\\") == 0))
    isslash = 1;

  if (!isslash)
    strcpy(namebuf, dir); /* Only add the dir if it's real */
  else if (drive) {
    namebuf[0] = drive;
    namebuf[1] = DRIVESEP;
    namebuf[2] = '\0';
  } else
    *namebuf = '\0';

  strcat(namebuf, DIRSEPSTR);
  strcat(namebuf, name);

  TIMEOUT(rval = _dos_findfirst(namebuf, _A_NORMAL | _A_SUBDIR, &dirp));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (nextp = prevp = (FINFO *)NULL, n = 0; rval == 0;
       S_TOUT(rval = _dos_findnext(&dirp)), prevp = nextp) {
    if (strcmp(dirp.name, ".") == 0 || strcmp(dirp.name, "..") == 0) continue;
    MatchP(dirp.name, name, ver, match, unmatch);
  unmatch:
    continue;
  match:
    AllocFinfo(nextp);
    if (nextp == (FINFO *)NULL) {
      FreeFinfo(prevp);
      *Lisp_errno = errno;
      return (-1);
    }
    nextp->next = prevp;
    if (isslash) {
      if (drive)
        sprintf(namebuf, "%c:\\%s", drive, dirp.name);
      else
        sprintf(namebuf, "\\%s", dirp.name);
    } else
      sprintf(namebuf, "%s\\%s", dir, dirp.name);
    TIMEOUT(rval = stat(namebuf, &sbuf));
    if (rval == -1 && errno != ENOENT) {
      /*
       * ENOENT error might be caused by missing symbolic
       * link. We should ignore such error here.
       */
      FreeFinfo(nextp);
      *Lisp_errno = errno;
      return (-1);
    }

    strcpy(namebuf, dirp.name); /* moved from below 2/26/93 */
    if (S_ISDIR(sbuf.st_mode)) {
      nextp->dirp = 1;
      quote_dname(namebuf);
      strcpy(nextp->lname, namebuf);
      len = strlen(namebuf);
      *(nextp->lname + len) = DIRCHAR;
      *(nextp->lname + len + 1) = '\0';
      nextp->lname_len = len + 1;
    } else {
      /* All other types than directory. */
      nextp->dirp = 0;
      strcat(namebuf, ".~1~");
      quote_fname(namebuf);
      len = strlen(namebuf);
      strcpy(nextp->lname, namebuf);
      *(nextp->lname + len) = '\0';
      nextp->lname_len = len;
    }

    strcpy(namebuf, dirp.name); /* to get real versionless name */
    len = strlen(namebuf);
    DOWNCASE(namebuf);
    strcpy(nextp->no_ver_name, namebuf);
    nextp->version = 1;
    nextp->ino = sbuf.st_ino;
    n++;
  }

  /***********************/
  /* Now go looking for version-0 entries */
  /***********************/

  for (nextp = prevp; nextp; nextp = nextp->next) {
    FINFO *newp;

    if (!make_old_version(old, nextp->no_ver_name)) continue;

    if (isslash) {
      if (drive)
        sprintf(namebuf, "%c:\\%s", drive, old);
      else
        sprintf(namebuf, "\\%s", old);
    } else
      sprintf(namebuf, "%s\\%s", dir, old);
    TIMEOUT(rval = stat(namebuf, &sbuf));

    if (rval == -1) continue;

    AllocFinfo(newp);
    newp->next = prevp;
    /* All other types than directory. */
    newp->dirp = 0;
    sprintf(namebuf, "%s.~00~", nextp->no_ver_name);
    quote_fname(namebuf);
    len = strlen(namebuf);
    strcpy(newp->lname, namebuf);
    *(newp->lname + len) = '\0';
    newp->lname_len = len;

    strcpy(newp->no_ver_name, old);
    newp->version = 0;
    newp->ino = sbuf.st_ino;
    n++;
    prevp = newp;
  }

  if (n > 0) *finfo_buf = prevp;
  return (n);
}

#else  /* DOS */

static int enum_dsk(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct dirent *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, rval;
  size_t len;
  DIR *dirp;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];
  char fver[VERSIONLEN];

  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (S_TOUT(dp = readdir(dirp)), nextp = prevp = (FINFO *)NULL, n = 0;
       dp != (struct dirent *)NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)), prevp = nextp)
    if (dp) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_ino == 0) continue;
      MatchP((char *)dp->d_name, name, ver, match, unmatch);
    unmatch:
      continue;
    match:
      AllocFinfo(nextp);
      if (nextp == (FINFO *)NULL) {
        FreeFinfo(prevp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }
      nextp->next = prevp;
      sprintf(namebuf, "%s/%s", dir, dp->d_name);
      TIMEOUT(rval = stat(namebuf, &sbuf));
      if (rval == -1 && errno != ENOENT) {
        /*
         * ENOENT error might be caused by missing symbolic
         * link. We should ignore such error here.
         */
        FreeFinfo(nextp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }

      strcpy(namebuf, dp->d_name);
      if (S_ISDIR(sbuf.st_mode)) {
        nextp->dirp = 1;
        quote_dname(namebuf);
        strcpy(nextp->lname, namebuf);
        len = strlen(namebuf);
        *(nextp->lname + len) = DIRCHAR;
        *(nextp->lname + len + 1) = '\0';
        nextp->lname_len = len + 1;
      } else {
        /* All other types than directory. */
        nextp->dirp = 0;
        quote_fname(namebuf);
        len = strlen(namebuf);
        strcpy(nextp->lname, namebuf);
        *(nextp->lname + len) = '\0';
        nextp->lname_len = len;
      }

      strcpy(namebuf, dp->d_name);
      len = strlen(namebuf);
      separate_version(namebuf, fver, 1);
      DOWNCASE(namebuf);
      strcpy(nextp->no_ver_name, namebuf);
      if (*fver == '\0')
        nextp->version = 0;
      else
        nextp->version = strtoul(fver, (char **)NULL, 10);
      nextp->ino = sbuf.st_ino;
      n++;
    }
  closedir(dirp);
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#endif /* DOS */

/*
 * Name:	enum_ufs_prop
 *
 * Argument:	char	*dir	Absolute path of directory in UNIX format.
 *		char	*name	Pattern specify the files to be enumerated.
 *		char	*ver    String representation of version should be
 *				enumerated.
 *		FINFO	**finfo_buf
 *				The place where linked list of FINFO structures
 *				result of the enumeration will be stored.
 *
 * Value:	If succeed, returns the number of enumerated files, otherwise -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Enumerates files and directories matching to the pattern on the specified
 * directory.  The pattern matching is done in case sensitive manner.
 * File properties Lisp will need later are also stored in the result linked list
 * of FINFO structures.
 */
#ifdef DOS
static int enum_ufs_prop(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct direct *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, len, rval;
  struct find_t dirp;
  /* struct passwd *pwd; -- From author support */
  struct stat sbuf;
  char namebuf[MAXPATHLEN];

  TIMEOUT(rval = _dos_findfirst(dir, _A_SUBDIR, &dirp));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (nextp = prevp = (FINFO *)NULL, n = 0; rval == 0;
       S_TOUT(rval = _dos_findnext(&dirp)), prevp = nextp) {
    if (strcmp(dirp.name, ".") == 0 || strcmp(dirp.name, "..") == 0) continue;
    MatchP_Case(dirp.name, name, ver, match, unmatch);
  unmatch:
    continue;
  match:
    AllocFinfo(nextp);
    if (nextp == (FINFO *)NULL) {
      FreeFinfo(prevp);
      *Lisp_errno = errno;
      return (-1);
    }
    nextp->next = prevp;
    sprintf(namebuf, "%s\\%s", dir, dirp.name);
    TIMEOUT(rval = stat(namebuf, &sbuf));
    if (rval == -1 && errno != ENOENT) {
      /*
       * ENOENT error might be caused by missing symbolic
       * link. We should ignore such error here.
       */
      FreeFinfo(nextp);
      *Lisp_errno = errno;
      return (-1);
    }

    strcpy(namebuf, dirp.name);
    if (S_ISDIR(sbuf.st_mode)) {
      nextp->dirp = 1;
      quote_dname(namebuf);
      strcpy(nextp->lname, namebuf);
      len = strlen(namebuf);
      *(nextp->lname + len) = DIRCHAR;
      *(nextp->lname + len + 1) = '\0';
      nextp->lname_len = len + 1;
    } else {
      /* All other types than directory. */
      nextp->dirp = 0;
      quote_fname_ufs(namebuf);
      len = strlen(namebuf);
      strcpy(nextp->lname, namebuf);
      *(nextp->lname + len) = '\0';
      nextp->lname_len = len;
    }

    strcpy(namebuf, dirp.name);
    len = strlen(namebuf);
    nextp->ino = sbuf.st_ino;
    nextp->prop->length = (unsigned)sbuf.st_size;
    nextp->prop->wdate = (unsigned)ToLispTime(sbuf.st_mtime);
    nextp->prop->rdate = (unsigned)ToLispTime(sbuf.st_atime);
    nextp->prop->protect = (unsigned)sbuf.st_mode;
    /*
                    TIMEOUT(pwd = getpwuid(sbuf.st_uid));
                    if (pwd == (struct passwd *)NULL) {
                            nextp->prop->au_len = 0;
                    } else {
                            len = strlen(pwd->pw_name);
                            strcpy(nextp->prop->author, pwd->pw_name);
                            *(nextp->prop->author + len) = '\0';
                            nextp->prop->au_len = len;
                    }
    */
    n++;
  }
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#else  /* DOS */
static int enum_ufs_prop(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct dirent *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, rval;
  size_t len;
  DIR *dirp;
  /* struct passwd *pwd; -- From author support */
  struct stat sbuf;
  char namebuf[MAXPATHLEN];

  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (S_TOUT(dp = readdir(dirp)), nextp = prevp = (FINFO *)NULL, n = 0;
       dp != (struct dirent *)NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)), prevp = nextp)
    if (dp) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_ino == 0) continue;
      MatchP_Case((char *)dp->d_name, name, ver, match, unmatch);
    unmatch:
      continue;
    match:
      AllocFinfo(nextp);
      if (nextp == (FINFO *)NULL) {
        FreeFinfo(prevp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }
      nextp->next = prevp;
      sprintf(namebuf, "%s/%s", dir, dp->d_name);
      TIMEOUT(rval = stat(namebuf, &sbuf));
      if (rval == -1 && errno != ENOENT) {
        /*
         * ENOENT error might be caused by missing symbolic
         * link. We should ignore such error here.
         */
        FreeFinfo(nextp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }

      strcpy(namebuf, dp->d_name);
      if (S_ISDIR(sbuf.st_mode)) {
        nextp->dirp = 1;
        quote_dname(namebuf);
        strcpy(nextp->lname, namebuf);
        len = strlen(namebuf);
        *(nextp->lname + len) = DIRCHAR;
        *(nextp->lname + len + 1) = '\0';
        nextp->lname_len = len + 1;
      } else {
        /* All other types than directory. */
        nextp->dirp = 0;
        quote_fname_ufs(namebuf);
        len = strlen(namebuf);
        strcpy(nextp->lname, namebuf);
        *(nextp->lname + len) = '\0';
        nextp->lname_len = len;
      }

      strcpy(namebuf, dp->d_name);
      len = strlen(namebuf);
      nextp->ino = sbuf.st_ino;
      nextp->prop->length = (unsigned)sbuf.st_size;
      nextp->prop->wdate = (unsigned)ToLispTime(sbuf.st_mtime);
      nextp->prop->rdate = (unsigned)ToLispTime(sbuf.st_atime);
      nextp->prop->protect = (unsigned)sbuf.st_mode;
      /*
                      TIMEOUT(pwd = getpwuid(sbuf.st_uid));
                      if (pwd == (struct passwd *)NULL) {
                              nextp->prop->au_len = 0;
                      } else {
                              len = strlen(pwd->pw_name);
                              strcpy(nextp->prop->author, pwd->pw_name);
                              *(nextp->prop->author + len) = '\0';
                              nextp->prop->au_len = len;
                      }
      */
      n++;
    }
  closedir(dirp);
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#endif /* DOS */

/*
 * Name:	enum_ufs
 *
 * Argument:	char	*dir	Absolute path of directory in UNIX format.
 *		char	*name	Pattern specify the files to be enumerated.
 *		char	*ver    String representation of version should be
 *				enumerated.
 *		FINFO	**finfo_buf
 *				The place where linked list of FINFO structures
 *				result of the enumeration will be stored.
 *
 * Value:	If succeed, returns the number of enumerated files, otherwise -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Similar to enum_ufs_prop, but file properties are not stored.
 */
#ifdef DOS
static int enum_ufs(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct direct *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, len, rval;
  struct find_t dirp;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];

  TIMEOUT(rval = _dos_findfirst(dir, _A_SUBDIR, &dirp));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (nextp = prevp = (FINFO *)NULL, n = 0; rval == 0;
       S_TOUT(rval = _dos_findnext(&dirp)), prevp = nextp) {
    if (strcmp(dirp.name, ".") == 0 || strcmp(dirp.name, "..") == 0) continue;
    MatchP_Case(dirp.name, name, ver, match, unmatch);
  unmatch:
    continue;
  match:
    AllocFinfo(nextp);
    if (nextp == (FINFO *)NULL) {
      FreeFinfo(prevp);
      *Lisp_errno = errno;
      return (-1);
    }
    nextp->next = prevp;
    sprintf(namebuf, "%s\\%s", dir, dirp.name);
    TIMEOUT(rval = stat(namebuf, &sbuf));
    if (rval == -1 && errno != ENOENT) {
      /*
       * ENOENT error might be caused by missing symbolic
       * link. We should ignore such error here.
       */
      FreeFinfo(nextp);
      *Lisp_errno = errno;
      return (-1);
    }

    strcpy(namebuf, dirp.name);
    if (S_ISDIR(sbuf.st_mode)) {
      nextp->dirp = 1;
      quote_dname(namebuf);
      strcpy(nextp->lname, namebuf);
      len = strlen(namebuf);
      *(nextp->lname + len) = DIRCHAR;
      *(nextp->lname + len + 1) = '\0';
      nextp->lname_len = len + 1;
    } else {
      /* All other types than directory. */
      nextp->dirp = 0;
      quote_fname_ufs(namebuf);
      len = strlen(namebuf);
      strcpy(nextp->lname, namebuf);
      *(nextp->lname + len) = '\0';
      nextp->lname_len = len;
    }

    strcpy(namebuf, dirp.name);
    len = strlen(namebuf);
    nextp->ino = sbuf.st_ino;
    n++;
  }
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#else  /* DOS */
static int enum_ufs(char *dir, char *name, char *ver, FINFO **finfo_buf)
{
  struct dirent *dp;
  FINFO *prevp;
  FINFO *nextp;
  int n, rval;
  size_t len;
  DIR *dirp;
  struct stat sbuf;
  char namebuf[MAXPATHLEN];

  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (-1);
  }

  for (S_TOUT(dp = readdir(dirp)), nextp = prevp = (FINFO *)NULL, n = 0;
       dp != (struct dirent *)NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)), prevp = nextp)
    if (dp) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_ino == 0) continue;
      MatchP_Case((char *)dp->d_name, name, ver, match, unmatch);
    unmatch:
      continue;
    match:
      AllocFinfo(nextp);
      if (nextp == (FINFO *)NULL) {
        FreeFinfo(prevp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }
      nextp->next = prevp;
      sprintf(namebuf, "%s/%s", dir, dp->d_name);
      TIMEOUT(rval = stat(namebuf, &sbuf));
      if (rval == -1 && errno != ENOENT) {
        /*
         * ENOENT error might be caused by missing symbolic
         * link. We should ignore such error here.
         */
        FreeFinfo(nextp);
        closedir(dirp);
        *Lisp_errno = errno;
        return (-1);
      }

      strcpy(namebuf, dp->d_name);
      if (S_ISDIR(sbuf.st_mode)) {
        nextp->dirp = 1;
        quote_dname(namebuf);
        strcpy(nextp->lname, namebuf);
        len = strlen(namebuf);
        *(nextp->lname + len) = DIRCHAR;
        *(nextp->lname + len + 1) = '\0';
        nextp->lname_len = len + 1;
      } else {
        /* All other types than directory. */
        nextp->dirp = 0;
        quote_fname_ufs(namebuf);
        len = strlen(namebuf);
        strcpy(nextp->lname, namebuf);
        *(nextp->lname + len) = '\0';
        nextp->lname_len = len;
      }

      strcpy(namebuf, dp->d_name);
      len = strlen(namebuf);
      nextp->ino = sbuf.st_ino;
      n++;
    }
  closedir(dirp);
  if (n > 0) *finfo_buf = prevp;
  return (n);
}
#endif /* DOS*/

/*
 * Name:	trim_finfo
 *
 * Argument:	FINFO	**fp	Linked list of the numerated FINFO structures.
 *
 * Value:	Returns the total number of files still remaining in **fp.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Giving the linked list of FINFO, take care of versionless files.  If the
 * versionless file is not linked any higher versioned file, it is given a highest
 * version.  If versionless file is linked to one of versioned file, it is got
 * rid of.
 * This routine is only used by DSK codes.
 */

static int trim_finfo(FINFO **fp)
{
#ifndef DOS
  FINFO *tp, *sp, *mp, *cp, *pp;
  int num, pnum;
  int linkp;
  char ver[VERSIONLEN];

  sp = mp = cp = *fp;
  pp = (FINFO *)NULL;
  num = pnum = 0;

  do {
    if (cp->dirp) {
      pp = cp;
      sp = cp = cp->next;
      pnum++;
      num++;
      continue;
    }

    if (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
      mp = cp = cp->next;
      num++;
      while (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
        cp = cp->next;
        num++;
      }
    } else {
      mp = cp;
    }

    if (sp->version == 0) {
      if (cp != sp) {
        /*
         * Both versionless and versioned files exists.
         */
        linkp = 0;
        tp = sp;
        do {
          tp = tp->next;
          if (tp->ino == sp->ino) {
            linkp = 1;
            break;
          }
        } while (cp != tp);

        if (!linkp) {
          /*
           * Versionless is not linked to any versioned
           * file.
           */
          sprintf(ver, ";%u", mp->version + 1);
          strcat(sp->lname, ver);
          sp->lname_len = strlen(sp->lname);
          pnum = ++num;
          pp = cp;
          sp = cp = cp->next;
        } else {
          /*
           * Versionless is linked to one of versionless
           * files. We can remove it.
           */
          sp->next = (FINFO *)NULL;
          FreeFinfo(sp);
          pnum = num;
          if (pp != (FINFO *)NULL)
            pp->next = mp;
          else
            *fp = mp;
          pp = cp;
          sp = cp = cp->next;
        }
      } else {
        /*
         * Only versionless file exists. It is regarded as
         * version 1.
         */
        strcat(cp->lname, ";1");
        cp->lname_len += 2;
        pp = cp;
        sp = cp = cp->next;
        num = ++pnum;
      }
    } else {
      if (cp != sp) {
        /*
         * All files are versioned.
         */
        pnum = ++num;
      } else {
        /*
         * A versioned file only exists.
         */
        num = ++pnum;
      }
      pp = cp;
      sp = cp = cp->next;
    }
  } while (sp != (FINFO *)NULL);

#else  /* DOS version */
  int num = 0;
  FINFO *tp;
  tp = *fp;
  while (tp) {
    num++;
    tp = tp->next;
  }
#endif /* DOS */

  return (num);
}

/*
 * Name:	trim_finfo_highest
 *
 * Argument:	FINFO	**fp	Linked list of the numerated FINFO structures.
 *
 * Value:	Returns the total number of files still remaining in **fp.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Similar to true_finfo, but the files but the highest versioned file are got rid
 * of.
 */

static int trim_finfo_highest(FINFO **fp, int highestp)
{
  FINFO *tp, *sp, *mp, *cp, *pp;
  int num, pnum;
  int linkp;
  char ver[VERSIONLEN];

  sp = mp = cp = *fp;
  pp = (FINFO *)NULL;
  num = pnum = 0;

  do {
    if (cp->dirp) {
      pp = cp;
      sp = cp = cp->next;
      pnum++;
      num++;
      continue;
    }

    if (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
      mp = cp = cp->next;
      num++;
      while (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
        cp = cp->next;
        num++;
      }
    } else {
      mp = cp;
    }

    if (sp->version == 0) {
      if (cp != sp) {
        /*
         * Both versionless and versioned files exists.
         */
        linkp = 0;
        tp = sp;
        do {
          tp = tp->next;
          if (tp->ino == sp->ino) {
            linkp = 1;
            break;
          }
        } while (cp != tp);

        if (!linkp) {
          /*
           * Versionless is not linked to any versioned
           * file.
           */
          sprintf(ver, ";%u", mp->version + 1);
          strcat(sp->lname, ver);
          sp->lname_len = strlen(sp->lname);
          /*
           * Lower versioned files, mp to cp
           * inclusive, should be removed.
           */
          sp->next = cp->next;
          cp->next = (FINFO *)NULL;
          FreeFinfo(mp);
          num = ++pnum;
          pp = sp;
          sp = cp = pp->next;
        } else {
          /*
           * Versionless is linked to one of versionless
           * files. We can remove it.
           */

          if (mp != cp) {
            sp->next = mp->next;
            mp->next = cp->next;
            cp->next = (FINFO *)NULL;
          } else {
            sp->next = (FINFO *)NULL;
          }
          FreeFinfo(sp);
          num = ++pnum;
          if (pp != (FINFO *)NULL)
            pp->next = mp;
          else
            *fp = mp;
          pp = mp;
          sp = cp = mp->next;
        }
      } else {
        /*
         * Only versionless file exists. It is regarded as
         * version 1.
         */
        strcat(cp->lname, ";1");
        cp->lname_len += 2;
        pp = cp;
        sp = cp = cp->next;
        num = ++pnum;
      }
    } else {
      if (cp != sp) {
        /*
         * All files are versioned.
         * Lower versioned files can be removed.
         */
        tp = sp->next;
        sp->next = cp->next;
        cp->next = (FINFO *)NULL;
        FreeFinfo(tp);
        num = ++pnum;
      } else {
        /*
         * A versioned file only exists.
         */
        num = ++pnum;
      }
      pp = sp;
      sp = cp = sp->next;
    }
  } while (sp != (FINFO *)NULL);

  return (num);
}

/*
 * Name:	trim_finfo_version
 *
 * Argument:	FINFO	**fp	Linked list of the numerated FINFO structures.
 *		unsigned rver	Requested version number.
 *
 * Value:	Returns the total number of files still remaining in **fp.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Similar to true_finfo, but the files but the versioned file with specified version
 * are got rid of.
 */

static int trim_finfo_version(FINFO **fp, unsigned rver)
{
  FINFO *tp, *sp, *mp, *cp, *pp, *vp;
  int num, pnum;
  int linkp;
  char ver[VERSIONLEN];

  sp = mp = cp = *fp;
  pp = (FINFO *)NULL;
  num = pnum = 0;

  do {
    if (cp->dirp) {
      /*
       * Directory has no version, thus they should be removed.
       */
      sp = cp = cp->next;
      continue;
    }

    if (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
      mp = cp = cp->next;
      num++;
      while (cp->next != (FINFO *)NULL && strcmp(cp->next->no_ver_name, cp->no_ver_name) == 0) {
        cp = cp->next;
        num++;
      }
    } else {
      mp = cp;
    }

    for (tp = sp, vp = (FINFO *)NULL; tp != cp->next; tp = tp->next) {
      if (tp->version == rver) {
        vp = tp;
        break;
      }
    }

    if (vp != (FINFO *)NULL) {
      /*
       * Specified version file exists.  Other files should be
       * removed.
       */
      if (vp == sp) {
        if (cp != sp) {
          vp->next = cp->next;
          cp->next = (FINFO *)NULL;
          FreeFinfo(mp);
        }
      } else {
        for (tp = sp; tp->next != vp; tp = tp->next) {}
        if (vp != cp) {
          tp->next = vp->next;
          vp->next = cp->next;
          cp->next = (FINFO *)NULL;
        } else {
          tp->next = (FINFO *)NULL;
        }
        if (pp != (FINFO *)NULL)
          pp->next = vp;
        else
          *fp = vp;
        FreeFinfo(sp);
      }
      pp = vp;
      sp = cp = vp->next;
      num = ++pnum;
      continue;
    }

    /*
     * Although there is no file with specified version, versionless
     * file might be interpreted the specified version.
     */
    if (sp->version == 0) {
      if (cp != sp) {
        /*
         * Both versionless and versioned files exists.
         */
        linkp = 0;
        tp = sp;
        do {
          tp = tp->next;
          if (tp->ino == sp->ino) {
            linkp = 1;
            break;
          }
        } while (cp != tp);

        if (!linkp) {
          /*
           * Versionless is not linked to any versioned
           * file.
           */
          if (mp->version + 1 == rver) {
            sprintf(ver, ";%u", rver);
            strcat(sp->lname, ver);
            sp->lname_len = strlen(sp->lname);
            /*
             * Lower versioned files, mp to cp
             * inclusive, should be removed.
             */
            sp->next = cp->next;
            cp->next = (FINFO *)NULL;
            FreeFinfo(mp);
            num = ++pnum;
            pp = sp;
            sp = cp = pp->next;
          } else {
            /*
             * sp to cp inclusive, all files,
             * should be removed.
             */
            tp = cp->next;
            if (pp != (FINFO *)NULL)
              pp->next = tp;
            else
              *fp = tp;
            cp->next = (FINFO *)NULL;
            FreeFinfo(sp);
            sp = cp = tp;
          }
        } else {
          /*
           * Versionless is linked to one of versionless
           * files.  We can remove all files, because
           * no versioned file match with rver.
           */
          tp = cp->next;
          if (pp != (FINFO *)NULL)
            pp->next = tp;
          else
            *fp = tp;
          cp->next = (FINFO *)NULL;
          FreeFinfo(sp);
          sp = cp = tp;
        }
      } else {
        /*
         * Only versionless file exists. It is regarded as
         * version 1.  Unless rver is 1, we can remove it.
         */
        if (rver != 1) {
          cp = sp->next;
          if (pp != (FINFO *)NULL)
            pp->next = cp;
          else
            *fp = cp;
          sp->next = (FINFO *)NULL;
          FreeFinfo(sp);
          sp = cp;
        } else {
          strcat(cp->lname, ";1");
          cp->lname_len += 2;
          pp = cp;
          sp = cp = cp->next;
          num = ++pnum;
        }
      }
    }
  } while (sp != (FINFO *)NULL);

  return (num);
}

/************************************************************************/
/************ E N D   O F   F I L E - I N F O   C O D E *****************/
/************************************************************************/

/************************************************************************/
/********* B E G I N  O F   F I L E - S O R T I N G   C O D E ***********/
/************************************************************************/

/*
 * Name:	prepare_sort_buf
 *
 * Argument:	FINFO	*fp	Linked list of FINFO structures being sorted.
 *		int	n       Total number of structures in the above list.
 *
 * Value:	If succeed, returns the pointer to the buffer, otherwise NULL
 *		pointer.
 *
 * Side Effect:	(sizeof(FINFO *) * n) bytes storage are allocated as a sort buffer.
 *
 * Description:
 *
 * Prepare an area to be used as a sort buffer by qsort routine, and arrange the
 * contents of the buffer to be convenience to qsort routine.
 * Caller have to free the area after sorting done.
 */

static FINFO **prepare_sort_buf(FINFO *fp, size_t n)
{
  FINFO **bp;
  FINFO **bufp;

  if ((bufp = (FINFO **)malloc(sizeof(FINFO *) * n)) == NULL) {
    *Lisp_errno = errno;
    return ((FINFO **)NULL);
  }
  for (bp = bufp; fp != (FINFO *)NULL; fp = fp->next, bp++) *bp = fp;

  return (bufp);
}

/*
 * Name:	dsk_filecmp
 *
 * Argument:	FINFO	*fp1	A FINFO structure, a file name in it is being compared.
 *		FINFO	*fp2	A FINFO structure, a file name in it is being compared.
 *
 * Value:	Returns -1, 0, or 1, according as s1 is lexically but with case
 *		insensitive mode greater than, equal to, or less than c2.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Compares two file names lexically but with case insensitive mode.  Two file names
 * should be processed by UnixVersionToLispVersion to make sure that they have a
 * valid version fields.  The version fields comparison is done in numerical manner.
 * Note that the result is in the reversed order.
 */

static int dsk_filecmp(const void *p1, const void *p2)
{
  FINFO * const *fp1 = p1; /* declare fp1 as pointer to constant pointer to structure finfo */
  FINFO * const *fp2 = p2;
  int res;
  unsigned v1, v2;

  if ((res = strcmp((*fp1)->no_ver_name, (*fp2)->no_ver_name)) != 0) return (res);

  if ((*fp1)->version == (*fp2)->version) return (0);
#ifndef DOS
  if ((v1 = (*fp1)->version) == 0) return (-1);
  if ((v2 = (*fp2)->version) == 0) return (1);
#endif /* DOS */
  return ((v1 < v2) ? 1 : -1);
}

/*
 * Name:	unix_filecmp
 *
 * Argument:	FINFO	*f1	A FINFO structure, a file name in it is being compared.
 *		FINFO	*f2	A FINFO structure, a file name in it is being compared.
 *
 * Value:	Returns -1, 0, or 1, according as s1 is lexically greater than,
 *		equal to, or less than c2.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Compares two file names lexically mode.
 * Note that the result is in the reversed order.
 */

static int unix_filecmp(const void *f1, const void *f2)
{
  return (strcmp((*(FINFO * const *)f1)->lname, (*(FINFO * const *)f2)->lname));
}

/*
 * Name:	file_sort
 *
 * Argument:	FINFO	**fpp	A pointer to a pointer addressing the linked FINFO
 *				being sorted.
 *		int	n	A number of FINFO structure linked.
 *		int	(*sortfn)()
 *				A pointer to a function to be used to sort the FINFOs.
 *
 * Value:	If succeed, returns 1, otherwise, 0.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Sorts the files to be appropriate for Lisp.   dsk_filecmp and unix_filecmp are
 * used for {DSK} and {UNIX} device respectively as a sort function.
 */

static int file_sort(FINFO **fpp, size_t n, int (*sortfn)(const void *, const void *))
{
  FINFO **fp;
  FINFO **sort_bufp;

  if ((sort_bufp = prepare_sort_buf(*fpp, n)) == (FINFO **)NULL) return (0);

  qsort(sort_bufp, n, sizeof(FINFO *), sortfn);

  /*
   * Relink FINFO structures in a buffer.
   */
  for (fp = sort_bufp; n > 1; fp++, n--) (*fp)->next = *(fp + 1);
  (*fp)->next = (FINFO *)NULL;

  *fpp = *sort_bufp;

  free((char *)sort_bufp);

  return (1);
}

/************************************************************************/
/************ E N D   O F   F I L E - S O R T I N G   C O D E ***********/
/************************************************************************/

#ifndef BYTESWAP
#ifdef BIGVM
typedef struct ufsgfs {
  unsigned finfoid;
  unsigned fileid;
  unsigned totalnum;
  LispPTR directory;
  unsigned propp : 1;
  unsigned padding : 3;
  unsigned dev : 28;
  LispPTR thisfile;
  int errnum;
  LispPTR name;
  unsigned length;
  unsigned wdate;
  unsigned rdate;
  unsigned protection;
  LispPTR author;
  unsigned aulen;
} UFSGFS;
#else
typedef struct ufsgfs {
  unsigned finfoid;
  unsigned fileid;
  unsigned totalnum;
  LispPTR directory;
  unsigned propp : 1;
  unsigned padding : 7;
  unsigned dev : 24;
  LispPTR thisfile;
  int errnum;
  LispPTR name;
  unsigned length;
  unsigned wdate;
  unsigned rdate;
  unsigned protection;
  LispPTR author;
  unsigned aulen;
} UFSGFS;
#endif /* BIGVM */

#else /* BYTESWAP */

#ifdef BIGVM
typedef struct ufsgfs {
  unsigned finfoid;
  unsigned fileid;
  unsigned totalnum;
  LispPTR directory;
  unsigned dev : 28;
  unsigned padding : 3;
  unsigned propp : 1;
  LispPTR thisfile;
  int errnum;
  LispPTR name;
  unsigned length;
  unsigned wdate;
  unsigned rdate;
  unsigned protection;
  LispPTR author;
  unsigned aulen;
} UFSGFS;
#else
typedef struct ufsgfs {
  unsigned finfoid;
  unsigned fileid;
  unsigned totalnum;
  LispPTR directory;
  unsigned dev : 24;
  unsigned padding : 7;
  unsigned propp : 1;
  LispPTR thisfile;
  int errnum;
  LispPTR name;
  unsigned length;
  unsigned wdate;
  unsigned rdate;
  unsigned protection;
  LispPTR author;
  unsigned aulen;
} UFSGFS;
#endif /* BIGVM */
#endif /* BYTESWAP */

/*
 * Name:	COM_gen_files
 *
 * Argument:	LispPTR	*args	args[0]
 *				 The pattern of file name to be enumerated in Lisp
 *				 format.  Includes the host field.
 *				args[1]
 *				 Flag indicating whether Lisp needs property or not.
 *				args[2]
 *				 The place where the file info ID should be placed.
 *				args[3]
 *				 The place where the error number should be stored.
 *
 * Value:	If succeed, returns the Lisp positive integer which represents the
 *		total number of enumerated files, otherwise Lisp -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * The implementation of GENERATEFILES FDEV method for DSK and UNIX device.
 * Enumerates files matching pattern.
 */

LispPTR COM_gen_files(LispPTR *args)
{
  char fbuf[MAXPATHLEN + 5], dir[MAXPATHLEN], pattern[MAXPATHLEN];
  char host[MAXNAMLEN], name[MAXNAMLEN], ver[VERSIONLEN];
#ifdef DOS
  char drive[1];
#endif
  int dskp, count, highestp, fid;
  unsigned propp, version;
  char *cp;
  FINFO *fp;

  ERRSETJMP(SMALLP_MINUSONE);

  Lisp_errno = (int *)(NativeAligned4FromLAddr(args[3]));

  LispStringLength(args[0], count, dskp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  count = dskp ? count + 4 + 1 : count + 2 + 1;
  /* Add 5 for the host name field in Lisp format. */
  if (count > MAXPATHLEN + 5) FileNameTooLong((SMALLP_MINUSONE));

  LispStringToCString(args[0], fbuf, MAXPATHLEN);
#ifdef DOS
  separate_host(fbuf, host, drive);
#else
  separate_host(fbuf, host);
#endif /* DOS */

  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

  if (args[1] == NIL)
    propp = 0;
  else
    propp = 1;

/*
 * The way to deal with the version field in file enumeration is a little
 * bit tricky because of the bad specification of original {UNIX} device.
 *
 * According to the Medley 1.1 release note, in the representation
 * "{UNIX}<dir>name.ext;3", ';' and '3' are regarded as a part of the
 * file name, not its version.  On the other hand, in 1.1 implementation,
 * in the pattern "{UNIX}<dir>*.*;*", the ';' and the last '*' are regarded
 * as a version field, not part of the file name.  Actually the pattern
 * "{UNIX}<tmp>*.*;*" enumerates all of the files on /tmp directory
 * even if they never include ';' character in its name, as well as '.'.
 *
 * Thus I believe, the specification should be clean upped as like,
 * "UNIX device always ignores the version field in it file name representation
 * even if a user specifies it explicitly".
 * But to keep a compatibility to an already released version, we have
 * to do some trick here.
 */

#ifdef DOS
  if (!unixpathname(fbuf, pattern, 1, 1, drive, 0, 0)) {
#else
  if (!unixpathname(fbuf, pattern, 1, 1)) {
#endif /* DOS */
    /* Yes, always dskp is on */
    return (SMALLP_MINUSONE);
  }

  if (!unpack_filename(pattern, dir, name, ver, 0)) return (SMALLP_MINUSONE);

  if (dskp) {
    /*
     * On {DSK}, we have to make sure dir is case insensitively existing
     * directory.
     */
    if (true_name(dir) != -1) return (SMALLP_MINUSONE);

    if (*ver != '\0') {
      highestp = 0;
      version = strtoul(ver, (char **)NULL, 10);
      if (version > 0) strcpy(ver, "*");
    } else {
      version = 0;
      for (cp = fbuf; *cp; cp++) {}
      if (*(cp - 1) == ';' && *(cp - 2) != '\'') {
        /*
         * An empty version is interpreted as wanting the
         * highest version.  In this case, at first enumerate
         * all version.  trim_finfo_highest will get rid of
         * lower versions.
         */
        strcpy(ver, "*");
        highestp = 1;
      } else {
        highestp = 0;
      }
    }
    if (propp)
      count = enum_dsk_prop(dir, name, ver, &fp);
    else
      count = enum_dsk(dir, name, ver, &fp);
  } else {
    /* Makes UNIX device matches any version. */
    strcpy(ver, "*");

    if (propp)
      count = enum_ufs_prop(dir, name, ver, &fp);
    else
      count = enum_ufs(dir, name, ver, &fp);
  }

  switch (count) {
    case -1: return (SMALLP_MINUSONE);

    case 0: return (SMALLP_ZERO);

    default:
      if (!file_sort(&fp, (size_t)count, dskp ? dsk_filecmp : unix_filecmp)) return (SMALLP_MINUSONE);
      if (dskp) {
        if (highestp)
          count = trim_finfo_highest(&fp, highestp);
        else if (version > 0 && count > 0)
          count = trim_finfo_version(&fp, version);
        else
          count = trim_finfo(&fp);
      }

      if ((fid = get_finfo_id()) < 0) return (SMALLP_MINUSONE);
      *(int *)(NativeAligned4FromLAddr(args[2])) = fid;
      FinfoArray[fid].head = fp;
      FinfoArray[fid].next = fp;
      return (GetSmallp(count));
  }
}

/*
 * Name:	COM_next_file
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Lisp pointer to UFSGFS structure.
 *
 * Value:	If succeed, returns the length of the file name as a Lisp positive
 *		integer, otherwise -1.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * The implementation of NEXTFILEFN File Generator Component for DSK and UNIX device.
 * Because of the efficiency reason, if propp, stores properties as well as file
 * name.
 */

LispPTR COM_next_file(LispPTR *args)
{
  LispPTR laddr;
  FPROP *pp;
  FINFO *fp;
  char *base;
  DFINFO *dfp;
  UFSGFS *gfsp;
  int finfoid;
  unsigned propp;

  ERRSETJMP(SMALLP_MINUSONE);
  Lisp_errno = &Dummy_errno;

  gfsp = (UFSGFS *)(NativeAligned4FromLAddr(args[0]));

  finfoid = (int)gfsp->finfoid;

  if (finfoid < 0 || MAXFINFO - 1 < finfoid) return (SMALLP_MINUSONE);

  propp = gfsp->propp;

  dfp = &FinfoArray[finfoid];
  fp = dfp->next;
  if (dfp->head == NULL || fp == NULL) return (SMALLP_MINUSONE);
  dfp->next = fp->next;

  laddr = gfsp->name;
  STRING_BASE(laddr, base);
#ifndef BYTESWAP
  strncpy(base, fp->lname, fp->lname_len);
#else
  StrNCpyFromCToLisp(base, fp->lname, fp->lname_len);
#endif /* BYTESWAP	 */

  if (!propp) return (GetPosSmallp(fp->lname_len));

  pp = fp->prop;
  gfsp->length = pp->length;
  gfsp->wdate = pp->wdate;
  gfsp->rdate = pp->rdate;
  gfsp->protection = pp->protect;

  laddr = gfsp->author;
  STRING_BASE(laddr, base);
#ifndef BYTESWAP
  strncpy(base, pp->author, pp->au_len);
#else
  StrNCpyFromCToLisp(base, pp->author, pp->au_len);
#endif /* BYTESWAP	 */

  gfsp->aulen = pp->au_len;

  return (GetPosSmallp(fp->lname_len));
}

/*
 * Name:	COM_finish_finfo
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Finfo ID.
 *
 * Value:	If succeed, returns ATOM_T, otherwise Lisp NIL.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * When Lisp directory enumeration file generator is exhausted or the enumerating
 * operation is aborted, this routine is called.
 * Abandon all cached information corresponding to the generator.
 */

LispPTR COM_finish_finfo(LispPTR *args)
{
  DFINFO *dfp;
  FINFO *fp;
  int finfoid;

  ERRSETJMP(NIL);

  Lisp_errno = &Dummy_errno;

  finfoid = LispNumToCInt(args[0]);

  if (finfoid < 0 || MAXFINFO - 1 < finfoid) return (NIL);

  dfp = &FinfoArray[finfoid];
  if ((fp = dfp->head) == (FINFO *)0) {
    dfp->next = (FINFO *)0;
    return (NIL);
  }

  dfp->head = (FINFO *)0;
  dfp->next = (FINFO *)0;
  FreeFinfo(fp);

  return (ATOM_T);
}
