/* $Id: dsk.c,v 1.4 2001/12/24 01:09:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>          // for errno, EINTR, ENOENT, ENFILE, EPERM
#include <fcntl.h>          // for O_RDWR, O_CREAT, open, O_RDONLY, O_TRUNC
#include <stdio.h>          // for NULL, sprintf, size_t, rename, SEEK_SET
#include <stddef.h>         // for ptrdiff_t
#include <stdlib.h>         // for strtoul
#include <string.h>         // for strcpy, strcmp, strlen, strncpy, strchr
#include <sys/stat.h>       // for stat, fstat, mkdir, S_ISREG, st_atime, chmod
#include <sys/types.h>      // for ino_t, time_t, off_t
#include <unistd.h>         // for unlink, close, link, lseek, access, chdir

#include "adr68k.h"         // for NativeAligned4FromLAddr
#include "arith.h"          // for GetSmallp
#ifdef BYTESWAP
#include "byteswapdefs.h"   // for word_swap_page
#endif
#include "car-cdrdefs.h"    // for cdr, car
#include "dskdefs.h"        // for COM_changedir, COM_closefile, COM_getfile...
#include "lispemul.h"       // for NIL, LispPTR, ATOM_T
#include "locfile.h"        // for ConcDirAndName, LASTVERSIONARRAY, ConcNam...
#include "lspglob.h"
#include "lsptypes.h"
#include "timeout.h"        // for TIMEOUT, ERRSETJMP, S_TOUT, TIMEOUT0
#include "ufsdefs.h"        // for unixpathname, lisppathname

#ifndef DOS
#include <dirent.h>         // for MAXNAMLEN, readdir, closedir, opendir
#include <pwd.h>            // for getpwuid, passwd
#include <sys/param.h>      // for MAXPATHLEN
#include <sys/statvfs.h>    // for statvfs
#include <sys/time.h>       // for timeval, utimes
#else
#include <direct.h>
#include <dos.h>
#include <time.h>
#include <io.h>
#define MAXPATHLEN _MAX_PATH
#define MAXNAMLEM _MAX_PATH
#define alarm(x) 0
#endif

extern int *Lisp_errno;
extern int Dummy_errno;

typedef struct filename_entry {
  char name[MAXPATHLEN]; /* With version, foo.~3~ or foo */
  unsigned version_no;
} FileName;

typedef struct current_varray {
  char path[MAXPATHLEN]; /* pathname of directory */
  char file[MAXPATHLEN]; /* file name  (down cased name) */
  time_t mtime;
} CurrentVArray;

static FileName VersionArray[VERSIONARRAYLENGTH];
static CurrentVArray VArrayInfo;

static int locate_file(char *dir, char *name);
static int make_directory(char *dir);
static int maintain_version(char *file, FileName *varray, int forcep);
static int get_versionless(FileName *varray, char *file, char *dir);
static int check_vless_link(char *vless, FileName *varray, char *to_file, int *highest_p);
static int get_old(char *dir, FileName *varray, char *afile, char *vfile);
static int get_oldest(char *dir, FileName *varray, char *afile, char *vfile);
static int get_new(char *dir, FileName *varray, char *afile, char *vfile);
static int get_old_new(char *dir, FileName *varray, char *afile, char *vfile);
static int get_version_array(char *dir, char *file, FileName varray[], CurrentVArray *cache);

#ifdef DOS
static void separate_drive(char *lfname, char *drive)
{
  char *cp;

  cp = lfname;

  /* Check if there's a drive specified. */

  if (*(cp + 1) == DRIVESEP) {
    *drive = *cp; /* copy the drive letter, if there is one */
    cp++;
    cp++;       /* Move to the real `<`/ */
    while (*cp) /* Move the rest to the left to cover. */
    {
      *(cp - 2) = *cp;
      cp++;
    }
    *(cp - 2) = '\0';
  } else
    *drive = '\0'; /* no drive */
}
#endif /* DOS */

/*
 * Name:	separate_host
 *
 * Argument:	char	*lfname	Lisp full file name including host field.
 *		char	*host	The place where host field will be stored.
 *
 * Value:	void
 *
 * Side Effect:	lfname will be replaced with the file name except host field,
 *		and host will be replaced with the host field.
 *
 * Description:
 *
 * Accepts a Lisp full file name from Lisp code, and separate the host field
 * from other components.  The result will be appropriate form to pass to
 * unixpathname.  For convenience to unixpathname, the initial directory
 * delimiter will be removed from lfname except the case lfname specifies the
 * very root directory.  And if the lfname is regarded as other directory,
 * the trail directory delimiter is also removed.
 *
 */

#ifdef DOS
void separate_host(char *lfname, char *host, char *drive)
#else
void separate_host(char *lfname, char *host)
#endif /* DOS */
{
  char *cp;
  ptrdiff_t diff;

  cp = lfname + 1; /* Skip the initial "{". */

  while (*cp != '}') *host++ = *cp++;
  *host = '\0';

  cp++; /* Now, *cp == '<' or drive letter. */
#ifdef DOS
  /* Check if there's a drive specified. */

  if (*(cp + 1) == DRIVESEP) {
    *drive = *cp; /* copy the drive letter, if there is one */
    cp++;
    cp++; /* Move to the real `<`/ */
  } else
    *drive = '\0'; /* no drive */
#endif             /* DOS */

  if (*(cp + 1) == '\0') {
    /* Root directory is specified. */
    *lfname = '<';
    *(lfname + 1) = '\0';
  } else {
    diff = cp - lfname;
    if (*cp == '<' || *cp == DIRSEP
#ifdef DOS
        || *cp == UNIXDIRSEP
#endif /* DOS */
        ) {
      /*
       * Skip the initial directory delimiter.
       */
      cp++;
      diff++;
    }
    while (*cp) {
      *(cp - diff) = *cp;
      cp++;
    }
    if (*(cp - 1) == '>' && *(cp - 2) != '\'') {
      /*
       * The last character is a not quoted directory
       * delimiter.  We have to remove it from the result
       * lfname for the convenience of unixpathname.
       */
      *(cp - diff - 1) = '\0';
    } else {
      *(cp - diff) = '\0';
    }
  }
}

/*
 * Name:	COM_openfile
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name which is following the Xerox
 *				 Lisp file naming convention.
 *				args[1] Recognition mode.  See IRM.
 *				args[2] Access mode.  See IRM.
 *				args[3]	The place where the creation date of the
 *				        opened file should be stored.
 *				args[4] The place where the size of the opened file
 *				        should be stored.
 *				args[5] The place where the error number should be
 *				        stored.
 *
 * Value:	If succeed, returns the Lisp smallp which represents the open
 *		file descriptor, otherwise Lisp NIL.
 *
 * Side Effect:	If succeed, cdate(args[3]) and size(args[4]) will hold the
 *		creation date and file size respectively.
 *
 * Description:
 *
 * The implementation of OPENFILE FDEV method for DSK and UNIX device.  Try to
 * open a specified file.
 */

LispPTR COM_openfile(LispPTR *args)
{
  char lfname[MAXPATHLEN + 5], file[MAXPATHLEN], host[MAXNAMLEN];
  char dir[MAXPATHLEN], name[MAXNAMLEN], ver[VERSIONLEN];
  int fatp, dskp, rval, fd, link_check_flg, flags, *bufp;
  size_t slen;
  struct stat sbuf;
#ifdef DOS
  char drive[1]; /* Drive designator */
  int extlen;    /* length of the raw file extension */
  char rawname[MAXNAMLEN];
#endif /* DOS */
  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[5]);

  LispStringLength(args[0], slen, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  slen = fatp ? slen + 4 + 1 : slen + 2 + 1;
  /* Add five for the host name field in Lisp format. */
  if (slen > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);

#ifdef DOS
  separate_host(lfname, host, drive);
#else
  separate_host(lfname, host);
#endif /* DOS */
  UPCASE(host);

  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

/*
 * Convert a Lisp file name to UNIX one.  If host is DSK, we also have to
 * convert a version field.
 */
#ifdef DOS
  unixpathname(lfname, file, dskp, 0, drive, &extlen, rawname);
#else
  unixpathname(lfname, file, dskp, 0);
#endif

  /*
   * Set up the flags argument for open system call.
   * And we have to handle the non existing directory case if the device is
   * DSK.
   * link_check_flg is used to determine whether we have to check a hard-link
   * based version control after opening a file.
   */
  link_check_flg = 0;
  switch (args[1]) {
    case RECOG_OLD:
    case RECOG_OLDEST:
      switch (args[2]) {
        case ACCESS_INPUT: flags = O_RDONLY; break;

        case ACCESS_OUTPUT:
          /*
           * The current implementation of Lisp page mapped device requires
           * that the output stream being "readable"!
           */
          flags = O_RDWR | O_TRUNC;
          break;

        case ACCESS_BOTH: flags = O_RDWR; break;

        case ACCESS_APPEND:
          /*
           * Should be O_WRONLY | O_APPEND.  But Lisp needs it.
           */
          flags = O_RDWR;
          break;
      }
      break;
    case RECOG_NEW:
    case RECOG_OLD_NEW:
      /*
       * In DSK device, the not existing yet file can be recognized.  In this
       * case, if there is a chance to create a new file, we have to make
       * sure that all directory to reach the recognized file exists.
       * Also we have to check the versionless file is correctly maintained
       * or not when we have a chance to create a new file.
       */
      switch (args[2]) {
        case ACCESS_INPUT:
          if (args[1] == RECOG_NEW) {
            /*
             * Opening a input stream to a new, not yet
             * existing, file does not make sense.
             */
            return (NIL);
          } else {
            /*
             * Even if OLD/NEW recognition, opening a input
             * stream never try to create a new file.  Thus,
             * without O_CREAT.
             */
            flags = O_RDONLY;
          }
          break;

        case ACCESS_OUTPUT:
          flags = O_RDWR | O_TRUNC | O_CREAT;
          unpack_filename(file, dir, name, ver, 1);
          if (make_directory(dir) == 0) return (NIL);
          if (dskp) link_check_flg = 1;
          break;

        case ACCESS_BOTH:
          flags = O_RDWR | O_CREAT;
          unpack_filename(file, dir, name, ver, 1);
          if (make_directory(dir) == 0) return (NIL);
          if (dskp) link_check_flg = 1;
          break;

        case ACCESS_APPEND:
          flags = O_RDWR | O_CREAT;
          unpack_filename(file, dir, name, ver, 1);
          if (make_directory(dir) == 0) return (NIL);
          if (dskp) link_check_flg = 1;
          break;
      }
      break;

    default: return (NIL);
  }

  /*
   * The file name which has been passed from Lisp is sometimes different
   * from the actual file name on DSK, even after the Lisp name is converted
   * to UNIX form with unixpathname.  Lisp always recognize a file on DSK
   * with version.  If the versionless file exists and it is not correctly
   * maintained, that is it is not hard linked to the existing highest
   * versioned file, Lisp regards such link missing versionless file as
   * the highest versioned file, but the actual name on the file system
   * is still versionless.
   * get_old, get_oldest, get_new, get_old_new routines handle all of the
   * complicated cases correctly and let us know the "Lisp recognizing"
   * name and "Real" name.  Both of them are UNIX format.
   * At this point, we will use one of the four routines and get the
   * real name.  We can use it to open a file which is requested from Lisp
   * with the "Lisp recognizing" name.
   */

  if (dskp) {
    if (unpack_filename(file, dir, name, ver, 1) == 0) return (NIL);
    if (true_name(dir) != -1) return (0);
    if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);
    ConcNameAndVersion(name, ver, file);

    switch (args[1]) {
      case RECOG_OLD:
        if (get_old(dir, VersionArray, file, name) == 0) return (NIL);
        break;

      case RECOG_OLDEST:
        if (get_oldest(dir, VersionArray, file, name) == 0) return (NIL);
        break;

      case RECOG_NEW:
        if (get_new(dir, VersionArray, file, name) == 0) return (NIL);
        break;

      case RECOG_OLD_NEW:
        if (get_old_new(dir, VersionArray, file, name) == 0) return (NIL);
        break;

      default: return (NIL);
    }
  }

  /*
   * DSK device only allow to open a regular file.
   */
  if (dskp) {
    TIMEOUT(rval = stat(file, &sbuf));
    if (rval == 0) {
      if (!S_ISREG(sbuf.st_mode)) {
        /*
         * The Lisp code handles this case as same as "file table
         * overflow" error.  Final error message is "File won't
         * open".
         */
        *Lisp_errno = ENFILE;
        return (NIL);
      }
    } else {
      /*
       * When stat failed, only if the reason is "file does not
       * exist" and we are trying to open a file with a mode we can
       * create a new file, we can proceed.
       */
      if (errno != ENOENT || !link_check_flg) {
        *Lisp_errno = errno;
        return (NIL);
      }
    }
  }
  if (dskp && link_check_flg) {
    /*
     * When we are opening a file with a mode we might create a new file,
     * we have to make sure that versionless file is maintained
     * correctly before we actually creating a new file, because a
     * created new file will change the status and the recognition on
     * the same file with the same recognition mode will return the
     * different result.
     * At this point, the third argument for maintain_version, forcep is
     * 1, because a lonly versionless file should be linked to version 1.
     * If we are opening a file recognized with new mode, version 2,
     * without pre-linking a versionless to version 1, the final
     * clean up maintain_version will link the versionless to version 3.
     */
    TIMEOUT(rval = access(file, F_OK));
    if (rval == -1) {
      if (errno == ENOENT) {
        /*
         * Actually we are creating a new file.  We have to
         * maintain a version status.
         */
        if (maintain_version(file, (FileName *)NULL, 1) == 0) {
          TIMEOUT(rval = close(fd));
          *Lisp_errno = errno;
          return (NIL);
        }
      } else {
        /*
         * Because of other reason, access call failed.
         */
        *Lisp_errno = errno;
        return (NIL);
      }
    } else {
/*
 * The subjective file has already existed.  We don't need
 * to maintain a version.
 */
#ifdef DOS
      if (args[1] == RECOG_NEW) {
        char old[MAXPATHLEN];
        make_old_version(old, file);
        unlink(old);
        rename(file, old); /* make old version */
      }
#endif /* DOS */
    }
  }

  /*
   * If a new file is created, its actual mode is computed from the
   * third argument for open and the process's umask.  I'm pretty sure
   * 0666 would be most appropriate mode to specify here.
   */
  TIMEOUT(fd = open(file, flags, 0666));
  if (fd == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

  if (dskp && link_check_flg) {
    /*
     * Again we have to maintain version to clean up the directory.
     * This time we invoke maintain_version with forcep off, because
     * the entirely newly created file, versionless file, should not
     * be linked to any file.
     */
    if (maintain_version(file, (FileName *)NULL, 0) == 0) {
      TIMEOUT(close(fd));
      *Lisp_errno = errno;
      return (NIL);
    }
  }

  TIMEOUT(rval = fstat(fd, &sbuf));
  if (rval == -1) {
    TIMEOUT(close(fd));
    *Lisp_errno = errno;
    return (NIL);
  }

  bufp = (int *)NativeAligned4FromLAddr(args[3]);
  *bufp = ToLispTime(sbuf.st_mtime);

  bufp = (int *)NativeAligned4FromLAddr(args[4]);
  if (!dskp && (!S_ISREG(sbuf.st_mode)) && (!S_ISDIR(sbuf.st_mode))) {
    /*
     * Not a regular file or directory file.  Put on a marker.
     */
    *bufp = SPECIALFILEMARK;
  } else {
    *bufp = sbuf.st_size;
  }

  return (GetSmallp(fd));
}

