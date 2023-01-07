/* $Id: ether.c,v 1.4 2001/12/24 01:09:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1996 inclusive Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#if defined(MAIKO_ENABLE_ETHERNET) && !defined(MAIKO_ENABLE_NETHUB)

#include "version.h"

#if defined(USE_DLPI)
#define PKTFILTER 1
#define NIOCSFLAGS SBIOCSFLAGS
#endif
#ifdef OS4
#define PKTFILTER 1
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifndef DOS
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#ifdef MAIKO_ENABLE_ETHERNET
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#if defined(USE_DLPI)
#include "dlpidefs.h"
#elif defined(USE_NIT)
#include <net/nit.h>
#endif

#include <sys/ioctl.h>
#ifdef PKTFILTER
#include <stropts.h>
#include <fcntl.h>
#ifdef OS4
#include <net/nit_if.h>
#include <net/nit_pf.h>
/* #include <net/nit_buf.h> */
#include <net/packetfilt.h>
#else
#include <sys/pfmod.h>
#include <sys/bufmod.h>
#endif /* OS4 */
#endif /* PKTFILTER */

#if defined(USE_NIT)
#include <sys/mbuf.h>
#endif
#include <nlist.h>
#endif /* DOS */
#endif /* MAIKO_ENABLE_ETHERNET */

#include "commondefs.h"
#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "adr68k.h"
#include "ether.h"
#include "dbprint.h"
#include "etherdefs.h"
#include "ifpage.h"

#if defined(USE_DLPI)
#define NIOCSETF PFIOCSETF
#endif

extern int      ether_fd;      /* file descriptor for ether socket */
static int      ether_intf_type = 0;
extern u_char   ether_host[6]; /* 48 bit address of this node */
extern const u_char   broadcast[6];
extern int      ether_bsize;   /* if nonzero then a receive is pending */
extern u_char  *ether_buf;     /* address of receive buffer */
static u_char   nit_buf[3000]; /* the current chunk read from NIT (one packet) */
extern LispPTR *PENDINGINTERRUPT68k;
extern fd_set   LispReadFds;

extern int ETHEREventCount;

#define PacketTypeIP 0x0800
#define PacketTypeARP 0x0806
#define PacketTypeRARP 0x8035
#define PacketTypeXNS 0x0600
#define PacketTypePUP 0x0200
#define PacketType3TO10 0x0201

#ifdef MAIKO_ENABLE_ETHERNET
#ifdef PKTFILTER
/* the receiving packetfilter structure */
/* if this is changed, be sure to get the references to it in init_ether
        and check the length (the second entry in the structure) */
