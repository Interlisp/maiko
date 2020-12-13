/* $Id: nfswatch.h,v 1.2 1999/01/03 02:06:18 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/*
 * $Header: /disk/disk3/cvsroot/medley/inc/nfswatch.h,v 1.2 1999/01/03 02:06:18 sybalsky Exp $
 *
 * nfswatch.h - definitions for nfswatch.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * $Log: nfswatch.h,v $
 * Revision 1.2  1999/01/03 02:06:18  sybalsky
 * Add ID comments / static to files for CVS use
 *
 * Revision 1.1.1.1  1998/12/17 05:03:18  sybalsky
 * Import of Medley 3.5 emulator
 *
 * Revision 4.8  1996/02/12 13:22:29  davy
 * Updated version number.
 *
 * Revision 4.7  1995/03/20 01:07:34  davy
 * Upped version number.
 *
 * Revision 4.6  1995/03/20  01:05:20  davy
 * Fixed int32 declaration.
 *
 * Revision 4.5  1993/11/30  21:55:38  davy
 * Upgraded version number.
 *
 * Revision 4.4  1993/10/01  14:56:51  mogul
 * Bugfix to compile on SunOS
 *
 * Revision 4.3  93/10/01  10:45:54  mogul
 * Revert to int32, u_int32 names
 * 
 * Revision 4.2  93/09/30  20:33:44  davy
 * Increased MAXCLIENTS, MAXEXPORTS, MAXAUTHS.
 * Fixed the int32 and u_int32 type names for portability.
 * 
 * Revision 4.1  1993/09/28  21:27:40  mogul
 * Portable internal representation for file handle info
 * explicit 32-bit data types
 * explicit data type for IP addresses
 *
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.9  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.8  1993/01/16  19:12:54  davy
 * Moved cpp controls to left margin.
 *
 * Revision 3.7  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.6  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.5  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.4  1993/01/13  21:24:40  davy
 * Portability change.
 *
 * Revision 3.3  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.2  1992/07/24  18:49:09  mogul
 * Changed version number to 3.1
 *
 * Revision 3.1  1992/07/24  18:47:57  mogul
 * Added FDDI support
 *
 * Revision 3.0  1991/01/23  08:23:13  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.4  91/01/17  10:12:29  davy
 * New features from Jeff Mogul.
 * 
 * Revision 1.6  91/01/07  15:34:42  mogul
 * Support for client hash table
 * 
 * Revision 1.5  91/01/07  14:10:01  mogul
 * Added SHOWHELP, SHOW_MAXCODE
 * 
 * Revision 1.4  91/01/04  14:12:11  mogul
 * Support for client counters
 * 
 * Revision 1.3  91/01/03  17:38:18  mogul
 * Support for per-procedure counters
 * 
 * Revision 1.2  90/08/17  15:47:04  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:28  davy
 * NFSWATCH Release 1.0
 * 
 */

/*
 * Version number.
 */
#define VERSION		"4.3 of 12 February 1996"

/*
 * Stuff for representing NFS file handles
 */
#include "nfsfh.h"

/*
 * General definitions.
 */
#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

/*
 * Declarations of several data types that must be 32 bits wide,
 * no matter what machine we are running on.  "long" is unsafe
 * because on DEC Alpha machines that means 64 bits.  "int" is
 * unsafe because on some machines that means 16 bits.
 *
 * Use int32 or u_int32 whenever you mean "32 bits" and not
 * "some large number of bits".
 *
 * NEVER use int or int32 or u_int32 (or, for that matter, long)
 * when the variable might contain a pointer value.
 */
#if  defined(pdp11)
/* other 16-bit machines? */
typedef	long int32;
typedef	unsigned long u_int32;
#else
/* works almost everywhere */
#if	!defined(SUNOS54)
	/* SunOS 5.4 declares int32 in <rpc/rpc_sztypes.h> */
typedef	int int32;
#endif
#if	!defined(U_INT32_DECLARED_IN_AUTH) || !defined(AUTH_UNIX)
	/* SunOS declares u_int32 in <rpc/auth.h> */
typedef	unsigned int u_int32;
#endif
#endif

/* Define a specific type for representing IP addresses */
typedef	u_int32	ipaddrt;

#ifdef SVR4
#define MOUNTTABLE	"/etc/mnttab"		/* mounted file systems */
#define SHARETAB	"/etc/dfs/sharetab"	/* shared file systems	*/
#endif

#define PROMPT		"nfswatch>"	/* prompt string		*/
#define LOGFILE		"nfswatch.log"	/* log file name		*/
#define MAXEXPORT	1024		/* max exported file systems	*/
#define CYCLETIME	10		/* screen update cycle time	*/
#define PACKETSIZE	4096		/* max size of a packet		*/
#define MAXNFSPROC	18		/* max number of NFS procedures	*/
#define MAXHOSTADDR	16		/* max. network addrs per host	*/
#define	MAXCLIENTS	1024		/* max. # of client counters	*/
					/* MUST be even number 		*/