/*
 * Name:	COM_closefile
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name which is following the Xerox
 *				 Lisp file naming convention.  Including a host
 *				 field.
 *				args[1]
 *				 The Lisp integer representing a file descriptor
 *				 of the file being closed.
 *				args[2]
 *				 The creation date of the file.
 *				args[3]
 *				 The place where the error number should be
 *				 stored.
 *
 * Value:	If succeed, returns the Lisp T otherwise, Lisp NIL.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * The implementation of CLOSEFILE FDEV method for DSK and UNIX device.  Try to
 * close a specified file.
 * The creation date file attribute in Lisp sense is kept in st_mdate field of
 * UNIX structure.  To keep the creation date of the file, it is maintained by
 * Lisp and it is passed to COM_closefile.
 */

LispPTR COM_closefile(LispPTR *args)
{
#ifdef DOS

  int fd, dskp, rval;
  time_t cdate;
  char lfname[MAXPATHLEN + 5], host[MAXNAMLEN];
  char file[MAXPATHLEN], dir[MAXPATHLEN], name[MAXNAMLEN + 1];
  char ver[VERSIONLEN], drive[1];
  struct find_t dirp;
  int dp;
  struct stat sbuf;
  ino_t ino;
  int extlen;
  char rawname[MAXNAMLEN];

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  LispStringLength(args[0], rval, dskp);

  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = dskp ? rval + 4 + 1 : rval + 2 + 1;
  /* Add five for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);

  separate_host(lfname, host, drive);

  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

  /*
   * Convert a Lisp file name to UNIX one.  If host is DSK, we also have to
   * convert a version field.
   */
  dskp ? unixpathname(lfname, file, 1, 0, drive, &extlen, rawname)
       : unixpathname(lfname, file, 0, 0, drive, &extlen, rawname);
  fd = LispNumToCInt(args[1]);
  cdate = (time_t)LispNumToCInt(args[2]);
  if (!dskp) {
    TIMEOUT(rval = fstat(fd, &sbuf));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (NIL);
    }
  }

  if (cdate == 0) {
    /* Just close. */
    TIMEOUT(rval = close(fd));
    if (rval == -1) {
      if (!dskp && errno == EPERM && !S_ISREG(sbuf.st_mode)) {
        /*
         * On {UNIX} device, closing a special file we are not
         * the owner of it.  Although I don't think close fails
         * because of EPERM, in honor of Medley 1.1 code, I put
         * this segment here.
         */
        return (ATOM_T);
      } else {
        *Lisp_errno = errno;
        return (NIL);
      }
    } else {
      return (ATOM_T);
    }
  }

  if (!unpack_filename(file, dir, name, ver, 1)) return (NIL);

  if (dskp) {
    /*
     * On {DSK}, we have to make sure dir is case sensitively existing
     * directory.
     */
    if (true_name(dir) != -1) return (NIL);

    /*
     * There is a very troublesome problem here.  The file name Lisp
     * recognizes is not always the same as the name which COM_openfile
     * used to open the file.  Sometimes COM_openfile uses the versionless
     * file name to open a file, although Lisp always recognizes with
     * *versioned* file name.
     * Thus, we compare i-node number of the requested file with ones of all
     * of files on the directory.   This is time spending implementation.
     * More clean up work is needed.
     */
    TIMEOUT(rval = fstat(fd, &sbuf));
    if (rval != 0) {
      *Lisp_errno = errno;
      return (NIL);
    }
    ino = sbuf.st_ino;
    TIMEOUT(rval = _dos_findfirst(dir, _A_SUBDIR, &dirp));
    if (rval < 0) {
      *Lisp_errno = errno;
      return (NIL);
    }

    for (; rval == 0; S_TOUT(rval = _dos_findnext(&dirp))) {
      sprintf(file, "%s\\%s", dir, dirp.name);
    }
  }
#ifndef DOS /* effectively NEVER, since we're in an ifdef DOS */
  time[0].tv_sec = (long)sbuf.st_atime;
  time[0].tv_usec = 0L;
  time[1].tv_sec = (long)ToUnixTime(cdate);
  time[1].tv_usec = 0L;
#endif /* DOS */
  TIMEOUT(rval = close(fd));
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }
#ifndef DOS
  TIMEOUT(rval = utimes(file, time));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (NIL);
  }
#endif /* DOS, internal */
#else  /* UNIX version of CLOSEFILE */
  int fd, fatp, dskp, rval;
  time_t cdate;
  char lfname[MAXPATHLEN + 5], host[MAXNAMLEN];
  char file[MAXPATHLEN], dir[MAXPATHLEN], name[MAXNAMLEN + 1];
  char ver[VERSIONLEN];
  DIR *dirp;
  struct dirent *dp;
  struct stat sbuf;
  struct timeval time[2];
  ino_t ino;

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  LispStringLength(args[0], rval, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = fatp ? rval + 4 + 1 : rval + 2 + 1;
  /* Add five for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);

  separate_host(lfname, host);
  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

  /*
   * Convert a Lisp file name to UNIX one.  If host is DSK, we also have to
   * convert a version field.
   */
  dskp ? unixpathname(lfname, file, 1, 0) : unixpathname(lfname, file, 0, 0);

  fd = LispNumToCInt(args[1]);
  cdate = (time_t)LispNumToCInt(args[2]);

  if (!dskp) {
    TIMEOUT(rval = fstat(fd, &sbuf));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (NIL);
    }
  }

  if (cdate == 0) {
    /* Just close. */
    TIMEOUT(rval = close(fd));
    if (rval == -1) {
      if (!dskp && errno == EPERM && !S_ISREG(sbuf.st_mode)) {
        /*
         * On {UNIX} device, closing a special file we are not
         * the owner of it.  Although I don't think close fails
         * because of EPERM, in honor of Medley 1.1 code, I put
         * this segment here.
         */
        return (ATOM_T);
      } else {
        *Lisp_errno = errno;
        return (NIL);
      }
    } else {
      return (ATOM_T);
    }
  }

  if (!unpack_filename(file, dir, name, ver, 1)) return (NIL);

  if (dskp) {
    /*
     * On {DSK}, we have to make sure dir is case sensitively existing
     * directory.
     */
    if (true_name(dir) != -1) return (NIL);

    /*
     * There is a very troublesome problem here.  The file name Lisp
     * recognizes is not always the same as the name which COM_openfile
     * used to open the file.  Sometimes COM_openfile uses the versionless
     * file name to open a file, although Lisp always recognizes with
     * *versioned* file name.
     * Thus, we compare i-node number of the requested file with ones of all
     * of files on the directory.   This is time spending implementation.
     * More clean up work is needed.
     */
    TIMEOUT(rval = fstat(fd, &sbuf));
    if (rval != 0) {
      *Lisp_errno = errno;
      return (NIL);
    }
    ino = sbuf.st_ino;

    errno = 0;
    TIMEOUT0(dirp = opendir(dir));
    if (dirp == (DIR *)NULL) {
      *Lisp_errno = errno;
      return (NIL);
    }

    for (S_TOUT(dp = readdir(dirp)); dp != (struct dirent *)NULL || errno == EINTR;
         errno = 0, S_TOUT(dp = readdir(dirp)))
      if (dp) {
        if (ino == (ino_t)dp->d_ino) sprintf(file, "%s/%s", dir, dp->d_name);
      }
    TIMEOUT(closedir(dirp));
  }

  time[0].tv_sec = (long)sbuf.st_atime;
  time[0].tv_usec = 0L;
  time[1].tv_sec = (long)ToUnixTime(cdate);
  time[1].tv_usec = 0L;

  TIMEOUT(rval = close(fd));
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

  TIMEOUT(rval = utimes(file, time));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (NIL);
  }

#endif /* DOS */

  return (ATOM_T);
}

/*
 * Name:	DSK_getfilename
 *
 * Argument:	LispPTR	*args	args[0] Full file name in Lisp format.
 *				args[1] Recognition mode.  See IRM.
 *				args[2] Name area where the recognized full file name
 *				 will be stored.
 *				args[3] The place where the error number should be
 *				 stored.
 *
 * Value:	If succeed, returns the Lisp smallp which represents the length
 *		of the recognized full file name, otherwise Lisp NIL.
 *
 * Side Effect:	If succeed, name area (args[2]) will be replaced with the
 *		recognized full file name.
 *
 * Description:
 *
 * The implementation of GETFILENAME FDEV method for DSK device.  Performs the
 * recognition on the specified name.  Does not check if OPENFILE actually
 * can open the file with the specified mode or not.
 */

LispPTR DSK_getfilename(LispPTR *args)
{
  char *base;
  size_t len;
  int rval;
  int dirp;
  int fatp;
  char lfname[MAXPATHLEN];
  char aname[MAXNAMLEN];
  char vname[MAXPATHLEN];
  char file[MAXPATHLEN];
  char dir[MAXPATHLEN];
  char name[MAXNAMLEN];
  char ver[VERSIONLEN];
#ifdef DOS
  char drive[1], rawname[MAXNAMLEN];
  int extlen; /* len of extension, for making backup filename */
#endif        /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  LispStringLength(args[0], len, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  len = fatp ? len + 4 + 1 : len + 2 + 1;
  if (len > MAXPATHLEN) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);

#ifdef DOS
  separate_drive(lfname, drive);
#endif

/*
 * Convert a Lisp file name to UNIX one.  This is a DSK device method.
 * Thus we have to convert a version field too.  Third argument for
 * unixpathname specifies it.
 */
#ifdef DOS
  if (unixpathname(lfname, file, 1, 0, drive, &extlen, rawname) == 0) return (NIL);
#else
  if (unixpathname(lfname, file, 1, 0) == 0) return (NIL);
#endif

  if (unpack_filename(file, dir, name, ver, 1) == 0) return (NIL);

  switch (args[1]) {
    case RECOG_OLD:
      /*
       * "Old" file means the "newest existing" file.  Thus, we have to
       * check dir is an existing directory or not.  The search has to
       * be done in case insensitive manner.  true_name does this work.
       */

      if (true_name(dir) != -1) {
        /* No such directory. */
        return (NIL);
      }
      /*
       * At this point, true_name has converted dir to the "true" name
       * of the directory.
       */
      if (strcmp(name, "") == 0) {
        /*
         * The file name is specified with a trail directory delimiter.
         * We should recognize it as a directory.
         */
        strcpy(aname, dir);
        strcpy(vname, dir);
        dirp = 1;
      } else {
        /*
         * Recognizing a file on DSK device needs the version information.
         * We gather version information in a version array first.
         */
        if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);

        ConcNameAndVersion(name, ver, aname);
        if (get_old(dir, VersionArray, aname, vname) == 0) return (NIL);

        if ((rval = true_name(aname)) == 0) return (NIL);
        if (rval == -1) {
          /*
           * The specified file is a directory file.
           */
          strcpy(vname, aname);
          dirp = 1;
        } else {
#ifdef DOS
          strcpy(vname, aname);
#endif
          dirp = 0;
        }
      }

      break;

    case RECOG_OLDEST:
      /*
       * "Oldest" file means the "oldest existing" file.  Thus, we have to
       * check dir is an existing directory or not.
       */
      if (true_name(dir) != -1) {
        /* No such directory. */
        return (NIL);
      }
      if (strcmp(name, "") == 0) {
        /*
         * The file name is specified with a trail directory delimiter.
         * We should recognize it as a directory.
         */
        strcpy(aname, dir);
        strcpy(vname, dir);
        dirp = 1;
      } else {
        if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);

        ConcNameAndVersion(name, ver, aname);
        if (get_oldest(dir, VersionArray, aname, vname) == 0) return (NIL);

        if ((rval = true_name(aname)) == 0) return (NIL);
        if (rval == -1) {
          /*
           * The specified file is a directory file.
           */
          strcpy(vname, aname);
          dirp = 1;
        } else {
#ifdef DOS
          strcpy(vname, aname);
#endif
          dirp = 0;
        }
      }
      break;

    case RECOG_NEW:
      /*
       * "New" file means the "not existing" file.  Thus it is not
       * necessary that dir is an existing directory.  If dir is not
       * an existing directory, we returns the specified file name
       * as if, the subsequent OPENFILE will find the truth.
       */
      if (true_name(dir) != -1) {
        strcpy(vname, file);
        dirp = 0;
      } else if (strcmp(name, "") == 0) {
        /*
         * The file name is specified with a trail directory delimiter.
         * We should recognize it as a directory.
         */
        strcpy(aname, dir);
        strcpy(vname, dir);
        dirp = 1;
      } else {
        ConcDirAndName(dir, name, aname);
        if ((rval = true_name(aname)) == -1) {
          strcpy(vname, aname);
          dirp = 1;
        } else {
          /*
           * Here, dir is an existing directory.  We have to perform
           * "new" recognition with the version information.
           */
          if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);

          ConcNameAndVersion(name, ver, aname);
          if (get_new(dir, VersionArray, aname, vname) == 0) return (NIL);
          dirp = 0;
        }
      }
      break;

    case RECOG_OLD_NEW:
      /*
       * "Old/new" file means the "newest existing" or "not existing" file.
       * Thus, if dir is not an existing directory, we can return the
       * specified file name.  If it is an existing one, we have to
       * try "old" recognition on the directory first.  If the recognition
       * fails, we try "new" recognition.
       */
      if (true_name(dir) != -1) {
        strcpy(vname, file);
        dirp = 0;
      } else {
        ConcDirAndName(dir, name, aname);
        if ((rval = true_name(aname)) == -1) {
          strcpy(vname, aname);
          dirp = 1;
        } else {
          if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);

          ConcNameAndVersion(name, ver, aname);
          if (get_old_new(dir, VersionArray, aname, vname) == 0) return (NIL);
          dirp = 0;
        }
      }
      break;

    case RECOG_NON:
      /*
       * "Non" recognition is used to recognize a sysout file.  The sysout
       * file is dealt with specially, it does not have any version, even
       * if it is on {DSK} device.  Only we have to do here is to make
       * sure the path to reach to the specified file is an existing
       * directories.  The file name itself is recognized as if.
       */
      if (true_name(dir) != -1) return (NIL);
      ConcDirAndName(dir, name, vname);
      strcpy(aname, vname);
      if (true_name(aname) == -1) {
        strcpy(vname, aname);
        dirp = 1;
      } else {
        dirp = 0;
      }
      if (lisppathname(vname, lfname, dirp, 0) == 0) return (NIL);
      STRING_BASE(args[2], base);
      len = strlen(lfname);

#ifndef BYTESWAP
      strncpy(base, lfname, len + 1);
#else
      StrNCpyFromCToLisp(base, lfname, len + 1);
#endif /* BYTESWAP */

      return (GetPosSmallp(len));
  }

  /*
   * DSK device does not recognize a directory file as a file.  Thus we should
   * return NIL when the recognized file is a directory.
   */
  if (dirp) return (NIL);

/*
 * Now, vname holds the "versioned" full name of the recognized file in UNIX
 * format.  We have to convert it back to Lisp format.  The version field
 * have to be converted.  The fourth argument for lisppathname specifies it.
 */
#ifdef DOS
  /* For DOS, have to assure we use the name asked for, not the */
  /* faked-up oversion-0 name, so reported names match. */
  {
    char dver[VERSIONLEN];
    separate_version(vname, dver, 0);
    ConcDirAndName(dir, name, aname);
    ConcNameAndVersion(aname, dver, vname);
  }
