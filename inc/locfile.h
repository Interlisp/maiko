#ifndef LOCFILE_H
#define LOCFILE_H 1
/* $Id: locfile.h,v 1.2 1999/01/03 02:06:13 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-94 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include <ctype.h>     /* for isdigit */
#include <errno.h>
#include <limits.h>   /* for NAME_MAX */
#include <string.h>   /* for strlen */
#include <sys/param.h> /* for MAXPATHLEN */
#include <dirent.h>   /* for MAXNAMLEN */
#include "lispemul.h" /* for DLword */
#include "commondefs.h" /* for error */

#define	FDEV_PAGE_SIZE		512	/* 1 page == 512 byte */

#define	RECOG_OLD		(S_POSITIVE | 0)
#define	RECOG_OLDEST		(S_POSITIVE | 1)
#define	RECOG_NEW		(S_POSITIVE | 2)
#define	RECOG_OLD_NEW		(S_POSITIVE | 3)
#define	RECOG_NON		(S_POSITIVE | 5)

#define	ACCESS_INPUT		(S_POSITIVE | 0)
#define	ACCESS_OUTPUT		(S_POSITIVE | 1)
#define	ACCESS_BOTH		(S_POSITIVE | 2)
#define	ACCESS_APPEND		(S_POSITIVE | 3)

/* For getfileinfo */
#define	LENGTH			(S_POSITIVE | 1)
#define	WDATE			(S_POSITIVE | 2)
#define	RDATE			(S_POSITIVE | 3)
#define	AUTHOR			(S_POSITIVE | 5)
#define	PROTECTION		(S_POSITIVE | 6)
#define	EOL			(S_POSITIVE | 7)
#define	ALL			(S_POSITIVE | 8)

#define	ToLispTime(x)	((int)(x) + 29969152)
			/* For getfileinfo. For WDATE&RDATE */
			/* 29969152 == (timer.c)LISP_UNIX_TIME_DIFF */
			/* - 61200 == - 17hours */

#define	ToUnixTime(x)	((int)(x) - 29969152)
			/* For getfileinfo. For WDATE&RDATE */
			/* 29969152 == (timer.c)LISP_UNIX_TIME_DIFF */

#define StrNCpyFromCToLisp(lispbuf, cbuf ,len)	do {	\
			char *lf_sptr = (cbuf);		\
                        char *lf_dptr = (lispbuf);                      \
			for(size_t lf_i=0;lf_i<(len);lf_i++)\
				GETBYTE(lf_dptr++) = *lf_sptr++;		\
  } while (0)

#define StrNCpyFromLispToC(cbuf , lispbuf, len)	do {	\
			char *lf_sptr = (lispbuf);                                          \
			char *lf_dptr = (cbuf);                       \
			for(size_t lf_i=0;lf_i<(len);lf_i++)\
				*lf_dptr++ = GETBYTE(lf_sptr++);		\
  } while (0)

#define FGetNum(ptr, place) do { \
             if(((ptr) & SEGMASK)== S_POSITIVE) {(place) = ((ptr) & 0xffff);}\
        else if(((ptr) & SEGMASK)== S_NEGATIVE) {(place) = (int)((ptr)| 0xffff0000);}\
        else {return(NIL);}} while (0)

#ifndef min
#define min(a, b) (((a) <= (b))?(a):(b))
#endif /* min */

