/* $Id: locfile.h,v 1.2 1999/01/03 02:06:13 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-94 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


#define	PAGE_SIZE		512	/* 1 page == 512 byte */

#define	RECOG_OLD		S_POSITIVE | 0
#define	RECOG_OLDEST		S_POSITIVE | 1
#define	RECOG_NEW		S_POSITIVE | 2
#define	RECOG_OLD_NEW		S_POSITIVE | 3
#define	RECOG_NON		S_POSITIVE | 5

#define	ACCESS_INPUT		S_POSITIVE | 0
#define	ACCESS_OUTPUT		S_POSITIVE | 1
#define	ACCESS_BOTH		S_POSITIVE | 2
#define	ACCESS_APPEND		S_POSITIVE | 3

/* For getfileinfo */
#define	LENGTH			S_POSITIVE | 1
#define	WDATE			S_POSITIVE | 2
#define	RDATE			S_POSITIVE | 3
#define	AUTHOR			S_POSITIVE | 5
#define	PROTECTION		S_POSITIVE | 6
#define	EOL			S_POSITIVE | 7
#define	ALL			S_POSITIVE | 8


extern	DLword	*Lisp_world;	/* To access LispSysout area */


#define	ToLispTime(x)	((int)x + 29969152)
			/* For getfileinfo. For WDATE&RDATE */
			/* 29969152 == (timer.c)LISP_UNIX_TIME_DIFF */
			/* - 61200 == - 17hours */

#define	ToUnixTime(x)	((int)x - 29969152)
			/* For getfileinfo. For WDATE&RDATE */
			/* 29969152 == (timer.c)LISP_UNIX_TIME_DIFF */

#define StrNCpyFromCToLisp(lispbuf, cbuf ,len)	{ register int i;	\
			register char *sptr,*dptr;			\
			for(i=0,sptr=(cbuf),dptr =(lispbuf);i<(len);i++)\
				GETBYTE(dptr++) = *sptr++;		\
		}

#define StrNCpyFromLispToC(cbuf , lispbuf, len)	{ register int i;	\
			register char *sptr,*dptr;			\
			for(i=0,sptr=(lispbuf),dptr =(cbuf);i<(len);i++)\
				*dptr++ = GETBYTE(sptr++);		\
		}

#define FGetNum(ptr, place) { \
             if(((ptr) & SEGMASK)== S_POSITIVE) {(place) = ((ptr) & 0xffff);}\
        else if(((ptr) & SEGMASK)== S_NEGATIVE) {(place) = (int)((ptr)| 0xffff0000);}\
        else {return(NIL);}}


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
#define	LispStringToCString(Lisp, C, MaxLen)				\
  {									\
    OneDArray	*arrayp;						\
    char	*base, *dp;						\
    short	*sbase;							\
    int	i, length;							\
    arrayp = (OneDArray *)(Addr68k_from_LADDR(Lisp));			\
    length = min(MaxLen, arrayp->fillpointer);				\
    switch(arrayp->typenumber)						\
      {									\
	case THIN_CHAR_TYPENUMBER:					\
		base = ((char *)(Addr68k_from_LADDR(arrayp->base)))  	\
		       + ((int)(arrayp->offset));			\
		strncpy(C, base, length);				\
		C[length] = '\0';					\
		break;							\
									\
	case FAT_CHAR_TYPENUMBER:					\
		sbase = ((short *)(Addr68k_from_LADDR(arrayp->base)))	\
		       + ((int)(arrayp->offset));			\
		for(i=0,dp=C;i<(length);i++)				\
		  *dp++ = (char)(*sbase++);				\
		*dp = '\0';						\
		break;							\
	default:							\
		error("LispStringToCString: Not a character array.\n");	\
      }									\
  }