#endif /* DOS */

  if (lisppathname(vname, lfname, dirp, (dirp ? 0 : 1)) == 0) return (NIL);

  STRING_BASE(args[2], base);
  len = strlen(lfname);

#ifndef BYTESWAP
  strncpy(base, lfname, len + 1);
#else
  StrNCpyFromCToLisp(base, lfname, len + 1);
#endif /* BYTESWAP */

  return (GetPosSmallp(len));
}

/*
 * Name:	DSK_deletefile
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name in Lisp format.
 *				args[1]
 *				 The place where the error number should be
 *				 stored.
 *
 * Value:	If succeed, returns the Lisp symbol T, otherwise Lisp NIL.
 *
 * Side Effect:	If succeed, the specified file is unlinked.
 *
 * Description:
 *
 * The implementation of DELETEFILE FDEV method for DSK device.  Try to delete
 * a specified file.
 */

LispPTR DSK_deletefile(LispPTR *args)
{
  char file[MAXPATHLEN], fbuf[MAXPATHLEN], vless[MAXPATHLEN];
  char dir[MAXPATHLEN], ver[VERSIONLEN];
  int rval, fatp;
  FileName *varray;
#ifdef DOS
  char drive[1], rawname[MAXNAMLEN];
  int extlen; /* len of extension, for making backup filename */
#endif        /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[1]);

  LispStringLength(args[0], rval, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = fatp ? rval + 4 + 1 : rval + 2 + 1;
  if (rval > MAXPATHLEN) FileNameTooLong(NIL);

  LispStringToCString(args[0], fbuf, MAXPATHLEN);
#ifdef DOS
  separate_drive(fbuf, drive);
  unixpathname(fbuf, file, 1, 0, drive, &extlen, rawname);
#else
  unixpathname(fbuf, file, 1, 0);
#endif

  if (unpack_filename(file, dir, fbuf, ver, 1) == 0) return (NIL);
  if (get_version_array(dir, fbuf, VersionArray, &VArrayInfo) == 0) return (NIL);
  varray = VersionArray;

  if (NoFileP(varray))
    return (NIL); /*
                   * If the specified file is deleted from
                   * outside of Lisp during the last time
                   * Lisp recognize it and now, this case
                   * will occur.
                   */

  /*
   * Although the file should have been recognized with "oldest" mode in Lisp
   * code, we have to recognize it again to know the "real" accessible name
   * of it.
   */

  ConcNameAndVersion(fbuf, ver, file);
  if (get_oldest(dir, varray, file, fbuf) == 0) return (NIL);

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file.  All we have to do is to simply
     * try to unlink the specified file.
     */
    TIMEOUT(rval = unlink(file));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (NIL);
    }
    return (ATOM_T);
  }

  /*
   * If a versionless file exists, we have to check the link status of it,
   * because deleting a versionless file or a file to which a versionless
   * file is linked will destroy the consistency of the version status.
   */

  if (check_vless_link(vless, varray, fbuf, &rval) == 0) return (NIL);

  if (strcmp(file, vless) == 0 || strcmp(file, fbuf) == 0) {
    if (*fbuf != '\0') {
      /*
       * Both of the versionless file and the file to which the
       * versionless file is linked have to be unlinked.
       */
      TIMEOUT(rval = unlink(vless));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (NIL);
      }
      TIMEOUT(rval = unlink(fbuf));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (NIL);
      }
      /*
       * Finally, we have to maintain the version status.
       */
      if (maintain_version(vless, (FileName *)NULL, 0) == 0) return (NIL);
      return (ATOM_T);
    } else {
      /*
       * Although the versionfile is specified, it is not linked
       * to any file in varray.  We should not maintain the version
       * status after deleting the versionless file, because
       * we cannot say whether the versionless file is actually under
       * control of the Medley DSK file system or not.
       */
      TIMEOUT(rval = unlink(vless));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (NIL);
      }
      return (ATOM_T);
    }
  } else {
    /*
     * Just unlink the specified file.
     */
    TIMEOUT(rval = unlink(file));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (NIL);
    }
    return (ATOM_T);
  }
}

/*
 * Name:	DSK_renamefile
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name in Lisp format.  The file which
 *				 is being renamed.
 *				args[1]
 *				 Full file name in Lisp format.  The file to which
 *				 args[0] is being renamed.
 *				args[2]
 *				 The place where the error number should be
 *				 stored.
 *
 * Value:	If succeed, returns the Lisp symbol T, otherwise Lisp NIL.
 *
 * Side Effect:	If succeed, the specified file is unlinked.
 *
 * Description:
 *
 * The implementation of RENAMEFILE FDEV method for DSK device.  Try to rename
 * a specified file.
 */

LispPTR DSK_renamefile(LispPTR *args)
{
  char src[MAXPATHLEN], dst[MAXPATHLEN];
  char fbuf[MAXPATHLEN], vless[MAXPATHLEN], svless[MAXPATHLEN];
  char dir[MAXPATHLEN], ver[VERSIONLEN];
  int rval, fatp;
  int need_maintain_flg;
  FileName *varray;
#ifdef DOS
  char drive1[1], drive2[1];
  int extlen1, extlen2; /* len of extension */
  char rawname1[MAXNAMLEN], rawname2[MAXNAMLEN];
#endif /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[2]);

  LispStringLength(args[0], rval, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = fatp ? rval + 4 + 1 : rval + 2 + 1;
  if (rval > MAXPATHLEN) FileNameTooLong(NIL);

  LispStringLength(args[1], rval, fatp);
  rval = fatp ? rval + 4 + 1 : rval + 2 + 1;
  if (rval > MAXPATHLEN) FileNameTooLong(NIL);

  LispStringToCString(args[0], fbuf, MAXPATHLEN);
#ifdef DOS
  separate_drive(fbuf, drive1);
  unixpathname(fbuf, src, 1, 0, drive1, &extlen1, rawname1);
#else  /* DOS */
  unixpathname(fbuf, src, 1, 0);
#endif /* DOS */

  LispStringToCString(args[1], fbuf, MAXPATHLEN);
#ifdef DOS
  separate_drive(fbuf, drive2);
  unixpathname(fbuf, dst, 1, 0, drive2, &extlen2, rawname2);
#else  /* DOS */
  unixpathname(fbuf, dst, 1, 0);
#endif /* DOS */

  if (unpack_filename(dst, dir, fbuf, ver, 1) == 0) return (NIL);
  /*
   * The destination file has been recognized as new file.  Thus we have
   * to make sure that the directory exists.
   */
  if (make_directory(dir) == 0) return (NIL);

  /*
   * We maintain the destination to handle the link damaged case correctly.
   */
  ConcDirAndName(dir, fbuf, dst);
  if (maintain_version(dst, (FileName *)NULL, 0) == 0) return (NIL);

  if (get_version_array(dir, fbuf, VersionArray, &VArrayInfo) == 0) return (NIL);
  varray = VersionArray;

  /*
   * Although the file should have been recognized with "new" mode in Lisp
   * code, we have to recognize it again to know the "real" accessible name
   * of it.
   */

  ConcNameAndVersion(fbuf, ver, dst);
  if (get_new(dir, varray, dst, fbuf) == 0) return (NIL);

  /*
   * At this point, there are three cases for the destination.  If there is
   * no member of the destination family, there is nothing to do.  If there
   * is only a versionless file and, if the "real" destination is not the
   * versionless, we have to rename it to version 1.  And last case, if the
   * "real" destination file is the file to which the versionless file is linked,
   * we have to unlink the versionless file.
   */
  if (!NoFileP(varray)) {
    if (OnlyVersionlessP(varray)) {
      get_versionless(varray, vless, dir);
      if (strcmp(dst, vless) != 0) {
        ConcNameAndVersion(vless, "1", fbuf);
        TIMEOUT(rval = rename(vless, fbuf));
        if (rval == -1) {
          *Lisp_errno = errno;
          return (NIL);
        }
      }
    } else {
      /*
       * We are sure that the versionless file is linked to one of
       * the higher versioned file here.
       */
      get_versionless(varray, vless, dir);
      if (check_vless_link(vless, varray, fbuf, &rval) == 0) { return (NIL); }
      if (strcmp(dst, fbuf) == 0) {
        TIMEOUT(rval = unlink(vless));
        if (rval == -1) {
          *Lisp_errno = errno;
          return (NIL);
        }
      }
    }
  }

  if (unpack_filename(src, dir, fbuf, ver, 1) == 0) return (NIL);
  if (get_version_array(dir, fbuf, varray, &VArrayInfo) == 0) return (NIL);

  if (NoFileP(varray))
    return (NIL); /*
                   * If the specified file is deleted from
                   * outside of Lisp during the last time
                   * Lisp recognize it and now, this case
                   * will occur.
                   */

  /*
   * Although the file should have been recognized with "old" mode in Lisp
   * code, we have to recognize it again to know the "real" accessible name
   * of it.
   */
  ConcNameAndVersion(fbuf, ver, src);
  if (get_old(dir, varray, src, fbuf) == 0) return (NIL);

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file.  All we have to do is to simply
     * try to rename the specified file.
     */
    need_maintain_flg = 0;
  } else {
    /*
     * If a versionless file exists, we have to check the link status
     * of it, because renaming a versionless file or a file to which a
     * versionless file is linked will destroy the consistency of the
     * version status.
     */
    if (check_vless_link(vless, varray, fbuf, &rval) == 0) return (NIL);

    if (strcmp(src, vless) == 0 && *fbuf != '\0') {
      /*
       * At this point, we only unlink the file to which the
       * versionless is linked.  The versionless fill will be
       * renamed later.
       */
      TIMEOUT(rval = unlink(fbuf));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (NIL);
      }
      need_maintain_flg = 1;
    } else if (strcmp(src, fbuf) == 0) {
      TIMEOUT(rval = unlink(vless));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (NIL);
      }
      need_maintain_flg = 1;
    } else {
      need_maintain_flg = 0;
    }
    strcpy(svless, vless);
  }

  /*
   * At this point, src holds the full file name to be renamed, and dst holds
   * the full file name to which src will be renamed.
   */

  TIMEOUT(rval = rename(src, dst));
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

  /*
   * The destination directory always have to be maintained, because it is
   * now under control of DSK device.
   * The source directory have to be maintained only if need_maintain_flg
   * is on.
   */

  if (maintain_version(dst, (FileName *)NULL, 0) == 0) return (NIL);
  if (need_maintain_flg) {
    if (maintain_version(src, (FileName *)NULL, 0) == 0) return (NIL);
  }

  return (ATOM_T);
}

/*
 * Name:	DSK_directorynamep
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Directory name in Lispformat.  Both of the initial
 *				 and trail directory delimiter are stripped by Lisp
 *				 code.  Only one exception is a "root directory".
 *				 "Root directory is represented as ">".
 *				args[1]
 *				 The place where the "true" name of the directory
 *				 in Lisp format will be stored.
 *				args[2]
 *				 The place where the error number should be stored.
 *				 Not used in the current Lisp code implementation.
 *
 * Value:	If succeed, returns the Lisp smallp which represents the length
 *		of the "true" name of the directory, otherwise Lisp NIL.
 *
 * Side Effect:	If the directory is recognized as a valid directory representation,
 *		args[1] is replaced with the "true" directory name.
 *
 * Description:
 *
 * The implementation of the DIRECTORYNAMEP FDEV method for DSK device.
 * Performs the recognition as well. Accepts the directory representation which
 * obeys the Xerox Lisp file naming convention. The "true" name which is stored
 * on the area specified with the second argument also follows the Xerox Lisp
 * file naming convention, and it includes the initial and trail directory
 * delimiter. Thus the Lisp code does not have to worry about the conversion of
 * the directory name representation.
 */