/*
 PACKET FILTERS
     A packet filter consists of the filter command  list  length
     (in  units of u_shorts), and the filter command list itself.
     (The priority field  mentioned  above  is  ignored  in  this
     implementation.)   Each  filter  command  list  specifies  a
     sequence of actions that operate on  an  internal  stack  of
     u_shorts ("shortwords").  Each shortword of the command list
     specifies one of the actions ENF_PUSHLIT,  ENF_PUSHZERO,  or
     ENF_PUSHWORD+n,  which  respectively push the next shortword
     of the command list, zero, or shortword  n  of  the  subject
     message  on  the stack, and a binary operator from the set {
     ENF_EQ, ENF_NEQ, ENF_LT, ENF_LE,  ENF_GT,  ENF_GE,  ENF_AND,
     ENF_OR,  ENF_XOR  }  which then operates on the top two ele-
     ments of the stack and replaces them with its result.   When
     both an action and operator are specified in the same short-
     word, the action is performed followed by the operation.
     The binary operator can also be  from  the  set  {  ENF_COR,
     ENF_CAND,  ENF_CNOR, ENF_CNAND }.  These are "short-circuit"
     operators, in that  they  terminate  the  execution  of  the
     filter immediately if the condition they are checking for is
     found, and continue otherwise.  All pop  two  elements  from
     the  stack  and  compare them for equality; ENF_CAND returns
     false if the result is false; ENF_COR returns  true  if  the
     result  is  true;  ENF_CNAND  returns  true if the result is
     false; ENF_CNOR returns false if the result is true.  Unlike
     the other binary operators, these four do not leave a result
     on the stack, even if they continue.
     The short-circuit operators should be used when possible, to
     reduce  the  amount  of time spent evaluating filters.  When
     they are used, you should also  arrange  the  order  of  the
     tests  so  that  the  filter will succeed or fail as soon as
     possible; for example, checking the IP destination field  of
     a  UDP  packet  is  more likely to indicate failure than the
     packet type field.
     The special  action  ENF_NOPUSH  and  the  special  operator
     ENF_NOP  can be used to only perform the binary operation or
     to only push a value on the stack.   Since  both  are  (con-
     veniently)  defined  to  be  zero, indicating only an action
     actually specifies the action followed by ENF_NOP, and indi-
     cating  only an operation actually specifies ENF_NOPUSH fol-
     lowed by the operation.
     After executing the filter command list,  a  non-zero  value
     (true)  left  on top of the stack (or an empty stack) causes
     the incoming packet to be accepted and a zero value  (false)
     causes  the  packet to be rejected.  (If the filter exits as
     the result of a  short-circuit  operator,  the  top-of-stack
     value  is  ignored.)  Specifying  an  undefined operation or
     action in the command list or performing an  illegal  opera-
     tion  or action (such as pushing a shortword offset past the
     end of the packet or executing a binary operator with  fewer
     than  two shortwords on the stack) causes a filter to reject
     the packet.
*/
/* ethertype == IP => reject
   ethertype == ARP => reject
   (dest[0:1] == us &&
   dest[2:3] == us &&
   dest[4:5] == us)
   ||
   (dest[0:1] == broadcast &&
   dest[2:3] == broadcast &&
   dest[4:5] == broadcast)
*/
struct packetfilt goodpf = {0,
                            29,
                            {ENF_PUSHWORD + 6,		/* Ethertype field */
                             ENF_PUSHLIT + ENF_CNOR,	
                             PacketTypeIP, /* punt if PacketTypeIP = 0x0800 */
                             ENF_PUSHWORD + 6,
                             ENF_PUSHLIT + ENF_CNOR,
                             PacketTypeARP, /* or PacketTypeARP = 0x0806 */
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

/* 	struct packetfilt allpf = {0, 1, {ENF_PUSHFFFF}}; */

/* a filter that rejects all packets to be used by ether_suspend and the
        initial routine to flush out ether_fd */
struct packetfilt nopf = {0, 1, {ENF_PUSHZERO}};

#endif /* PKTFILTER */

int ether_in = 0;  /* number of packets received */
int ether_out = 0; /* number of packets sent */
#ifndef PKTFILTER
static struct nit_ioc nioc;
#endif /* PKTFILTER */

#endif /* MAIKO_ENABLE_ETHERNET */

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
#ifdef MAIKO_ENABLE_ETHERNET
#ifdef PKTFILTER
  static struct packetfilt pf = {0, 1, {ENF_PUSHZERO}};
  struct strioctl si;
#endif /* PKTFILTER */

  if (ether_fd == -1) return (NIL);
#ifndef PKTFILTER
  nioc.nioc_typetomatch = NT_NOTYPES;
  if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
    printf("ether_suspend: ioctl failed\n");
    return (NIL);
  }
#else /* PKTFILTER */

/* The trick here is to install a packet filter */
/* that rejects all packets, I think... 	*/
#if defined(USE_DLPI)

  si.ic_cmd = PFIOCSETF;
  si.ic_timout = -1;
  si.ic_len = sizeof(nopf);
  si.ic_dp = (char *)&nopf;

