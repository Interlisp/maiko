/* $Id: oether.c,v 1.2 1999/01/03 02:07:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <ctype.h>
#ifndef DOS
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#ifndef NOETHER
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/nit.h>

#include <sys/ioctl.h>
#ifdef OS4
#include <stropts.h>
#include <net/nit_if.h>
#include <net/nit_pf.h>
/* #include <net/nit_buf.h> */
#include <net/packetfilt.h>
#endif /* OS4 */

#include <sys/mbuf.h>
#include <nlist.h>
#endif /* DOS */
#endif /* NOETHER */

#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "adr68k.h"
#include "ether.h"
#include "dbprint.h"

u_int EtherReadFds;

int ether_fd = -1;                         /* file descriptor for ether socket */
u_char ether_host[6] = {0, 0, 0, 0, 0, 0}; /* 48 bit address of this node */
u_char broadcast[6] = {255, 255, 255, 255, 255, 255};
int ether_bsize = 0;  /* if nonzero then a receive is pending */
u_char *ether_buf;    /* address of receive buffer */
u_char nit_buf[3000]; /* the current chunk read from NIT (one packet) */
extern LispPTR *PENDINGINTERRUPT68k;
extern u_int LispReadFds;

int ETHEREventCount = 0;

#define PacketTypeIP 0x0800
#define PacketTypeARP 0x0806
#define PacketTypeRARP 0x8035
#define PacketTypeXNS 0x0600
#define PacketTypePUP 0x0200
#define PacketType3TO10 0x0201

#ifndef NOETHER
#ifdef OS4
/* the receiving packetfilter structure */
/* if this is changed, be sure to get the references to it in init_ether
        and check the length (the second entry in the structure) */
struct packetfilt goodpf = {0,
                            29,
                            {ENF_PUSHWORD + 6,
                             ENF_PUSHLIT + ENF_CNOR,
                             PacketTypeIP, /* punt if PacketTypeIP */
                             ENF_PUSHWORD + 6,
                             ENF_PUSHLIT + ENF_CNOR,
                             PacketTypeARP, /* or PacketTypeARP */
                             ENF_PUSHWORD,
                             ENF_PUSHLIT + ENF_EQ,
                             8, /* check our addr */
                             ENF_PUSHWORD + 1,
                             ENF_PUSHLIT + ENF_EQ,
                             11, /* which is filled in */
                             ENF_PUSHWORD + 2,
                             ENF_PUSHLIT + ENF_EQ,
                             14, /* in init_ether */
                             ENF_AND,
                             ENF_AND,
                             ENF_PUSHWORD,
                             ENF_PUSHLIT + ENF_EQ,
                             0xFFFF, /* check broadcast */
                             ENF_PUSHWORD + 1,
                             ENF_PUSHLIT + ENF_EQ,
                             0xFFFF, /* which is all ones */
                             ENF_PUSHWORD + 2,
                             ENF_PUSHLIT + ENF_EQ,
                             0xFFFF,
                             ENF_AND,
                             ENF_AND,
                             ENF_OR}};

/* a filter that rejects all packets to be used by ether_suspend and the
        initial routine to flush out ether_fd */
struct packetfilt nopf = {0, 1, {ENF_PUSHZERO}};

#endif /* OS4 */

int ether_in = 0;  /* number of packets received */
int ether_out = 0; /* number of packets sent */
#ifndef OS4
static struct nit_ioc nioc;
#endif /* OS4 */

#endif /* NOETHER */

/************************************************************************/
/*									*/
/*			e t h e r _ s u s p e n d			*/
/*									*/
/*	Suspend receiving packets from the NIT socket.			*/
/*	175/70/0							*/
/*									*/
/************************************************************************/

LispPTR ether_suspend(LispPTR args[])
{
#ifndef NOETHER
#ifdef OS4
  static struct packetfilt pf = {0, 1, {ENF_PUSHZERO}};
#endif /* OS4 */

  if (ether_fd == -1) return (NIL);
#ifndef OS4
  nioc.nioc_typetomatch = NT_NOTYPES;
  if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
    printf("ether_suspend: ioctl failed\n");
#else  /* OS4 */

  /* The trick here is to install a packet filter */
  /* that rejects all packets, I think... 	*/
  if (ioctl(ether_fd, NIOCSETF, &nopf) != 0) {
    perror("ether_suspend: NIOCSETF failed\n");
#endif /* OS4 */

    return (NIL);
  }
#endif /* NOETHER */

  return (ATOM_T);
} /* ether_suspend */

