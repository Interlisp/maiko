/* $Id: dlpi.c,v 1.3 2001/12/24 01:09:00 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

#include "os.h"

#ifdef USE_DLPI
/*
 * dpli.c - routines for messing with the Data Link Provider Interface.
 *
 * The code in this module is based in large part (especially the dl*
 * routines) on the example code provided with the document "How to Use
 * DLPI", by Neal Nuckolls of Sun Internet Engineering.  Gotta give credit
 * where credit is due.  If it weren't for Neal's excellent document,
 * this module just plain wouldn't exist.
 *
 * David A. Curry
 * Purdue University
 * Engineering Computer Network
 * 1285 Electrical Engineering Building
 * West Lafayette, IN 47907-1285
 * davy@ecn.purdue.edu
 *
 * $Log: dlpi.c,v $
 * Revision 1.3  2001/12/24 01:09:00  sybalsky
 * past changes
 *
 * Revision 1.2  1999/01/03 02:06:54  sybalsky
 * Add ID comments / static to files for CVS use
 *
 * Revision 1.1.1.1  1998/12/17 05:03:20  sybalsky
 * Import of Medley 3.5 emulator
 *
 * Revision 4.1  1993/09/15  20:50:44  davy
 * GCC fixes from Guy Harris.
 *
 * Revision 4.1  1993/09/15  20:50:44  davy
 * GCC fixes from Guy Harris.
 *
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 1.5  1993/02/19  19:54:36  davy
 * Another change in hopes of making things work on SVR4.
 *
 * Revision 1.4  1993/01/26  13:19:05  davy
 * Fixed a goof in passing buffer size.
 *
 * Revision 1.3  1993/01/26  13:18:39  davy
 * Added ifdef's to make it work on DLPI 1.3.
 *
 * Revision 1.2  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 1.1  1993/01/15  15:42:32  davy
 * Initial revision
 *
 */
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/dlpi.h>
#ifdef OS5
#include <sys/bufmod.h>
#include <unistd.h>
#include <stdlib.h>
#include <stropts.h>
#include <malloc.h>
#endif
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "nfswatch.h"

static void dlbindreq(int fd, u_long sap, u_long max_conind, u_long service_mode, u_long conn_mgmt, u_long xidtest);
static void dlinforeq(int fd);
static void dlattachreq(int fd, u_long ppa);
static void dlpromisconreq(int fd, u_long level);
static void sigalrm(int sig);
static int dlokack(int fd, char *bufp);
static int dlinfoack(int fd, char *bufp);
static int dlbindack(int fd, char *bufp);
static int expecting(int prim, union DL_primitives *dlp);
static int strgetmsg(int fd, struct strbuf *ctlp, struct strbuf *datap, int *flagsp, char *caller);
static char *savestr(char *s);

static char *pname;

extern unsigned char ether_host[6];

int truncation = 1500;
/*
 * setup_dlpi_dev - set up the data link provider interface.
 */
int setup_dlpi_dev(char *device)
{
  char *p;
  u_int chunksz;
  char cbuf[BUFSIZ];
  struct ifconf ifc;
  struct ifreq *ifrp;
  struct strioctl si;
  char devname[BUFSIZ];
  int n, s, fd, devppa;
  struct timeval timeout;
  char buf[DLPI_CHUNKSIZE];

  /*
   * If the interface device was not specified,
   * get the default one.
   */
  if ((device == NULL) || *device == NULL) {
    /*
     * Grab a socket.
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket");
      return (-1);
    }

    ifc.ifc_buf = cbuf;
    ifc.ifc_len = sizeof(cbuf);

    /*
     * See what devices we've got.
     */
    if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
      perror("ioctl: SIOCGIFCONF");
      return (-1);
    }

    /*
     * Take the first device we encounter.
     */
    ifrp = ifc.ifc_req;
    for (n = ifc.ifc_len / sizeof(struct ifreq); n > 0; n--, ifrp++) {
      /*
       * Skip the loopback interface.
       */
      if (strcmp(ifrp->ifr_name, "lo0") == 0) continue;

      device = savestr(ifrp->ifr_name);
      break;
    }

    (void)close(s);
  }

  /*
   * Split the device name into type and unit number.
   */
  if ((p = strpbrk(device, "0123456789")) == NULL) return (-1);

  strcpy(devname, DLPI_DEVDIR);
  strncat(devname, device, p - device);
  devppa = atoi(p);

  /*
   * Open the device.
   */
  if ((fd = open(devname, O_RDWR)) < 0) {
    if (errno == ENOENT || errno == ENXIO) return (-1);

    /*perror(devname);*/
    return (-1);
  }

  /*
   * Attach to the device.  If this fails, the device
   * does not exist.
   */
  dlattachreq(fd, devppa);

  if (dlokack(fd, buf) < 0) {
    close(fd);
    return (-1);
  }

  /*
   * Bind to the specific unit.
   */
  dlbindreq(fd, 0x0600, 0, DL_CLDLS, 0, 0);

  if (dlbindack(fd, buf) < 0) {
    fprintf(stderr, "%s: dlbindack failed.\n", pname);
    return (-1);
  }

