/* $Id: ocrproc.c,v 1.2 1999/01/03 02:07:27 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: ocrproc.c,v 1.2 1999/01/03 02:07:27 sybalsky Exp $ Copyright (C) Venue";



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


#ifdef OCR

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <scsi/targets/ocrio.h>
#include <stdio.h>
#include "ocr.h"

/*
 * Cache parameter passed from Subr to postpone time consuming work
 */

static struct ocr_scan_para	OCR_scan_para;
static struct ocr_up_para	OCR_up_para;

/*
 * Image size cache
 */

static int OCR_image_size = 0;

/*
 * Number of regions to be read
 */

static int OCR_read_regions = 0;

/*
 * Socket fd to communicate the primary emulator process
 */

int	OCR_sv = -1;

/*
 * Fd for the OCR device file
 */

int	OCR_fd = -1;

/*
 * Local definitions for clarify code
 */

#ifndef	MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef	MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

/*
 * Local functions
 */

static int ocr_handle_comm(), ocr_init_sv();
static int ocr_proc_scan(), ocr_proc_iminfo(), ocr_proc_set_rpara();
static int ocr_proc_clr_rpara(), ocr_proc_read(), ocr_conv_read_data();
static void ocr_do_postponed_work(), ocr_proc_img_upload();
static void ocr_proc_exit(), ocr_init_signal(), notify_ready();
static void ocr_proc_post_read();

/*
 * OCR process
 *
 * Keep waiting 3 byte length command packet from the primary emulator process.
 * The first byte of the packet indicates the work requested.  The second and
 * third byte indicate the extra packet length.  The contents of the extra packet
 * is command dependent.
 *
 * After finishing the work, sends back the another 3 byte length packet.  The
 * first byte of the returned packet indicates how the work has been done.  1 is
 * success, and 0 is failure.
 *
 * If the work requested will take a long time, the packet with state 1 is just
 * returned and the actual work will be postponed, After finishing the postponed
 * work, another 3 byte length packet which indicates the result of the work
 * is sent back to the primary emulator process, and SIGUSR1 is signalled to
 * notify the completion of the initial request.
 */

void
ocr_proc(ppid)
pid_t	ppid;
{
	int		len, postponed = 0;
	u_char		pkt[PKTLEN];
	static u_char	epkt[EPKTLEN];	/* Maximum size of extra packet */

	if (!ocr_init_sv()) return;
	ocr_init_signal();

	while (1) {
		
		if (postponed) {
			ocr_do_postponed_work(postponed, ppid);
			postponed = 0;
		} else {
			len = read(OCR_sv, pkt, sizeof(pkt));
			if (len == 0) {
				/*
				 * Broken socket will return size 0.
				 * Assumes the parent dies
				 */
				ocr_proc_exit();
			} else if (len != sizeof(pkt)) {
				pkt[0] = 0;
				(void)write(OCR_sv, pkt, sizeof(pkt));
			} else {
				if (pkt[1] == 0 && pkt[2] == 0) {
					ocr_handle_comm(pkt, NULL, &postponed);
				} else {
					int	elen;

					elen = pkt[1] << 8 | pkt[2];
					len = read(OCR_sv, epkt, elen);
					if (len != elen) {
						pkt[0] = 0;
						(void)write(OCR_sv, pkt, sizeof(pkt));
					} else {
						ocr_handle_comm(pkt, epkt, &postponed);
					}
				}
			}
		}
	}
}

/*
 * Try to resett the device then exit.
 */

static void
ocr_proc_exit()
{
	if (OCR_fd > 0) {
		ioctl(OCR_fd, OCR_ABORT);
		close(OCR_fd);
		OCR_fd = -1;
	}
	exit(1);
}

/*
 * Handle the work requested from the primary emulator process
 */