LispPTR DSK_directorynamep(LispPTR *args)
{
  char dirname[MAXPATHLEN];
  char fullname[MAXPATHLEN];
  size_t len;
  int fatp;
  char *base;
#ifdef DOS
  char drive[1], rawname[MAXNAMLEN];
  int extlen; /* len of extension, for making backup filename */
#endif        /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[2]);

  LispStringLength(args[0], len, fatp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  len = fatp ? len + 4 + 1 : len + 2 + 1;
  /* -2 for the initial and trail directory delimiter. */
  if (len > MAXPATHLEN - 2) FileNameTooLong(NIL);

  LispStringToCString(args[0], dirname, MAXPATHLEN);

/* Convert Xerox Lisp file naming convention to Unix one. */
#ifdef DOS
  separate_drive(dirname, drive);
  if (unixpathname(dirname, fullname, 1, 0, drive, 0, 0) == 0) return (NIL);
#else  /* DOS*/
  if (unixpathname(dirname, fullname, 1, 0) == 0) return (NIL);
#endif /* DOS */

  if (true_name(fullname) != -1) return (NIL);

  /* Convert Unix file naming convention to Xerox Lisp one. */
  if (lisppathname(fullname, dirname, 1, 0) == 0) return (NIL);

  len = strlen(dirname);
  STRING_BASE(args[1], base);

#ifndef BYTESWAP
  strncpy(base, dirname, len + 1);
#else
  StrNCpyFromCToLisp(base, dirname, len + 1);
#endif /* BYTESWAP */

  return (GetPosSmallp(len));
}

/*
 * Name:	COM_getfileinfo
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name which is following the Xerox
 *				 Lisp file naming convention.  Including a host
 *				 field.
 *				args[1]
 *				 The Lisp pointer which represents the requested
 *				 file attribute.
 *				args[2]
 *				 The place where the requested value will be stored.
 *				args[3]
 *				 The place where the errno will be stored.
 *
 * Value:	If failed, returns Lisp NIL.  If succeed, returned value is
 *		different according to the attribute requested.
 *		In the case of LENGTH, WDATE, RDATE, and PROTECTION, returns Lisp T.
 *		In the case of AUTHOR and ALL, returns the length of the author name
 *		copied into the specified buffer.
 *
 * Side Effect:	The specified buffer will be replaced with the value of the requested
 *		attribute.
 *
 * Description:
 *
 * The implementation of GETFILEINFO FDEV method for DSK and UNIX device.  Try to
 * fetch a value of a specified file attribute.
 * The creation date file attribute in Lisp sense is kept in st_mdate field of
 * UNIX structure, and it is treated as same as the write date file attribute in
 * Lisp sense.
 */

LispPTR COM_getfileinfo(LispPTR *args)
{
  int dskp, rval;
  size_t len;
  unsigned *bufp;
#ifndef DOS
  struct passwd *pwd;
#endif
  char *base;
  char lfname[MAXPATHLEN + 5], file[MAXPATHLEN], host[MAXNAMLEN];
  char dir[MAXPATHLEN], name[MAXNAMLEN], ver[VERSIONLEN];
  struct stat sbuf;
  LispPTR laddr;
#ifdef DOS
  char drive[1], rawname[MAXNAMLEN];
  int extlen; /* len of extension, for making backup filename */
#endif        /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  LispStringLength(args[0], rval, dskp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = dskp ? rval + 4 + 1 : rval + 2 + 1;
  /* Add 5 for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);
#ifdef DOS
  separate_host(lfname, host, drive);
#else
  separate_host(lfname, host);
#endif

  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

/*
 * Convert a Lisp file name to UNIX one.  If host is DSK, we also have to
 * convert a version field.
 */
#ifdef DOS
  unixpathname(lfname, file, dskp, 0, drive, &extlen, rawname);
#else  /* DOS */
  unixpathname(lfname, file, dskp, 0);
#endif /* DOS */

  /*
   * The file name which has been passed from Lisp is sometimes different
   * from the actual file name on DSK, even after the Lisp name is converted
   * to UNIX form with unixpathname.  The Lisp code for GETFILEINFO always
   * recognizes a file with old mode.  Thus, we recognize it again using
   * get_old routine.  It will let us know the "real accessible" name of
   * the file.
   */
  if (dskp) {
    if (unpack_filename(file, dir, name, ver, 1) == 0) return (NIL);
    if (true_name(dir) != -1) return (0);
    if (strcmp(name, "") == 0) {
      /*
       * The directory is specified.
       */
      strcpy(file, dir);
    } else {
      if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);
      ConcNameAndVersion(name, ver, file);
      if (get_old(dir, VersionArray, file, name) == 0) return (NIL);
    }
  }

  TIMEOUT(rval = stat(file, &sbuf));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (NIL);
  }

  switch (args[1]) {
    case LENGTH:
      bufp = (unsigned *)NativeAligned4FromLAddr(args[2]);
      *bufp = (unsigned)sbuf.st_size;
      return (ATOM_T);

    case WDATE:
      bufp = (unsigned *)NativeAligned4FromLAddr(args[2]);
      *bufp = (unsigned)ToLispTime(sbuf.st_mtime);
      return (ATOM_T);

    case RDATE:
      bufp = (unsigned *)NativeAligned4FromLAddr(args[2]);
      *bufp = (unsigned)ToLispTime(sbuf.st_atime);
      return (ATOM_T);

    case PROTECTION:
      bufp = (unsigned *)NativeAligned4FromLAddr(args[2]);
      *bufp = sbuf.st_mode;
      return (ATOM_T);

    case AUTHOR: {
#ifndef DOS
      TIMEOUT0(pwd = getpwuid(sbuf.st_uid));
      if (pwd == (struct passwd *)NULL) {
        /*
         * Returns Lisp 0.  Lisp code handles this case as author
         * unknown.  The returned value from Lisp GETFILEINFO
         * function would be "".
         */
        return (SMALLP_ZERO);
      }
      STRING_BASE(args[2], base);
      len = strlen(pwd->pw_name);
#ifndef BYTESWAP
      strncpy(base, pwd->pw_name, len);
#else
      StrNCpyFromCToLisp(base, pwd->pw_name, len);
#endif /* BYTESWAP */
#endif /* DOS */
      return (GetPosSmallp(len));
    }
    case ALL: {
      /*
       * The format of the buffer which has been allocated by Lisp
       * is as follows.
       * 	((LENGTH	.	fixp)
       *	 (WDATE		.	fixp)
       *	 (RDATE		.	fixp)
       *	 (PROTECTION	.	fixp)
       *	 (AUTHOR	.	string))
       */
      laddr = cdr(car(args[2]));
      bufp = (unsigned *)NativeAligned4FromLAddr(laddr);
      *bufp = sbuf.st_size;

      laddr = cdr(car(cdr(args[2])));
      bufp = (unsigned *)NativeAligned4FromLAddr(laddr);
      *bufp = ToLispTime(sbuf.st_mtime);

      laddr = cdr(car(cdr(cdr(args[2]))));
      bufp = (unsigned *)NativeAligned4FromLAddr(laddr);
      *bufp = ToLispTime(sbuf.st_atime);

      laddr = cdr(car(cdr(cdr(cdr(args[2])))));
      bufp = (unsigned *)NativeAligned4FromLAddr(laddr);
      *bufp = sbuf.st_mode;
#ifndef DOS
      TIMEOUT0(pwd = getpwuid(sbuf.st_uid));
      if (pwd == (struct passwd *)NULL) { return (SMALLP_ZERO); }
      laddr = cdr(car(cdr(cdr(cdr(cdr(args[2]))))));
      STRING_BASE(laddr, base);
      len = strlen(pwd->pw_name);
#ifndef BYTESWAP
      strncpy(base, pwd->pw_name, len);
#else
      StrNCpyFromCToLisp(base, pwd->pw_name, len);
#endif /* BYTESWAP	 */
#endif /* DOS */
      return (GetPosSmallp(len));
    }
    default: return (NIL);
  }
}

/*
 * Name:	COM_setfileinfo
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file name which is following the Xerox
 *				 Lisp file naming convention.  Including a host
 *				 field.
 *				args[1]
 *				 The Lisp pointer which represents the requested
 *				 file attribute.
 *				args[2]
 *				 The value to be stored on the request attribute.
 *				args[3]
 *				 The place where the error number should be
 *				 stored.
 *
 * Value:	If succeed, returns Lisp T, otherwise Lisp NIL.
 *
 * Side Effect:	The specified attribute of a file will be replaced with the specified
 *		value.
 *
 * Description:
 *
 * The implementation of SETFILEINFO FDEV method for DSK and UNIX device.  Try to
 * replace a value of a specified file attribute.
 * In this implementation, only WDATE(as well as Creation Date) and PROTECTION
 * make sense.
 */

LispPTR COM_setfileinfo(LispPTR *args)
{
  int dskp, rval, date;
  char lfname[MAXPATHLEN + 5], file[MAXPATHLEN], host[MAXNAMLEN];
  char dir[MAXPATHLEN], name[MAXNAMLEN], ver[VERSIONLEN];
  struct stat sbuf;
#ifndef DOS
  struct timeval time[2];
#else
  char drive[1], rawname[MAXNAMLEN];
  int extlen;
#endif /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  LispStringLength(args[0], rval, dskp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = dskp ? rval + 4 + 1 : rval + 2 + 1;
  /* Add 5 for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);

#ifdef DOS
  separate_host(lfname, host, drive);
#else
  separate_host(lfname, host);
#endif /* DOS */
  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

/*
 * Convert a Lisp file name to UNIX one.  If host is DSK, we also have to
 * convert a version field.
 */
#ifdef DOS
  unixpathname(lfname, file, dskp, 0, drive, &extlen, rawname);
#else  /* DOS */
  unixpathname(lfname, file, dskp, 0);
#endif /* DOS */

  /*
   * The file name which has been passed from Lisp is sometimes different
   * from the actual file name on DSK, even after the Lisp name is converted
   * to UNIX form with unixpathname.  The Lisp code for SETFILEINFO always
   * recognizes a file with old mode.  Thus, we recognize it again using
   * get_old routine.  It will let us know the "real accessible" name of
   * the file.
   */
  if (dskp) {
    if (unpack_filename(file, dir, name, ver, 1) == 0) return (NIL);
    if (true_name(dir) != -1) return (0);
    if (get_version_array(dir, name, VersionArray, &VArrayInfo) == 0) return (NIL);
    ConcNameAndVersion(name, ver, file);
    if (get_old(dir, VersionArray, file, name) == 0) return (NIL);
  }

  switch (args[1]) {
    case WDATE:
      TIMEOUT(rval = stat(file, &sbuf));
      if (rval != 0) {
        *Lisp_errno = errno;
        return (NIL);
      }
#ifndef DOS
      date = LispNumToCInt(args[2]);
      time[0].tv_sec = (long)sbuf.st_atime;
      time[0].tv_usec = 0L;
      time[1].tv_sec = (long)ToUnixTime(date);
      time[1].tv_usec = 0L;
      TIMEOUT(rval = utimes(file, time));
#endif /* DOS */
      if (rval != 0) {
        *Lisp_errno = errno;
        return (NIL);
      }
      return (ATOM_T);

    case PROTECTION:
      rval = LispNumToCInt(args[2]);
      TIMEOUT(rval = chmod(file, rval));
      if (rval != 0) {
        *Lisp_errno = errno;
        return (NIL);
      }
      return (ATOM_T);

    default: return (NIL);
  }
}

/*
 * Name:	COM_readpage
 *
 * Argument:	LispPTR	*args	args[0]
 *				 The Lisp integer representing a file descriptor
 *				 of the file being read.
 *				args[1]
 *				 The Lisp integer representing a page number of the
 *				 file being read.
 *				args[2]
 *				 The place where the contents of the file will be
 *				 stored.
 *				args[3]
 *				 The place where the error number should be stored.
 *
 * Value:	If succeed, returns a Lisp integer representing a total number of
 *		bytes read, otherwise Lisp NIL.
 *
 * Side Effect:	The specified buffer will be filled with the specified region of the
 *		contents of the file.
 *
 * Description:
 *
 * The implementation of READPAGES FDEV method for DSK and UNIX device.  Try to
 * read a page into a buffer.
 * If a page being read is a last page in a file, and it is not a full page, the
 * remaining region of a buffer will be zero outed.  The size of a page is 512 bytes.
 */

LispPTR COM_readpage(LispPTR *args)
{
  char *bufp;
  int fd, npage, rval;
  ssize_t count;
  off_t offval;
  struct stat sbuf;

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);

  fd = LispNumToCInt(args[0]);
  npage = LispNumToCInt(args[1]);
  bufp = (char *)NativeAligned2FromLAddr(args[2]);

  TIMEOUT(rval = fstat(fd, &sbuf));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (NIL);
  }

  if (S_ISREG(sbuf.st_mode)) {
  /*
   * The request file is a regular file.  We have to make sure that
   * next byte read is at the beginning of the requested page of the
   * file.  If the request file is special file, lseek is not needed.
   */
  sklp:
    TIMEOUT(offval = lseek(fd, (npage * FDEV_PAGE_SIZE), SEEK_SET));
    if (offval == -1) {
      if (errno == EINTR) goto sklp; /* interrupted, retry */
      *Lisp_errno = errno;
      return (NIL);
    }
  }

rdlp:
  TIMEOUT(count = read(fd, bufp, FDEV_PAGE_SIZE));
  if (count == -1) {
    if (errno == EINTR) goto rdlp; /* interrupted; retry */
    *Lisp_errno = errno;
    return (NIL);
  }

  /* O out the remaining part of the buffer. */
  memset(&bufp[count], 0, FDEV_PAGE_SIZE - count);

#ifdef BYTESWAP
  word_swap_page((DLword *)bufp, FDEV_PAGE_SIZE / 4);
#endif /* BYTESWAP */

  return (GetSmallp(count));
}

/*
 * Name:	COM_writepage
 *
 * Argument:	LispPTR	*args	args[0]
 *				 The Lisp integer representing a file descriptor
 *				 of the file being read.
 *				args[1]
 *				 The Lisp integer representing a page number of the
 *				 file being written.
 *				args[2]
 *				 The place in where the next date to be written is
 *				 hold.
 *				args[3]
 *				 The Lisp integer representing a number of bytes
 *				 of data to be written.
 *				args[4]
 *				 The place where the error number should be stored.
 *
 * Value:	If succeed, returns a Lisp T, otherwise Lisp NIL.
 *
 * Side Effect:	The specified page of the file will be replaced with the contents of
 *		the buffer.
 *
 * Description:
 *
 * The implementation of WRITEPAGES FDEV method for DSK and UNIX device.  Try to
 * write a page into a buffer.
 * The actual size of data written is specified with args[3].
 */

LispPTR COM_writepage(LispPTR *args)
{
  int fd;
  int npage;
  char *bufp;
  int count;
  ssize_t rval;
  off_t offval;

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[4]);

  fd = LispNumToCInt(args[0]);
  npage = LispNumToCInt(args[1]);
  bufp = (char *)NativeAligned2FromLAddr(args[2]);
  count = LispNumToCInt(args[3]);

sklp2:
  TIMEOUT(offval = lseek(fd, (npage * FDEV_PAGE_SIZE), SEEK_SET));
  if (offval == -1) {
    if (errno == EINTR) goto sklp2; /* interrupted; retry */
    *Lisp_errno = errno;
    return (NIL);
  }

/* OK to write the page. */

#ifdef BYTESWAP
  word_swap_page((DLword *)bufp, (count + 3) >> 2);
#endif /* BYTESWAP */

wlp:
  TIMEOUT(rval = write(fd, bufp, count));
  if (rval == -1) {
    if (errno == EINTR) goto wlp; /* interrupted; retry */
    *Lisp_errno = errno;
#ifdef BYTESWAP
    word_swap_page((DLword *)bufp, (count + 3) >> 2);
#endif /* BYTESWAP */
    return (NIL);
  }

#ifdef BYTESWAP
  word_swap_page((DLword *)bufp, (count + 3) >> 2);
#endif /* BYTESWAP */

  return (ATOM_T);
}

/*
 * Name:	COM_truncatefile
 *
 * Argument:	LispPTR	*args	args[0]
 *				 The Lisp integer representing a file descriptor
 *				 of the file being truncated.
 *				args[1]
 *				 The Lisp integer representing a requested length to be
 *				 truncated.
 *				args[2]
 *				 The place where the error number should be stored.
 *
 * Value:	If succeed, returns a Lisp T, otherwise Lisp NIL.
 *
 * Side Effect:	The length of the specified file will be the specified length.
 *
 * Description:
 *
 * The implementation of TRUNCATEFILE FDEV method for DSK and UNIX device.  Try to
 * truncate a file.
 */

LispPTR COM_truncatefile(LispPTR *args)
{
  int fd;
  int length;
  int rval;
  struct stat sbuf;

  ERRSETJMP(NIL);
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[2]);

  fd = LispNumToCInt(args[0]);
  length = LispNumToCInt(args[1]);

  TIMEOUT(rval = fstat(fd, &sbuf));
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

  if (!S_ISREG(sbuf.st_mode)) {
    /*
     * The request file is not a regular file.  We don't need to
     * truncate such file.
     */
    return (ATOM_T);
  }
  if ((off_t)length != sbuf.st_size) {
#ifdef DOS
    TIMEOUT(rval = chsize(fd, (off_t)length));
#else
    TIMEOUT(rval = ftruncate(fd, (off_t)length));
#endif /* DOS */
    if (rval != 0) {
      *Lisp_errno = errno;
      return (NIL);
    }

/*
 * TRUNCATEFILE FDEV method is invoked from FORCEOUTPUT Lisp function.
 * Thus we have to sync the file state here.
 */
#ifndef DOS
    TIMEOUT(rval = fsync(fd));
#endif

    if (rval != 0) {
      *Lisp_errno = errno;
      return (NIL);
    }
  }
  return (ATOM_T);
}

/*
 * Name:	COM_changedir
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full file directory name which is following the Xerox
 *				 Lisp file naming convention.  Including a host
 *				 field.
 *
 * Value:	If succeed, returns a Lisp T, otherwise Lisp NIL.
 *
 * Side Effect:	If succeed, the current directory of this Lisp process will be changed.
 *
 * Description:
 *
 * Change the current directory of the process to the specified directory.
 */