#ifdef OS5
  /*
   * We really want all types of packets.  However, the SVR4 DLPI does
   * not let you have the packet frame header, so we won't be able to
   * distinguish protocol types.  But SunOS5 gives you the DLIOCRAW
   * ioctl to get the frame headers, so we can do this on SunOS5.
   */
  dlpromisconreq(fd, DL_PROMISC_SAP);

  if (dlokack(fd, buf) < 0) {
    fprintf(stderr, "%s: DL_PROMISC_SAP failed.\n", pname);
    return (-1);
  }

  dlpromisconreq(fd, DL_PROMISC_MULTI);

  if (dlokack(fd, buf) < 0) {
    fprintf(stderr, "%s: DL_PROMISC_MULTI failed.\n", pname);
    return (-1);
  }

  /*
   * We want raw packets with the packet frame header.  But we can
   * only get this in SunOS5 with the DLIOCRAW ioctl; it's not in
   * standard SVR4.
   */
  si.ic_cmd = DLIOCRAW;
  si.ic_timout = -1;
  si.ic_len = 0;
  si.ic_dp = 0;

  if (ioctl(fd, I_STR, &si) < 0) {
    perror("ioctl: I_STR DLIOCRAW");
    return (-1);
  }
#endif /* OS5 */

  /*
   * Arrange to get discrete messages.
   */
  if (ioctl(fd, I_SRDOPT, (char *)RMSGD) < 0) {
    perror("ioctl: I_SRDOPT RMSGD");
    return (-1);
  }

#ifdef OS5
/*
 * Push and configure the streams buffering module.  This is once
 * again SunOS-specific.
 */
#ifdef NEVER
  if (ioctl(fd, I_PUSH, DLPI_BUFMOD) < 0) {
    perror("ioctl: I_PUSH BUFMOD");
    return (-1);
  }
  /*
   * Set the read timeout.
   */
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  si.ic_cmd = SBIOCSTIME;
  si.ic_timout = INFTIM;
  si.ic_len = sizeof(timeout);
  si.ic_dp = (char *)&timeout;

  if (ioctl(fd, I_STR, (char *)&si) < 0) {
    perror("ioctl: I_STR SBIOCSTIME");
    return (-1);
  }

  /*
   * Set the chunk size.
   */
  chunksz = DLPI_CHUNKSIZE;

  si.ic_cmd = SBIOCSCHUNK;
  si.ic_len = sizeof(chunksz);
  si.ic_dp = (char *)&chunksz;

  if (ioctl(fd, I_STR, (char *)&si) < 0) {
    perror("ioctl: I_STR SBIOCSCHUNK");
    return (-1);
  }

  /*
   * Set snapshot mode.
   */
  si.ic_cmd = SBIOCSSNAP;
  si.ic_len = sizeof(truncation);
  si.ic_dp = (char *)&truncation;

  if (ioctl(fd, I_STR, (char *)&si) < 0) {
    perror("ioctl: I_STR SBIOCSSNAP");
    return (-1);
  }
#endif /* NEVER */

#endif /* OS5 */

  return (fd);
}

/*
 * flush_dlpi - flush data from the dlpi.
 */
void flush_dlpi(int fd)
{
  if (ioctl(fd, I_FLUSH, (char *)FLUSHR) < 0) {
    perror("ioctl: I_FLUSH");
    return;
  }
}

/*
 * dlpi_devtype - determine the type of device we're looking at.
 */
int dlpi_devtype(int fd)
{
  char buf[DLPI_CHUNKSIZE];
  union DL_primitives *dlp;

  dlp = (union DL_primitives *)buf;

  dlinforeq(fd);

  if (dlinfoack(fd, buf) < 0) return (DLT_EN10MB);

  bcopy((char *)dlp + dlp->info_ack.dl_addr_offset, ether_host, 6);

  switch (dlp->info_ack.dl_mac_type) {
    case DL_CSMACD:
    case DL_ETHER: return (DLT_EN10MB);
#ifdef DL_FDDI
    case DL_FDDI: return (DLT_FDDI);
#endif
    default:
      fprintf(stderr, "%s: DLPI MACtype %ld unknown, ", pname, (long)dlp->info_ack.dl_mac_type);
      fprintf(stderr, "assuming ethernet.\n");
      return (DLT_EN10MB);
  }
}