static int
ocr_handle_comm(pkt, epkt, reasonp)
u_char	pkt[], epkt[];
int	*reasonp;
{
	int	len;
	
	*reasonp = 0;
	switch (pkt[0]) {
	      case 'O':			/* Open */
		if (OCR_fd > 0) {
			pkt[0] = 0;
		} else {
			OCR_fd = open(OCRDEV, O_RDWR);
			if (OCR_fd < 0) {
				perror("ocr_proc : OPEN");
				pkt[0] = 0;
			} else {
				pkt[0] = 1;
			}
		}
		break;
			
	      case 'S':			/* Start scanning */
		{
			if ((pkt[1] << 8 | pkt[2]) ==
			    sizeof(struct ocr_scan_para)) {
				bcopy(epkt, (char *)&OCR_scan_para,
				      sizeof(struct ocr_scan_para));
				*reasonp = OCR_ST_SCANNING;
				pkt[0] = 1;
			} else {
				pkt[0] = 0;
			}
		}
		break;

	      case 'I':			/* Start getting image info */
		{
			if ((pkt[1] << 8 | pkt[2]) ==
			    sizeof(struct ocr_up_para)) {
				bcopy(epkt, (char *)&OCR_up_para,
				      sizeof(struct ocr_up_para));
				*reasonp = OCR_ST_UPLDING;
				pkt[0] = 1;
			} else {
				pkt[0] = 0;
			}
		}
		break;

	      case 'P':			/* Set read params */
		{
			int	len;

			len = pkt[1] << 8 | pkt[2];
			if (len % sizeof(struct ocr_read_para)) {
				pkt[0] = 0;
			} else {
				len = len / sizeof(struct ocr_read_para);
				if (len + OCR_read_regions > OCR_MAX_RD_PARAMS) {
					pkt[0] = 0;
				} else if (ocr_proc_set_rpara((struct ocr_read_para *)
							      epkt, len)) {
					pkt[0] = 1;
				} else {
					pkt[0] = 0;
				}
			}
		}
		break;

	      case 'C':			/* Clear read params */
		{
			if (ocr_proc_clr_rpara()) {
				pkt[0] = 1;
			} else {
				pkt[0] = 0;
			}
		}
		break;
		
	      case 'R':			/* Start reading */
		{
			if (OCR_read_regions < 0) {
				pkt[0] = 0;
			} else {
				*reasonp = OCR_ST_READING;
				pkt[0] = 1;
			}
		}
		break;

	      case 'A':
		if (OCR_fd > 0) ioctl(OCR_fd, OCR_ABORT);
		exit(1);

	      default:
		return 0;
	}
	if (write(OCR_sv, pkt, PKTLEN) < 0)
	  return 0;
	else
	  return 1;
}		

/*
 * Handle postponed time consuming work.
 */

static void
ocr_do_postponed_work(reason, ppid)
int	reason;
pid_t	ppid;
{
	u_char	pkt[PKTLEN];

	switch (reason) {
	      case OCR_ST_SCANNING:
		if (ocr_proc_scan(&OCR_scan_para)) {
			pkt[0] = 1;
		} else {
			pkt[0] = 0;
		}
		write(OCR_sv, pkt, sizeof(pkt));
		notify_ready(ppid);
		break;

	      case OCR_ST_UPLDING:
		{
			struct ocr_image_info	iminfo;
			
			if (!ocr_proc_iminfo(&OCR_up_para, &iminfo)) {
				pkt[0] = 0;
				write(OCR_sv, pkt, sizeof(pkt));
				notify_ready(ppid);
			} else {
				ocr_proc_img_upload(&iminfo, ppid);
			}
		}
		break;

	      case OCR_ST_READING:
		if (ocr_proc_read()) {
			ocr_proc_post_read(ppid);
		} else {
			pkt[0] = 0;
			write(OCR_sv, pkt, sizeof(pkt));
			notify_ready(ppid);
		}
		break;

	      default:
		pkt[0] = 0;
		write(OCR_sv, pkt, sizeof(pkt));
		notify_ready(ppid);
		break;
	}
	return;
}

/*
 * Notify primary emulator process that time consuming work has been done.
 */

static void
notify_ready(ppid)
pid_t	ppid;
{
	kill(ppid, SIGUSR1);
}
				
/*
 * Put the socket descriptor into blocking mode, and extend
 * the buffer size.
 */

static int
ocr_init_sv()
{

	int		flags;
	
	if (OCR_sv < 0) return 0;

	flags = fcntl(OCR_sv, F_GETFL, 0);
	if (flags < 0) {
		perror("ocr_init_sv: fcntl");
		return 0;
	}
	flags &= ~FNDELAY;
	if (fcntl(OCR_sv, F_SETFL, flags) < 0) {
		perror("ocr_init_sv: fcntl 2");
		return 0;
	}
	return 1;	
}	