/************************************************************************/
/*									*/
/*		  L i s p S t r i n g T o C S t r i n g			*/
/*									*/
/*	Convert the Lisp string in Lisp into a null-terminated C	*/
/*	string in C.  MaxLen is, for safety, the maximum length of	*/
/*	the resulting string, so the buffer doesn't overflow and	*/
/*	smash memory.							*/
/*									*/
/*	WARNINGS:  The Lisp string is truncated to fit the C string	*/
/*	without warning to the user.  FAT Lisp strings have only	*/
/*	the low 8 bits of each character copied over.			*/
/*									*/
/************************************************************************/
#ifndef BYTESWAP
static inline void LispStringToCString(LispPTR Lisp, char *C, size_t MaxLen)
{
    OneDArray	*lf_arrayp;
    char	*lf_base, *lf_dp;
    short	*lf_sbase;
    size_t		 lf_length;
    lf_arrayp = (OneDArray *)NativeAligned4FromLAddr(Lisp);
    lf_length = min(MaxLen, lf_arrayp->fillpointer);
    switch(lf_arrayp->typenumber)
      {
	case THIN_CHAR_TYPENUMBER:
		lf_base = ((char *)(NativeAligned2FromLAddr(lf_arrayp->base)))
		       + ((int)(lf_arrayp->offset));
		strncpy(C, lf_base, lf_length);
		(C)[lf_length] = '\0';
		break;

	case FAT_CHAR_TYPENUMBER:
		lf_sbase = ((short *)(NativeAligned2FromLAddr(lf_arrayp->base)))
		       + ((int)(lf_arrayp->offset));
                lf_dp = C;
		for(size_t lf_i=0;lf_i<(lf_length);lf_i++)
		  *lf_dp++ = (char)(*lf_sbase++);
		*lf_dp = '\0';
		break;
	default:
		error("LispStringToCString: Not a character array.\n");
      }
}
#else  /* BYTESWAP == T CHANGED-BY-TAKE */
static inline void LispStringToCString(LispPTR Lisp, char *C, size_t MaxLen)
{
    OneDArray	*lf_arrayp;
    char	*lf_base, *lf_dp;
    short	*lf_sbase;
    size_t 	lf_length;
    lf_arrayp = (OneDArray *)(NativeAligned4FromLAddr(Lisp));
    lf_length = min(MaxLen, lf_arrayp->fillpointer);
    switch(lf_arrayp->typenumber)
      {
	case THIN_CHAR_TYPENUMBER:
		lf_base = ((char *)(NativeAligned2FromLAddr(lf_arrayp->base)))
		       + ((int)(lf_arrayp->offset));
		StrNCpyFromLispToC(C , lf_base , lf_length );
		(C)[lf_length] = '\0';
		break;

	case FAT_CHAR_TYPENUMBER:
		lf_sbase = ((short *)(NativeAligned2FromLAddr(lf_arrayp->base)))
		       + ((int)(lf_arrayp->offset));
                lf_dp = C;
		for(size_t lf_ii=0;lf_ii<(lf_length);lf_ii++,lf_sbase++)
                    *lf_dp++ = (char)(GETWORD(lf_sbase));
		*lf_dp = '\0';
		break;
	default:
		error("LispStringToCString: Not a character array.\n");
      }
}

#endif /* BYTESWAP */


/************************************************************************/
/*									*/
/*		  L i s p S t r i n g L e n g t h			*/
/*									*/
/*	Get the byte length of the string in Lisp.			*/
/*                                                                      */
/*  As a side effect, sets FatP to 1 if LispString is fat, 0 otherwise  */
/*  Errors if LispString isn't a string (1-d array of characters)       */
/*									*/
/************************************************************************/
#define	LispStringLength(LispString, Length, FatP)			\
  do {									\
    OneDArray	*lf_arrayp;						\
    lf_arrayp = (OneDArray *)(NativeAligned4FromLAddr(LispString));		\
    switch(lf_arrayp->typenumber)					\
      {									\
	case THIN_CHAR_TYPENUMBER:					\
        	(Length) = lf_arrayp->fillpointer;			\
		(FatP) = 0;						\
		break;							\
									\
	case FAT_CHAR_TYPENUMBER:					\
            	(Length) = lf_arrayp->fillpointer * 2;			\
            	(FatP) = 1;                                             \
		break;							\
	default:							\
		error("LispStringLength: Not a character array.\n");	\
      }									\
    } while (0)




