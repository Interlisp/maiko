#ifndef LISPVER2_H
#define LISPVER2_H 1
/* $Id: lispver2.h,v 1.2 1999/01/03 02:06:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* non-DOS version of LispVersionToUnixVersion */

#define LispVersionToUnixVersion(pathname) do {				\
									\
	char	*lv_cp;						\
	char	*lv_vp;						\
	unsigned lv_ver;					\
	char		lv_ver_buf[VERSIONLEN];				\
									\
	lv_cp = pathname;						\
	lv_vp = NULL;							\
	while (*lv_cp) {						\
		switch (*lv_cp) {					\
									\
		      case ';':						\
			lv_vp = lv_cp;					\
			lv_cp++;					\
			break;						\
									\
		      case '\'':					\
			if (*(lv_cp + 1) != 0) lv_cp += 2; 		\
			else lv_cp++;					\
			break;						\
									\
		      default:						\
			lv_cp++;					\
			break;						\
		}							\
	}								\
									\
	if (lv_vp != NULL) {						\
		/*							\
		 * A semicolon which is not quoted has been found.	\
		 */							\
		if (*(lv_vp + 1) == 0) {				\
			/*						\
			 * The empty version field.			\
			 * This is regarded as a versionless file.	\
			 */						\
			*lv_vp = 0;					\
		} else {						\
			NumericStringP((lv_vp + 1), YES, NO);		\
		      YES:						\
			/*						\
			 * Convert the remaining field to digit.	\
			 */						\
			lv_ver = strtoul(lv_vp + 1, (char **)NULL, 10); \
			if (lv_ver == 0) {				\
				/* versionless */			\
				*lv_vp = 0;				\
			} else {					\
				sprintf(lv_ver_buf, ".~%u~", lv_ver);	\
				*lv_vp = 0;				\
				strcat(pathname, lv_ver_buf);		\
			}						\
			goto CONT;					\
									\
		      NO:						\
			strcpy(lv_ver_buf, lv_vp + 1);			\
			strcat(lv_ver_buf, "~");			\
			*lv_vp++ = '.';					\
			*lv_vp++ = '~';					\
			*lv_vp = 0;					\
			strcat(pathname, lv_ver_buf);			\
		      CONT:						\
			lv_vp--;	/* Just for label */		\
		}							\
	}								\
} while (0)
#endif /* LISPVER2_H */