/*
 * Make sure the fatal signall initialize the device then exit
 */

static void
ocr_init_signal()
{
	struct sigvec	sv;

	sv.sv_flags = sv.sv_mask = 0;
	sv.sv_handler = ocr_proc_exit;

	sigvec(SIGHUP,  &sv, NULL);
	sigvec(SIGINT,  &sv, NULL);
	sigvec(SIGQUIT, &sv, NULL);
	sigvec(SIGILL,  &sv, NULL);
	sigvec(SIGTRAP, &sv, NULL);
	sigvec(SIGABRT, &sv, NULL);
	sigvec(SIGEMT,  &sv, NULL);
	sigvec(SIGBUS,  &sv, NULL);
	sigvec(SIGSEGV, &sv, NULL);
	sigvec(SIGSYS,  &sv, NULL);
	sigvec(SIGPIPE, &sv, NULL);
	sigvec(SIGTERM, &sv, NULL);
	sigvec(SIGLOST, &sv, NULL);
	sigvec(SIGUSR1, &sv, NULL);
	sigvec(SIGUSR2, &sv, NULL);
	return;
}

/*
 * Scan image
 */

static int
ocr_proc_scan(spp)
struct ocr_scan_para	*spp;
{
	struct scan_params	sp;

	switch (spp->size) {
	      case OCR_SIZE_A4:
		sp.size = SIZE_A4;
		break;
	      case OCR_SIZE_B4:
		sp.size = SIZE_B4;
		break;
	      case OCR_SIZE_A5:
		sp.size = SIZE_A5;
		break;
	      case OCR_SIZE_B5:
		sp.size = SIZE_B5;
		break;
	      case OCR_SIZE_LG:
		sp.size = SIZE_LG;
		break;
	      case OCR_SIZE_LT:
		sp.size = SIZE_LT;
		break;
	      default:
		return 0;
	}
	switch (spp->direction) {
	      case OCR_DIRECT_VT:
		sp.direction = DIRECT_VT;
		break;
	      case OCR_DIRECT_HR:
		sp.direction = DIRECT_HR;
		break;
	      default:
		return 0;
	}
	switch (spp->resolution) {
	      case OCR_RES_200:
		sp.resolution = RES_200;
		break;
	      case OCR_RES_240:
		sp.resolution = RES_240;
		break;
	      case OCR_RES_300:
		sp.resolution = RES_300;
		break;
	      case OCR_RES_400:
		sp.resolution = RES_400;
		break;
	      default:
		return 0;
	}
	if (spp->adf) {
		sp.scanner = SCANNER_AD;
	} else {
		sp.scanner = SCANNER_FL;
	}
	if (spp->binary) {
		sp.dencity = DENC_AT;
		switch (spp->filter) {
		      case OCR_FILTER_NR:
			sp.filter = FILTER_NR;
			break;
		      case OCR_FILTER_BP:
			sp.filter = FILTER_BP;
			break;
		      default:
			return 0;
		}
		switch (spp->threshold) {
		      case OCR_THRES_0:
			sp.threshold = THRES_0;
			break;
		      case OCR_THRES_1:
			sp.threshold = THRES_1;
			break;
		      case OCR_THRES_2:
			sp.threshold = THRES_2;
			break;
		      case OCR_THRES_3:
			sp.threshold = THRES_3;
			break;
		      case OCR_THRES_4:
			sp.threshold = THRES_4;
			break;
		      case OCR_THRES_5:
			sp.threshold = THRES_5;
			break;
		      default:
			return 0;
		}
		switch (spp->noise) {
		      case OCR_NOISE_0:
			sp.noise = NOISE_0;
			break;
		      case OCR_NOISE_2:
			sp.noise = NOISE_2;
			break;
		      case OCR_NOISE_3:
			sp.noise = NOISE_3;
			break;
		      case OCR_NOISE_4:
			sp.noise = NOISE_4;
			break;
		      case OCR_NOISE_5:
			sp.noise = NOISE_5;
			break;
		      default:
			return 0;
		}
		switch (spp->doc) {
		      case OCR_DOC_NR:
			sp.doc = DOC_NR;
			break;
		      case OCR_DOC_NW:
			sp.doc = DOC_NW;
			break;
		      default:
			return 0;
		}
		switch (spp->smooth) {
		      case OCR_SMTH_CH:
			sp.smooth = SMTH_CH;
			break;
		      case OCR_SMTH_IM:
			sp.smooth = SMTH_IM;
			break;
		      default:
			return 0;
		}
		switch (spp->compo) {
		      case OCR_COMPO_LD:
			sp.compo = COMPO_LD;
			break;
		      case OCR_COMPO_PN:
			sp.compo = COMPO_PN;
			break;
		      case OCR_COMPO_PH:
			sp.compo = COMPO_PH;
			break;
		      case OCR_COMPO_PL:
			sp.compo = COMPO_PL;
			break;
		      default:
			return 0;
		}
	} else {
		switch (spp->dencity) {
		      case OCR_DENC_0:
			sp.dencity = DENC_0;
			break;
		      case OCR_DENC_1:
			sp.dencity = DENC_1;
			break;
		      case OCR_DENC_2:
			sp.dencity = DENC_2;
			break;
		      case OCR_DENC_3:
			sp.dencity = DENC_3;
			break;
		      case OCR_DENC_4:
			sp.dencity = DENC_4;
			break;
		      case OCR_DENC_5:
			sp.dencity = DENC_5;
			break;
		      case OCR_DENC_6:
			sp.dencity = DENC_6;
			break;
		      case OCR_DENC_7:
			sp.dencity = DENC_7;
			break;
		      default:
			return 0;
		}
		sp.filter = FILTER_NR;
		sp.threshold = THRES_0;
		sp.noise = NOISE_0;
		sp.doc = DOC_NR;
		sp.smooth = SMTH_CH;
		sp.compo = COMPO_LD;
	}
	if (ioctl(OCR_fd, OCR_SCAN, &sp) < 0) {
		/*
		 * We could return detailed info about error
		 * by examining the state field in sp.
		 */
		return 0;
	} else {
		return 1;
	}
}