/************************************************************************/
/*									*/
/*		S T R I N G _ B A S E				*/
/*									*/
/*  Return (C-pointer) to the characters in lstringp, in cstringp.      */
/*									*/
/************************************************************************/
#define STRING_BASE(lstringp, cstringp) \
do  {				\
	LispPTR	*lf_naddress;				  \
	lf_naddress = (LispPTR *)(NativeAligned4FromLAddr(lstringp));		  \
	(cstringp) = (char *)(NativeAligned2FromLAddr(((OneDArray *)lf_naddress)->base)); \
 } while (0)

#define	LispNumToCInt(Lisp)					\
       ( (((Lisp) & SEGMASK) == S_POSITIVE) ? ((Lisp) & 0xFFFF) : \
          (((Lisp) & SEGMASK) == S_NEGATIVE) ? ((Lisp) | 0xFFFF0000) : \
          (*((int *)(NativeAligned4FromLAddr(Lisp)))) )

#define	UPLOWDIFF	0x20

#define	DOWNCASE(name) do {						\
								\
	char	*lf_cp;						\
								\
	for(lf_cp = (name); *lf_cp!='\0'; ++lf_cp)                      \
	  if((*lf_cp >= 'A') && (*lf_cp <= 'Z')) *lf_cp += UPLOWDIFF;	\
  } while (0)

#define	UPCASE(name) do {						\
								\
	char	*lf_cp;					\
								\
	for(lf_cp = (name); *lf_cp!='\0'; ++lf_cp)                      \
	  if((*lf_cp >= 'a') && (*lf_cp <= 'z')) *lf_cp -= UPLOWDIFF;	\
  } while (0)

#define DIR_OR_FILE_P(name, type) do {				\
	int	lf_result;				\
	struct stat	lf_statbuf;				\
								\
	TIMEOUT(lf_result = stat(name, &lf_statbuf));		\
	if (lf_result < 0) {					\
		*Lisp_errno = errno;				\
		(type) = 0;					\
	} else {  						\
		switch (lf_statbuf.st_mode & S_IFMT) {		\
								\
		      case S_IFDIR:				\
                        (type) = -1;				\
			break;					\
								\
		      case S_IFREG:				\
                        (type) = 1;				\
			break;					\
								\
		      default:					\
			/*					\
			 * Should we deal with the other	\
			 * types?				\
			 */					\
                        (type) = 0;				\
			break;					\
		}						\
	}							\
  } while (0)

#define	DIRP(path, dir, buf) do {				\
	int lf_rval;					\
	struct	stat	lf_statbuf;			\
		strcpy(buf, path);			\
		strcat(buf, dir);			\
		TIMEOUT( lf_rval=stat(buf, &lf_statbuf) );	\
		if( lf_rval == 0){				\
			if( (lf_statbuf.st_mode & S_IFMT) == S_IFDIR ){	\
				strcat(path, dir);			\
				return(1);				\
			}				\
		}					\
		if( lf_rval == -1 && errno == 60){		\
			*Lisp_errno = 60;		\
			return(0);			\
		}					\
  } while (0)

#define	FILEP(path, file, buf) do {				\
	int lf_rval;					\
		strcpy(buf, path);			\
		strcat(buf, file);			\
		TIMEOUT( lf_rval=access(buf, F_OK) );	\
		if( access(buf, F_OK) == 0){		\
			strcat(path, file);		\
			return(1);			\
		}					\
  } while (0)

#define	STREQ(name1, name2)(	\
                (*(name1) == *(name2)) && (strcmp(name1, name2) == 0)   \
                )

#define	SPECIALFILEMARK		(-1)

#define NumericStringP(str, truetag, falsetag) do {			\
	char	*lfn_cp;						\
									\
        /* NOLINTNEXTLINE(bugprone-macro-parentheses) */		\
	if (*(str) == '\0') goto falsetag;				\
									\
	for(lfn_cp = str; *lfn_cp!='\0'; ++lfn_cp)			\
	  if(*lfn_cp < '0' || '9' < *lfn_cp)				\
	    goto falsetag; /* NOLINT(bugprone-macro-parentheses) */	\
	goto truetag;	/* NOLINT(bugprone-macro-parentheses) */	\
  } while (0)