LispPTR COM_changedir(LispPTR *args)
{
  int dskp, rval;
  char lfname[MAXPATHLEN + 5], dir[MAXPATHLEN], host[MAXNAMLEN];
#ifdef DOS
  char drive[1], rawname[MAXNAMLEN];
  int extlen;
#endif /* DOS */

  ERRSETJMP(NIL);
  Lisp_errno = &Dummy_errno;

  LispStringLength(args[0], rval, dskp);
  /*
   * Because of the version number convention, Lisp pathname might
   * be shorter than UNIX one.  For THIN string, the difference
   * is 2 bytes, for FAT string, 4 bytes.  Add 1 byte for NULL
   * terminating character.
   */
  rval = dskp ? rval + 4 + 1 : rval + 2 + 1;
  /* Add 5 for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);
#ifdef DOS
  separate_host(lfname, host, drive);
#else
  separate_host(lfname, host);
#endif /* DOS */
  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

#ifdef DOS
  if (!unixpathname(lfname, dir, 0, 0, drive, 0, 0)) return (NIL);
#else  /* DOS */
  if (!unixpathname(lfname, dir, 0, 0)) return (NIL);
#endif /* DOS */

  if (dskp) {
    /*
     * If {DSK} device, the directory name can be specified in a case
     * insensitive manner.  We have to convert it into a right case.
     */
    if (true_name(dir) != -1) return (NIL);
  }

  TIMEOUT(rval = chdir(dir));
  if (rval != 0) return (NIL);
#ifdef DOS
  if (*drive) {
    if (*drive <= 'Z')
      rval = _chdrive(*drive - ('A' - 1));
    else
      rval = _chdrive(*drive - ('a' - 1));
    if (rval != 0) return (NIL);
  }
#endif /* DOS */
  return (ATOM_T);
}

/************************************************************************/
/*									*/
/*		     C O M _ g e t f r e e b l o c k			*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*
 * Name:	COM_getfreeblock
 *
 * Argument:	LispPTR	*args	args[0]
 *				 Full sysout in Lisp format.  Including a host field.
 *				args[1]
 *				 The place where the available free block size will
 *				 be stored.
 *
 * Value:	If succeed, returns a Lisp T, otherwise Lisp NIL.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Examines the statistics of the file system on which the specified sysout file
 * resides, and returns the number of available free blocks through the specified
 * buffer.
 * This routine is invoked just before the Lisp flushes its image on a sysout file so
 * as to make sure the Lisp image is gracefully written on a sysout file.  See
 * \MAIKO.CHECKFREESPACE in MOD44IO.
 * The version control based on a file naming convention is not applicable to a
 * sysout file, even if the sysout file is specified on {DSK} device.  The Lisp code
 * strips the version field by invoking DSK_getfilename with NON recognition mode.
 */

LispPTR COM_getfreeblock(LispPTR *args)
{
  int dskp, rval, *buf;
  char lfname[MAXPATHLEN + 5], dir[MAXPATHLEN], host[MAXNAMLEN];
  char name[MAXNAMLEN + 1], file[MAXPATHLEN], ver[VERSIONLEN];
#ifdef DOS
  char drive[2];
  struct diskfree_t sfsbuf;
#else
  struct statvfs sfsbuf;
#endif

  ERRSETJMP(NIL);
  Lisp_errno = &Dummy_errno;

  LispStringLength(args[0], rval, dskp);
  /*
   * Add 1 byte for NULL terminating character.
   */
  rval += 1;
  /* Add 5 for the host name field in Lisp format. */
  if (rval > MAXPATHLEN + 5) FileNameTooLong(NIL);

  LispStringToCString(args[0], lfname, MAXPATHLEN);
  buf = (int *)NativeAligned4FromLAddr(args[1]);
#ifdef DOS
  separate_host(lfname, host, drive);
#else
  separate_host(lfname, host);
#endif /* DOS */
  UPCASE(host);
  if (strcmp(host, "DSK") == 0)
    dskp = 1;
  else if (strcmp(host, "UNIX") == 0)
    dskp = 0;
  else
    return (NIL);

#ifdef DOS
  if (!unixpathname(lfname, file, 0, 0, drive, 0, 0)) return (NIL);
#else  /* DOS */
  if (!unixpathname(lfname, file, 0, 0)) return (NIL);
#endif /* DOS */

  if (!unpack_filename(file, dir, name, ver, 0)) return (NIL);

  if (dskp) {
    /*
     * Although Lisp code guarantees the directory is an existing one,
     * by calling DSK_getfilename, we check it again for safety.
     */
    if (true_name(dir) != -1) return (NIL);
  }

/*
 * The specified file itself might be a file being created newly.  Thus,
 * we check the available block size, using the directory on which the file
 * will be exist.
 */
#ifdef DOS
  /* For DOS, we have to use either the disk drive the file
     will be on, or the default drive. */

  if (drive[0]) {
    drive[1] = 0;
    UPCASE(drive);
    if (_dos_getdiskfree((unsigned)drive[0] - (int)'@', &sfsbuf))
      return (NIL); /* call failed, so name is invalid */

    *buf = sfsbuf.avail_clusters * sfsbuf.sectors_per_cluster * sfsbuf.bytes_per_sector;
  } else {
    if (_dos_getdiskfree(0, &sfsbuf)) return (NIL); /* call failed, so name is invalid */

    *buf = sfsbuf.avail_clusters * sfsbuf.sectors_per_cluster * sfsbuf.bytes_per_sector;
  }
#else
  TIMEOUT(rval = statvfs(dir, &sfsbuf));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (NIL);
  }
  *buf = sfsbuf.f_bavail;
#endif /* DOS */
  return (ATOM_T);
}

/********************************************
        Subroutines
********************************************/

/*
 * Name:	separate_version
 *
 * Argument:	char	*name   UNIX file name.  It is recommended to pass only
 *				"root file name" (i.e. without directory) as
 *				name argument, but full file name is also
 *				acceptable.
 *		char	*ver	The place where extracted version will be stored.
 *		init	checkp	If 1, whether the version field contains only
 *				numbers or not is checked.  If 0, anything in the
 *				version field is stored in ver.  If GENERATEFILE
 *				want to contain a wild card character in a version
 *				field, checkp should be 0.
 *
 * Value:	void
 *
 * Side Effect:	If name contains a valid version field, name and version will
 *		be set to name and version respectively.
 *
 * Description:
 *
 * Check if name contains the valid version field and if so, separate version
 * field from name field.  After this operation, name contains only name and
 * version contains its version.  If name does not contain a valid version
 * field, version will be NULL string.
 * If checkp is 1, the version field which contains only numeric characters are
 * regarded valid.
 *
 */

void separate_version(char *name, char *ver, int checkp)
{
  char *start, *end, *cp;
  unsigned ver_no;
  size_t len;
  char ver_buf[VERSIONLEN];

  if ((end = (char *)strchr(name, '~')) != (char *)NULL) {
    start = end;
    cp = end + 1;
    while (*cp) {
      if (*cp == '~') {
        start = end;
        end = cp;
      }
      cp++;
    }

    if (start != end && *(start - 1) == '.' && end == (cp - 1)) {
      /*
       * name ends in the form ".~###~". But we have to check
       * ### are all numbers or not, if checkp is 1.
       */
      len = (end - start) - 1;
      strncpy(ver_buf, start + 1, len);
      ver_buf[len] = '\0';
      if (checkp) {
        NumericStringP(ver_buf, YES, NO);
      YES:
        /*
         * name contains a valid version field.
         */
        *(start - 1) = '\0';
        *end = '\0';
        /*
         * Use strtoul() to eliminate leading 0s.
         */
        ver_no = strtoul(start + 1, (char **)NULL, 10);
        sprintf(ver_buf, "%u", ver_no);
        strcpy(ver, ver_buf);
        return;
      } else {
        *(start - 1) = '\0';
        strcpy(ver, ver_buf);
        return;
      }
    }
  } else if (strchr(name, '%')) {
    strcpy(ver, "0");
    return;
  }
NO:
  /* name does not contain a valid version field. */
  *ver = '\0';
}

/*
 * Name:	unpack_filename
 *
 * Argument:	char	*file   UNIX file name in the UNIX format.  It must be an
 *				absolute path.
 *		char	*dir	The place where unpacked directory will be stored.
 *		char	*name	The place where unpacked "root file name" will be
 *				stored. "Root file name" contains the file name
 *				and the extension.
 *		char	*ver	The place where unpacked version will be stored.
 *		int	checkp	If 1, whether the version field contains only the
 *				numeric characters or not will be checked.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	dir, name, and  ver will be replaced with the unpacked corresponding
 *		fields.
 *
 * Description:
 *
 * Unpack the specified UNIX full pathname to three components.  If the file does
 * not include a version field, NULL string will be stored as a version.
 * unpack_filename assumes the pathname passed is a file, not a directory.
 *
 */

int unpack_filename(char *file, char *dir, char *name, char *ver, int checkp)
{
  char *cp;

#ifdef DOS
  if ((cp = (char *)max((UNSIGNED)strrchr(file, DIRSEP), (UNSIGNED)strrchr(file, UNIXDIRSEP))) == 0)
    return (0);

  if (file[1] == DRIVESEP) { /* There's a drive spec; copy it and ignore it from here on. */
    *dir++ = *file++;
    *dir++ = *file++;
  }
#else  /* DOS */
  if ((cp = (char *)strrchr(file, UNIXDIRSEP)) == NULL) return (0);
#endif /* DOS */

  if (cp == file) {
    /* File is on a root directory. */
    *dir = '/';
    *(dir + 1) = '\0';
  } else {
    while (file != cp) *dir++ = *file++;
    *dir = '\0';
  }

  strcpy(name, cp + 1);
  separate_version(name, ver, checkp);
  return (1);
}

/*
 * Name:	true_name
 *
 * Argument:	char	*path	The pathname which follows the UNIX file naming
 *				convention and does not include any meta character.
 *				Whether a tail directory delimiter is included
 *				in path or not is not a matter.  true_name handles
 *				both case correctly.
 *
 * Value:	If the pathname is recognized as an existing directory, returns
 *		-1, recognized as an existing file, returns 1, otherwise 0.
 *
 * Side Effect:	If succeed, the contents of path is replaced with the true name.
 *
 * Description:
 *
 * Try to find the file or directory specified with path.  The search is case
 * insensitive.
 *
 */

int true_name(char *path)
{
  char dir[MAXPATHLEN];
  char name[MAXNAMLEN];
#ifdef DOS
  char drive[1];
#endif
  char c, *sp, *cp;
  int type;

  if (strcmp(path, "/") == 0) return (-1);

#ifdef DOS
  if (*(path + 1) == DRIVESEP) {
    drive[0] = *path;
    dir[0] = drive[0]; /* but copy it to the working dir string */
    dir[1] = DRIVESEP;
    dir[2] = '\0';
    cp = path + 3; /* skip the drive spec & 1st dir delimiter */
  } else {
#endif           /* DOS */
    *dir = '\0'; /*
                  * locate_file does not accept the directory with
                  * the trail delimiter.  Thus, for the root
                  * directory, we have to set the null character
                  * as directory.
                  */
    cp = path + 1;
#ifdef DOS
  }
#endif /* DOS */
       /* If all there was was the root /, succeed easily */
  if (strcmp((cp - 1), DIRSEPSTR) == 0) return (-1);

  while (*cp) {
    /*
     * Copy the next subdirectory to name.
     * And examine if it is an existing subdirectory or the file on the
     * dir.
     * At this point dir has been guaranteed to be exist.
     */
    sp = name;
    for (c = *cp++, *sp++ = c; c != '/' && c != '\0' && c != DIRSEP; c = *cp++, *sp++ = c) {}
    if (c == '/') {
      /* Remove the trail delimiter */
      *(sp - 1) = '\0';
    } else {
      /* Move back cp to '\0' character to finish the loop. */
      cp--;
    }

    /* Try to locate name on dir*/
    if ((type = locate_file(dir, name)) == 0) {
      /*
       * No directory or file named name has been found on
       * dir.
       */
      return (0);
    }
    /*
     * Now, the true name including the name has been set
     * to dir by locate_file.
     */
  }
  strcpy(path, dir);
  return (type);
}

/*
 * Name:	locate_file
 *
 * Argument:	char	*dir	The existing directory name.  Does not include
 *				the trail delimiter.
 *
 *		char	*name	The name which is searched on dir.
 *
 * Value:	If name is recognized as an existing directory, returns -1,
 *		recognized as an existing file, returns 1, otherwise 0.
 *
 * Side Effect:	If succeed, the contents of dir is replaced with the true name
 *		including name.
 *
 * Description:
 *
 * Try to find the file or directory specified with name on the directory
 * specified with dir.  The search is case insensitive.
 *
 */

static int locate_file(char *dir, char *name)
{
#ifdef DOS
  char path[MAXPATHLEN];
  char nb1[MAXNAMLEN], nb2[MAXNAMLEN];
  int type, len;
  struct find_t dirp;
  struct direct *dp;

  /* First of all, recognize as if. */
  sprintf(path, "%s\\%s", dir, name);
  DIR_OR_FILE_P(path, type);
  if (type != 0) {
    strcpy(dir, path);
    return (type);
  }

  return (0);

#else  /* UNIX code follows */

  char path[MAXPATHLEN];
  char nb1[MAXNAMLEN], nb2[MAXNAMLEN];
  int type;
  size_t len;
  DIR *dirp;
  struct dirent *dp;

  /* First of all, recognize as if. */
  sprintf(path, "%s/%s", dir, name);
  DIR_OR_FILE_P(path, type);
  if (type != 0) {
    strcpy(dir, path);
    return (type);
  }

  /* Next try with all lower case name. */
  strcpy(nb1, name);
  DOWNCASE(nb1);
  sprintf(path, "%s/%s", dir, nb1);
  DIR_OR_FILE_P(path, type);
  if (type != 0) {
    strcpy(dir, path);
    return (type);
  }

  /* Next try with all upper case name. */
  UPCASE(nb1);
  sprintf(path, "%s/%s", dir, nb1);
  DIR_OR_FILE_P(path, type);
  if (type != 0) {
    strcpy(dir, path);
    return (type);
  }

  /* No way.  Read dir and compare with name. */
  len = strlen(name);
  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (0);
  }
  for (S_TOUT(dp = readdir(dirp)); dp != NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)))
    if (dp) {
      if (strlen(dp->d_name) == len) {
        strcpy(nb2, dp->d_name);
        UPCASE(nb2);
        if (strcmp(nb1, nb2) == 0) {
          sprintf(path, "%s/%s", dir, dp->d_name);
          DIR_OR_FILE_P(path, type);
          if (type != 0) {
            strcpy(dir, path);
            TIMEOUT(closedir(dirp));
            return (type);
          }
        }
      }
    }
  TIMEOUT(closedir(dirp));
  return (0);
#endif /* DOS */
}

/*
 * Name:	make_directory
 *
 * Argument:	char	*dir	The full directory name in UNIX format.  It does
 *				not include a tail delimiter.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	The directory specified as dir will be created in the file system.
 *		If succeed, dir will be replaced with the true name of the directory.
 *
 * Description:
 *
 * Try to create a specified directory.
 *
 */

