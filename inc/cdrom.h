/* $Id: cdrom.h,v 1.2 1999/01/03 02:05:54 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/***********************************

	file: cdrom_audio.h

***********************************/

/* function number for CD audio functions */
#define CD_OPEN		1
#define CD_CLOSE	2
#define CD_READ		3
#define CD_DISK_INFO	4
#define CD_TRACK_INFO	5
#define CD_START	6
#define CD_STOP		7
#define CD_PLAY		8
#define CD_Q_READ	9
#define CD_PAUSE	10
#define CD_RESUME	11
#define CD_VOLUME	12
#define CD_EJECT	13

/* function numbers for CD-ROM */
#define CDROM_INIT_DRV	1
#define CDROM_KEN_INT	2
#define CDROM_CHOSAKU	3
#define CDROM_MIKANA	4
#define CDROM_MIKANAT	5
#define CDROM_HYOKI	6
#define CDROM_HYOKIT	7
#define CDROM_EIJI	8
#define CDROM_EIJIT	9
#define CDROM_SUJI	10
#define CDROM_SUJIT	11
#define CDROM_ZENKANA	12
#define CDROM_ZENKANAT	13
#define CDROM_ZENKANJ	14
#define CDROM_ZENKANJT	15
#define CDROM_ZENEIJI	16
#define CDROM_ZENEIJIIT	17
#define CDROM_KOHKANA	18
#define CDROM_KOHKANAT	19
#define CDROM_KOHKANJ	20
#define CDROM_KOHKANJT	21
#define CDROM_KOHEIJI	22
#define CDROM_KOHEIJIT	23
#define CDROM_ZUHAN	24
#define CDROM_JYOKEN	25
#define CDROM_JYOKENT	26
#define CDROM_MENU	27
#define CDROM_AUDIOST	28
#define CDROM_AUDIOED	29
#define CDROM_AUDIOQ_GET	30
#define CDROM_GAIJI	31
#define CDROM_ZENGO	32
#define CDROM_KEIZOKU	33
#define CDROM_SYURYO	34



typedef unsigned char	BYTE;
typedef unsigned long	DWORD;

/*
extern DWORD	CDred_lbn();
extern void	CDlbn_red();
extern int	CDopen();
extern int	CDclose();
extern int	CDread();
extern int	CDdisk_info();
extern int	CDtrack_info();
extern int	CDstart();
extern int	CDstop();
extern int	CDplay();
extern int	CDqread();
extern int	CDpause();
extern int	CDresume();
extern int	CDvolume();
extern int	CDeject();
*/

#define	LispStringToCString(Lisp, C, MaxLen)				\
  {									\
    OneDArray	*arrayp;						\
    char	*base, *dp;						\
    short	*sbase;							\
    int	i, length;							\
    arrayp = (OneDArray *)(Addr68k_from_LADDR(Lisp));			\
    length = min(MaxLen, arrayp->totalsize);				\
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
		error("LispStringToCString can not handle\n");		\
      }									\
  }

#define	LispStringToCString2(Lisp, C, MaxLen)				\
  {									\
    OneDArray	*arrayp;						\
    char	*base, *dp;						\
    short	*sbase;							\
    int	i, length;							\
    arrayp = (OneDArray *)(Addr68k_from_LADDR(Lisp));			\
    length = min(MaxLen, arrayp->totalsize);				\
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
		for(i=0,dp=C;i<(length);i++) {				\
		  *dp++ = (char)(*sbase / 256 );			\
		  *dp++ = (char)(*sbase % 256 );			\
		  sbase++;						\
		}							\
		*dp = '\0';						\
		break;							\
	default:							\
		error("LispStringToCString can not handle\n");		\
      }									\
  }

#define min(a, b) ((a <= b)?a:b)