/*
 * Name:	LispVersionToUnixVersion
 *
 * Argument:	char	*pathname
 *				Xerox Lisp syntax pathname.
 *
 * Value:	If succeed, returns 1, otherwise 0.
 *
 * Side Effect:	The version part of pathname is destructively modified.
 *
 * Description:
 *
 * Destructively modify the version part of pathname which is following the
 * Xerox Lisp file naming convention to UNIX one.
 * If the file name which is passed from Lisp has the version part, it must be
 * a valid one (i.e. constructed with only number).  This is guaranteed by Lisp
 * code.
 * This macro should be called at the top of the routines which accept the
 * file name from lisp before converting it into UNIX file name, because
 * locating the version part, the informations about quoted characters are needed.
 * They might be lost in the course of the conversion.
 *
 */
#ifdef DOS

/* DOS version of LispVersionToUnixVersion */
/* * * * * This is done this way because DOS can't handle the non-DOS version -- */
/* * * * * it gave "Too many characters in a character constant" errors!        */
#include "lispver1.h"
#else /* DOS */
/* NON-DOS version of the macro LispVersionToUnixVersion */
#include "lispver2.h"
#endif /* DOS */


/*
 * Name:	UnixVersionToLispVersion
 *
 * Argument:	char *pathname  UNIX syntax pathname.
 *		int vlessp      If 0, versionless file is converted to version 1.
 *				Otherwise, remains as versionless.
 *
 * Side Effect:	The version part of pathname is destructively modified.
 *
 * Description:
 *
 * Destructively modify the version part of pathname which is following the
 * UNIX file naming convention to Xerox Lisp one.
 * This macro should be called, in the routines which convert the UNIX pathname
 * to Lisp one, just before it returns the result to Lisp, because converting
 * version field will append a semicolon and it might make the routine be
 * confused.
 * The file which has not a valid version field, that is ".~##~" form, is
 * dealt with as version 1.
 */

#define UnixVersionToLispVersion(pathname, vlessp)                      \
  do {                                                                  \
    char *n_end;                                                        \
    char *v_start;                                                      \
    int	v_len;                                                          \
                                                                        \
    if (!parse_file_version(pathname, 1, &n_end, &v_start, &v_len)) {   \
      if (!vlessp) strcat(pathname, ";1");                              \
    } else {                                                            \
      *n_end++ = ';';                                                   \
      while (v_len-- > 0) *n_end++ = *v_start++;                        \
      *n_end = '\0';                                                    \
    }                                                                   \
  }  while (0)

/*
 * Name:	ConcDirAndName
 *
 * Argument:	char	*dir	The name of the directory.
 *		char	*name	The name of a file.
 *		char	*fname	The place where the full file name should be
 *				stored.
 * Value:	N/A
 *
 * Side Effect:	fname is replaced with the full file name.
 *
 * Description:
 *
 * Concatenate the directory name and root file name.  Checks if dir contains
 * the trail directory delimiter or not.
 *
 */

static inline void ConcDirAndName(char *dir, char *name, char *fname)
{
  char	*lf_cp1, *lf_cp2;

  lf_cp1 = dir;
  lf_cp2 = dir;

  while (*lf_cp2 != '\0') {
    switch (*lf_cp2) {

    case '/':
      lf_cp1 = lf_cp2;
      lf_cp2++;
      break;

    default:
      lf_cp2++;
      break;
    }
  }
  if (lf_cp1 == (lf_cp2 - 1)) {
    if (lf_cp1 == (dir)) {
      /* dir is a root directory. */
      strcpy(fname, "/");
      strcat(fname, name);
    } else {
      /* The trail directory is included. */
      strcpy(fname, dir);
      strcat(fname, name);
    }
  } else {
    /* The trail directory is not included */
    strcpy(fname, dir);
    strcat(fname, "/");
    strcat(fname, name);
  }
}