static int make_directory(char *dir)
{
  char *cp, *dp;
  int maked, rval;
  char dir_buf[MAXPATHLEN];

  maked = 0;

  dp = dir_buf;
  cp = dir;

#ifdef DOS
  if (DRIVESEP == *(cp + 1)) {
    *dp++ - *cp++; /* copy the drive letter and colon */
    *dp++ = *cp++;
  }
#endif /* DOS */

  *dp++ = DIRSEP; /* For a root directory. */
  cp++;           /* Skip a root directory in dir. */

  for (;;) {
    switch (*cp) {
#ifdef DOS
      case DIRSEP:
#endif
      case '/':
      case '\0':
        *dp = '\0';
        /*
         * Now, dir_buf contains the absolute path to the next
         * subdirectory or file.  If one of the parent directories
         * are created, we have to create a new subdirectory
         * anyway.  If all of the parent directories are existing
         * directories, we have to check this subdirectory is an
         * existing or not.
         */
        if (maked) {
#ifdef DOS
          TIMEOUT(rval = mkdir(dir_buf));
#else
          TIMEOUT(rval = mkdir(dir_buf, 0777));
#endif /* DOS */
          if (rval == -1) {
            *Lisp_errno = errno;
            return (0);
          }
          if (*cp == '\0') {
            strcpy(dir, dir_buf);
            return (1);
          }
          *dp++ = DIRSEP;
          cp++;
        } else {
          switch (true_name(dir_buf)) {
            case -1: /* Directory */
              if (*cp == '\0') {
                /* Every subdirectories are examined. */
                strcpy(dir, dir_buf);
                return (1);
              } else {
                dp = dir_buf;
                while (*dp) dp++;
                *dp++ = DIRSEP;
                cp++;
              }
              break;

            case 1: /* Regular File */
              /*
               * UNIX does not allow to make a directory
               * and a file in the same name on a directory.
               */
              return (0);

            default:
/*
 * Should handle other cases. (special file).
 */
#ifdef DOS
              TIMEOUT(rval = mkdir(dir_buf));
#else
              TIMEOUT(rval = mkdir(dir_buf, 0777));
#endif /* DOS */
              if (rval == -1) {
                *Lisp_errno = errno;
                return (0);
              }
              if (*cp == '\0') return (1);
              *dp++ = DIRSEP;
              cp++;
              maked = 1;
              break;
          }
        }
        break;

      default: *dp++ = *cp++; break;
    }
  }
}

/*
 * Name:	FindHighestVersion
 *
 * Argument:	FileName *varray
 *				The version array.  It has to include at
 *				least one versioned file.
 *		FileName *mentry
 *				The place where an entry in varray corresponding
 *				to the highest versioned file will be stored.
 *		int	max_no	The place where the version number of the highest
 *				versioned file will be stored.
 *
 * Value:	N/A
 *
 * Side Effect:	mentry and max_no will be replaced with the highest versioned entry
 *		and highest version number respectively.
 *
 * Description:
 *
 * Find the highest versioned entry in varray.  Varray has to include at least
 * one versioned file, that is varray has to satisfy (!NoFileP(varray) &&
 * !OnlyVersionlessP(varray)).
 *
 */
#ifdef DOS
#define FindHighestVersion(varray, mentry, max_no)                                         \
  do {                                                                                        \
    FileName *centry;                                                             \
    for (centry = varray, max_no = -1; centry->version_no != LASTVERSIONARRAY; centry++) { \
      if (centry->version_no > max_no) {                                                   \
        max_no = centry->version_no;                                                       \
        mentry = centry;                                                                   \
      }                                                                                    \
    }                                                                                      \
    } while (0)
#else
#define FindHighestVersion(varray, mentry, max_no)                                        \
  do {                                                                                       \
    FileName *centry;                                                            \
    for (centry = (varray), (max_no) = 0; centry->version_no != LASTVERSIONARRAY; centry++) { \
      if (centry->version_no > (max_no)) {                                                  \
        (max_no) = centry->version_no;                                                      \
        (mentry) = centry;                                                                  \
      }                                                                                   \
    }                                                                                     \
    } while (0)
#endif /* DOS */

/*
 * Name:	FindLowestVersion
 *
 * Argument:	FileName *varray
 *				The version array.  It has to include at
 *				least one versioned file.
 *		FileName *mentry
 *				The place where an entry in varray corresponding
 *				to the lowest versioned file will be stored.
 *		int	min_no	The place where the version number of the highest
 *				versioned file will be stored.
 *
 * Value:	N/A
 *
 * Side Effect:	mentry and min_no will be replaced with the lowest versioned entry
 *		and lowest version number respectively.
 *
 * Description:
 *
 * Find the lowest versioned entry in varray.  Varray has to include at least
 * one versioned file, that is varray has to satisfy (!NoFileP(varray) &&
 * !OnlyVersionlessP(varray)).
 *
 */
#ifdef DOS
#define FindLowestVersion(varray, mentry, min_no)                                                  \
  do {                                                                                                \
    FileName *centry;                                                                     \
    for (centry = varray, min_no = MAXVERSION; centry->version_no != LASTVERSIONARRAY; centry++) { \
      if (centry->version_no < min_no) {                                                           \
        min_no = centry->version_no;                                                               \
        mentry = centry;                                                                           \
      }                                                                                            \
    }                                                                                              \
    } while (0)
#else
#define FindLowestVersion(varray, mentry, min_no)                                                  \
  do {                                                                                                \
    FileName *centry;                                                                     \
    for (centry = (varray), (min_no) = MAXVERSION; centry->version_no != LASTVERSIONARRAY; centry++) { \
      if (centry->version_no < (min_no) && centry->version_no != 0) {                                \
        (min_no) = centry->version_no;                                                               \
        (mentry) = centry;                                                                           \
      }                                                                                            \
    }                                                                                              \
  } while (0)
#endif /* DOS */

/*
 * Name:	FindSpecifiedVersion
 *
 * Argument:	FileName *varray
 *				The version array.  It has to include at
 *				least one versioned file.
 *		FileName *sentry
 *				The place where an entry in varray corresponding
 *				to the file which has the specified version will
 *				be stored.
 *		int	ver_no	The version number to be found.
 *
 * Value:	N/A
 *
 * Side Effect:	sentry will be replaced with the specified versioned entry.
 *
 * Description:
 *
 * Find the specified versioned entry in varray. Varray has to include at least
 * one versioned file, that is varray has to satisfy (!NoFileP(varray) &&
 * !OnlyVersionlessP(varray)).
 *
 */

#define FindSpecifiedVersion(varray, sentry, ver_no)                        \
  do {                                                                         \
    FileName *centry;                                              \
                                                                            \
    (sentry) = (FileName *)NULL;                                              \
    for (centry = varray; centry->version_no != LASTVERSIONARRAY; centry++) \
      if (centry->version_no == (ver_no)) {                                   \
        (sentry) = centry;                                                    \
        break;                                                              \
      }                                                                     \
  } while (0)

/************************************************************************/
/*									*/
/*		    g e t _ v e r s i o n _ a r r a y			*/
/*									*/
/*	Given a file, create an array of file/version# pairs for all	*/
/*	versions of that file.  Returns 1 on success, 0 on failure.	*/
/*									*/
/*	dir	UNIX directory specified absolutely.  Caller must	*/
/*		guarantee that the directory exists.			*/
/*	file	File name, optionally including a (unix) version	*/
/*	varray	Place to put the version array entries.			*/
/*	cache	Place to hold info about the new version array		*/
/*									*/
/*	Read thru DIR and gather all files that match FILE into		*/
/*	VARRAY.  DIR's case must match existing directory's, but	*/
/*	FILE name matching is case-insensitive.  For UNIX, the 		*/
/*	versionless file is marked with a version# of 0; for DOS,	*/
/*	version 0 is the back-up copy of the file.			*/
/*									*/
/************************************************************************/

static int get_version_array(char *dir, char *file, FileName varray[], CurrentVArray *cache)
{
#ifdef DOS
  /* DOS version-array builder */
  char lcased_file[MAXPATHLEN];
  char old_file[MAXPATHLEN];
  char name[MAXNAMLEN];
  char ver[VERSIONLEN];
  int varray_index = 0;
  struct find_t dirp;
  struct direct *dp;
  int rval, drive = 0, isslash = 0;
  struct stat sbuf;
  int res;

  /*
   * First of all, prepare a lower cased file name for the case insensitive
   * search.  Also we have to separate file name from its version field.
   */
  if (dir[1] == DRIVESEP) drive = dir[0];

  if (strcmp(dir, "\\") == 0)
    isslash = 1;
  else if (drive && (strcmp(dir + 2, "\\") == 0))
    isslash = 1;

  if (!isslash)
    strcpy(lcased_file, dir); /* Only add the dir if it's real */
  else if (drive) {
    lcased_file[0] = drive;
    lcased_file[1] = DRIVESEP;
    lcased_file[2] = '\0';
  } else
    *lcased_file = '\0';

  /*  strcpy(lcased_file, dir);   removed when above code added 3/4/93 */
  strcat(lcased_file, DIRSEPSTR);
  strcat(lcased_file, file);
  separate_version(lcased_file, ver, 1);
  DOWNCASE(lcased_file);

  /*************************************************/
  /* First, look up the backup version of the file */
  /*************************************************/

  /* First, make the "backup-file-name" for this file */

  make_old_version(old_file, lcased_file);

  TIMEOUT(res = _dos_findfirst(old_file, _A_NORMAL | _A_SUBDIR, &dirp));
  if (res == 0) {
    strcpy(name, dirp.name);
    strcpy(varray[varray_index].name, name);
    varray[varray_index].version_no = 0;
    varray_index++;
  }

  /*******************************/
  /* Now look up the file itself */
  /*******************************/

  TIMEOUT(res = _dos_findfirst(lcased_file, _A_NORMAL | _A_SUBDIR, &dirp));
  /*    if (res != 0)
        {
          *Lisp_errno = errno;
          return(0);
        }
  */
  for (; res == 0; S_TOUT(res = _dos_findnext(&dirp))) {
    strcpy(name, dirp.name);
    separate_version(name, ver, 1);
    DOWNCASE(name);

    strcpy(varray[varray_index].name, dirp.name);
    if (*ver == '\0') {
      /* Versionless file */
      varray[varray_index].version_no = 1;
    } else {
      /*
       * separator_version guarantees ver is a numeric
       * string.
       */
      varray[varray_index].version_no = strtoul(ver, (char **)NULL, 10);
    }
    varray_index++;
    if (varray_index >= VERSIONARRAYLENGTH) {
      /* how does the specific error get signalled in the DOS case? */
      return (0);
     }
  }

  /*
   * The last entry of varray is indicated by setting LASTVERSIONARRAY into
   * version_no field.
   */
  varray[varray_index].version_no = LASTVERSIONARRAY;

  /*
   * If more than one files have been stored in varray, we store the name
   * without version in the last marker entry.
   */
  if (!NoFileP(varray)) {
    strcpy(name, varray->name);
    separate_version(name, ver, 1);
    strcpy(varray[varray_index].name, name);
  }

  return (1);

#else
  /* UNIX version-array builder */
  char lcased_file[MAXNAMLEN];
  char name[MAXNAMLEN];
  char ver[VERSIONLEN];
  int varray_index = 0;
  DIR *dirp;
  struct dirent *dp;
  int rval;
  struct stat sbuf;

  /*
   * First of all, prepare a lower cased file name for the case insensitive
   * search.  Also we have to separate file name from its version field.
   */
  strcpy(lcased_file, file);
  separate_version(lcased_file, ver, 1);
  DOWNCASE(lcased_file);

  TIMEOUT(rval = stat(dir, &sbuf));
  if (rval == -1) {
    *Lisp_errno = errno;
    return(0);
  }

  /*
   * Cache mechanism was not used because of a bug in Sun OS.
   * Sometimes just after unlinking a file on a directory, the st_mtime
   * of the directory does not change.  This will make Maiko believe
   * cached version array is still valid, although it is already invalid.
   * sync(2) has no effect on such case.
   */

  /*
   * If the cached version array is still valid, we can return immediately.
   */

#if 0
  /* there is a (different?) problem (#1661) with the caching - disable until it's solved */
  if ((sbuf.st_mtime == cache->mtime) && strcmp(dir, cache->path) == 0
      && strcmp(lcased_file, cache->file) == 0) return(1);
#endif

  errno = 0;
  TIMEOUT0(dirp = opendir(dir));
  if (dirp == NULL) {
    *Lisp_errno = errno;
    return (0);
  }

  for (S_TOUT(dp = readdir(dirp)); dp != NULL || errno == EINTR;
       errno = 0, S_TOUT(dp = readdir(dirp)))
    if (dp) {
      strcpy(name, dp->d_name);
      separate_version(name, ver, 1);
      DOWNCASE(name);
      if (strcmp(name, lcased_file) == 0) {
        /*
         * This file can be regarded as a same file in Lisp sense.
         */
        strcpy(varray[varray_index].name, dp->d_name);
        if (*ver == '\0') {
          /* Versionless file */
          varray[varray_index].version_no = 0;
        } else {
          /*
           * separator_version guarantees ver is a numeric
           * string.
           */
          varray[varray_index].version_no = strtoul(ver, (char **)NULL, 10);
        }
        varray_index++;
        if (varray_index >= VERSIONARRAYLENGTH) {
          *Lisp_errno = EIO;
          return (0);
        }
      }
    }
  /*
   * The last entry of varray is indicated by setting LASTVERSIONARRAY into
   * version_no field.
   */
  varray[varray_index].version_no = LASTVERSIONARRAY;

  /*
   * If more than one files have been stored in varray, we store the name
   * without version in the last marker entry.
   */
  if (!NoFileP(varray)) {
    strcpy(name, varray->name);
    separate_version(name, ver, 1);
    strcpy(varray[varray_index].name, name);
  }

  /*
   * Update cache information.
   */
  strcpy(cache->path, dir);
  strcpy(cache->file, lcased_file);
  cache->mtime = sbuf.st_mtime;
  TIMEOUT(closedir(dirp));
  return (1);
#endif /* DOS */
}

/*
 * Name:	maintain_version
 *
 * Argument:	char	*file	The full file name in UNIX format.
 *		FileName *varray
 *				The version array.
 *		int	forcep	If 1, a versionless file will be linked to version
 *				1 file when there is no other files having the same
 *				name.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	None.
 *
 * Description:
 *
 * Try to make sure that the version control based on the name convention
 * is well maintained.  The versionless file corresponding to file will be
 * maintained.  The second argument varray is an optional.  If it is not
 * NULL pointer, maintain_version maintains the version control using the
 * version array.  If varray is NULL pointer, a version array is re collected
 * by this routine.
 * Currently, forcep is 1 only if maintain_version is called from COM_openfile
 * to maintain the directory on which a file is being created.
 */

static int maintain_version(char *file, FileName *varray, int forcep)
{
  char dir[MAXPATHLEN], fname[MAXNAMLEN], ver[VERSIONLEN];
  char old_file[MAXPATHLEN], vless[MAXPATHLEN];
  int highest_p;
  int rval, max_no;
  FileName *entry;

  if (varray == (FileName *)NULL) {
    if (unpack_filename(file, dir, fname, ver, 1) == 0) return (0);
    /*
     * We have to make sure that dir is the existing directory.
     */
    if (true_name(dir) != -1) return (0);
    if (get_version_array(dir, fname, VersionArray, &VArrayInfo) == 0) return (0);
    varray = VersionArray;
  }

  if (NoFileP(varray)) {
    /*
     * We don't need to care about such case that there is no such file
     * or an only versionless file exists.
     */
    return (1);
  }

  if (OnlyVersionlessP(varray)) {
    if (forcep) {
/*
 * If forcep, we link the versionless file to the version
 * 1 file.
 */
#ifndef DOS
      get_versionless(varray, vless, dir);
      ConcNameAndVersion(vless, "1", fname);
      TIMEOUT(rval = link(vless, fname));
      if (rval == -1) {
        *Lisp_errno = errno;
        return (0);
      }
#endif /* DOS */
    }
    return (1);
  }

  /*
   * At this point, we are sure that at least one file with version number
   * exists.  Thus, FindHighestVersion works fine from now on.
   */

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is not a versionless file, but at least one versioned file.
     * Thus, the thing we have to do is to link a versionless file
     * to the existing highest versioned file.
     */
    FindHighestVersion(varray, entry, max_no);
    ConcDirAndName(dir, entry->name, old_file);
/*
 * The versionless file should have the same case name as the old
 * file.
 */
#ifndef DOS
    strcpy(fname, entry->name);
    separate_version(fname, ver, 1);
    ConcDirAndName(dir, fname, vless);
    TIMEOUT(rval = link(old_file, vless));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (0);
    }