/*
 * Kick start the image uploading and place the information about the
 * image being uploaded into the ocr_image_infor structure.
 */

static int
ocr_proc_iminfo(upp, infop)
struct ocr_up_para	*upp;
struct ocr_image_info	*infop;
{
	struct img_params	upara;
	struct ocr_stat		st;
				
	switch (upp->resolution) {
	      case OCR_RES_200:
		upara.resolution = RES_200;
		break;
	      case OCR_RES_240:
		upara.resolution = RES_240;
		break;
	      case OCR_RES_300:
		upara.resolution = RES_300;
		break;
	      case OCR_RES_400:
		upara.resolution = RES_400;
		break;
	      default:
		return 0;
	}
	switch (upp->compress) {
	      case OCR_COMP_NONE:
		upara.compress = COMP_NONE;
		break;
	      case OCR_COMP_2:
		upara.compress = COMP_2;
		break;
	      case OCR_COMP_4:
		upara.compress = COMP_4;
		break;
	      case OCR_COMP_8:
		upara.compress = COMP_8;
		break;
	      default:
		return 0;
	}
	upara.region.xs = upp->xs;
	upara.region.ys = upp->ys;
	upara.region.xe = upp->xe;
	upara.region.ye = upp->ye;
	
	
	if (ioctl(OCR_fd, OCR_IMG_UPLOAD, &upara) < 0) return 0;
	{
		int i;
		i = ioctl(OCR_fd, OCR_STAT, &st);
		if (i < 0) {
			return 0;
		} else if (st.state != STATE_UPLDING) {
			return 0;
		}
	}
/*	if (!ioctl(OCR_fd, OCR_STAT, &st) || st.state != STATE_UPLDING) return 0; */