/************************************************************************/
/*									*/
/*			e t h e r _ r e s u m e				*/
/*									*/
/*	resume nit socket to receive all types of packets 175/71/0	*/
/*									*/
/************************************************************************/

LispPTR ether_resume(LispPTR args[])
{
  if (ether_fd == -1) return (NIL);
#ifndef NOETHER
#ifndef OS4
  nioc.nioc_typetomatch = NT_ALLTYPES;
  if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
    printf("ether_resume: ioctl failed\n");
#else  /* OS4 */

  /* Install a packet filter that accepts all packets we want */
  if (ioctl(ether_fd, NIOCSETF, &goodpf) != 0) {
    perror("ether_resume: NIOCSETF failed\n");
#endif /* OS4 */

    return (NIL);
  }
#endif /* NOETHER */

  return (ATOM_T);
} /* ether_resume */

/************************************************************************/
/*									*/
/*			e t h e r _ c t r l r				*/
/*									*/
/*	return T if ether controller is available 175/72/0		*/
/*									*/
/************************************************************************/

LispPTR ether_ctrlr(LispPTR args[])
{
  if (ether_fd < 0) return (NIL);
  return (ATOM_T);
}

/**********************************************************************
 *	ether_reset(args) 175/73/0
 *	reset ether controller and disable receipt of packets
 **********************************************************************/
LispPTR ether_reset(LispPTR args[])
{
  int i;
  char hostnumber[6];

  if (ether_fd < 0) { return (NIL); }
  /* JRB - host number check removed here; if ether_fd is open here,
              net is on... */
  ether_bsize = 0; /* deactivate receiver */
  return (ATOM_T);
} /* ether_reset */

/************************************************************************/
/*									*/
/*			  e t h e r _ g e t (175/74/2)			*/
/*									*/
/*	Set up the Ethernet driver to receive a packet.  The driver	*/
/*	first tries to read any pending packet from the net, and if	*/
/*	there is one, ether_get returns T.  If there is no pending	*/
/*	packet, the failing read sets us up to get an interrupt when	*/
/*	a packet DOES arrive, and ether_get returns NIL.		*/
/*									*/
/*		args[0] Length of the buffer we're passed		*/
/*		args[1] LISP address of a packet buffer			*/
/*									*/
/*	sets ether_buf to the buffer address, for check_ether's use	*/
/*	sets ether_bsize to the buffer size.  ether_bsize>0 means	*/
/*	it's OK to read packets from the network on interrupt.		*/
/*									*/
/************************************************************************/

LispPTR ether_get(LispPTR args[])
{
  LispPTR MaxByteCount;
  LispPTR result = NIL;
  int interrupt_mask;

#ifndef NOETHER
  MaxByteCount = 2 * (0xFFFF & args[0]); /* words to bytes */

  DBPRINT(("Ether Get.  "));

  interrupt_mask = sigblock(sigmask(SIGIO)); /* turn off ENET interrupts */
  if (ether_fd >= 0 && (MaxByteCount > 0)) {
    ether_buf = (u_char *)Addr68k_from_LADDR(args[1]);
    ether_bsize = MaxByteCount; /* do this LAST; it enables reads */
    result = get_packet();
    /*	check_ether(); for old behavior, move comment to above line */
  }
  sigsetmask(interrupt_mask); /* interrupts back on */
#endif                        /* NOETHER */

  return (result);
} /* ether_get */

/**********************************************************************
 *	ether_send(args) 175/75/2 max_words,buffer_addr
 *	send a packet
 **********************************************************************/
#define OFFSET sizeof(sa.sa_data)