#endif /* DOS */
    return (1);
  }

  if (check_vless_link(vless, varray, old_file, &highest_p) == 0) return (0);

  if (*old_file == '\0') {
    /*
     * The versionless file is not linked to any file in varray.
     * Thus, we have to link the versionless file to the file which
     * is versioned one higher than the existing highest version.
     */
    FindHighestVersion(varray, entry, max_no);
    sprintf(ver, "%u", max_no + 1);
/*
 * The old file should have the same case name as the versionless
 * file.
 */
#ifndef DOS
    ConcNameAndVersion(vless, ver, old_file);
    TIMEOUT(rval = link(vless, old_file));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (0);
    }
#endif /* DOS */
    return (1);
  }

  if (highest_p) {
    /*
     * The versionless file has been already linked to the highest
     * versioned file.  Thus, there is nothing to do.
     */
    return (1);
  } else {
    /*
     * Although the  versionless file is linked to a file in varray,
     * the file is not the highest versioned file.  We have to unlink
     * the wrongly linked versionless file, and link the highest versioned
     * file to a versionless file.
     */
    TIMEOUT(rval = unlink(vless));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (0);
    }
    FindHighestVersion(varray, entry, max_no);
    ConcDirAndName(dir, entry->name, old_file);
/*
 * The versionless file should have the same case name as the old
 * file.
 */
#ifndef DOS
    strcpy(fname, entry->name);
    separate_version(fname, ver, 1);
    ConcDirAndName(dir, fname, vless);
    TIMEOUT(rval = link(old_file, vless));
    if (rval == -1) {
      *Lisp_errno = errno;
      return (0);
    }
#endif /* DOS */
    return (1);
  }
}

/*
 * Name:	get_versionless
 *
 * Argument:	FileName *varray
 *				The version array already filled by
 *				get_version_array routine.
 *		char	*file	The place where the full file name of the found
 *				versionless file will be stored.
 *		char	*dir	Directory absolute path following the UNIX
 *				file naming convention on which varray
 *				has been gathered.
 *
 * Value:	If a versionless file found, returns 1, otherwise 0.
 *
 * Side Effect:	file will be replaced with the full name of the found versionless
 *		file when it is found.
 *
 * Description:
 *
 * Accepts a version array and try to find a versionless file within it.
 *
 */

static int get_versionless(FileName *varray, char *file, char *dir)
{
#ifdef DOS
  return (0);
#endif /* DOS */
  if (NoFileP(varray)) return (0);

  while (varray->version_no != LASTVERSIONARRAY) {
    if (varray->version_no == 0) {
      ConcDirAndName(dir, varray->name, file);
      return (1);
    } else
      varray++;
  }
  return (0);
}

/*
 * Name:	check_vless_link
 *
 * Argument:	char	*vless	The full file name of the versionless file in
 *				UNIX format.
 *		FileName *varray
 *				The version array already filled by get_version_array.
 *		char	*to_file
 *				The place where the full file name of the file
 *				to which the versionless file is hard linked will
 *				be stored.
 *		int	*highest_p
 *				If to_file is the highest versioned file in varray,
 *				highest_p will point to 1, otherwise, 0.
 *
 * Value:	If succeed, returns 1, otherwise, 0.
 *
 * Side Effect:	to_file will be replaced with the full file name to which the
 *		versionless file is hard linked.  highest_p will be replaced with
 *		1 or 0.
 *
 * Description:
 *
 * Examines the link status of a specified versionless file.  If there is a file
 * to which the versionless file is hard linked in a version array, it will be
 * stored in to_file.  If there is no such file, to_file will be NULL string.
 * When hard linked file is found, if highest_p is 1, the file is the highest
 * versioned file in the version array.  This is the such case that a versionless
 * file is well maintained by DSK code.
 * Notice that even if to_file is NULL, it does not mean that the versionless file
 * is not hard linked to any file.  It only means that there is no file to which the
 * versionless file is hard linked in the version array.  The versionless file
 * might be hard linked to the other file entirely differently named.
 *
 */

static int check_vless_link(char *vless, FileName *varray, char *to_file, int *highest_p)
{
  int rval, found;
  unsigned max_no;
  ino_t vless_ino;
  struct stat sbuf;
  char dir[MAXPATHLEN], name[MAXNAMLEN], ver[VERSIONLEN];
  FileName *max_entry, *linked_entry;

  TIMEOUT(rval = stat(vless, &sbuf));
  if (rval != 0) {
    *Lisp_errno = errno;
    return (0);
  }

  if (sbuf.st_nlink == 1) {
    /* There is no file to which vless is hard linked. */
    *to_file = '\0';
    return (1);
  }
  vless_ino = sbuf.st_ino;

  if (unpack_filename(vless, dir, name, ver, 1) == 0) return (0);

  max_no = 0;
  found = 0;
  max_entry = NULL;
  linked_entry = NULL;

  while (varray->version_no != LASTVERSIONARRAY) {
    if (varray->version_no > max_no) {
      max_no = varray->version_no;
      max_entry = varray;
    }
    if (!found && varray->version_no != 0) {
      ConcDirAndName(dir, varray->name, name);
      TIMEOUT(rval = stat(name, &sbuf));
      if (rval != 0) {
        *Lisp_errno = errno;
        return (0);
      }
      if (sbuf.st_ino == vless_ino) {
        found = 1;
        linked_entry = varray;
      }
    }
    varray++;
  }

  if (linked_entry != NULL) {
    if (linked_entry == max_entry) {
      *highest_p = 1;
    } else {
      *highest_p = 0;
    }
    strcpy(to_file, name);
  } else {
    *to_file = '\0';
  }
  return (1);
}

/*
 * Name:	get_old
 *
 * Argument:	char	*dir	Directory absolute path following the UNIX
 *				file naming convention on which varray
 *				has been gathered.
 *		FileName *varray
 *				The version array already filled by
 *				get_version_array routine.
 *		char	*afile	File name.  It might include a version field.
 *				The version field have to be following the
 *				UNIX convention, that is "name.~##~", not
 *				Xerox Lisp one.  The versionless file is
 *				also acceptable.
 *				afile is also used as a place where the true
 *				name of the  recognized file will be stored.
 *		char	*vfile	The place where the versioned name of the
 *				recognized file will be stored.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	afile will be replaced with the full name of the recognized file
 *		when the recognition succeed.  vfile will be replaced with the
 *		versioned full name of the recognized file when the recognition
 *		succeed.
 *
 * Description:
 *
 * Accepts a version array and file name, and recognize a file in the version
 * array with the old mode.
 * "Old" file means the "newest existing" file.
 * Notice that, afile and vfile are not necessary same name, because the versionless
 * file is recognized as an old file, afile will hold the true versionless file name,
 * although vfile will hold the versioned file name anyway.
 * afile could be used as the real UNIX pathname to access the recognized file, on
 * the other hand, vfile could be used as file name from which the file name
 * which should be returned to Lisp could be produced.
 *
 */

static int get_old(char *dir, FileName *varray, char *afile, char *vfile)
{
  char name[MAXPATHLEN], vless[MAXPATHLEN], to_file[MAXPATHLEN];
  char ver[VERSIONLEN], vbuf[VERSIONLEN];
  unsigned ver_no, max_no;
  int highest_p;
  FileName *entry;

  /* "Old" file have to be existing, thus varray should not be empty. */
  if (NoFileP(varray)) return (0);

  strcpy(name, afile);
  separate_version(name, ver, 1);

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file, but at least one versioned
     * file exists.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The highest versioned file
       * is an old file.
       */
      FindHighestVersion(varray, entry, max_no);
      ConcDirAndName(dir, entry->name, afile);
      strcpy(vfile, afile);
      return (1);
    } else {
      /*
       * A version is specified.  We have to walk thorough
       * varray and try to find the file with the specified
       * version.
       */
      ver_no = strtoul(ver, (char **)NULL, 10);
      FindSpecifiedVersion(varray, entry, ver_no);
      if (entry != NULL) {
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else
        return (0);
    }
  } else if (OnlyVersionlessP(varray)) {
    /*
     * There is only versionless file in varray.
     * If the specified version is 1 the versionless file is regarded
     * as version 1 file.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The versionless file is dealt
       * with as version 1.
       */
      ConcNameAndVersion(vless, "1", vfile);
      strcpy(afile, vless);
      return (1);
    } else {
      ver_no = strtoul(ver, (char **)NULL, 10);
      if (ver_no == 1) {
        /*
         * Version 1 is specified.  The versionless file is
         * dealt with as a version 1 file.
         */
        ConcNameAndVersion(name, "1", afile);
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /*
         * Other versions than 1 are not recognized as an old
         * file.
         */
        return (0);
      }
    }
  } else {
    if (check_vless_link(vless, varray, to_file, &highest_p) == 0) return (0);
    if (*to_file == '\0') {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is not linked
       * to any file in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The one higher than an
         * existing maximum version is dealt with as the
         * old version, and it should be a version of the
         * link missing versionless file.
         */
        FindHighestVersion(varray, entry, max_no);
        sprintf(vbuf, "%u", max_no + 1);
        ConcNameAndVersion(vless, vbuf, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /* A version is specified. */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindHighestVersion(varray, entry, max_no);
        if (ver_no == max_no + 1) {
          /*
           * If the version is one higher than the
           * existing highest version is specified, it
           * is dealt with as a version of the link
           * missing versionless file.
           */
          sprintf(vbuf, "%u", ver_no);
          ConcNameAndVersion(vless, vbuf, vfile);
          strcpy(afile, vless);
          return (1);
        } else {
          /*
           * We have to walk through varray and try
           * to find the file with the specified version.
           */
          FindSpecifiedVersion(varray, entry, ver_no);
          if (entry != NULL) {
            ConcDirAndName(dir, entry->name, afile);
            strcpy(vfile, afile);
            return (1);
          } else
            return (0);
        }
      }
    } else {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is linked to
       * one of files in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The highest versioned file
         * in varray is an old file.
         */
        FindHighestVersion(varray, entry, max_no);
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else {
        /*
         * A version is specified.  We have to walk thorough
         * varray and try to find the file with the specified
         * version.
         */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindSpecifiedVersion(varray, entry, ver_no);
        if (entry != NULL) {
          ConcDirAndName(dir, entry->name, afile);
          strcpy(vfile, afile);
          return (1);
        } else
          return (0);
      }
    }
  }
}

/*
 * Name:	get_oldeset
 *
 * Argument:	char	*dir	Directory absolute path following the UNIX
 *				file naming convention on which varray
 *				has been gathered.
 *		FileName *varray
 *				The version array already filled by
 *				get_version_array routine.
 *		char	*afile	File name.  It might include a version field.
 *				The version field have to be following the
 *				UNIX convention, that is "name.~##~", not
 *				Xerox Lisp one.  The versionless file is
 *				also acceptable.
 *				afile is also used as a place where the true
 *				name of the  recognized file will be stored.
 *		char	*vfile	The place where the versioned name of the
 *				recognized file will be stored.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	afile will be replaced with the full name of the recognized file
 *		when the recognition succeed.  vfile will be replaced with the
 *		versioned full name of the recognized file when the recognition
 *		succeed.
 *
 * Description:
 *
 * Accepts a version array and filename, and recognize a file in the version
 * array with oldest mode.
 * "Oldest" file means the "oldest existing" file.
 * Notice that, afile and vfile are not necessary same name, because the versionless
 * file is recognized as an old file, afile will hold the true versionless file name,
 * although vfile will hold the versioned file name anyway.
 * afile could be used as the real UNIX pathname to access the recognized file, on
 * the other hand, vfile could be used as file name from which the file name
 * which should be returned to Lisp could be produced.
 *
 */

static int get_oldest(char *dir, FileName *varray, char *afile, char *vfile)
{
  char name[MAXPATHLEN], vless[MAXPATHLEN], to_file[MAXPATHLEN];
  char ver[VERSIONLEN], vbuf[VERSIONLEN];
  unsigned ver_no, min_no;
  int highest_p;
  FileName *entry;

  /* "Oldest" file have to be existing, thus varray should not be empty. */
  if (NoFileP(varray)) return (0);

  strcpy(name, afile);
  separate_version(name, ver, 1);

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file, but at least one versioned
     * file exists.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The lowest versioned file
       * is an oldest file.
       */
      FindLowestVersion(varray, entry, min_no);
      ConcDirAndName(dir, entry->name, afile);
      strcpy(vfile, afile);
      return (1);
    } else {
      /*
       * A version is specified.  We have to walk thorough
       * varray and try to find the file with the specified
       * version.
       */
      ver_no = strtoul(ver, (char **)NULL, 10);
      FindSpecifiedVersion(varray, entry, ver_no);
      if (entry != NULL) {
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else
        return (0);
    }
  } else if (OnlyVersionlessP(varray)) {
    /*
     * There is only versionless file in varray.
     * If the specified version is 1 the versionless file is regarded
     * as version 1 file.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The versionless file is dealt
       * with as version 1.
       */
      ConcNameAndVersion(vless, "1", vfile);
      strcpy(afile, vless);
      return (1);
    } else {
      ver_no = strtoul(ver, (char **)NULL, 10);
      if (ver_no == 1) {
        /*
         * Version 1 is specified.  The versionless file is
         * dealt with as a version 1 file.
         */
        ConcNameAndVersion(name, "1", afile);
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /*
         * Other versions than 1 are not recognized as an
         * oldest file.
         */
        return (0);
      }
    }
  } else {
    if (check_vless_link(vless, varray, to_file, &highest_p) == 0) return (0);
    if (*to_file == '\0') {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is not linked
       * to any file in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The lowest version is
         * dealt with as the oldest version.
         */
        FindLowestVersion(varray, entry, min_no);
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else {
        /* A version is specified. */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindHighestVersion(varray, entry, min_no);
        if (ver_no == min_no + 1) {
          /*
           * If the version is one higher than the
           * existing highest version is specified, it
           * is dealt with as a version of the link
           * missing versionless file.
           */
          sprintf(vbuf, "%u", ver_no);
          ConcNameAndVersion(vless, vbuf, vfile);
          strcpy(afile, vless);
          return (1);
        } else {
          /*
           * We have to walk through varray and try
           * to find the file with the specified version.
           */
          FindSpecifiedVersion(varray, entry, ver_no);
          if (entry != NULL) {
            ConcDirAndName(dir, entry->name, afile);
            strcpy(vfile, afile);
            return (1);
          } else
            return (0);
        }
      }
    } else {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is linked to
       * one of files in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The lowest versioned file
         * in varray is an old file.
         */
        FindLowestVersion(varray, entry, min_no);
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else {
        /*
         * A version is specified.  We have to walk thorough
         * varray and try to find the file with the specified
         * version.
         */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindSpecifiedVersion(varray, entry, ver_no);
        if (entry != NULL) {
          ConcDirAndName(dir, entry->name, afile);
          strcpy(vfile, afile);
          return (1);
        } else
          return (0);
      }
    }
  }
}

