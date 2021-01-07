#ifndef NFSFH_H
#define NFSFH_H 1
/* $Id: nfsfh.h,v 1.2 1999/01/03 02:06:18 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
 *
 * nfsfh.h - NFS file handle definitions (for portable use)
 *
 * Jeffrey C. Mogul
 * Digital Equipment Corporation
 * Western Research Laboratory
 *
 * $Log: nfsfh.h,v $
 * Revision 1.2  1999/01/03 02:06:18  sybalsky
 * Add ID comments / static to files for CVS use
 *
 * Revision 1.1.1.1  1998/12/17 05:03:18  sybalsky
 * Import of Medley 3.5 emulator
 *
 * Revision 1.1  93/10/01  16:09:01  mogul
 * Initial revision
 * 
 */

/*
 * Internal representation of dev_t, because different NFS servers
 * that we might be spying upon use different external representations.
 */
typedef struct {
	u_long	Minor;	/* upper case to avoid clashing with macro names */
	u_long	Major;
} my_devt;

#define	dev_eq(a,b)	((a.Minor == b.Minor) && (a.Major == b.Major))

/*
 * Many file servers now use a large file system ID.  This is
 * our internal representation of that.
 */
typedef	struct {
	my_devt	fsid_dev;
	u_long	fsid_code;
} my_fsid;

#define	fsid_eq(a,b)	((a.fsid_code == b.fsid_code) &&\
			 dev_eq(a.fsid_dev, b.fsid_dev))
#endif /* NFSFH_H */
