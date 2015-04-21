/* $Id: ocr.h,v 1.2 1999/01/03 02:06:19 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */






/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


#ifdef OCR

extern int	OCR_sock;
extern int	OCR_sv;
extern int	OCR_fd;
extern int	OCR_procID;

/*
 * Device file for OCR
 */

#define OCRDEV	"/dev/ocr0"

/*
 * SUBR_OCR_COMM alpha byte
 */

#define DO_OPEN		0
#define DO_SCAN		1
#define DO_IMG_INFO	2
#define DO_IMG_UPLD	3
#define DO_SET_RPARA	4
#define DO_CLR_RPARA	5
#define DO_READ		6
#define DO_READ_INFO	7
#define DO_GET_RESULT	8
#define DO_ABORT	9
#define DO_CLOSE	10
#define DO_CODE_CONV	11
#define DO_TEST		12

/*
 * OCR state indicated in IL:\OCR.STATE.FLAGS
 */

#define OCR_ST_FAIL		1
#define OCR_ST_SCANNING		2
#define OCR_ST_UPLDING		3
#define OCR_ST_PROC_DEAD	4
#define OCR_ST_READING		5


/*
 * OCR SCAN parameter. In Lisp, BLOCKRECORD OCR.SCAN.PARAMS
 */

struct ocr_scan_para {
	u_char	size		: 3,
		direction	: 1,
		dencity		: 3,
		binary		: 1;
	u_char	resolution	: 3,
		adf		: 1,
		filter		: 1,
		threshold	: 3;
	u_char	noise		: 3,
		doc		: 1,
		smooth		: 1,
		compo		: 3;
};

#define OCR_SIZE_A4		0
#define OCR_SIZE_B4		1
#define OCR_SIZE_A5		2
#define OCR_SIZE_B5		3
#define OCR_SIZE_LG		4
#define OCR_SIZE_LT		5

#define OCR_DIRECT_VT		0
#define OCR_DIRECT_HR		1

#define OCR_RES_200		0
#define OCR_RES_240		1
#define OCR_RES_300		2
#define OCR_RES_400		3

#define OCR_DENC_0		0
#define OCR_DENC_1		1
#define OCR_DENC_2		2
#define OCR_DENC_3		3
#define OCR_DENC_4		4
#define OCR_DENC_5		5
#define OCR_DENC_6		6
#define OCR_DENC_7		7

#define OCR_SCANNER_FL		0
#define OCR_SCANNER_AD		1

#define OCR_FILTER_NR		0
#define OCR_FILTER_BP		1

#define OCR_THRES_0		0
#define OCR_THRES_1		1
#define OCR_THRES_2		2
#define OCR_THRES_3		3
#define OCR_THRES_4		4
#define OCR_THRES_5		5

#define OCR_NOISE_0		0
#define OCR_NOISE_2		1
#define OCR_NOISE_3		2
#define OCR_NOISE_4		3
#define OCR_NOISE_5		4

#define OCR_DOC_NR		0
#define OCR_DOC_NW		1

#define OCR_SMTH_CH		0
#define OCR_SMTH_IM		1

#define OCR_COMPO_LD		0
#define OCR_COMPO_PN		1
#define OCR_COMPO_PH		2
#define OCR_COMPO_PL		3

/*
 * OCR Image Info structure. In Lisp, BLOCKRECORD OCR.IMG.INFO
 * On SPARC, the size of this structure is 20.
 */

struct ocr_image_info {
	u_short xs;
	u_short	ys;
	u_short	xe;
	u_short	ye;
	u_short	line_width;
	u_int	size;
	u_char	resolution	: 3,
		compress	: 3,
				: 2;
};

/*
 * OCR Image Upload parameter. In Lisp, BLOCKRECORD OCR.IMG.UPLD.PARA
 * On SPARC, the size of this structure is 10.
 */

struct ocr_up_para {
	u_short xs;
	u_short	ys;
	u_short	xe;
	u_short	ye;
	u_char	resolution	: 3,
		compress	: 3,
				: 2;
};
	
#define OCR_COMP_NONE		0
#define OCR_COMP_2		1
#define OCR_COMP_4		2
#define OCR_COMP_8		3

/*
 * Maximum number of regions to read per page
 */

#define OCR_MAX_RD_PARAMS	200

/*
 * OCR Read Parameter. In Lisp, BLOCKRECORD OCR.IMG.READ.PARA
 * On SPARC, the size of this structure is 14.
 */

struct ocr_read_para {
	u_short xs;
	u_short	ys;
	u_short	xe;
	u_short	ye;
	u_char	format;
	u_char	csize;
	u_char	reject;
	u_char	cunit		: 2,
		deform		: 4,
				: 2;
	u_char	ck_num		: 1,
		ck_alph		: 1,
		ck_grk		: 1,
		ck_jvt		: 1,
		ck_jhr		: 1,
		ck_sym		: 1,
		ck_kana		: 1,
		ck_joyou	: 1;
	u_char	ck_jis1		: 1,
		ck_jmisc	: 1,
		ck_gaiji	: 1,
				: 5;
};

#define OCR_FMT_H1	0
#define OCR_FMT_H2	1
#define OCR_FMT_H3	2
#define OCR_FMT_H4	3
#define OCR_FMT_V1	4
#define OCR_FMT_V2	5
#define OCR_FMT_V3	6
#define OCR_FMT_V4	7

#define OCR_CUNIT_PO	0
#define OCR_CUNIT_KY	1
#define OCR_CUNIT_MM	2

#define OCR_REJ_0	0
#define OCR_REJ_1	1
#define OCR_REJ_2	2
#define OCR_REJ_3	3
#define OCR_REJ_4	4
#define OCR_REJ_5	5

#define	OCR_DFRM_1	0
#define	OCR_DFRM_2	1
#define	OCR_DFRM_3	2
#define	OCR_DFRM_4	3
#define	OCR_DFRM_NR	4
#define	OCR_DFRM_6	5
#define	OCR_DFRM_7	6
#define	OCR_DFRM_8	7
#define	OCR_DFRM_9	8

/*
 * Header for bulk data transfer
 */

struct bd_header {
	u_int	len;
	u_int	cont;
};

#define BD_LAST			0
#define BD_CONT			1

/*
 * The comm/res packet length used by OCR process
 */

#define PKTLEN			3
#define EPKTLEN			65536

#endif /* OCR */

