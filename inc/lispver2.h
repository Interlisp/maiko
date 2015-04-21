/* $Id: lispver2.h,v 1.2 1999/01/03 02:06:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* non-DOS version of LispVersionToUnixVersion */

#define LispVersionToUnixVersion(pathname){				\
									\
	register char	*cp;						\
	register char	*vp;						\
	register int	ver;						\
	char		ver_buf[VERSIONLEN];				\
									\
	cp = pathname;							\
	vp = NULL;							\
	while (*cp) {							\
		switch (*cp) {						\
									\
		      case ';':						\
			vp = cp;					\
			cp++;						\
			break;						\
									\
		      case '\'':					\
			if (*(cp + 1) != 0) cp += 2; 		\
			else cp++;					\
			break;						\
									\
		      default:						\
			cp++;						\
			break;						\
		}							\
	}								\
									\
	if (vp != NULL) {						\
		/*							\
		 * A semicolon which is not quoted has been found.	\
		 */							\
		if (*(vp + 1) == 0) {				\
			/*						\
			 * The empty version field.			\
			 * This is regared as a versionless file.	\
			 */						\
			*vp = 0;					\
		} else {						\
			NumericStringP((vp + 1), YES, NO);		\
		      YES:						\
			/*						\
			 * Convert the remaining field to digit.	\
			 */						\
			ver = atoi(vp + 1);				\
			if (ver == 0) {					\
				/* versionless */			\
				*vp = 0;				\
			} else {					\
				sprintf(ver_buf, ".~%d~", ver);		\
				*vp = 0;				\
				strcat(pathname, ver_buf);		\
			}						\
			goto CONT;					\
									\
		      NO:						\
			strcpy(ver_buf, vp + 1);			\
			strcat(ver_buf, "~");				\
			*vp++ = '.';					\
			*vp++ = '~';					\
			*vp = 0;					\
			strcat(pathname, ver_buf);			\
		      CONT:						\
			vp--;	/* Just for label */			\
		}							\
	}								\
}
