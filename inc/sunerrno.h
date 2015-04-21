/* $Id: sunerrno.h,v 1.2 1999/01/03 02:06:24 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)errno.h 2.11 88/02/08 SMI; from UCB 4.1 82/12/28	*/

/*
 * Error codes
 */

#define	sunEPERM	1		/* Not owner */
#define	sunENOENT	2		/* No such file or directory */
#define	sunESRCH	3		/* No such process */
#define	sunEINTR	4		/* Interrupted system call */
#define	sunEIO		5		/* I/O error */
#define	sunENXIO	6		/* No such device or address */
#define	sunE2BIG	7		/* Arg list too long */
#define	sunENOEXEC	8		/* Exec format error */
#define	sunEBADF	9		/* Bad file number */
#define	sunECHILD	10		/* No children */
#define	sunEAGAIN	11		/* No more processes */
#define	sunENOMEM	12		/* Not enough core */
#define	sunEACCES	13		/* Permission denied */
#define	sunEFAULT	14		/* Bad address */
#define	sunENOTBLK	15		/* Block device required */
#define	sunEBUSY	16		/* Mount device busy */
#define	sunEEXIST	17		/* File exists */
#define	sunEXDEV	18		/* Cross-device link */
#define	sunENODEV	19		/* No such device */
#define	sunENOTDIR	20		/* Not a directory*/
#define	sunEISDIR	21		/* Is a directory */
#define	sunEINVAL	22		/* Invalid argument */
#define	sunENFILE	23		/* File table overflow */
#define	sunEMFILE	24		/* Too many open files */
#define	sunENOTTY	25		/* Not a typewriter */
#define	sunETXTBSY	26		/* Text file busy */
#define	sunEFBIG	27		/* File too large */
#define	sunENOSPC	28		/* No space left on device */
#define	sunESPIPE	29		/* Illegal seek */
#define	sunEROFS	30		/* Read-only file system */
#define	sunEMLINK	31		/* Too many links */
#define	sunEPIPE	32		/* Broken pipe */

/* math software */
#define	sunEDOM		33		/* Argument too large */
#define	sunERANGE	34		/* Result too large */

/* non-blocking and interrupt i/o */
#define	sunEWOULDBLOCK	35		/* Operation would block */
#define	sunEINPROGRESS	36		/* Operation now in progress */
#define	sunEALREADY	37		/* Operation already in progress */
/* ipc/network software */

	/* argument errors */
#define	sunENOTSOCK	38		/* Socket operation on non-socket */
#define	sunEDESTADDRREQ	39		/* Destination address required */
#define	sunEMSGSIZE	40		/* Message too long */
#define	sunEPROTOTYPE	41		/* Protocol wrong type for socket */
#define	sunENOPROTOOPT	42		/* Protocol not available */
#define	sunEPROTONOSUPPORT	43	/* Protocol not supported */
#define	sunESOCKTNOSUPPORT	44	/* Socket type not supported */
#define	sunEOPNOTSUPP	45		/* Operation not supported on socket */
#define	sunEPFNOSUPPORT	46		/* Protocol family not supported */
#define	sunEAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define	sunEADDRINUSE	48		/* Address already in use */
#define	sunEADDRNOTAVAIL	49	/* Can't assign requested address */

	/* operational errors */
#define	sunENETDOWN	50		/* Network is down */
#define	sunENETUNREACH	51		/* Network is unreachable */
#define	sunENETRESET	52		/* Network dropped connection on reset */
#define	sunECONNABORTED	53		/* Software caused connection abort */
#define	sunECONNRESET	54		/* Connection reset by peer */
#define	sunENOBUFS	55		/* No buffer space available */
#define	sunEISCONN	56		/* Socket is already connected */
#define	sunENOTCONN	57		/* Socket is not connected */
#define	sunESHUTDOWN	58		/* Can't send after socket shutdown */
#define	sunETOOMANYREFS	59		/* Too many references: can't splice */
#define	sunETIMEDOUT	60		/* Connection timed out */
#define	sunECONNREFUSED	61		/* Connection refused */

	/* */
#define	sunELOOP	62		/* Too many levels of symbolic links */
#define	sunENAMETOOLONG	63		/* File name too long */

/* should be rearranged */
#define	sunEHOSTDOWN	64		/* Host is down */
#define	sunEHOSTUNREACH	65		/* No route to host */
#define	sunENOTEMPTY	66		/* Directory not empty */

/* quotas & mush */
#define	sunEPROCLIM	67		/* Too many processes */
#define	sunEUSERS	68		/* Too many users */
#define	sunEDQUOT	69		/* Disc quota exceeded */

/* Network File System */
#define	sunESTALE	70		/* Stale NFS file handle */
#define	sunEREMOTE	71		/* Too many levels of remote in path */

/* streams */
#define	sunENOSTR	72		/* Device is not a stream */
#define	sunETIME	73		/* Timer expired */
#define	sunENOSR	74		/* Out of streams resources */
#define	sunENOMSG	75		/* No message of desired type */
#define	sunEBADMSG	76		/* Trying to read unreadable message */

/* SystemV IPC */
#define sunEIDRM	77		/* Identifier removed */

/* SystemV Record Locking */
#define sunEDEADLK	78		/* Deadlock condition. */
#define sunENOLCK	79		/* No record locks available. */

/* RFS */
#define sunENONET	80		/* Machine is not on the network */
#define sunERREMOTE	81		/* Object is remote */
#define sunENOLINK	82		/* the link has been severed */
#define sunEADV		83		/* advertise error */
#define sunESRMNT	84		/* srmount error */
#define sunECOMM	85		/* Communication error on send */
#define sunEPROTO	86		/* Protocol error */
#define sunEMULTIHOP	87		/* multihop attempted */
#define sunEDOTDOT	88		/* Cross mount point (not an error) */
#define sunEREMCHG	89		/* Remote address changed */
