/* $Id: externs.h,v 1.2 1999/01/03 02:05:59 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/*
 *
 * externs.h - external definitons for nfswatch.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * $Log: externs.h,v $
 * Revision 1.2  1999/01/03 02:05:59  sybalsky
 * Add ID comments / static to files for CVS use
 *
 * Revision 1.1.1.1  1998/12/17 05:03:16  sybalsky
 * Import of Medley 3.5 emulator
 *
 * Revision 4.2  93/10/04  11:00:24  mogul
 * Added fhdebugf flag
 * 
 * Revision 4.1  93/09/28  21:23:40  mogul
 * explicit IP address data type.
 * 
 * Revision 4.0  93/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 * 
 * Revision 3.7  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.6  1993/01/20  14:52:30  davy
 * Added -T maxtime option.
 *
 * Revision 3.5  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.4  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.3  1993/01/13  15:12:05  davy
 * Added background mode.
 *
 * Revision 3.2  1993/01/13  13:00:04  davy
 * Fixed a bug in finish() routine, closing too many file descriptors.
 *
 * Revision 3.1  1992/07/24  18:47:57  mogul
 * Added FDDI support
 *
 * Revision 3.0  1991/01/23  08:23:02  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.3  91/01/04  15:52:07  davy
 * New features from Jeff Mogul.
 * 
 * Revision 1.2  90/08/17  15:46:43  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:27  davy
 * NFSWATCH Release 1.0
 * 
 */

extern char		*pname;

extern FILE		*logfp;

extern Counter		pkt_total;
extern Counter		pkt_drops;
extern Counter		int_pkt_total;
extern Counter		int_pkt_drops;
extern Counter		dst_pkt_total;
extern Counter		int_dst_pkt_total;

extern int		errno;
extern int		bgflag;
extern int		if_fd[];
extern int		allintf;
extern int		fhdebugf;
extern int		dstflag;
extern int		srcflag;
extern int		allflag;
extern int		logging;
extern int		learnfs;
extern int		if_dlt[];
extern int		do_update;
extern int		cycletime;
extern int		totaltime;
extern int		showwhich;
extern int		serverflag;
extern int		truncation;
extern int		ninterfaces;
extern int		sortbyusage;
extern int		nnfscounters;
extern int		nfilecounters;
extern int		nauthcounters;
extern int		screen_inited;
extern int		nclientcounters;

extern ipaddrt		thisdst;
extern ipaddrt		srcaddrs[];
extern ipaddrt		dstaddrs[];
extern ipaddrt		serveraddrs[];

extern struct timeval	starttime;

extern char		myhost[];
extern char		srchost[];
extern char		dsthost[];
extern char		serverhost[];

extern char		*prompt;
extern char		*logfile;
extern char		*mapfile;
extern char		*filelist;
extern char		*snapshotfile;

extern NFSCounter	nfs_counters[];
extern FileCounter	fil_counters[];
extern PacketCounter	pkt_counters[];
extern ProcCounter	prc_counters[];
extern int		prc_countmap[];
extern ClientCounter	clnt_counters[];
extern AuthCounter	auth_counters[];

extern NFSCall		nfs_calls[NFSCALLHASHSIZE];

char			*dlt_name();
char			*prtime();
char			*savestr();

int			auth_comp();
int			clnt_comp();
int			dlpi_devtype();
int			fil_comp();
int			is_exported();
int			nfs_comp();
int			nit_devtype();
int			pfilt_devtype();
int			prc_comp();
int			setup_nit_dev();
int			setup_dlpi_dev();
int			setup_pfilt_dev();
int			setup_snoop_dev();
int			snoop_devtype();
int			udprpc_recv();
int			want_packet();

void			clear_vars();
void			command();
void			error();
void			finish();
void			flush_nit();
void			flush_dlpi();
void			flush_pfilt();
void			flush_snoop();
void			get_net_addrs();
void			icmp_filter();
void			ip_filter();
void			label_screen();
void			nd_filter();
void			nfs_count();
void			nfs_filter();
void			nfs_hash_call();
void			nfs_hash_reply();
void			nfswatch();
void			pkt_filter_ether();
void			pkt_filter_fddi();
void			rpc_callfilter();
void			rpc_filter();
void			rpc_replyfilter();
void			setup_auth_counters();
void			setup_fil_counters();
void			setup_map_file();
void			setup_nfs_counters();
void			setup_pkt_counters();
void			setup_screen();
void			setup_rpcxdr();
void			snapshot();
void			sort_auth_counters();
void			sort_nfs_counters();
void			tcp_filter();
void			udp_filter();
void			update_logfile();
void			update_screen();
void			usage();
void			wakeup();