#else  /* BYTESWAP == T CHANGED-BY-TAKE */
#define	LispStringToCString(Lisp, C, MaxLen)				\
  {									\
    OneDArray	*arrayp;						\
    char	*base, *dp;						\
    short	*sbase;							\
    int	i, length;							\
    arrayp = (OneDArray *)(Addr68k_from_LADDR(Lisp));			\
    length = min(MaxLen, arrayp->fillpointer);				\
    switch(arrayp->typenumber)						\
      {									\
	case THIN_CHAR_TYPENUMBER:					\
		base = ((char *)(Addr68k_from_LADDR(arrayp->base)))  	\
		       + ((int)(arrayp->offset));			\
		/*for(i=0,dp=C;i<length;i++)				\
		{*dp++ =(char)(GETBYTE(base++));}**/			\
		StrNCpyFromLispToC(C , base , length );			\
		C[length] = '\0';					\
		break;							\
									\
	case FAT_CHAR_TYPENUMBER:					\
		sbase = ((short *)(Addr68k_from_LADDR(arrayp->base)))	\
		       + ((int)(arrayp->offset));			\
		for(i=0,dp=C;i<(length);i++,sbase++)				\
		  *dp++ = (char)(GETWORD(sbase));				\
		*dp = '\0';						\
		break;							\
	default:							\
		error("LispStringToCString: Not a character array.\n");	\
      }									\
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
  {									\
    OneDArray	*arrayp;						\
    arrayp = (OneDArray *)(Addr68k_from_LADDR(LispString));		\
    switch(arrayp->typenumber)						\
      {									\
	case THIN_CHAR_TYPENUMBER:					\
		Length = arrayp->fillpointer;				\
		FatP = 0;						\
		break;							\
									\
	case FAT_CHAR_TYPENUMBER:					\
		Length = arrayp->fillpointer * 2;			\
		FatP = 1;						\
		break;							\
	default:							\
		error("LispStringLength: Not a character array.\n");	\
      }									\
  }




/************************************************************************/
/*									*/
/*		S T R I N G _ B A S E				*/
/*									*/
/*  Return (C-pointer) to the characters in lstringp, in cstringp.      */
/*									*/
/************************************************************************/
#define STRING_BASE(lstringp, cstringp) \
  {				\
	register LispPTR	*naddress;				  \
	naddress = (LispPTR *)(Addr68k_from_LADDR(lstringp));		  \
	cstringp  = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));  \
  }

#ifndef min
#define min(a, b) ((a <= b)?a:b)
#endif /* min */

#define	LispNumToCInt(Lisp)					\
		( ((Lisp & SEGMASK) == S_POSITIVE) ?		\
		(Lisp & 0xFFFF) : (*((int *)(Addr68k_from_LADDR(Lisp)))) );

#define	UPLOWDIFF	0x20

#define	DOWNCASE(name){						\
								\
	register char	*cp;					\
								\
	for(cp = name; *cp!='\0'; ++cp)				\
	  if((*cp >= 'A') && (*cp <= 'Z')) *cp += UPLOWDIFF;	\
}

#define	UPCASE(name){						\
								\
	register char	*cp;					\
								\
	for(cp = name; *cp!='\0'; ++cp)				\
	  if((*cp >= 'a') && (*cp <= 'z')) *cp -= UPLOWDIFF;	\
}

#define DIR_OR_FILE_P(name, type){				\
	register int	result;					\
	struct stat	sbuf;					\
								\
	TIMEOUT(result = stat(name, &sbuf));			\
	if (result < 0) {					\
		*Lisp_errno = errno;				\
		type = 0;					\
	} else {  						\
		switch (sbuf.st_mode & S_IFMT) {		\
								\
		      case S_IFDIR:				\
			type = -1;				\
			break;					\
								\
		      case S_IFREG:				\
			type = 1;				\
			break;					\
								\
		      default:					\
			/*					\
			 * Should we deal with the other	\
			 * types?				\
			 */					\
			type = 0;				\
			break;					\
		}						\
	}							\
}

#ifdef FSERROR
#define	DIRP(path, dir, buf){				\
	int rval;					\
	struct	stat	statbuf;			\
		strcpy(buf, path);			\
		strcat(buf, dir);			\
		TIMEOUT( rval=stat(buf, &statbuf) );	\
		if( rval == 0){				\
			if( (statbuf.st_mode & S_IFMT) == S_IFDIR ){	\
				strcat(path, dir);			\
				return(1);				\
			}				\
		}					\
		if( rval == -1 && errno == 60){		\
			*Lisp_errno = 60;		\
			return(0);			\
		}					\
		}
#else
#define	DIRP(path, dir, buf){				\
	int rval;					\
	struct	stat	statbuf;			\
		strcpy(buf, path);			\
		strcat(buf, dir);			\
		TIMEOUT( rval=stat(buf, &statbuf) );	\
		if( rval == 0){				\
			if( (statbuf.st_mode & S_IFMT) == S_IFDIR ){	\
				strcat(path, dir);			\
				return(1);				\
			}				\
		}					\
		}
#endif

#define	FILEP(path, file, buf){				\
	int rval;					\
		strcpy(buf, path);			\
		strcat(buf, file);			\
		TIMEOUT( rval=access(buf, F_OK) );	\
		if( access(buf, F_OK) == 0){		\
			strcat(path, file);		\
			return(1);			\
		}					\
		}