	switch (st.data.image.resolution) {
	      case RES_200:
		infop->resolution = OCR_RES_200;
		break;
	      case RES_240:
		infop->resolution = OCR_RES_240;
		break;
	      case RES_300:
		infop->resolution = OCR_RES_300;
		break;
	      case RES_400:
		infop->resolution = OCR_RES_400;
		break;
	      default:
		return 0;
	}
	switch (st.data.image.compress) {
	      case COMP_NONE:
		infop->compress = OCR_COMP_NONE;
		break;
	      case COMP_2:
		infop->compress = OCR_COMP_2;
		break;
	      case COMP_4:
		infop->compress = OCR_COMP_4;
		break;
	      case COMP_8:
		infop->compress = OCR_COMP_8;
		break;
	      default:
		return 0;
	}
	infop->line_width = (u_int)st.data.image.line_width;
	infop->size = (u_int)st.data.image.size;
	infop->xs = st.data.image.region.xs;
	infop->ys = st.data.image.region.ys;
	infop->xe = st.data.image.region.xe;
	infop->ye = st.data.image.region.ye;
		
	OCR_image_size = (int)st.data.image.size;

	return 1;
	
}

/*
 * Set Read Parameters
 */

static int
ocr_proc_set_rpara(rpp, len)
register struct ocr_read_para	*rpp;
int				len;
{
	register int		cnt = len;
	struct read_params	rpara;
	
	if (OCR_fd < 0) {
		return 0;
	} else {
		for (; cnt > 0; cnt--, rpp++) {
			switch (rpp->format) {
			      case OCR_FMT_H1:
				rpara.format = FMT_H1;
				break;
			      case OCR_FMT_H2:
				rpara.format = FMT_H2;
				break;
			      case OCR_FMT_H3:
				rpara.format = FMT_H3;
				break;
			      case OCR_FMT_H4:
				rpara.format = FMT_H4;
				break;
			      case OCR_FMT_V1:
				rpara.format = FMT_V1;
				break;
			      case OCR_FMT_V2:
				rpara.format = FMT_V2;
				break;
			      case OCR_FMT_V3:
				rpara.format = FMT_V3;
				break;
			      case OCR_FMT_V4:
				rpara.format = FMT_V4;
				break;
			      default:
				return 0;
			}
			rpara.cunit = CUNIT_PO;
			rpara.csize = CSIZE_DFLT;
			rpara.ckind = 0;
			rpara.ckind |= (rpp->ck_num) ? CK_NUM : 0;
			rpara.ckind |= (rpp->ck_alph) ? CK_ALPH : 0;
			rpara.ckind |= (rpp->ck_grk) ? CK_GRK : 0;
			rpara.ckind |= (rpp->ck_jvt) ? CK_JVT : 0;
			rpara.ckind |= (rpp->ck_jhr) ? CK_JHR : 0;
			rpara.ckind |= (rpp->ck_sym) ? CK_SYM : 0;
			rpara.ckind |= (rpp->ck_kana) ? CK_KANA : 0;
			rpara.ckind |= (rpp->ck_joyou) ? CK_JOYOU : 0;
			rpara.ckind |= (rpp->ck_jis1) ? CK_JIS1 : 0;
			rpara.ckind |= (rpp->ck_jmisc) ? CK_JMISC : 0;
			rpara.ckind |= (rpp->ck_gaiji) ? CK_GAIJI : 0;
			switch (rpp->deform) {
			      case OCR_DFRM_1:
				rpara.cprop |= DFRM_1;
				break;
			      case OCR_DFRM_2:
				rpara.cprop |= DFRM_2;
				break;
			      case OCR_DFRM_3:
				rpara.cprop |= DFRM_3;
				break;
			      case OCR_DFRM_4:
				rpara.cprop |= DFRM_4;
				break;
			      case OCR_DFRM_NR:
				rpara.cprop |= DFRM_NR;
				break;
			      case OCR_DFRM_6:
				rpara.cprop |= DFRM_6;
				break;
			      case OCR_DFRM_7:
				rpara.cprop |= DFRM_7;
				break;
			      case OCR_DFRM_8:
				rpara.cprop |= DFRM_8;
				break;
			      case OCR_DFRM_9:
				rpara.cprop |= DFRM_9;
				break;
			      defaul:
				return 0;
			}
			if (OCR_REJ_0 <= rpp->reject &&
			    rpp->reject <= OCR_REJ_5) {
				rpara.cprop |= rpp->reject;
			} else {
				return 0;
			}
			rpara.region.xs = rpp->xs;
			rpara.region.ys = rpp->ys;
			rpara.region.xe = rpp->xe;
			rpara.region.ye = rpp->ye;

			if (ioctl(OCR_fd, OCR_READ_PARA, &rpara) < 0) {
				return 0;
			}
		}
		OCR_read_regions += len;
		return 1;
	}
}