/*
 * Name:	ConcNameAndVersion
 *
 * Argument:	char	*name	The root file name.
 *		char	*ver	The file version.
 *		char	*rname 	The place where the concatenated file name will be
 *				stored.
 * Value:	N/A
 *
 * Side Effect:	rname is replaced with the concatenated file name.
 *
 * Description:
 *
 * Concatenate the root file name and its version in UNIX format.
 *
 * XXX: this code is unsafe and could result in memory smashes if the
 * sizes of the arguments are not correctly specified
 *
 */

static inline void ConcNameAndVersion(char *name, char *ver, char *rname)
{
	if (*ver != '\0') {
		strcpy(rname, name);
		strcat(rname, ".~");
		strcat(rname, ver);
		strcat(rname, "~");
	} else {
		strcpy(rname, name);
	}
}

#define	 VERSIONLEN		10

#define	MAXVERSION		999999999

#define	LASTVERSIONARRAY	((unsigned) -1)
#define	VERSIONARRAYCHUNKLENGTH	200
#define	VERSIONARRAYMAXLENGTH   2000

#define NoFileP(varray)						\
        (((varray)->version_no == LASTVERSIONARRAY)? 1 : 0)


#ifdef DOS
#define OnlyVersionlessP(varray) 0
#else
#define OnlyVersionlessP(varray)							 \
        (((varray)->version_no == 0 && ((varray) + 1)->version_no == LASTVERSIONARRAY) ? \
	 1 : 0)
#endif /* DOS */

/* An argument of AddDotNoExtension must be LispVersion convention */
/* Like "foo/fee.fee;3" or "/foo/foo;3" */
/* AddDotNoExtension must be put after UnixVersionToLispVersion */

#define	AddDotNoExtension(file) do {			\
		char	*lf_cp;			\
		char	*lf_cp1;			\
		if( (strrchr(file,'.')== 0) && ((lf_cp=strrchr(file,';'))!=0) ){	\
			for(lf_cp1=lf_cp;*lf_cp1!='\0';lf_cp1++);	\
			*(lf_cp1+1) = '\0';		\
			for(;lf_cp!=lf_cp1;lf_cp1--)		\
				*lf_cp1 = *(lf_cp1-1);	\
			*lf_cp = '.';			\
		}					\
  } while (0)

/* An argument of RemoveDotNoExtension must be LispVersion convention */
/* Like "foo/fee.fee;3" or "/foo/foo.;3" */
/* RemoveDotNoExtension must be put before LispVersionToUnixVersion */

#define RemoveDotNoExtension(file) do {					\
		char	*lf_cp;					\
		if( ((lf_cp=strrchr(file, ';'))!=0) && (*(lf_cp-1)=='.') ){	\
			for(;*lf_cp!='\0';++lf_cp)				\
				*(lf_cp-1) = *lf_cp;				\
			*(lf_cp-1) = '\0';					\
		}							\
  } while (0)

#define	ChangeToVersionless(pathname) do {			\
		char	*lf_cp;			\
		if( (lf_cp=strrchr(pathname, ';')) != 0)	\
			*lf_cp = '\0';			\
  } while (0)


#define	UNLINK(x) do {					\
		TIMEOUT(lf_rval=unlink(x));		\
		if(lf_rval == -1){				\
			err_mess("unlink", errno);	\
			*Lisp_errno = errno;		\
			return(0);			\
		}					\
  } while (0)

#define LINK(x,y) do {					\
		TIMEOUT(lf_rval=link(x, y));		\
		if(lf_rval == -1){				\
			if(errno == 2)			\
				return(1);		\
			else{				\
				err_mess("link", errno);\
				*Lisp_errno = errno;	\
				return(0);		\
			}				\
		}					\
  } while (0)

#define RENAME(x,y) do {					\
		TIMEOUT(lf_rval=rename(x, y));		\
		if(lf_rval == -1){				\
			switch(errno){			\
			case ENOENT:				\
				return(1);		\
			case EXDEV:			\
				*Lisp_errno = errno;	\
				return(0);		\
			default:			\
				err_mess("rename", errno);\
				*Lisp_errno = errno;	\
				return(0);		\
			}				\
		}					\
  } while (0)