LispPTR ether_send(LispPTR args[])
{
#ifndef NOETHER
  /*
   *	Send a packet.
   */
  struct sockaddr sa;

  LispPTR MaxByteCount;
  char *BufferAddr; /* buffer address pointer(in native address) */

  MaxByteCount = 2 * (0xFFFF & args[0]); /* words to bytes */
  BufferAddr = (char *)Addr68k_from_LADDR(args[1]);

  if (ether_fd >= 0) {
#ifdef OS4
    struct strbuf ctl, data;

#endif /* OS4 */

    sa.sa_family = AF_UNSPEC; /* required for the NIT protocol */
    bcopy(BufferAddr, sa.sa_data, OFFSET);
#ifndef OS4
    if (sendto(ether_fd, BufferAddr + OFFSET, MaxByteCount - OFFSET, 0, &sa, sizeof(sa)) ==
        (MaxByteCount - OFFSET))
      ;
    else
      perror("Lisp Ether: sendto");
    ether_out++;
#else  /* OS4 */

    ctl.maxlen = ctl.len = sizeof(sa);
    ctl.buf = (char *)&sa;
    data.maxlen = data.len = MaxByteCount - OFFSET;
    data.buf = BufferAddr + OFFSET;
    if (putmsg(ether_fd, &ctl, &data, 0) < 0) {
      perror("Ether_send lost");
      return (NIL);
    } else {
      ether_out++;
      /* flush the buffers to make sure the packet leaves */
      /* maybe we'll use the buffering module some day... */
      ioctl(ether_fd, I_FLUSH, FLUSHW);
    }
#endif /* OS4 */
  }
#endif /* NOETHER */

  return (ATOM_T);
} /* ether_send */

/**********************************************************************
 *	ether_setfilter(args) 175/75/1 filterbits
 *	check whether a packet has come. if does, notify iocb
 **********************************************************************/

LispPTR ether_setfilter(LispPTR args[])
{ return (NIL); } /* ether_setfilter */

/**********************************************************************
 *	ether_debug()
 *	returns the ethernet statistics.
 **********************************************************************/

int estat[3];

int *ether_debug() {
#ifndef NOETHER
  estat[0] = 0;
  if (ether_fd < 0) return (NIL);
  printf("fd %d bsize %d buf %X icb %X in %d out %d\n ", ether_fd, ether_bsize, (int)ether_buf,
         IOPage->dlethernet[3], ether_in, ether_out);
#endif /* NOETHER */

  return (estat);
} /* end ether_debug */

#ifndef NOETHER
static struct timeval EtherTimeout = {0, 0};
#endif /* NOETHER */

/**********************************************************************
 *	check_ether()
 *	checks an incoming packet
 **********************************************************************/

#ifndef OS4
static int nitpos = 0, nitlen = 0; /* for NIT read buffer in OS3 */
#endif