#define	STREQ(name1, name2)(	\
		(*name1 == *name2) && (strcmp(name1, name2) == 0)	\
		)

#define	SPECIALFILEMARK		-1

#define NumericStringP(str, truetag, falsetag) {			\
	register char	*cp;						\
									\
	if (*str == '\0') goto falsetag;				\
									\
	for(cp = str; *cp!='\0'; ++cp)					\
	  if(*cp < '0' || '9' < *cp)					\
	    goto falsetag;						\
	goto truetag;							\
}

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
 * a valid one (i.e. construected with only number).  This is guaranteed by Lisp
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
/* * * * * it gived "Too many characters in a character constant" errors!        */
#include "lispver1.h"
#else /* DOS */
/* NON-DOS version of the macro LispVersionToUnixVersion */
#include "lispver2.h"
#endif /* DOS */


/*		
 * Name:	UnixVersionToLispVersion
 *
 * Argument:	char	*pathname
 *				UNIX syntax pathname.
 *		int	vlessp
 *				If 0, versionless file is converted to version 1.
 *				Otherwise, remains as versionless.
 *
 * Value:	If succeed, returns 1, otherwise 0.
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

#define UnixVersionToLispVersion(pathname, vlessp){				\
										\
	register char	*start;							\
	register char	*end;							\
	register char	*cp;							\
	register int	len, ver_no;						\
	char		ver_buf[VERSIONLEN];					\
										\
	if ((start = strchr(pathname, '~')) != NULL) {				\
		/* First of all, find the version field in pathname. */		\
		end = start;							\
		cp = start + 1;							\
		while (*cp) {							\
			if (*cp == '~') {					\
				start = end;					\
				end = cp;					\
				cp++;						\
			} else {						\
				cp++;						\
			}							\
		}								\
										\
		if (start != end && *(start - 1) == '.' && end == (cp - 1)) {	\
			/*							\
			 * pathname ends in the form ".~###~".  But we		\
			 * check ### is a valid number or not.			\
			 */							\
			len = (int)end - (int)start - 1;			\
			strncpy(ver_buf, start + 1, len);			\
			ver_buf[len] = '\0';					\
			NumericStringP(ver_buf, YES, NO);			\
		      YES:							\
			*(start - 1) = ';';					\
			*start = '\0';						\
			*end = '\0';						\
			/* call ato i to eliminate leading 0s. */		\
			ver_no = atoi(start + 1);				\
			sprintf(ver_buf, "%d", ver_no);				\
			strcat(pathname, ver_buf);				\
			goto CONT;						\
										\
		      NO:							\
			/* Dealt with as version 1 unless vlessp */		\
			if (!vlessp) strcat(pathname, ";1");			\
		      CONT:							\
			cp--;	/* Just for label */				\
		} else {							\
			/* Dealt with as version 1 unless vlessp. */		\
			if (!vlessp) strcat(pathname, ";1");			\
		}								\
	} else {								\
		/* Dealt with as version 1 unless vlessp. */			\
		if (!vlessp) strcat(pathname, ";1");				\
	}									\
}

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

#define ConcDirAndName(dir, name, fname){					\
									\
	register char	*cp1, *cp2;					\
									\
	cp1 = dir;							\
	cp2 = dir;							\
									\
	while (*cp2 != '\0') {						\
		switch (*cp2) {						\
									\
		      case '/':						\
			cp1 = cp2;					\
			cp2++;						\
			break;						\
									\
		      default:						\
			cp2++;						\
			break;						\
		}							\
	}								\
	if (cp1 == (cp2 - 1)) {						\
		if (cp1 == dir) {					\
			/* dir is a root directory. */			\
			strcpy(fname, "/");				\
			strcat(fname, name);				\
		} else {						\
			/* The trail directory is included. */		\
			strcpy(fname, dir);				\
			strcat(fname, name);				\
		}							\
	} else {							\
		/* The trail directory is not included */		\
		strcpy(fname, dir);					\
		strcat(fname, "/");					\
		strcat(fname, name);					\
	}								\
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
 */

#define ConcNameAndVersion(name, ver, rname){				\
	if (*ver != '\0') {						\
		strcpy(rname, name);					\
		strcat(rname, ".~");					\
		strcat(rname, ver);					\
		strcat(rname, "~");					\
	} else {							\
		strcpy(rname, name);					\
	}								\
}

#define	 VERSIONLEN		16

#define	MAXVERSION		999999999