  if (ioctl(ether_fd, I_STR, &si) < 0) {
    perror("ether_suspend nopf ioctl: I_STR PFIOCSETF");
    return (NIL);
  }

#elif defined(USE_NIT)
  if (ioctl(ether_fd, NIOCSETF, &nopf) != 0) {
    perror("ether_suspend: NIOCSETF failed\n");
    return (NIL);
  }
#endif /* USE_DLPI */
#endif /* PKTFILTER */
#endif /* MAIKO_ENABLE_ETHERNET */

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
#ifdef MAIKO_ENABLE_ETHERNET
  struct strioctl si;
  if (ether_fd == -1) return (NIL);
#ifndef PKTFILTER
  nioc.nioc_typetomatch = NT_ALLTYPES;
  if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
    printf("ether_resume: ioctl failed\n");
    return (NIL);
  }
#else /* PKTFILTER */

/* Install a packet filter that accepts all packets we want */
#if defined(USE_DLPI)

  si.ic_cmd = PFIOCSETF;
  si.ic_timout = -1;
  si.ic_len = sizeof(goodpf);
  si.ic_dp = (char *)&goodpf;

  if (ioctl(ether_fd, I_STR, &si) < 0) {
    perror("ether_resume goodpf ioctl: I_STR PFIOCSETF");
    return (NIL);
  }

#elif defined(USE_NIT)
  if (ioctl(ether_fd, NIOCSETF, &goodpf) != 0) {
    perror("ether_resume: NIOCSETF failed\n");
    return (NIL);
  }
#endif /* USE_DLPI */
#endif /* PKTFILTER */

#endif /* MAIKO_ENABLE_ETHERNET */

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
  LispPTR result = NIL;
#ifdef MAIKO_ENABLE_ETHERNET
  LispPTR MaxByteCount;
  sigset_t signals;

  MaxByteCount = 2 * (0xFFFF & args[0]); /* words to bytes */

  DBPRINT(("Ether Get.  "));

  sigemptyset(&signals);
  sigaddset(&signals, SIGIO);

  /* turn off ENET interrupts */
  sigprocmask(SIG_BLOCK, &signals, NULL);

  if (ether_fd > 0 && (MaxByteCount > 0)) {
    ether_buf = (u_char *)NativeAligned2FromLAddr(args[1]);
    ether_bsize = MaxByteCount; /* do this LAST; it enables reads */
    result = get_packet();
    /*	check_ether(); for old behavior, move comment to above line */
  }

  /* enable interrupts */
  sigprocmask(SIG_UNBLOCK, &signals, NULL);

#endif /* MAIKO_ENABLE_ETHERNET */

  return (result);
} /* ether_get */

/**********************************************************************
 *	ether_send(args) 175/75/2 max_words,buffer_addr
 *	send a packet
 **********************************************************************/
#define OFFSET sizeof(sa.sa_data)