#define	STAT(x,y) do {					\
		TIMEOUT(lf_rval=stat(x, y));		\
		if(lf_rval != 0){				\
			err_mess("stat", errno);	\
			*Lisp_errno = errno;		\
			return(-1);			\
		}					\
  } while (0)

/*
 * For file name length check
 */

#define FileNameTooLong(val) do {			\
	*Lisp_errno = ENAMETOOLONG;                     \
	return((val));					\
  } while (0)

static inline int parse_file_version(char *name, int digitsonly, char **n_end,
                                      char **v_start, int *v_length)
{
  char *sp, *ep;
  size_t name_len;

  name_len = strlen(name);
  ep = &name[name_len - 1];

  /* handle special case of Alto/IFS names with !nnn version.
     To be considered for this case the name MUST end with ![0-9]+, however
     version 0 is not valid.
   */
  sp = strrchr(name, '!');
  if (sp != NULL) {
    sp++;         /* "!nnn" => "nnn" or "!" => ""  */
    if (*sp != '\0' && sp[strspn(sp, "0123456789")] == '\0') {
      /* it was all digits after the '!', so go with it */
      *n_end = sp - 1; /* name ends at '!'  */
      while (*sp == '0' && sp < ep) sp++; /* skip leading zeroes */
      if (*sp == '0') return (0); /* version 0 is not valid */
      *v_start = sp;   /* version start after '!' */
      *v_length = (ep - sp) + 1;
      return ((*v_length >= VERSIONLEN) ? 0 : 1); /* fail on version too long */
    }
  }

  /* if the name is too short to have a name and a version number
     ".~#~" or doesn't end with "~" then there is no version number
  */
  if (name_len < 4 || *ep != '~')
    return (0);

  /* The name ends with a "~" so scan back in the filename looking for
     another "~" terminating early if we need only digits and find
     something else
  */
  sp = ep - 1;
  while (sp > name && *sp != '~') {
    if (digitsonly && !isdigit(*sp)) return (0);
    --sp;
  }

  /* test for no initial "~" or no "." before "~", or
   * version number length not at least 1
   */
  if (sp == name || *(sp - 1) != '.' || (ep - sp) - 1 < 1)
    return (0);

  /* After this point we have a version number in the form .~#~ with sp
     pointing at the starting "~", ep pointing at the last "~",
     and there must be at least one digit. Scan past any leading
     zeros in the version, taking care not to remove the last digit.
  */

  *n_end = sp - 1;    /* save location of "." */

  sp++;                  /* skip over the "." */
  while (*sp == '0' && sp < (ep - 1)) {
    sp++;
  }
  if (*sp == '0') return (0);  /* version 0 is not valid */
  *v_start = sp;           /* save location of first significant digit in version */
  *v_length = (ep - sp);   /* save length of version */
  return ((*v_length >= VERSIONLEN) ? 0 : 1); /* fail on version too long */
}

/********************************************************/
/*  file-system-specific defns                */
/*                            */
/*  DIRSEP = OS-specific directory separator character.   */
/*  UNIXDIRSEP = UNIX's    "              */
/*  DRIVESEP = OS-specific drive separator character. */
/*    (only used with DOS as of 3/93)         */
/********************************************************/
#ifdef DOS
#define DIRSEP '\\'
#define DIRSEPSTR "\\"
#define DRIVESEP ':'
#define UNIXDIRSEP '/'
#define MAXNAMLEN _MAX_PATH
#else
#define DIRSEPSTR "/"
#define DIRSEP '/'
#define UNIXDIRSEP '/'
/* system includes may already define MAXNAMLEN */
#if !defined(MAXNAMLEN)
#define MAXNAMLEN NAME_MAX
#endif
#endif

#endif /* LOCFILE_H */