#define MAXAUTHS	1024		/* max. # of auth counters	*/
					/* MUST be even number		*/
#define MAXINTERFACES	16		/* Max. number of interfaces	*/
#define SNAPSHOTFILE	"nfswatch.snap"	/* snapshot file name		*/

#define SHOWINDVFILES	1		/* show individual files	*/
#define SHOWFILESYSTEM	2		/* show NFS file systems	*/
#define SHOWNFSPROCS	3		/* show NFS procedure counts	*/
#define SHOWCLIENTS	4		/* show client host names	*/
#define SHOWAUTH	5		/* show authorizations		*/
#define	SHOWHELP	6		/* show help text		*/
#define	SHOW_MAXCODE	6		/* number of different displays */

/*
 * Network Interface Tap (NIT) definitions.
 */
#ifdef USE_NIT
#define NIT_DEV	"/dev/nit"		/* network interface tap device	*/
#define NIT_BUF	"nbuf"			/* nit stream buffering module	*/
#define NIT_CHUNKSIZE	8192		/* chunk size for grabbing pkts	*/
#endif

/*
 * Data Link Provider Interface (DLPI) definitions.
 */
#ifdef USE_DLPI
#define DLPI_DEVDIR		"/dev/"		/* path to device dir	*/
#define DLPI_BUFMOD		"bufmod"	/* streams buffering	*/
#define DLPI_MAXWAIT		15		/* max timeout		*/
#define DLPI_MAXDLBUF		8192		/* buffer size		*/
#define DLPI_MAXDLADDR		1024		/* max address size	*/
#define DLPI_CHUNKSIZE		(8192 * sizeof(long))	/* buffer size	*/
#define DLPI_DEFAULTSAP	0x0800			/* IP protocol		*/
#endif /* USE_DLPI */

/*
 * Packet counter definitions.
 */
#define PKT_NCOUNTERS	16		/* number of packet counters	*/

#define PKT_NDREAD	0		/* ND read requests		*/
#define PKT_NDWRITE	1		/* ND write requests		*/
#define PKT_NFSREAD	2		/* NFS read requests		*/
#define PKT_NFSWRITE	3		/* NFS write requests		*/
#define PKT_NFSMOUNT	4		/* NFS mount requests		*/
#define PKT_YELLOWPAGES	5		/* Yellow Pages requests	*/
#define PKT_RPCAUTH	6		/* RPC authorization requests	*/
#define PKT_OTHERRPC	7		/* other RPC requests		*/
#define PKT_TCP		8		/* TCP packets			*/
#define PKT_UDP		9		/* UDP packets			*/
#define PKT_ICMP	10		/* ICMP packets			*/
#define PKT_ROUTING	11		/* routing control packets	*/
#define PKT_ARP		12		/* address resolution packets	*/
#define PKT_RARP	13		/* reverse addr resol packets	*/
#define PKT_BROADCAST	14		/* ethernet broadcast packets	*/
#define PKT_OTHER	15		/* none of the above packets	*/

typedef unsigned long	Counter;

/*
 * Packet counting structure.
 */
typedef struct {
	char	*pc_name;		/* name of counter		*/

	Counter	pc_interval;		/* packets this interval	*/
	Counter	pc_total;		/* packets since start		*/

	short	pc_intx, pc_inty;	/* screen coords of pc_interval	*/
	short	pc_totx, pc_toty;	/* screen coords of pc_total	*/
	short	pc_pctx, pc_pcty;	/* screen coords of percentage	*/
	short	pc_namex, pc_namey;	/* screen coords of pc_name	*/
} PacketCounter;

/*
 * NFS request counting structure.
 */
typedef struct {
	my_devt	nc_dev;			/* device numbers of file sys	*/
	my_fsid	nc_fsid;		/* for "learning" file systems	*/
	ipaddrt	nc_ipaddr;		/* keep track of server address	*/
	char	*nc_name;		/* name of file system		*/

	Counter	nc_total;		/* requests since start		*/
	Counter	nc_interval;		/* requests this interval	*/
	Counter nc_proc[MAXNFSPROC];	/* each nfs proc counters	*/

	short	nc_intx, nc_inty;	/* screen coords of nc_interval	*/
	short	nc_totx, nc_toty;	/* screen coords of nc_total	*/
	short	nc_pctx, nc_pcty;	/* screen coords of percentage	*/
	short	nc_namex, nc_namey;	/* screen coords of nc_name	*/
} NFSCounter;

/*
 * Specific file request counting structure.
 */
typedef struct {
	my_devt	fc_dev;			/* device number of file sys	*/
	ino_t	fc_ino;			/* inode number of file		*/
	char	*fc_name;		/* file name			*/

	Counter	fc_total;		/* requests since start		*/
	Counter	fc_interval;		/* requests this interval	*/
	Counter	fc_proc[MAXNFSPROC];	/* each nfs proc counters	*/

	short	fc_intx, fc_inty;	/* screen coords of fc_interval	*/
	short	fc_totx, fc_toty;	/* screen coords of fc_total	*/
	short	fc_pctx, fc_pcty;	/* screen coords of percentage	*/
	short	fc_namex, fc_namey;	/* screen coords of fc_name	*/
} FileCounter;