LispPTR ether_send(LispPTR args[])
{
#ifdef MAIKO_ENABLE_ETHERNET
  /*
   *	Send a packet.
   */
  struct sockaddr sa;

  LispPTR MaxByteCount;
  u_char *BufferAddr; /* buffer address pointer(in native address) */

  MaxByteCount = 2 * (0xFFFF & args[0]); /* words to bytes */
  BufferAddr = (u_char *)NativeAligned2FromLAddr(args[1]);

  if (ether_fd > 0) {
#ifdef PKTFILTER
    struct strbuf ctl, data;

#endif /* PKTFILTER */

    sa.sa_family = AF_UNSPEC; /* required for the NIT protocol */
    memcpy(sa.sa_data, BufferAddr, OFFSET);
#ifndef PKTFILTER
    if (sendto(ether_fd, BufferAddr + OFFSET, MaxByteCount - OFFSET, 0, &sa, sizeof(sa)) ==
        (MaxByteCount - OFFSET))
      ;
    else
      perror("Lisp Ether: sendto");
    ether_out++;
#elif defined(USE_DLPI)

    if (dlunitdatareq(ether_fd, BufferAddr, 6, 0, 0, BufferAddr, MaxByteCount)) {
      perror("unitdatareq");
      return (NIL);
    }
    ether_out++;
    ioctl(ether_fd, I_FLUSH, FLUSHW);

#else  /* PKTFILTER */

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
#endif /* PKTFILTER */
  }
#endif /* MAIKO_ENABLE_ETHERNET */

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

int *ether_debug(void) {
#ifdef MAIKO_ENABLE_ETHERNET
  estat[0] = 0;
  if (ether_fd < 0) return (NIL);
  printf("fd %d bsize %d buf %p icb %X in %d out %d\n ", ether_fd, ether_bsize, (void *)ether_buf,
         IOPage->dlethernet[3], ether_in, ether_out);
#endif /* MAIKO_ENABLE_ETHERNET */

  return (estat);
} /* end ether_debug */

#ifdef MAIKO_ENABLE_ETHERNET
static struct timeval EtherTimeout = {0, 0};
#endif /* MAIKO_ENABLE_ETHERNET */

/**********************************************************************
 *	check_ether()
 *	checks an incoming packet
 **********************************************************************/

#ifdef MAIKO_ENABLE_ETHERNET
#ifndef PKTFILTER
static int nitpos = 0, nitlen = 0; /* for NIT read buffer in OS3 */
#endif
#endif

LispPTR check_ether(void) {
/*
 *	If receiver active then check if any packets are
 *	available from the ethernet.  If so, read the packet
 *	and signal the icb and return T.
 */

#ifdef MAIKO_ENABLE_ETHERNET
#ifndef PKTFILTER
  fd_set rfds;
  int result, fromlen;
  struct nit_hdr header;
  int posi, i;
#else  /* PKTFILTER */

  fd_set rfds;
  int result;
  int i;
  int plen;
  struct strbuf ctl, data;
  char ctlbuf[2000];
#endif /* PKTFILTER */

  FD_SET(ether_fd, &rfds);
#ifndef PKTFILTER
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
          memcpy(&header, &nit_buf[nitpos], sizeof(header));
          nitpos += sizeof(header);
          switch (header.nh_state) {
            case NIT_CATCH:
              fromlen = header.nh_datalen;
              if (check_filter(&nit_buf[nitpos])) {
                memcpy(&ether_buf[0], &nit_buf[nitpos], fromlen);
                ether_bsize = 0; /* deactivate receiver */
                ether_in++;
                IOPage->dlethernet[3] = fromlen;
                DBPRINT(
                    ("Found packet len %d, at pos %d in buflen %d.\n", fromlen, nitpos, nitlen));
                nitpos += fromlen;
                ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->ETHERInterrupt = 1;
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
#else  /* PKTFILTER */

  if (ether_fd >= 0 && ether_bsize > 0
      /*   && select(32, &rfds, NULL, NULL, &EtherTimeout) >= 0
       *     -- [on '90/02/14: getsignsldata() check this] */
      && (FD_ISSET(ether_fd, &rfds))) {
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
        memcpy(&ether_buf[0], nit_buf, data.len);
        ether_bsize = 0;
        ether_in++;
        IOPage->dlethernet[3] = data.len;
        ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->ETHERInterrupt = 1;
        ETHEREventCount++;
        Irq_Stk_Check = Irq_Stk_End = 0;
        *PENDINGINTERRUPT68k = ATOM_T;
        return (NIL); /* return(ATOM_T); */
      }
    } else if (errno != EWOULDBLOCK) {
      perror("Check_ether read error:\n");
    }
  }
#endif /* PKTFILTER */

#endif /* MAIKO_ENABLE_ETHERNET */

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

LispPTR get_packet(void) {
#ifdef MAIKO_ENABLE_ETHERNET
#ifndef PKTFILTER
  fd_set rfds;
  int result, fromlen;
  struct nit_hdr header;
  int posi, i;
#else  /* PKTFILTER */

  fd_set rfds;
  int result;
  int i;
  int plen;
  struct strbuf ctl, data;
  char ctlbuf[2000];
#endif /* PKTFILTER */

#ifndef PKTFILTER
  while (1) {
    if (nitpos >= nitlen) { /* Used up last NIT buffer full; read another. */
      nitlen = read(ether_fd, nit_buf, sizeof(nit_buf));
      if (nitlen < 0) return (NIL); /* No more packets to try. */
      nitpos = 0;
    }

    /* enumerate the NIT headers until the packet is found */
    while (nitpos < nitlen) {
      memcpy(&header, &nit_buf[nitpos], sizeof(header));
      nitpos += sizeof(header);
      switch (header.nh_state) {
        case NIT_CATCH:
          fromlen = header.nh_datalen;
          if (check_filter(&nit_buf[nitpos])) {
            memcpy(&ether_buf[0], &nit_buf[nitpos], fromlen);
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

#else  /* PKTFILTER */

  data.maxlen = sizeof(nit_buf);
  data.len = 0;
  data.buf = (char *)nit_buf;
  ctl.maxlen = sizeof(ctlbuf);
  ctl.len = 0;
  ctl.buf = ctlbuf;
  plen = 0;
  result = getmsg(ether_fd, &ctl, &data, &plen);
  if (result >= 0) {
    if (ctl.len > 0) printf("ctl msg rcvd.\n");
    if (data.len <= ether_bsize && data.len > 0) {
      memcpy(&ether_buf[0], nit_buf, data.len);
      ether_bsize = 0;
      ether_in++;
      IOPage->dlethernet[3] = data.len;
      return (ATOM_T);
    }
  } else if (errno != EWOULDBLOCK)
    perror("Check_ether read error:\n");
#endif /* PKTFILTER */

#endif /* MAIKO_ENABLE_ETHERNET */

  return (NIL);
} /* end get_packet */

#ifdef MAIKO_ENABLE_ETHERNET
/**********************************************************************
 *	ether_addr_equal(add1, add2)
 *	checks ethernet addresses equality
 *	Also believed obsolete
 **********************************************************************/

static int ether_addr_equal(u_char add1[], u_char add2[])
{
  int i;
  for (i = 0; i < 6; i++)
    if (add1[i] != add2[i]) return (0);
  return (1);
}

/**********************************************************************
 *	check_filter(buffer)
 *	see if this packet passes the current filter setting
 *	This is believed obsolete with packet filtering enabled
 **********************************************************************/

static int check_filter(u_char *buffer)
{
  /* broadcast packets */
  if (ether_addr_equal(buffer, broadcast)) switch (((short *)buffer)[6]) {
      case PacketTypeIP: return (0);
      case PacketTypeARP: return (0);
      default: return (1);
    }
  /* my address */
  if (ether_addr_equal(buffer, ether_host)) switch (((short *)buffer)[6]) {
      case PacketTypeIP: return (0);
      case PacketTypeARP: return (0);
      default: return (1);
    }
  return (0);
}

/**********************************************************************
 *	init_uid()
 *	sets effective user-id to real user-id
 **********************************************************************/
static void init_uid(void) {
  int rid;
  rid = getuid();
  setuid(rid);
}

/* this needs to be a global so the name can be set by main() in Ctest */
/* But MAIKO_ENABLE_ETHERNET doesn't support NIT, so dyke it out for MAIKO_ENABLE_ETHERNET */
#if defined(USE_NIT)
struct sockaddr_nit snit;
#endif /* USE_NIT */
#endif /* MAIKO_ENABLE_ETHERNET */

/************************************************************************/
/*			    i n i t _ e t h e r				*/
/*      								*/
/*	open nit socket, called from main before starting BCE.		*/
/*      								*/
/************************************************************************/
void init_ether(void) {
#ifdef MAIKO_ENABLE_ETHERNET

  /* JRB - This code will have to be a bit different for SUN 4.0; the			probable
     differences are in commented-out code below
      (not ifdefed because they're untested...)
  */
  struct strioctl si;
  unsigned long snaplen = 0;

  /*   ((INTSTAT*)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->ETHERInterrupt = 0;
     ((INTSTAT2 *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word))->handledmask = 0;
 */

  if (ether_fd < 0) {
/* it's not open yet, try and open it;
   if it's already open here, it was opened by ldeether and
   all the appropriate stuff was done to it there.
*/
#if defined(USE_DLPI)
    /* Use DLPI to connect to the ethernet.  This code is stolen
       from NFSWATCH4.3
    */

    /* if (getuid() != geteuid()) */
    {
      if ((ether_fd = setup_dlpi_dev(NULL)) > 0) { /* Open an ether interface */

        ether_intf_type = dlpi_devtype(ether_fd);

        /* first and foremost, get the packet filter module attached
               (used for ether_suspend and ether_resume) */

        if (ioctl(ether_fd, I_PUSH, "pfmod") < 0) {
          perror("IOCTL push of pf lost");
          close(ether_fd);
          goto I_Give_Up;
        }

        si.ic_cmd = PFIOCSETF;
        si.ic_timout = -1;
        si.ic_len = sizeof(nopf);
        si.ic_dp = (char *)&nopf;

        if (ioctl(ether_fd, I_STR, &si) < 0) {
          perror("ioctl: I_STR PFIOCSETF");
          return;
        }

        fcntl(ether_fd, F_SETFL, fcntl(ether_fd, F_GETFL, 0) | O_NONBLOCK);

      } else {
      I_Give_Up:
        /* JDS 991228 remove	perror("Can't open network; XNS unavailable.\n"); */
        ether_fd = -1;
      }
      setuid(getuid());
    }
#elif defined(USE_NIT)
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
                memset(ifbuf, 0, sizeof(ifbuf))
        */
        if (ioctl(ether_fd, SIOCGIFCONF, &if_data) < 0) {
          perror("Couldn't GIFCONF socket; Net is off");
#else  /* OS4 */

    if (getuid() != geteuid()) {
      if ((ether_fd = open("/dev/nit", O_RDWR | O_ASYNC)) >= 0) {
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
        memset(ifbuf, 0, sizeof(ifbuf));
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
          return;
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
        memcpy(ether_host, if_data.ifc_req[0].ifr_addr.sa_data, 6);
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
    memcpy(ether_host, if_data.ifc_req[0].ifr_addr.sa_data, 6);
    DBPRINT(("init_ether: **** Ethernet starts ****\n"));
  } else {
  I_Give_Up:
    perror("Can't open network; XNS unavailable.\n");
    ether_fd = -1;
  }
  setuid(getuid());
}

#endif /* OS4 */
#endif /* USE_DLPI */
  }

  /******************************/
  /* Have now either opened enet here, or in ldeether */
  /***************************************************/

  if (ether_fd >= 0) {
#ifndef PKTFILTER
    /* bind the socket to an interface */
    snit.snit_family = AF_NIT;
    bind(ether_fd, &snit, sizeof(snit));
#else  /* PKTFILTER */

  /* I think all you really have to do here is set the SNAP length, flags,
          and configure the buffering module */
  struct timeval zerotime;
#endif /* PKTFILTER */

#ifndef PKTFILTER
    /* establish the operating modes */
    memset(&nioc, 0, sizeof(nioc));
    nioc.nioc_bufspace = 20000;
    nioc.nioc_chunksize = 50; /* small chunks so each packet read */
    nioc.nioc_typetomatch = NT_ALLTYPES;
    nioc.nioc_snaplen = 32767;
    nioc.nioc_flags = 0;
    if (ioctl(ether_fd, SIOCSNIT, &nioc) != 0) {
      printf("init_ether: ioctl failed\n");
      close(ether_fd);
      ether_fd = -1;
      return;
    }
#else /* PKTFILTER */

/* first and foremost, flush out ether_fd's buffers and filter it */
/* install packetfilter that rejects everything */
#if defined(USE_DLPI)

  si.ic_cmd = PFIOCSETF;
  si.ic_timout = -1;
  si.ic_len = sizeof(nopf);
  si.ic_dp = (char *)&nopf;

  if (ioctl(ether_fd, I_STR, &si) < 0) {
    perror("init_ether nopf ioctl: I_STR PFIOCSETF");
    close(ether_fd);
    ether_fd = -1;
    return;
  }

#elif defined(USE_NIT)
        if (ioctl(ether_fd, NIOCSETF, &nopf) != 0) {
          perror("init_ether: nopf NIOCSETF failed:\n");

          close(ether_fd);
          ether_fd = -1;
          return;
        }
#endif /* USE_DLPI */
#endif /* PKTFILTER -- jds 23 sep 96 unmatched if fix */
#ifndef PKTFILTER
    if (fcntl(ether_fd, F_SETFL, fcntl(ether_fd, F_GETFL, 0) | O_ASYNC | O_NONBLOCK) < 0)
      perror("Ether setup SETFLAGS fcntl");
    if (fcntl(ether_fd, F_SETOWN, getpid()) < 0) perror("Ether setup SETOWN");
#else /* PKTFILTER */

/* then throw away everything that's currently buffered there;
        this descriptor may have been open since ldeether ran, with
        no filtering; a busy net will have stuffed it full */
#if defined(USE_DLPI)
  if (ioctl(ether_fd, I_FLUSH, (char *)FLUSHR) < 0) { perror("init_ether I_FLUSH"); }
#elif defined(USE_NIT)
        {
          FD_SET(ether_fd, &rfds);
          while (select(32, &rfds, NULL, NULL, &EtherTimeout) > 0)
            read(ether_fd, nit_buf, sizeof(nit_buf));
        }
#endif /* USE_DLPI */

  /* put the address into the packetfilter structure */
  /* DANGER! Vulnerable to byte ordering! DANGER! */
  goodpf.Pf_Filter[8] = (DLword)((ether_host[0] << 8) + ether_host[1]);
  goodpf.Pf_Filter[11] = (DLword)((ether_host[2] << 8) + ether_host[3]);
  goodpf.Pf_Filter[14] = (DLword)((ether_host[4] << 8) + ether_host[5]);
/* and set up the packetfilter */
#if defined(USE_DLPI)

  si.ic_cmd = PFIOCSETF;
  si.ic_timout = -1;
  si.ic_len = sizeof(goodpf);
  si.ic_dp = (char *)&goodpf;

  if (ioctl(ether_fd, I_STR, &si) < 0) {
    perror("init_ether goodpf ioctl: I_STR PFIOCSETF");
    close(ether_fd);
    ether_fd = -1;
    return;
  }

#else
        if (ioctl(ether_fd, NIOCSETF, &goodpf) != 0) {
          perror("init_ether: NIOCSETF failed:\n");
          close(ether_fd);
          ether_fd = -1;
          return;
        }
#endif /* USE_DLPI */
#if defined(USE_NIT)
  DBPRINT(("INIT ETHER:  Doing I_SETSIG.\n"));
  if (ioctl(ether_fd, I_SETSIG, S_INPUT) != 0) {
    perror("init_ether: I_SETSIG failed:\n");
    close(ether_fd);
    ether_fd = -1;
    return;
  }
#endif /* USE_NIT */
#endif /* PKTFILTER */

    if (ether_fd < 0) error ("ether_fd is -1, but enet opened??");
    FD_SET(ether_fd, &LispReadFds);

    DBPRINT(("init_ether: **** Ethernet starts ****\n"));
  }
#endif /* MAIKO_ENABLE_ETHERNET */
}

#endif /* defined(MAIKO_ENABLE_ETHERNET) */
