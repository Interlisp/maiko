/* $Id: sunttold.h,v 1.2 1999/01/03 02:06:26 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)ttold.h 1.6 88/02/08 SMI; from S5R2 6.1	*/

#ifndef sun_TTOLD_
#define _sunTTOLD_

struct suntchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};

struct sunltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};

/*
 * Structure for TIOCGETP and TIOCSETP ioctls.
 */

#ifndef _sunSGTTYB_
#define _sunSGTTYB_
struct	sunsgttyb {
	char	sunsg_ispeed;		/* input speed */
	char	sunsg_ospeed;		/* output speed */
	char	sunsg_erase;		/* erase character */
	char	sunsg_kill;		/* kill character */
	short	sunsg_flags;		/* mode flags */
};
#endif

#include <sunioccom.h>

/*
 * 4.3BSD tty ioctl commands that are either:
 *  1) deprecated
 *  2) not implemented (and never were implemented)
 *  3) implemented on top of new-style "ioctl"s.
 */
#define	sunTIOCGETD	_sunIOR(t, 0, int)		/* get line discipline */
#define	sunTIOCSETD	_sunIOW(t, 1, int)		/* set line discipline */
#define	sunTIOCHPCL	_sunIO(t, 2)		/* hang up on last close */
#define	sunTIOCMODG	_sunIOR(t, 3, int)		/* get modem state - OBSOLETE */
#define	sunTIOCMODS	_sunIOW(t, 4, int)		/* set modem state - OBSOLETE */
#define	sunTIOCGETP	_sunIOR(t, 8,struct sunsgttyb)/* get parameters -- gtty */
#define	sunTIOCSETP	_sunIOW(t, 9,struct sunsgttyb)/* set parameters -- stty */
#define	sunTIOCSETN	_sunIOW(t,10,struct sunsgttyb)/* as above, but no flushtty */
#define	sunTIOCEXCL	_sunIO(t, 13)		/* set exclusive use of tty */
#define	sunTIOCNXCL	_sunIO(t, 14)		/* reset exclusive use of tty */
#define	sunTIOCFLUSH	_sunIOW(t, 16, int)	/* flush buffers */
#define	sunTIOCSETC	_sunIOW(t,17,struct suntchars)/* set special characters */
#define	sunTIOCGETC	_sunIOR(t,18,struct suntchars)/* get special characters */
#define		sunO_TANDEM	0x00000001	/* send stopc on out q full */
#define		sunO_CBREAK	0x00000002	/* half-cooked mode */
#define		sunO_LCASE	0x00000004	/* simulate lower case */
#define		sunO_ECHO	0x00000008	/* echo input */
#define		sunO_CRMOD	0x00000010	/* map \r to \r\n on output */
#define		sunO_RAW	0x00000020	/* no i/o processing */
#define		sunO_ODDP	0x00000040	/* get/send odd parity */
#define		sunO_EVENP	0x00000080	/* get/send even parity */
#define		sunO_ANYP	0x000000c0	/* get any parity/send none */
#define		sunO_NLDELAY	0x00000300	/* \n delay */
#define			sunO_NL0	0x00000000
#define			sunO_NL1	0x00000100	/* tty 37 */
#define			sunO_NL2	0x00000200	/* vt05 */
#define			sunO_NL3	0x00000300
#define		sunO_TBDELAY	0x00000c00	/* horizontal tab delay */
#define			sunO_TAB0	0x00000000
#define			sunO_TAB1	0x00000400	/* tty 37 */
#define			sunO_TAB2	0x00000800
#define		sunO_XTABS	0x00000c00	/* expand tabs on output */
#define		sunO_CRDELAY	0x00003000	/* \r delay */
#define			sunO_CR0	0x00000000
#define			sunO_CR1	0x00001000	/* tn 300 */
#define			sunO_CR2	0x00002000	/* tty 37 */
#define			sunO_CR3	0x00003000	/* concept 100 */
#define		sunO_VTDELAY	0x00004000	/* vertical tab delay */
#define			sunO_FF0	0x00000000
#define			sunO_FF1	0x00004000	/* tty 37 */
#define		sunO_BSDELAY	0x00008000	/* \b delay */
#define			sunO_BS0	0x00000000
#define			sunO_BS1	0x00008000
#define 	sunO_ALLDELAY	(sunO_NLDELAY|sunO_TBDELAY|sunO_CRDELAY|sunO_VTDELAY|sunO_BSDELAY)
#define		sunO_CRTBS	0x00010000	/* do backspacing for crt */
#define		sunO_PRTERA	0x00020000	/* \ ... / erase */
#define		sunO_CRTERA	0x00040000	/* " \b " to wipe out char */
#define		sunO_TILDE	0x00080000	/* hazeltine tilde kludge */
#define		sunO_MDMBUF	0x00100000	/* start/stop output on carrier intr */
#define		sunO_LITOUT	0x00200000	/* literal output */
#define		sunO_TOSTOP	0x00400000	/* SIGSTOP on background output */
#define		sunO_FLUSHO	0x00800000	/* flush output to terminal */
#define		sunO_NOHANG	0x01000000	/* no SIGHUP on carrier drop */
#define		sunO_L001000	0x02000000
#define		sunO_CRTKIL	0x04000000	/* kill line with " \b " */
#define		sunO_PASS8	0x08000000
#define		sunO_CTLECH	0x10000000	/* echo control chars as ^X */
#define		sunO_PENDIN	0x20000000	/* tp->t_rawq needs reread */
#define		sunO_DECCTQ	0x40000000	/* only ^Q starts after ^S */
#define		sunO_NOFLSH	0x80000000	/* no output flush on signal */
/* locals, from 127 down */
#define	sunTIOCLBIS	_sunIOW(t, 127, int)	/* bis local mode bits */
#define	sunTIOCLBIC	_sunIOW(t, 126, int)	/* bic local mode bits */
#define	sunTIOCLSET	_sunIOW(t, 125, int)	/* set entire local mode word */
#define	sunTIOCLGET	_sunIOR(t, 124, int)	/* get local modes */
#define		sunLCRTBS	(sunO_CRTBS>>16)
#define		sunLPRTERA	(sunO_PRTERA>>16)
#define		sunLCRTERA	(sunO_CRTERA>>16)
#define		sunLTILDE	(sunO_TILDE>>16)
#define		sunLMDMBUF	(sunO_MDMBUF>>16)
#define		sunLLITOUT	(sunO_LITOUT>>16)
#define		sunLTOSTOP	(sunO_TOSTOP>>16)
#define		sunLFLUSHO	(sunO_FLUSHO>>16)
#define		sunLNOHANG	(sunO_NOHANG>>16)
#define		sunLCRTKIL	(sunO_CRTKIL>>16)
#define		sunLPASS8	(sunO_PASS8>>16)
#define		sunLCTLECH	(sunO_CTLECH>>16)
#define		sunLPENDIN	(sunO_PENDIN>>16)
#define		sunLDECCTQ	(sunO_DECCTQ>>16)
#define		sunLNOFLSH	(sunO_NOFLSH>>16)
#define	sunTIOCSBRK	_sunIO(t, 123)		/* set break bit */
#define	sunTIOCCBRK	_sunIO(t, 122)		/* clear break bit */
#define	sunTIOCSDTR	_sunIO(t, 121)		/* set data terminal ready */
#define	sunTIOCCDTR	_sunIO(t, 120)		/* clear data terminal ready */
#define	sunTIOCSLTC	_sunIOW(t,117,struct sunltchars)/* set local special chars */
#define	sunTIOCGLTC	_sunIOR(t,116,struct sunltchars)/* get local special chars */
#define	sunTIOCSTOP	_sunIO(t, 111)		/* stop output, like ^S */
#define	sunTIOCSTART	_sunIO(t, 110)		/* start output, like ^Q */