/*
 * Per-procedure counting structure.
 */
typedef struct {
	int	pr_type;		/* procedure type		*/
	char 	*pr_name;		/* procedure name		*/
	Counter	pr_total;		/* requests since start		*/
	Counter	pr_interval;		/* requests this interval	*/

	Counter pr_complete;		/* requests with replies	*/
	double	pr_response;		/* sum of all response times	*/
	double	pr_respsqr;		/* sum of squares of resp times	*/
	double	pr_maxresp;		/* maximum response time	*/

	short	pr_intx, pr_inty;	/* screen coords of pr_interval	*/
	short	pr_totx, pr_toty;	/* screen coords of pr_total	*/
	short	pr_pctx, pr_pcty;	/* screen coords of percentage	*/
	short	pr_namex, pr_namey;	/* screen coords of pr_name	*/

	short	pr_compx, pr_compy;	/* screen coords of pr_complete	*/
	short	pr_respx, pr_respy;	/* screen coords of pr_response	*/
	short	pr_rsqrx, pr_rsqry;	/* screen coords of pr_respsqr	*/
	short	pr_rmaxx, pr_rmaxy;	/* screen coords of pr_maxresp	*/
} ProcCounter;

/*
 * NFS client counting structure.
 */
typedef struct _cl_ {
	ipaddrt	cl_ipaddr;		/* client IP address		*/
	char	*cl_name;		/* name of client system	*/

	Counter	cl_total;		/* requests since start		*/
	Counter	cl_interval;		/* requests this interval	*/

	short	cl_intx, cl_inty;	/* screen coords of cl_interval	*/
	short	cl_totx, cl_toty;	/* screen coords of cl_total	*/
	short	cl_pctx, cl_pcty;	/* screen coords of percentage	*/
	short	cl_namex, cl_namey;	/* screen coords of cl_name	*/
	
	struct	_cl_ *cl_next;		/* hash chain link		*/
} ClientCounter;

/*
 * NFS authentication counting structure.
 */
typedef struct _ac_ {
	long	ac_uid;			/* authorization type		*/
	char	*ac_name;		/* name of user id		*/

	Counter	ac_total;		/* requests since start		*/
	Counter ac_interval;		/* requests this interval	*/

	short	ac_intx, ac_inty;	/* screen coords of ac_interval	*/
	short	ac_totx, ac_toty;	/* screen coords of ac_total	*/
	short	ac_pctx, ac_pcty;	/* screen coords of percentage	*/
	short	ac_namex, ac_namey;	/* screen coords of ac_name	*/

	struct	_ac_	*ac_next;	/* hash chain link		*/
} AuthCounter;

/*
 * NFS call structure, for remembering relevant timing information.
 */
#define NFSCALLHASHSIZE		255

typedef struct _nc_ {
	int		used;
	ipaddrt		client;
	u_short		clientport;
	u_int32	xid;
	u_long		proc;
	u_long		time_sec;
	u_long		time_usec;
} NFSCall;

/*
 * Device type definitions (borrowed from the Berkeley Packet Filter)
 */
#ifndef	DLT_NULL
#define DLT_NULL	0	/* no link-layer encapsulation */
#define DLT_EN10MB	1	/* Ethernet (10Mb) */
#define DLT_EN3MB	2	/* Experimental Ethernet (3Mb) */
#define DLT_AX25	3	/* Amateur Radio AX.25 */
#define DLT_PRONET	4	/* Proteon ProNET Token Ring */
#define DLT_CHAOS	5	/* Chaos */
#define DLT_IEEE802	6	/* IEEE 802 Networks */
#define DLT_ARCNET	7	/* ARCNET */
#define DLT_SLIP	8	/* Serial Line IP */
#define DLT_PPP	9	/* Point-to-point Protocol */
#define DLT_FDDI	10	/* FDDI */
#endif

/*
 * Definitions for earlier systems which don't have these from 4.3BSD.
 */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif

#ifndef NFDBITS
typedef long		fd_mask;

#define NFDBITS	(sizeof(fd_mask) * NBBY)

#define FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	(void) bzero((char *)(p), sizeof(*(p)))
#endif /* NFDBITS */




/*
 * Maximum control/data buffer size (in long's !!) for getmsg().
 */
#define		MAXDLBUF	8192

/*
 * Maximum number of seconds we'll wait for any
 * particular DLPI acknowledgment from the provider
 * after issuing a request.
 */
#define		MAXWAIT		15

/*
 * Maximum address buffer length.
 */
#define		MAXDLADDR	1024


/*
 * Handy macro.
 */
#define		OFFADDR(s, n)	(u_char*)((char*)(s) + (int)(n))