/*
 * Clear Read Parameters
 */

static int
ocr_proc_clr_rpara()
{
	if (OCR_fd < 0 || ioctl(OCR_fd, OCR_READ_CLR) < 0) {
		return 0;
	} else {
		return 1;
	}
}

/*
 * Read
 */

static int
ocr_proc_read()
{
	if (OCR_fd < 0 || ioctl(OCR_fd, OCR_READ) < 0) {
		return 0;
	} else {
		return 1;
	}
}

/*
 * Upload and transfer image data.  Data transfer is done in such way that is
 * compatible with ocr_bulk_read in 'ocr.c'.
 */

static void
ocr_proc_img_upload(iminfop, ppid)
struct ocr_image_info	*iminfop;
pid_t			ppid;
{
	register int		len, resid, firsttime = 1;
	int			cnt;
	char			rbuf[65536], pkt[PKTLEN];
	register char		*ptr;
	struct bd_header	hd;

	resid = OCR_image_size;

	while (resid > 0) {
		len = MIN(resid, sizeof(rbuf));
		cnt = read(OCR_fd, rbuf, len);
		if (cnt < 0) {
			pkt[0] = 0;
			write(OCR_sv, pkt, sizeof(pkt));
			return;
		}			
		resid -= cnt;
		hd.len = (u_int)cnt;
		hd.cont = (resid > 0) ? BD_CONT : BD_LAST;
		if (firsttime) {
			pkt[0] = 1;
			write(OCR_sv, pkt, sizeof(pkt));
			write(OCR_sv, (char *)iminfop,
			      sizeof(struct ocr_image_info));
			write(OCR_sv, &hd, sizeof(struct bd_header));
			notify_ready(ppid);
			firsttime = 0;
		} else {
			write(OCR_sv, &hd, sizeof(struct bd_header));
		}
		for (ptr = rbuf; cnt > 0; cnt -= len, ptr += len) {
			len = write(OCR_sv, ptr, cnt);
		}
	}
	return;
}

/*
 * Get the read result from the device driver.  The format of the data
 * is converted to make it easy Lisp to use it.  After converting the data,
 * transfer data.  Data transfer is done in such way that is
 * compatible with ocr_bulk_read in 'ocr.c'.
 */

static void
ocr_proc_post_read(ppid)
pid_t	ppid;
{
	static u_char		rbuf[65536];
	static u_char		fbuf[65536];
	register u_char		*ptr;
	struct ocr_stat		st;
	register int		cnt, len;
	int			amt, firsttime = 1;
	struct bd_header	hd;

	if (OCR_fd < 0) goto fail;

	if (ioctl(OCR_fd, OCR_STAT, &st) < 0 ||
	    st.state != STATE_RDING) goto fail;

	while (st.state == STATE_RDING) {
		if (sizeof(rbuf) - 2048 > st.data.rd_resid) {
			/*
			 * 2048 is a margin to reformat the data.
			 */
			amt = len = st.data.rd_resid;
		} else {
			amt = len = sizeof(rbuf) - 2048;
		}
		ptr = rbuf;
		do {
			cnt = read(OCR_fd, ptr, len);
			if (cnt < 0) {
				goto fail;
			} else {
				len -= cnt;
				ptr += cnt;
			}
		} while (len > 0);

		if (ioctl(OCR_fd, OCR_STAT, &st) < 0) goto fail;

		len = ocr_conv_read_data(rbuf, fbuf, amt);
		if (len < 0) {
			goto fail;
		} else if (firsttime) {
			u_char	pkt[PKTLEN];

			pkt[0] = 1;
			write(OCR_sv, pkt, sizeof(pkt));
			notify_ready(ppid);
			firsttime = 0;
		}

		hd.len = (u_int)len;
		hd.cont = (st.state == STATE_RDING) ? BD_CONT : BD_LAST;

		write(OCR_sv, &hd, sizeof(struct bd_header));
		for (ptr = fbuf; len > 0; len -= cnt, ptr += cnt) {
			cnt = write(OCR_sv, ptr, len);
		}
	}
	return;

      fail:
	{
		u_char	pkt[PKTLEN];
		pkt[0] = 0;
		write(OCR_sv, pkt, sizeof(pkt));
		notify_ready(ppid);
		return;
	}
}