LispPTR check_ether() {
/*
 *	If receiver active then check if any packets are
 *	available from the ethernet.  If so, read the packet
 *	and signal the icb and return T.
 */

#ifndef NOETHER
#ifndef OS4
  static int rfds;
  int result, fromlen;
  struct nit_hdr header;
  int posi, i;
#else  /* OS4 */

  static int rfds;
  int result;
  int i;
  u_long plen;
  struct strbuf ctl, data;
  char ctlbuf[2000];
#endif /* OS4 */

  rfds = EtherReadFds;
#ifndef OS4
  i = 2;
  if (/* select(32, &rfds, NULL, NULL, &EtherTimeout) >= 0 ) */ (1)) {
    if ((ether_fd >= 0) && (ether_bsize > 0)) {
      while ((select(32, &rfds, NULL, NULL, &EtherTimeout) >= 0) && (i-- > 0)) {
        if (nitpos >= nitlen) { /* Used up last NIT buffer full; read another. */
          nitlen = read(ether_fd, nit_buf, sizeof(nit_buf));
          nitpos = 0;
        }
        /* enumerate the NIT headers until the packet is found */
        while (nitpos < nitlen) {
          bcopy(&nit_buf[nitpos], &header, sizeof(header));
          nitpos += sizeof(header);
          switch (header.nh_state) {
            case NIT_CATCH:
              fromlen = header.nh_datalen;
              if (check_filter(&nit_buf[nitpos])) {
                bcopy(&nit_buf[nitpos], &ether_buf[0], fromlen);
                ether_bsize = 0; /* deactivate receiver */
                ether_in++;
                IOPage->dlethernet[3] = fromlen;
                DBPRINT(
                    ("Found packet len %d, at pos %d in buflen %d.\n", fromlen, nitpos, nitlen));
                nitpos += fromlen;
                ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->ETHERInterrupt = 1;
                ETHEREventCount++;
                Irq_Stk_Check = Irq_Stk_End = 0;
                *PENDINGINTERRUPT68k = ATOM_T;
                /* return(NIL); */
                return (ATOM_T);
              }
              nitpos += fromlen;
              break;

            /* ignore all the other header types */
            case NIT_QUIET: break;
            case NIT_NOMBUF: DBPRINT(("No MBUFs\n")); break;
            case NIT_NOCLUSTER: DBPRINT(("No Clusters\n")); break;
            case NIT_NOSPACE: DBPRINT(("No Space\n")); break;
            case NIT_SEQNO: break;
          }
        }
      }
    }
  }
#else  /* OS4 */

  if (ether_fd >= 0 && ether_bsize > 0
      /*   && select(32, &rfds, NULL, NULL, &EtherTimeout) >= 0
       *     -- [on '90/02/14: getsignsldata() chech this] */
      && (rfds & (1 << ether_fd))) {
    data.maxlen = sizeof(nit_buf);
    data.len = 0;
    data.buf = (char *)nit_buf;
    ctl.maxlen = sizeof(ctlbuf);
    ctl.len = 0;
    ctl.buf = ctlbuf;
    plen = 0;
    result = getmsg(ether_fd, &ctl, &data, &plen);
    if (result >= 0) {
      if (data.len <= ether_bsize && data.len > 0) {
        bcopy(nit_buf, &ether_buf[0], data.len);
        ether_bsize = 0;
        ether_in++;
        IOPage->dlethernet[3] = data.len;
        ((INTSTAT *)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->ETHERInterrupt = 1;
        ETHEREventCount++;
        Irq_Stk_Check = Irq_Stk_End = 0;
        *PENDINGINTERRUPT68k = ATOM_T;
        return (NIL); /* return(ATOM_T); */
      }
    } else if (errno != EWOULDBLOCK) {
      perror("Check_ether read error:\n");
    }
  }
#endif /* OS4 */

#endif /* NOETHER */

  return (NIL);
} /* end check_ether */

/************************************************************************/
/*									*/
/*				g e t _ p a c k e t			*/
/*									*/
/*	Try getting a packet, ala check_ether, returning NIL if none,	*/
/*	T if one was received.  This is used by ether_get only.		*/
/*									*/
/************************************************************************/

LispPTR get_packet() {
#ifndef NOETHER
#ifndef OS4
  static int rfds;
  int result, fromlen;
  struct nit_hdr header;
  int posi, i;
#else  /* OS4 */

  static int rfds;
  int result;
  int i;
  u_long plen;
  struct strbuf ctl, data;
  char ctlbuf[2000];
#endif /* OS4 */

#ifndef OS4
  while (1) {
    if (nitpos >= nitlen) { /* Used up last NIT buffer full; read another. */
      nitlen = read(ether_fd, nit_buf, sizeof(nit_buf));
      if (nitlen < 0) return (NIL); /* No more packets to try. */
      nitpos = 0;
    }

    /* enumerate the NIT headers until the packet is found */
    while (nitpos < nitlen) {
      bcopy(&nit_buf[nitpos], &header, sizeof(header));
      nitpos += sizeof(header);
      switch (header.nh_state) {
        case NIT_CATCH:
          fromlen = header.nh_datalen;
          if (check_filter(&nit_buf[nitpos])) {
            bcopy(&nit_buf[nitpos], &ether_buf[0], fromlen);
            ether_bsize = 0; /* deactivate receiver */
            ether_in++;
            IOPage->dlethernet[3] = fromlen;
            DBPRINT(("Found packet len %d, at pos %d in buflen %d.\n", fromlen, nitpos, nitlen));
            nitpos += fromlen;
            return (ATOM_T);
          }
          nitpos += fromlen;
          break;

        /* ignore all the other header types */
        case NIT_QUIET: break;
        case NIT_NOMBUF: DBPRINT(("No MBUFs\n")); break;
        case NIT_NOCLUSTER: DBPRINT(("No Clusters\n")); break;
        case NIT_NOSPACE: DBPRINT(("No Space\n")); break;
        case NIT_SEQNO: break;
      }
    }
  }

#else  /* OS4 */

  data.maxlen = sizeof(nit_buf);
  data.len = 0;
  data.buf = (char *)nit_buf;
  ctl.maxlen = sizeof(ctlbuf);
  ctl.len = 0;
  ctl.buf = ctlbuf;
  plen = 0;
  result = getmsg(ether_fd, &ctl, &data, &plen);
  if (result >= 0) {
    if (data.len <= ether_bsize && data.len > 0) {
      bcopy(nit_buf, &ether_buf[0], data.len);
      ether_bsize = 0;
      ether_in++;
      IOPage->dlethernet[3] = data.len;
      return (ATOM_T);
    }
  } else if (errno != EWOULDBLOCK)
    perror("Check_ether read error:\n");
#endif /* OS4 */

#endif /* NOETHER */

  return (NIL);
} /* end get_packet */

/**********************************************************************
 *	check_filter(buffer)
 *	see if this packet passes the current filter setting
 *	This is believed obsolete with packet filtering enabled
 **********************************************************************/

 int check_filter(u_char *buffer)
{
  /* broadcast packets */
  if (ether_addr_equal(buffer, broadcast)) switch (((short *)buffer)[6]) {
      case PacketTypeIP: return (0);
      case PacketTypeARP: return (0);
      default: return (1);
    };
  /* my address */
  if (ether_addr_equal(buffer, ether_host)) switch (((short *)buffer)[6]) {
      case PacketTypeIP: return (0);
      case PacketTypeARP: return (0);
      default: return (1);
    };
  return (0);
}

/**********************************************************************
 *	ether_addr_equal(add1, add2)
 *	checks ethernet addresses equality
 *	Also believed obsolete
 **********************************************************************/

int ether_addr_equal(u_char add1[], u_char add2[])
{
  register int i;
  for (i = 0; i < 6; i++)
    if (add1[i] != add2[i]) return (0);
  return (1);
}

/**********************************************************************
 *	init_uid()
 *	sets effective user-id to real user-id
 **********************************************************************/
void init_uid() {
#ifndef NOETHER
  int rid;
  rid = getuid();
  seteuid(rid);
#endif /* NOETHER */
}

/************************************************************************/
/*		i n i t _ i f p a g e _ e t h e r			*/
/*									*/
/*      sets Lisp's idea of \my.nsaddress. Clears it if ether not	*/
/*      enabled								*/
/*									*/
/************************************************************************/

void init_ifpage_ether() {
  InterfacePage->nshost0 = (DLword)((ether_host[0] << 8) + ether_host[1]);
  InterfacePage->nshost1 = (DLword)((ether_host[2] << 8) + ether_host[3]);
  InterfacePage->nshost2 = (DLword)((ether_host[4] << 8) + ether_host[5]);
}

#ifndef NOETHER
/* this needs to be a global so the name can be set by main() in Ctest */
/* But NOETHER doesn't support NIT, so dyke it out for NOETHER */
struct sockaddr_nit snit;
#endif /* NOETHER */

/************************************************************************/
/*			    i n i t _ e t h e r				*/
/*      								*/
/*	open nit socket, called from main before starting BCE.		*/
/*      								*/
/************************************************************************/
void init_ether() {
#ifndef NOETHER

  /* JRB - This code will have to be a bit different for SUN 4.0; the			probable
     differences are in commented-out code below
      (not ifdefed because they're untested...)
  */
  int flags;

  /*   ((INTSTAT*)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->ETHERInterrupt = 0;
     ((INTSTAT2 *)Addr68k_from_LADDR(*INTERRUPTSTATE_word))->handledmask = 0;
 */
  if (ether_fd < 0) {
/* it's not open yet, try and open it;
   if it's already open here, it was opened by ldeether and
   all the appropriate stuff was done to it there.
*/
#ifndef OS4
    if (getuid() != geteuid()) {
      if ((ether_fd = socket(AF_NIT, SOCK_RAW, NITPROTO_RAW)) >= 0) {
        /* 4.0: socket -> open("/dev/nit", O_BOTH) */
        /* it's open, now query it and find out its name and address */
        /* JRB - must document that Maiko uses the first net board as
           found by SIOCGIFCONF (see if(4)).  Maybe we need an option to
           specify which net board (suspect more than one net board on a
           Maiko machine will be rare, but...).
        */
        struct ifconf if_data;
        struct ifreq ifbuf[20];

        if_data.ifc_len = sizeof(ifbuf);
        if_data.ifc_req = ifbuf;
        /* 4.0 - before the SIOCGIFCONF, do:
                bzero(ifbuf, sizeof(ifbuf))
        */
        if (ioctl(ether_fd, SIOCGIFCONF, &if_data) < 0) {
          perror("Couldn't GIFCONF socket; Net is off");
#else  /* OS4 */

    if (getuid() != geteuid()) {
      if ((ether_fd = open("/dev/nit", O_RDWR | FASYNC)) >= 0) {
        /* it's open, now query it and find out its name and address */
        /* JRB - must document that LDE uses the first net board as
           found by SIOCGIFCONF (see if(4)).  Maybe we need an option
           to specify which net board (suspect more than one net
           board on an LDE machine will be rare, but...).
        */
        struct ifconf if_data;
        struct ifreq ifbuf[20];

        /* first and foremost, get the packet filter module attached
           (used for ether_suspend and ether_resume) */

        if (ioctl(ether_fd, I_PUSH, "pf") < 0) {
          perror("IOCTL push of pf lost");
          close(ether_fd);
          goto I_Give_Up;
        }
        if_data.ifc_len = sizeof(ifbuf);
        if_data.ifc_req = ifbuf;
        bzero(ifbuf, sizeof(ifbuf));
        {
          /* we have to get the interface name from another socket, since
                  /dev/nit doesn't know anything until it gets bound, and we
                  can't bind it without knowing the interface name... */
          int s;

          if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("No socket for interface name");
            close(s);
#endif /* OS4 */

          close(ether_fd);
          ether_fd = -1;
#ifndef OS4
          return (NIL);
#else  /* OS4 */

            goto I_Give_Up;
#endif /* OS4 */
        }
#ifndef OS4
        /* got its name, copy it into snit */
        strcpy(snit.snit_ifname, if_data.ifc_req[0].ifr_name);
        /* 4.0, before the SIOCGIFADDR, do:
              ioctl(ether_fd, NIOCBIND, &if_data.ifc_req[0])
        */
        /* now for the address */
        if (ioctl(ether_fd, SIOCGIFADDR, &if_data.ifc_req[0]) < 0) {
          perror("Couldn't GIFADDR socket: Net is off");
#else  /* OS4 */

          if (ioctl(s, SIOCGIFCONF, (char *)&if_data) < 0) {
            perror("Couldn't get interface name from socket");
            close(s);
#endif /* OS4 */

          close(ether_fd);
          ether_fd = -1;
#ifndef OS4
          return (NIL);
#else  /* OS4 */

            goto I_Give_Up;
#endif /* OS4 */
        }
#ifndef OS4
        bcopy(if_data.ifc_req[0].ifr_addr.sa_data, ether_host, 6);
        init_uid();
      }
#else  /* OS4 */

          (void)close(s);
#endif /* OS4 */
    }
#ifdef OS4
    if (ioctl(ether_fd, NIOCBIND, &if_data.ifc_req[0]) < 0) {
      perror("Couldn't NIOCBIND socket: Net is off");
      close(ether_fd);
      ether_fd = -1;
      goto I_Give_Up;
    }
    /* now for the address */
    if (ioctl(ether_fd, SIOCGIFADDR, &if_data.ifc_req[0]) < 0) {
      perror("Couldn't GIFADDR socket: Net is off");
      close(ether_fd);
      ether_fd = -1;
      goto I_Give_Up;
    }
    bcopy(if_data.ifc_req[0].ifr_addr.sa_data, ether_host, 6);
    DBPRINT(("init_ether: **** Ethernet starts ****\n"));
  } else {
  I_Give_Up:
    perror("init_ether: can't open NIT socket\n");
    ether_fd = -1;
    exit();
  }
  seteuid(getuid());
}

#endif /* OS4 */
}
if (ether_fd >= 0) {
#ifndef OS4
  /* bind the socket to an interface */
  snit.snit_family = AF_NIT;
  bind(ether_fd, &snit, sizeof(snit));
#else  /* OS4 */

        /* I think all you really have to do here is set the SNAP length, flags,
                and configure the buffering module */
        unsigned long snaplen = 0;
        struct timeval zerotime;
#endif /* OS4 */

#ifndef OS4
  /* establish the operating modes */
  bzero(&nioc, sizeof(nioc));
  nioc.nioc_bufspace = 20000;
  nioc.nioc_chunksize = 50; /* small chunks so each packet read */
  nioc.nioc_typetomatch = NT_ALLTYPES;
  nioc.nioc_snaplen = 32767;
  nioc.nioc_flags = 0;
  if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
    printf("init_ether: ioctl failed\n");
#else  /* OS4 */

        EtherReadFds |= (1 << ether_fd);

        /* first and foremost, flush out ether_fd's buffers and filter it */
        /* install packetfilter that rejects everything */
        if (ioctl(ether_fd, NIOCSETF, &nopf) != 0) {
          perror("init_ether: nopf NIOCSETF failed:\n");
#endif /* OS4 */

    close(ether_fd);
    ether_fd = -1;
    return (NIL);
  }
#ifndef OS4
  EtherReadFds |= (1 << ether_fd);
  if (fcntl(ether_fd, F_SETFL, fcntl(ether_fd, F_GETFL, 0) | FASYNC | FNDELAY) < 0)
    perror("Ether setup SETFLAGS fcntl");
  if (fcntl(ether_fd, F_SETOWN, getpid()) < 0) perror("Ether setup SETOWN");
#else  /* OS4 */

        /* then throw away everything that's currently buffered there;
                this descriptor may have been open since ldeether ran, with
                no filtering; a busy net will have stuffed it full */
        {
          int rfds = EtherReadFds;
          while (select(32, &rfds, NULL, NULL, &EtherTimeout) > 0)
            read(ether_fd, nit_buf, sizeof(nit_buf));
        }

        /* put the address into the packetfilter structure */
        /* DANGER! Vulnerable to byte ordering! DANGER! */
        goodpf.Pf_Filter[8] = *((short *)&ether_host[0]);
        goodpf.Pf_Filter[11] = *((short *)&ether_host[2]);
        goodpf.Pf_Filter[14] = *((short *)&ether_host[4]);
        /* and set up the packetfilter */
        if (ioctl(ether_fd, NIOCSETF, &goodpf) != 0) {
          perror("init_ether: NIOCSETF failed:\n");
          close(ether_fd);
          ether_fd = -1;
          return (NIL);
        }
        /* clobber the flags */
        if (ioctl(ether_fd, NIOCSFLAGS, &snaplen) != 0) {
          perror("init_ether: NIOCSFLAGS failed:\n");
          close(ether_fd);
          ether_fd = -1;
          return (NIL);
        }
        DBPRINT(("INIT ETHER:  Doing I_SETSIG.\n"));
        if (ioctl(ether_fd, I_SETSIG, S_INPUT) != 0) {
          perror("init_ether: I_SETSIG failed:\n");
          close(ether_fd);
          ether_fd = -1;
          return (NIL);
        }
#endif /* OS4 */

  if (EtherReadFds == 0) error("EtherReadFds is zero, but enet opened??");
  LispReadFds |= EtherReadFds;

  DBPRINT(("init_ether: **** Ethernet starts ****\n"));
}
#endif /* NOETHER */
}

#define MASKWORD1 0xffff

/************************************************************************/
/*									*/
/*			c h e c k _ s u m				*/
/*									*/
/*	Implements the CHECKSUM opcode; compute the checksum for an	*/
/*	ethernet packet.						*/
/*									*/
/*	args[0] LispPTR base;						*/
/*	args[1] LispPTR nwords;						*/
/*	args[2] LispPTR initsum;					*/
/*									*/
/*									*/
/************************************************************************/

LispPTR check_sum(LispPTR *args)
{
  register LispPTR checksum;
  register DLword *address;
  register int nwords;

  address = (DLword *)Addr68k_from_LADDR(*args++);
  nwords = *args++;

  if (*args != NIL)
    checksum = (*args) & MASKWORD1;
  else
    checksum = 0;

  for (; nwords > (S_POSITIVE); address++, nwords--) {
    checksum = checksum + GETWORD(address);
    if (checksum > 0xffff) checksum = (checksum & 0xffff) + 1; /* add carry */

    if (checksum > 0x7fff) /* ROTATE LEFT 1 */
      checksum = ((checksum & 0x7fff) << 1) | 1;
    else
      checksum = checksum << 1;
  }

  if (checksum == MASKWORD1)
    return (S_POSITIVE); /* ret 0 */
  else
    return (S_POSITIVE | checksum);

} /*check_sum */
