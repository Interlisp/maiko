/* $Id: suntermios.h,v 1.2 1999/01/03 02:06:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*	@(#)termios.h 1.8 88/02/08 SMI	*/

#ifndef sun_TERMIOS_
#define sun_TERMIOS_


/* input modes */
#define	sunIGNBRK	0x00000001
#define	sunBRKINT	0x00000002
#define	sunIGNPAR	0x00000004
#define	sunPARMRK	0x00000008
#define	sunINPCK	0x00000010
#define	sunISTRIP	0x00000020
#define	sunINLCR	0x00000040
#define	sunIGNCR	0x00000080
#define	sunICRNL	0x00000100
#define	sunIUCLC	0x00000200
#define	sunIXON	0x00000400
#define	sunIXANY	0x00000800
#define	sunIXOFF	0x00001000
#define	sunIMAXBEL	0x00002000

/* output modes */
#define	sunOPOST	0x00000001
#define	sunOLCUC	0x00000002
#define	sunONLCR	0x00000004
#define	sunOCRNL	0x00000008
#define	sunONOCR	0x00000010
#define	sunONLRET	0x00000020
#define	sunOFILL	0x00000040
#define	sunOFDEL	0x00000080
#define	sunNLDLY	0x00000100
#define	sunNL0	0
#define	sunNL1	0x00000100
#define	sunCRDLY	0x00000600
#define	sunCR0	0
#define	sunCR1	0x00000200
#define	sunCR2	0x00000400
#define	sunCR3	0x00000600
#define	sunTABDLY	0x00001800
#define	sunTAB0	0
#define	sunTAB1	0x00000800
#define	sunTAB2	0x00001000
#define	sunXTABS	0x00001800
#define	sunTAB3	sunXTABS
#define	sunBSDLY	0x00002000
#define	sunBS0	0
#define	sunBS1	0x00002000
#define	sunVTDLY	0x00004000
#define	sunVT0	0
#define	sunVT1	0x00004000
#define	sunFFDLY	0x00008000
#define	sunFF0	0
#define	sunFF1	0x00008000
#define	sunPAGEOUT	0x00010000
#define	sunWRAP	0x00020000

/* control modes */
#define	sunCBAUD	0x0000000f
#define	sunCSIZE	0x00000030
#define	sunCS5	0
#define	sunCS6	0x00000010
#define	sunCS7	0x00000020
#define	sunCS8	0x00000030
#define	sunCSTOPB	0x00000040
#define	sunCREAD	0x00000080
#define	sunPARENB	0x00000100
#define	sunPARODD	0x00000200
#define	sunHUPCL	0x00000400
#define	sunCLOCAL	0x00000800
#define	sunLOBLK	0x00001000
#define	sunCIBAUD	0x000f0000
#define	sunCRTSCTS	0x80000000

#define	sunIBSHIFT	16

/* line discipline 0 modes */
#define	sunISIG	0x00000001
#define	sunICANON	0x00000002
#define	sunXCASE	0x00000004
#define	sunECHO	0x00000008
#define	sunECHOE	0x00000010
#define	sunECHOK	0x00000020
#define	sunECHONL	0x00000040
#define	sunNOFLSH	0x00000080
#define	sunTOSTOP	0x00000100
#define	sunECHOCTL	0x00000200
#define	sunECHOPRT	0x00000400
#define	sunECHOKE	0x00000800
#define	sunDEFECHO	0x00001000
#define	sunFLUSHO	0x00002000
#define	sunPENDIN	0x00004000		/* sigh */

/*
 * sunIoctl control packet
 */
struct suntermios {
	unsigned long	sunc_iflag;	/* input modes */
	unsigned long	sunc_oflag;	/* output modes */
	unsigned long	sunc_cflag;	/* control modes */
	unsigned long	sunc_lflag;	/* line discipline modes */
	char		sunc_line;		/* line discipline XXX */
	unsigned char	sunc_cc[037];	/* control chars */
};

/* codes 1 through 5 are old "termio" calls */
#define	sunTCXONC		_sunIO(T, 6)
#define	sunTCFLSH		_sunIO(T, 7)
#define	sunTCGETS		_sunIOR(T, 8, struct suntermios)
#define	sunTCSETS		_sunIOW(T, 9, struct suntermios)
#define	sunTCSETSW		_sunIOW(T, 10, struct suntermios)
#define	sunTCSETSF		_sunIOW(T, 11, struct suntermios)
#define	sunTCSNDBRK	_sunIO(T, 12)
#define	sunTCDRAIN		_sunIO(T, 13)

#include "sunttycom.h"

#endif