/*
 * Name:	get_new
 *
 * Argument:	char	*dir	Directory absolute path following the UNIX
 *				file naming convention on which varray
 *				has been gathered.
 *		FileName *varray
 *				The version array already filled by
 *				get_version_array routine.
 *		char	*afile	File name.  It might include a version field.
 *				The version field have to be following the
 *				UNIX convention, that is "name.~##~", not
 *				Xerox Lisp one.  The versionless file is
 *				also acceptable.
 *				afile is also used as a place where the true
 *				name of the  recognized file will be stored.
 *		char	*vfile	The place where the versioned name of the
 *				recognized file will be stored.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	afile will be replaced with the full name of the recognized file
 *		when the recognition succeed.  vfile will be replaced with the
 *		versioned full name of the recognized file when the recognition
 *		succeed.
 *
 * Description:
 *
 * Accepts a version array and filename, and recognize a file in the version
 * array with new mode.
 * "New" file means the "not yet existing" file with the version which is one
 * higher than the current highest version.
 * Notice that, afile and vfile are not necessary same name, because the versionless
 * file is recognized as an old file, afile will hold the true versionless file name,
 * although vfile will hold the versioned file name anyway.
 * afile could be used as the real UNIX pathname to access the recognized file, on
 * the other hand, vfile could be used as file name from which the file name
 * which should be returned to Lisp could be produced.
 *
 */

static int get_new(char *dir, FileName *varray, char *afile, char *vfile)
{
  char name[MAXPATHLEN], vless[MAXPATHLEN], to_file[MAXPATHLEN];
  char ver[VERSIONLEN], vbuf[VERSIONLEN];
  unsigned ver_no, max_no;
  int highest_p;
  FileName *entry;

  strcpy(name, afile);
  separate_version(name, ver, 1);

#ifndef DOS
  if (NoFileP(varray)) {
    /*
     * If there is no file with such name, "new" file is always
     * recognized.
     */
    if (*ver == '\0' || strcmp(ver, "1") == 0)
#endif /* DOS */
    {
      /*
       * If version is not specified or 1 is specified,
       * we can return versionless file as afile.
       */
      ConcNameAndVersion(name, "1", afile);
      ConcDirAndName(dir, afile, vfile);
      ConcDirAndName(dir, name, afile);
      return (1);
    }
#ifndef DOS
    else {
      /*
       * A version other than 1 is specified.  "New" file
       * is recognized as if.
       */
      ConcDirAndName(dir, afile, vfile);
      strcpy(afile, vfile);
      return (1);
    }
  }

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file, but at least one versioned
     * file exists.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The new file is one higher than
       * the existing highest version.
       */
      FindHighestVersion(varray, entry, max_no);
      sprintf(vbuf, "%u", max_no + 1);
      /*
       * We will use the file name of the existing highest
       * versioned file as the name of the new file, so that
       * new file is as the same case as old.
       */
      strcpy(name, entry->name);
      separate_version(name, ver, 1);
      ConcDirAndName(dir, name, afile);
      ConcNameAndVersion(afile, vbuf, vfile);
      strcpy(afile, vfile);
      return (1);
    } else {
      /*
       * A version is specified.  We have to walk thorough
       * varray and try to find the file with the specified
       * version.
       */
      ver_no = strtoul(ver, (char **)NULL, 10);
      FindSpecifiedVersion(varray, entry, ver_no);
      if (entry != NULL) {
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      }
      /*
       * There is not a file with the specified version in varray.
       * The specified file can be recognized as if.
       * Most user will hope to create a new file in same case as
       * old.   One of case sensitive names in the files are stored
       * in the trail marker entry in varray by get_version_array
       * routine.
       * We will use it, although we cannot say all of the gathered
       * files has the name in same case.
       */
      while (varray->version_no != LASTVERSIONARRAY) varray++;
      ConcNameAndVersion(varray->name, ver, afile);
      ConcDirAndName(dir, afile, vfile);
      strcpy(afile, vfile);
      return (1);
    }
  } else if (OnlyVersionlessP(varray)) {
    /*
     * There is only versionless file in varray.
     * If the specified version is 1 the versionless file is regarded
     * as version 1 file.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The versionless file is dealt
       * with as version 1.  Thus new version is 2.
       */
      ConcNameAndVersion(vless, "2", vfile);
      strcpy(afile, vfile);
      return (1);
    } else {
      ver_no = strtoul(ver, (char **)NULL, 10);
      if (ver_no == 1) {
        /*
         * Version 1 is specified.  The versionless file is
         * dealt with as a version 1 file.
         */
        ConcNameAndVersion(name, "1", afile);
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /*
         * Other versions than 1 are recognized as if.
         */
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vfile);
        return (1);
      }
    }
  } else {
    if (check_vless_link(vless, varray, to_file, &highest_p) == 0) return (0);
    if (*to_file == '\0') {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is not linked
       * to any file in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The two higher than an
         * existing maximum version is dealt with as the
         * new version, because the one higher version is
         * dealt with as the actual version of the link
         * missing versionless file.
         */
        FindHighestVersion(varray, entry, max_no);
        sprintf(vbuf, "%u", max_no + 2);
        ConcNameAndVersion(vless, vbuf, vfile);
        strcpy(afile, vfile);
        return (1);
      } else {
        /* A version is specified. */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindHighestVersion(varray, entry, max_no);
        if (ver_no == max_no + 1) {
          /*
           * If the version is one higher than the
           * existing highest version is specified, it
           * is dealt with as a version of the link
           * missing versionless file.
           */
          sprintf(vbuf, "%u", ver_no);
          ConcNameAndVersion(vless, vbuf, vfile);
          strcpy(afile, vless);
          return (1);
        } else {
          /*
           * We have to walk through varray and try
           * to find the file with the specified version.
           */
          FindSpecifiedVersion(varray, entry, ver_no);
          if (entry != NULL) {
            ConcDirAndName(dir, entry->name, afile);
            strcpy(vfile, afile);
            return (1);
          }
          /*
           * There is not a file with the specified
           * version in varray.  The specified file can
           * be recognized as if.
           * Most user will hope to create a new file in
           * same case as old.   One of case sensitive
           * names in the files are stored in the trail
           * marker entry in varray by get_version_array
           * routine.
           * We will use it, although we cannot say all
           * of the gathered files has the name in same
           * case.
           */
          while (varray->version_no != LASTVERSIONARRAY) varray++;
          ConcNameAndVersion(varray->name, ver, afile);
          ConcDirAndName(dir, afile, vfile);
          strcpy(afile, vfile);
          return (1);
        }
      }
    } else {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is linked to
       * one of files in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The one higher than the
         * existing highest versioned file in varray is a
         * new file.
         */
        FindHighestVersion(varray, entry, max_no);
        sprintf(vbuf, "%u", max_no + 1);
        /*
         * We will use the name of the highest versioned file
         * as the name of the new file.
         */
        strcpy(vless, entry->name);
        separate_version(vless, ver, 1);
        ConcDirAndName(dir, vless, afile);
        ConcNameAndVersion(afile, vbuf, vfile);
        strcpy(afile, vfile);
        return (1);
      } else {
        /*
         * A version is specified.  We have to walk thorough
         * varray and try to find the file with the specified
         * version.
         */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindSpecifiedVersion(varray, entry, ver_no);
        if (entry != NULL) {
          ConcDirAndName(dir, entry->name, afile);
          strcpy(vfile, afile);
          return (1);
        }
        /*
         * There is not a file with the specified
         * version in varray.  The specified file can
         * be recognized as if.
         * Most user will hope to create a new file in
         * same case as old.   We will use the name of
         * the highest versioned file as the name of the
         * new file.
         */
        FindHighestVersion(varray, entry, max_no);
        strcpy(vless, entry->name);
        separate_version(vless, vbuf, 1);
        ConcDirAndName(dir, vless, afile);
        ConcNameAndVersion(afile, ver, vfile);
        strcpy(afile, vfile);
        return (1);
      }
    }
  }
#endif /* DOS */
}

/*
 * Name:	get_old_new
 *
 * Argument:	char	*dir	Directory absolute path following the UNIX
 *				file naming convention on which varray
 *				has been gathered.
 *		FileName *varray
 *				The version array already filled by
 *				get_version_array routine.
 *		char	*afile	File name.  It might include a version field.
 *				The version field have to be following the
 *				UNIX convention, that is "name.~##~", not
 *				Xerox Lisp one.  The versionless file is
 *				also acceptable.
 *				afile is also used as a place where the true
 *				name of the  recognized file will be stored.
 *		char	*vfile	The place where the versioned name of the
 *				recognized file will be stored.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	afile will be replaced with the full name of the recognized file
 *		when the recognition succeed.  vfile will be replaced with the
 *		versioned full name of the recognized file when the recognition
 *		succeed.
 *
 * Description:
 *
 * Accepts a version array and filename, and recognize a file in the version
 * array with old/new mode.
 * "Old/new" file means the "newest existing" or "not yet existing" file with the
 * version which is one higher than the current highest version.
 * Notice that, afile and vfile are not necessary same name, because the versionless
 * file is recognized as an old file, afile will hold the true versionless file name,
 * although vfile will hold the versioned file name anyway.
 * afile could be used as the real UNIX pathname to access the recognized file, on
 * the other hand, vfile could be used as file name from which the file name
 * which should be returned to Lisp could be produced.
 *
 */

static int get_old_new(char *dir, FileName *varray, char *afile, char *vfile)
{
  char name[MAXPATHLEN], vless[MAXPATHLEN], to_file[MAXPATHLEN];
  char ver[VERSIONLEN], vbuf[VERSIONLEN];
  unsigned ver_no, max_no;
  int highest_p;
  FileName *entry;

  strcpy(name, afile);
  separate_version(name, ver, 1);

  if (NoFileP(varray)) {
    /*
     * If there is no file with such name, "old/new" file is always
     * recognized.
     */
    if (*ver == '\0' || strcmp(ver, "1") == 0) {
      /*
       * If version is not specified or 1 is specified,
       * we can return versionless file as afile.
       */
      ConcNameAndVersion(name, "1", afile);
      ConcDirAndName(dir, afile, vfile);
      ConcDirAndName(dir, name, afile);
      return (1);
    } else {
      /*
       * A version other than 1 is specified.  "New" file
       * is recognized as if.
       */
      ConcDirAndName(dir, afile, vfile);
      strcpy(afile, vfile);
      return (1);
    }
  }

  if (get_versionless(varray, vless, dir) == 0) {
    /*
     * There is no versionless file, but at least one versioned
     * file exists.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The highest versioned file
       * is an old file.
       */
      FindHighestVersion(varray, entry, max_no);
      ConcDirAndName(dir, entry->name, afile);
      strcpy(vfile, afile);
      return (1);
    } else {
      /*
       * A version is specified.  We have to walk thorough
       * varray and try to find the file with the specified
       * version.
       */
      ver_no = strtoul(ver, (char **)NULL, 10);
      FindSpecifiedVersion(varray, entry, ver_no);
      if (entry != NULL) {
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      }
      /*
       * There is not a file with the specified version in varray.
       * The specified file can be recognized as if.
       * Most user will hope to create a new file in same case as
       * old.   One of case sensitive names in the files are stored
       * in the trail marker entry in varray by get_version_array
       * routine.
       * We will use it, although we cannot say all of the gathered
       * files has the name in same case.
       */
      while (varray->version_no != LASTVERSIONARRAY) varray++;
      ConcNameAndVersion(varray->name, ver, afile);
      ConcDirAndName(dir, afile, vfile);
      strcpy(afile, vfile);
      return (1);
    }
  } else if (OnlyVersionlessP(varray)) {
    /*
     * There is only versionless file in varray.
     * If the specified version is 1 the versionless file is regarded
     * as version 1 file.
     */
    if (*ver == '\0') {
      /*
       * No version is specified.  The versionless file is dealt
       * with as version 1.
       */
      ConcNameAndVersion(vless, "1", vfile);
      strcpy(afile, vless);
      return (1);
    } else {
      ver_no = strtoul(ver, (char **)NULL, 10);
      if (ver_no == 1) {
        /*
         * Version 1 is specified.  The versionless file is
         * dealt with as a version 1 file.
         */
        ConcNameAndVersion(name, "1", afile);
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /*
         * Other versions than 1 are recognized as if.
         */
        ConcDirAndName(dir, afile, vfile);
        strcpy(afile, vfile);
        return (1);
      }
    }
  } else {
    if (check_vless_link(vless, varray, to_file, &highest_p) == 0) return (0);
    if (*to_file == '\0') {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is not linked
       * to any file in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The one higher than an
         * existing maximum version is dealt with as the
         * old version, and it should be a version of the
         * link missing versionless file.
         */
        FindHighestVersion(varray, entry, max_no);
        sprintf(vbuf, "%u", max_no + 1);
        ConcNameAndVersion(vless, vbuf, vfile);
        strcpy(afile, vless);
        return (1);
      } else {
        /* A version is specified. */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindHighestVersion(varray, entry, max_no);
        if (ver_no == max_no + 1) {
          /*
           * If the version is one higher than the
           * existing highest version is specified, it
           * is dealt with as a version of the link
           * missing versionless file.
           */
          sprintf(vbuf, "%u", ver_no);
          ConcNameAndVersion(vless, vbuf, vfile);
          strcpy(afile, vless);
          return (1);
        } else {
          /*
           * We have to walk through varray and try
           * to find the file with the specified version.
           */
          FindSpecifiedVersion(varray, entry, ver_no);
          if (entry != NULL) {
            ConcDirAndName(dir, entry->name, afile);
            strcpy(vfile, afile);
            return (1);
          }
          /*
           * There is not a file with the specified
           * version in varray.  The specified file can
           * be recognized as if.
           * Most user will hope to create a new file in
           * same case as old.   One of case sensitive
           * names in the files are stored in the trail
           * marker entry in varray by get_version_array
           * routine.
           * We will use it, although we cannot say all
           * of the gathered files has the name in same
           * case.
           */
          while (varray->version_no != LASTVERSIONARRAY) varray++;
          ConcNameAndVersion(varray->name, ver, afile);
          ConcDirAndName(dir, afile, vfile);
          strcpy(afile, vfile);
          return (1);
        }
      }
    } else {
      /*
       * There is a versionless file in varray and at least one
       * versioned file exists.  The versionless file is linked to
       * one of files in varray.
       */
      if (*ver == '\0') {
        /*
         * No version is specified.  The highest versioned file
         * in varray is an old file.
         */
        FindHighestVersion(varray, entry, max_no);
        ConcDirAndName(dir, entry->name, afile);
        strcpy(vfile, afile);
        return (1);
      } else {
        /*
         * A version is specified.  We have to walk thorough
         * varray and try to find the file with the specified
         * version.
         */
        ver_no = strtoul(ver, (char **)NULL, 10);
        FindSpecifiedVersion(varray, entry, ver_no);
        if (entry != NULL) {
          ConcDirAndName(dir, entry->name, afile);
          strcpy(vfile, afile);
          return (1);
        }
        /*
         * There is not a file with the specified
         * version in varray.  The specified file can
         * be recognized as if.
         * Most user will hope to create a new file in
         * same case as old.   We will use the name of
         * the highest versioned file as the name of the
         * new file.
         */
        FindHighestVersion(varray, entry, max_no);
        strcpy(vless, entry->name);
        separate_version(vless, vbuf, 1);
        ConcDirAndName(dir, vless, afile);
        ConcNameAndVersion(afile, ver, vfile);
        strcpy(afile, vfile);
        return (1);
      }
    }
  }
}