/*
 * Sun-specific ioctls, which have been moved to the Sun-specific range.
 * The old codes will be kept around for binary compatibility; the
 * codes for TIOCCONS and TIOCGSIZE don't collide with the 4.3BSD codes
 * because the structure size and copy direction fields are different.
 * Unfortunately, the old TIOCSSIZE code does collide with TIOCSWINSZ,
 * but they can be disambiguated by checking whether a "struct ttysize"
 * structure's "ts_lines" field is greater than 64K or not.  If so,
 * it's almost certainly a "struct winsize" instead.
 */
#define	sun_O_TIOCCONS	_sunIO(t, 104)		/* get console I/O */
#define	sun_O_TIOCSSIZE	_sunIOW(t,103,struct ttysize)/* get tty size */
#define	sun_O_TIOCGSIZE	_sunIOR(t,102,struct ttysize)/* get tty size */

/*
 * Sun-specific ioctls.
 */
#define	sunTIOCSETX	_sunIOW(t, 34, int)	/* set extra modes for S5 compatibility */
#define	sunTIOCGETX	_sunIOR(t, 35, int)	/* get extra modes for S5 compatibility */
#define		sunNOPOST		0x00000001	/* no processing on output (LITOUT with 7 bits + parity) */
#define		sunNOISIG		0x00000002	/* disable all signal-generating characters */
#define		sunSTOPB		0x00000004	/* two stop bits */

#define	sunOTTYDISC	0		/* old, v7 std tty driver */
#define	sunNETLDISC	1		/* line discip for berk net */
#define	sunNTTYDISC	2		/* new tty discipline */
#define	sunTABLDISC	3		/* hitachi tablet discipline */
#define	sunNTABLDISC	4		/* gtco tablet discipline */
#define	sunMOUSELDISC	5		/* mouse discipline */
#define	sunKBDLDISC	6		/* up/down keyboard trans (console) */

#include <sunttycom.h>

#endif