#define	LASTVERSIONARRAY	0xFFFFFFFF
#define	VERSIONARRAYLENGTH	200

#define NoFileP(varray)						\
	((varray->version_no == LASTVERSIONARRAY)? 1 : 0)


#ifdef DOS
#define OnlyVersionlessP(varray) 0
#else
#define OnlyVersionlessP(varray)							\
	((varray->version_no == 0 && (varray + 1)->version_no == LASTVERSIONARRAY) ?	\
	 1 : 0)
#endif /* DOS */

/* An argument of AddDodNoExtention must be LispVersion convention */
/* Like "foo/fee.fee;3" or "/foo/foo;3" */
/* AddDodNoExtention must be put after UnixVersionToLispVersion */

#define	AddDodNoExtention(file){			\
		register char	*cp;			\
		register char	*cp1;			\
		if( (strrchr(file,'.')== 0) && ((cp=strrchr(file,';'))!=0) ){	\
			for(cp1=cp;*cp1!='\0';cp1++);	\
			*(cp1+1) = '\0';		\
			for(;cp!=cp1;cp1--)		\
				*cp1 = *(cp1-1);	\
			*cp = '.';			\
		}					\
		}

/* An argument of RemoveDodNoExtenstion must be LispVersion convention */
/* Like "foo/fee.fee;3" or "/foo/foo.;3" */
/* RemoveDodNoExtenstion must be put before LispVersionToUnixVersion */

#define RemoveDodNoExtenstion(file){					\
		register char	*cp;					\
		if( ((cp=strrchr(file, ';'))!=0) && (*(cp-1)=='.') ){	\
			for(;*cp!='\0';++cp)				\
				*(cp-1) = *cp;				\
			*(cp-1) = '\0';					\
		}							\
		}
		

extern	int	errno;

#define	ChangeToVersionless(pathname){			\
		register char	*cp;			\
		if( (cp=strrchr(pathname, ';')) != 0)	\
			*cp = '\0';			\
		}


#ifdef FSERROR
#define	UNLINK(x){					\
		TIMEOUT(rval=unlink(x));		\
		if(rval == -1){				\
			err_mess("unlink", errno);	\
			*Lisp_errno = errno;		\
			return(0);			\
		}					\
		}
#else
#define	UNLINK(x){					\
		TIMEOUT(rval=unlink(x));		\
		if(rval == -1){				\
			err_mess("unlink", errno);	\
			return(0);			\
		}					\
		}
#endif

#ifdef FSERROR
#define LINK(x,y){					\
		TIMEOUT(rval=link(x, y));		\
		if(rval == -1){				\
			if(errno == 2)			\
				return(1);		\
			else{				\
				err_mess("link", errno);\
				*Lisp_errno = errno;	\
				return(0);		\
			}				\
		}					\
		}
#else
#define LINK(x,y){					\
		TIMEOUT(rval=link(x, y));		\
		if(rval == -1){				\
			if(errno == 2)			\
				return(1);		\
			else{				\
				err_mess("link", errno);\
				return(0);		\
			}				\
		}					\
		}
#endif

#ifdef FSERROR
#define RENAME(x,y){					\
		TIMEOUT(rval=rename(x, y));		\
		if(rval == -1){				\
			switch(errno){			\
			case 2:				\
				return(1);		\
			case 18:			\
				*Lisp_errno = errno;	\
				return(0);		\
			default:			\
				err_mess("rename", errno);\
				*Lisp_errno = errno;	\
				return(0);		\
			}				\
		}					\
		}
#else
#define RENAME(x,y){					\
		TIMEOUT(rval=rename(x, y));		\
		if(rval == -1){				\
			switch(errno){			\
			case 2:				\
				return(1);		\
			default:			\
				err_mess("rename", errno);\
				return(0);		\
			}				\
		}					\
		}
#endif

#ifdef FSERROR
#define	STAT(x,y){					\
		TIMEOUT(rval=stat(x, y));		\
		if(rval != 0){				\
			err_mess("stat", errno);	\
			*Lisp_errno = errno;		\
			return(-1);			\
		}					\
		}
#else
#define	STAT(x,y){					\
		TIMEOUT(rval=stat(x, y));		\
		if(rval != 0){				\
			err_mess("stat", errno);	\
			return(-1);			\
		}					\
		}
#endif

/*
 * For file name length check
 */
#define FNAMETOOLONG	200

#define FileNameTooLong(val) {				\
	*Lisp_errno = FNAMETOOLONG;			\
	return((val));					\
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
#endif