/*
 * dlinforeq - request information about the data link provider.
 */
static void dlinforeq(int fd)
{
  dl_info_req_t info_req;
  struct strbuf ctl;
  int flags;

  info_req.dl_primitive = DL_INFO_REQ;

  ctl.maxlen = 0;
  ctl.len = sizeof(info_req);
  ctl.buf = (char *)&info_req;

  flags = RS_HIPRI;

  if (putmsg(fd, &ctl, (struct strbuf *)NULL, flags) < 0) {
    perror("putmsg");
    return;
  }
}

/*
 * dlattachreq - send a request to attach.
 */
static void dlattachreq(int fd, u_long ppa)
{
  dl_attach_req_t attach_req;
  struct strbuf ctl;
  int flags;

  attach_req.dl_primitive = DL_ATTACH_REQ;
  attach_req.dl_ppa = ppa;

  ctl.maxlen = 0;
  ctl.len = sizeof(attach_req);
  ctl.buf = (char *)&attach_req;

  flags = 0;

  if (putmsg(fd, &ctl, (struct strbuf *)NULL, flags) < 0) {
    perror("putmsg");
    return;
  }
}

#ifdef DL_PROMISCON_REQ
/*
 * dlpromisconreq - send a request to turn promiscuous mode on.
 */
static void dlpromisconreq(int fd, u_long level)
{
  dl_promiscon_req_t promiscon_req;
  struct strbuf ctl;
  int flags;

  promiscon_req.dl_primitive = DL_PROMISCON_REQ;
  promiscon_req.dl_level = level;

  ctl.maxlen = 0;
  ctl.len = sizeof(promiscon_req);
  ctl.buf = (char *)&promiscon_req;

  flags = 0;

  if (putmsg(fd, &ctl, (struct strbuf *)NULL, flags) < 0) {
    perror("putmsg");
    return;
  }
}
#endif /* DL_PROMISCON_REQ */

/*
 * dlbindreq - send a request to bind.
 */
static void dlbindreq(int fd, u_long sap, u_long max_conind, u_long service_mode, u_long conn_mgmt, u_long xidtest)
{
  dl_bind_req_t bind_req;
  struct strbuf ctl;
  int flags;

  bind_req.dl_primitive = DL_BIND_REQ;
  bind_req.dl_sap = sap;
  bind_req.dl_max_conind = max_conind;
  bind_req.dl_service_mode = service_mode;
  bind_req.dl_conn_mgmt = conn_mgmt;
#ifdef DL_PROMISC_PHYS
  /*
   * DLPI 2.0 only?
   */
  bind_req.dl_xidtest_flg = xidtest;
#endif

  ctl.maxlen = 0;
  ctl.len = sizeof(bind_req);
  ctl.buf = (char *)&bind_req;

  flags = 0;

  if (putmsg(fd, &ctl, (struct strbuf *)NULL, flags) < 0) {
    perror("putmsg");
    return;
  }
}

/*
 * dlokack - general acknowledgement reception.
 */
static int dlokack(int fd, char *bufp)
{
  union DL_primitives *dlp;
  struct strbuf ctl;
  int flags;

  ctl.maxlen = DLPI_MAXDLBUF;
  ctl.len = 0;
  ctl.buf = bufp;

  if (strgetmsg(fd, &ctl, (struct strbuf *)NULL, &flags, "dlokack") < 0) return (-1);

  dlp = (union DL_primitives *)ctl.buf;

  if (expecting(DL_OK_ACK, dlp) < 0) return (-1);

  if (ctl.len < sizeof(dl_ok_ack_t)) return (-1);

  if (flags != RS_HIPRI) return (-1);

  if (ctl.len < sizeof(dl_ok_ack_t)) return (-1);

  return (0);
}

/*
 * dlinfoack - receive an ack to a dlinforeq.
 */