/*
 * Reformat the read data to make a life of Lisp tremendously easy
 */

static int
ocr_conv_read_data(sp, dp, len)
register u_char		*sp, *dp;
register int		len;
{
	bcopy(sp, dp, len);
	return(len);
}

/****************************************************
 ****************************************************
 
#define INC_SP(pos) {\
	if (--len == 0) {\
		statep->jmp = (pos);\
		return(amt);\
	} else {\
		sp++;\
	}\
}

#define CONV_CODE_BLOCK {
	static int	pos, hi, lo;

	pos = 0;

	if (*sp == 0xEB) {
		INC_SP(POS16);
	      pos16:
		if (*sp == 0xA0) {
			
		if (*sp == 0x9F) {
			/+ Lower certencity +/
			*dp++ = REJECT;
			INC_SP(POS17);
		      pos17:
			pos += 2;
			hi = *sp;
			INC_SP
			
		

static int
ocr_conv_read_data(sp, dp, len, statep)
register u_char		*sp, *dp;
register int		len;
struct parse_state	*statep;
{
	/+
	 * for now...
	 +/

	bcopy(sp, dp, len);
	return(len);

	switch (statep->jmp) {
	      case POS1:	goto pos1;
	}

	amt = 0;
	while (1) {
		if (*sp == '0') {
			INC_SP(POS1);
		      pos1:
			if (*sp == '0') {
				/+ Region start +/
				*dp++ = REG_START_CODE;
				/+ Skip region number +/
				INC_SP(POS2);
			      pos2:
				INC_SP(POS3);
			      pos3:
				INC_SP(POS4);
			      pos4:
				if (*sp == 0xFC) {
					INC_SP(POS5);
				      pos5:
					if (*sp == 0xFA) {
						/+ Blank region +/
						*dp++ = REG_END_CODE;
						continue;
					}
				}
				/+ Skip a region header +/
				INC_SP(POS6);
			      pos6:
				INC_SP(POS7);
			      pos7:
				INC_SP(POS8);
			      pos8:
				INC_SP(POS9);
			      pos9:
				INC_SP(POS10);
			      pos10:
				INC_SP(POS11);
			      pos11:
				INC_SP(POS12);
			      pos12:
				while (1) {
					if (*sp == 0xFC) {
						INC_SP(POS13);
					      pos13:
						if (*sp == 0xFA) {
							/+ Region end +/
							*dp++ = REG_END_CODE;
							break;
						}
					}
					/+ Skip a line number +/
					INC_SP(POS14);
				      pos14:
					/+ Process a line +/
					while (1) {
						if (*sp == 0xFC) {
							INC_SP(POS15);
						      pos15:
							if (*sp == 0xFB) {
								/+ EOL +/
								break;
							}
						}
						/+ Some code exists +/
						CONV_CODE_BLOCK;
					}
					if (statep.cr_line) *dp++ = EOL_CODE;
				}
			}
		}
		/+ Page end +/
		return(1)
	}
}


		if (*sp == '0' && *(sp + 1) == '0') {
			/+ Region start +/
			*dp++ = REG_START_CODE;
			/+ Skip region number +/
			sp += 4;
			if (*sp == 0xFC && *(sp + 1) == 0xFA) {
				/+ Blank region +/
				*dp++ = REG_END_CODE;
				continue;
			} else {
				/+ Skip a region header +/
				sp += 8;
			}
			while (1) {
				if (*sp == 0xFC && *(sp + 1) == 0xFA) {
					/+ Region end +/
					*dp++ = REG_END_CODE;
					break;
				} else {
					/+ Skip a line number +/
					sp += 2;
					/+ Process one line +/
					while (*sp != 0xFC || *(sp + 1) != 0xFB) {
						/+ Some codes exists +/
						CONV_CODE_BLOCK(sp, bp);
					}
					if (statep.cr_line) *dp++ = EOL_CODE;
				}
			}
		} else {
			/+ Page end +/
		}
	}
}
*********************************************
*********************************************/

#endif /* OCR */