static int dlinfoack(int fd, char *bufp)
{
  union DL_primitives *dlp;
  struct strbuf ctl;
  int flags;

  ctl.maxlen = DLPI_MAXDLBUF;
  ctl.len = 0;
  ctl.buf = bufp;

  if (strgetmsg(fd, &ctl, (struct strbuf *)NULL, &flags, "dlinfoack") < 0) return (-1);

  dlp = (union DL_primitives *)ctl.buf;

  if (expecting(DL_INFO_ACK, dlp) < 0) return (-1);

  if (ctl.len < sizeof(dl_info_ack_t)) return (-1);

  if (flags != RS_HIPRI) return (-1);

  if (ctl.len < sizeof(dl_info_ack_t)) return (-1);

  return (0);
}

/*
 * dlbindack - receive an ack to a dlbindreq.
 */
static int dlbindack(int fd, char *bufp)
{
  union DL_primitives *dlp;
  struct strbuf ctl;
  int flags;

  ctl.maxlen = DLPI_MAXDLBUF;
  ctl.len = 0;
  ctl.buf = bufp;

  if (strgetmsg(fd, &ctl, (struct strbuf *)NULL, &flags, "dlbindack") < 0) return (-1);

  dlp = (union DL_primitives *)ctl.buf;

  if (expecting(DL_BIND_ACK, dlp) < 0) return (-1);

  if (flags != RS_HIPRI) return (-1);

  if (ctl.len < sizeof(dl_bind_ack_t)) return (-1);

  return (0);
}

/*
 * expecting - see if we got what we wanted.
 */
static int expecting(int prim, union DL_primitives *dlp)
{
  if (dlp->dl_primitive != (u_long)prim) return (-1);

  return (0);
}

/*
 * strgetmsg - get a message from a stream, with timeout.
 */
static int strgetmsg(int fd, struct strbuf *ctlp, struct strbuf *datap, int *flagsp, char *caller)
{
  int rc;

  /*
   * Start timer.
   */
  (void)sigset(SIGALRM, sigalrm);

  if (alarm(DLPI_MAXWAIT) < 0) {
    perror("alarm");
    return (-1);
  }

  /*
   * Set flags argument and issue getmsg().
   */
  *flagsp = 0;
  if ((rc = getmsg(fd, ctlp, datap, flagsp)) < 0) {
    perror("getmsg");
    return (-1);
  }

  /*
   * Stop timer.
   */
  if (alarm(0) < 0) {
    perror("alarm");
    return (-1);
  }

  /*
   * Check for MOREDATA and/or MORECTL.
   */
  if ((rc & (MORECTL | MOREDATA)) == (MORECTL | MOREDATA)) return (-1);
  if (rc & MORECTL) return (-1);
  if (rc & MOREDATA) return (-1);

  /*
   * Check for at least sizeof (long) control data portion.
   */
  if (ctlp->len < sizeof(long)) return (-1);

  return (0);
}

/*
 * sigalrm - handle alarms.
 */
static void sigalrm(int sig) { (void)fprintf(stderr, "dlpi: timeout\n"); }

/*
 * savestr - save string in dynamic memory.
 */
static char *savestr(char *s)
{
  char *t;

  if ((t = malloc(strlen(s) + 1)) == NULL) {
    (void)fprintf(stderr, "%s: out of memory.\n", pname);
    (void)exit(1);
  }

  (void)strcpy(t, s);

  return (t);
}

int dlunitdatareq(int fd, u_char *addrp, int addrlen, u_long minpri, u_long maxpri, u_char *datap, int datalen)
{
  char buf[DLPI_CHUNKSIZE];
  union DL_primitives *dlp;
  struct strbuf data, ctl;

  dlp = (union DL_primitives *)buf;

  dlp->unitdata_req.dl_primitive = DL_UNITDATA_REQ;
  dlp->unitdata_req.dl_dest_addr_length = addrlen + 2;
  dlp->unitdata_req.dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
  dlp->unitdata_req.dl_priority.dl_min = minpri;
  dlp->unitdata_req.dl_priority.dl_max = maxpri;

  (void)memcpy(OFFADDR(dlp, sizeof(dl_unitdata_req_t)), addrp, addrlen);
  (void)memcpy(OFFADDR(dlp, sizeof(dl_unitdata_req_t) + addrlen), (char *)addrp + 12, 2);

  ctl.maxlen = 0;
  ctl.len = sizeof(dl_unitdata_req_t) + addrlen + 2;
  ctl.buf = (char *)buf;

  data.maxlen = 0;
  data.len = datalen;
  data.buf = (char *)datap;

#ifdef NEVER
  if (putmsg(fd, &ctl, &data, 0) < 0)
#else
  if (putmsg(fd, NULL, &data, 0) < 0)
#endif /* NEVER tst on 9/30/96 jds to see if raw out works */
    return -1;
  else
    return 0;
}
#endif /* USE_DLPI */
