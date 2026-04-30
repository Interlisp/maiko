/* $Id: ether.c,v 1.4 2001/12/24 01:09:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1996 inclusive Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#if defined(MAIKO_ENABLE_ETHERNET) && defined(USE_PCAP)

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#ifdef MAIKO_OS_SOLARIS
#include <sys/ethernet.h>
#else
#include <net/ethernet.h>
#endif
#include <net/if.h>
#ifdef MAIKO_OS_LINUX
#include <netpacket/packet.h>
#else
#include <net/if_dl.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <fcntl.h>
#include <nlist.h>
#include <pcap/pcap.h>

#include "commondefs.h"
#include "byteswapdefs.h"
#include "lispemul.h"
#include "lispmap.h"
#include "lisp2cdefs.h"
#include "emlglob.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "adr68k.h"
#include "dbprint.h"
#include "etherdefs.h"
#include "ifpage.h"
#include "iopage.h"

extern int      ether_enabled; /* 0/1 should ethernet be turned on */
extern int      ether_fd;      /* file descriptor for ether socket */
extern u_char   ether_host[6]; /* 48 bit address of this node */
extern char     ether_ifname[32]; /* name of ethernet interface */
extern u_char   broadcast[6];
extern int      ether_bsize;   /* if nonzero then a receive is pending */
extern u_char  *ether_buf;     /* address of receive buffer */

extern LispPTR *PENDINGINTERRUPT68k;
extern fd_set   LispReadFds;

#ifndef ETHERTYPE_PUP
#define ETHERTYPE_PUP 0x0200
#endif
#ifndef ETHERTYPE_XNS
#define ETHERTYPE_XNS 0x0600
#endif
#ifndef ETHERTYPE_3TO10
#define ETHERTYPE_3TO10 0x0201
#endif
#ifndef AF_PACKET
#define AF_PACKET (AF_MAX+1)
#endif

static pcap_t *pcap_handle;

/* PCAP packet filter for
 *  "(not arp and not ip) and (ether broadcast or ether dst 11:22:33:44:55:66)"
 * into which we will insert our MAC address in place of 11:22:...
 * Generate with tcpdump -dd '(...)'
 * Could be compiled from the string by pcap_compile(), but then we wouldn't know the
 * offset to substitute our ethernet address - if we used pcap_compile() we might want
 * to just put our ethernet address into the text before compiling.
 *
 * Idea: let's not put the interface in promiscuous mode - then we don't have to test
 * for broadcast and our mac address and this simplifies the setup by quite a bit.
 * We'll still need to get the mac address for use in the interface page.
 */

static struct bpf_insn bpf_insns_none[] = {
  { 0x6, 0, 0, 0x00000000 }
};

static struct bpf_program filter_match_xns;

static struct bpf_program filter_match_none = {
  sizeof(bpf_insns_none) / sizeof(struct bpf_insn),
  bpf_insns_none
};

static int ether_in = 0;  /* number of packets received */
static int ether_out = 0; /* number of packets sent */

/**********************************************************************
 *	recvPacket()
 *	retrieves an ether packet from the network and deposits it
 *      in the expected byte order in ether_buf, and sets ether_bsize
 *      to 0.
 *      Returns: 0 if no packet was read,
 *               length in bytes of the packet if one was read.
 * 
 **********************************************************************/
static int recvPacket() {
  int pcap_rval = 0;
  bpf_u_int32 hlen;
  const uint16_t *packet = NULL;
  struct pcap_pkthdr *header = NULL;

  switch (pcap_next_ex(pcap_handle, &header, (void *)&packet)) {
  case 0:
    return (0);
  case PCAP_ERROR:
    pcap_perror(pcap_handle, "recvPacket");
    return (0);
  }
  hlen = header->len;
  if (hlen > ether_bsize || hlen == 0) return (0);

#ifdef BYTESWAP
  for (int i = 0; i < (hlen + 1) / 2; i++) {
    GETBASEWORD((DLword *)ether_buf, i) = ntohs(packet[i]);
  }
#else
  memcpy(ether_buf, packet, hlen);
#endif

  ether_bsize = 0;
  ((DLETHERCSB *)IOPage->dlethernet)->DLFIRSTICB = hlen;
  return (hlen);
}

/************************************************************************/
/*									*/
/*			e t h e r _ s u s p e n d			*/
/*									*/
/*	Suspend receiving packets.					*/
/*									*/
/************************************************************************/

LispPTR ether_suspend(LispPTR args[])
{
  if (ether_fd == -1) return (NIL);

  /* The trick here is to install a packet filter */
  /* that rejects all packets, I think... 	*/

  pcap_setfilter(pcap_handle, &filter_match_none);
  return (ATOM_T);
} /* ether_suspend */

/************************************************************************/
/*									*/
/*			e t h e r _ r e s u m e				*/
/*									*/
/*	resume receiving packets					*/
/*									*/
/************************************************************************/

LispPTR ether_resume(LispPTR args[])
{
  if (ether_fd == -1) return (NIL);

  /* Install a packet filter that accepts all packets we want */
  pcap_setfilter(pcap_handle, &filter_match_xns);
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
  LispPTR MaxByteCount;
  int length;
  
  DBPRINT(("Ether Get.  "));
  if (ether_fd < 0) return (NIL);
  MaxByteCount = BYTESPER_DLWORD * LispIntToCInt(args[0]); /* words to bytes */
  if (MaxByteCount == 0) return (NIL);
  ether_buf = (u_char *)NativeAligned2FromLAddr(args[1]);
  ether_bsize = MaxByteCount; /* do this LAST; it enables reads */
  length = recvPacket();
  if (length == 0) return (NIL);
  return (T);
} /* ether_get */

/**********************************************************************
 *	ether_send(args) 175/75/2 max_words,buffer_addr
 *	send a packet
 **********************************************************************/
LispPTR ether_send(LispPTR args[])
{
  DLword wordCount;
  DLword *bufferAddr; /* buffer address pointer(in native address) */
#ifdef BYTESWAP
  DLword networkOrderBuffer[750];
#endif
  if (ether_fd < 0) return (NIL);
  wordCount = LispIntToCInt(args[0]);
  bufferAddr = NativeAligned2FromLAddr(args[1]);
#ifdef BYTESWAP
  for (int i = 0; i < wordCount; i++) {
    networkOrderBuffer[i] = htons(GETBASEWORD(bufferAddr, i));
  }
  bufferAddr = networkOrderBuffer;
#endif
  pcap_inject(pcap_handle, bufferAddr, 2 * wordCount);
  ether_out++;
  return (ATOM_T);
} /* ether_send */

/**********************************************************************
 *	ether_setfilter(args) 175/75/1 filterbits
 *	check whether a packet has come. if does, notify iocb
 **********************************************************************/

LispPTR ether_setfilter(LispPTR args[])
{ return (NIL); }
/* ether_setfilter */

/**********************************************************************
 *	ether_debug()
 *	returns the ethernet statistics.
 **********************************************************************/

int estat[3];

int *ether_debug() {
  estat[0] = 0;
  if (ether_fd < 0) return (NIL);
  printf("fd %d bsize %d buf %p icb %X in %d out %d\n ", ether_fd, ether_bsize, (void *)ether_buf,
         ((DLETHERCSB *)IOPage->dlethernet)->DLFIRSTICB, ether_in, ether_out);
  return (estat);
} /* end ether_debug */

/**********************************************************************
 *	check_ether()
 *	checks for an incoming packet
 **********************************************************************/

LispPTR check_ether() {
/*
 * If the receiver is active then check if any packets are available
 * from the ethernet, with the side-effect of reading a packet
 * and setting the length in the ethernet ICB.
 * Returns T if a packet was read, otherwise NIL.
 */

  if (ether_fd < 0 || ether_bsize == 0) return (NIL);

  return (0 == recvPacket()) ? NIL : ATOM_T;
} /* end check_ether */

/************************************************************************/
/*			    i n i t _ e t h e r				*/
/*      								*/
/*	open pcap handle, called from main before starting BCE.		*/
/*      								*/
/************************************************************************/
void init_ether() {
  /* Uses the following pcap library routines - 
   * pcap_create => gets a pcap_t handle
   * pcap_findalldevs => list of network devices to capture on
   * pcap_freealldevs => free the list of network devices from pcap_findalldevs
   * pcap_set_snaplen => set the snapshot length for  a  not-yet-activated  pcap_t handle
   * pcap_set_buffer_size => set  buffer  size for a not-yet-activated pcap_t handle
   * pcap_get_selectable_fd => fd to use in select/poll
   * pcap_setnonblock => put pcap_t into non-blocking mode
   * pcap_compile => compile filter
   * pcap_setfilter => set filter on a pcap_t handle
   * pcap_inject/pcap_sendpacket => transmit a packet
   */

  int pcap_rval = 0;
  //  char *etherdev = getenv("LDEETHERDEV"); /* name of interface we want */
  char devname[32] = {0};
  char errbuf[PCAP_ERRBUF_SIZE] = {0};
  char filter_exp[256];
  
  /* ethernet may already be initialized - don't do it again */
  if (ether_fd >= 0 || ether_enabled == 0) return;

  /* select an ethernet interface if one was not provided */
  if (strlen(ether_ifname) == 0) {
    pcap_if_t *alldevs = NULL;
    pcap_if_t *dev = NULL;
    pcap_rval = pcap_findalldevs(&alldevs, errbuf);
    for (pcap_if_t *d = alldevs; d; d = d->next) {
      if ((d->flags & PCAP_IF_UP) && (d->flags & PCAP_IF_RUNNING) && !(d->flags & PCAP_IF_LOOPBACK)) {
        dev = d;
        break;
      }
    }
    /* did we find one */
    if (dev) {
      strlcpy(ether_ifname, dev->name, sizeof(ether_ifname));
      for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
#if defined(MAIKO_OS_SOLARIS)        
        /* Solaris defines both AF_LINK and AF_PACKET but neither give us
         * the desired result
         */
#elif defined(AF_LINK)
        /* this is BSD-like, macOS? */
        if (a->addr->sa_family == AF_LINK) {
          memcpy(ether_host, LLADDR(((struct sockaddr_dl *)(a->addr))), sizeof(ether_host));
          break;
        }
#elif defined(AF_PACKET)
        /* this is Linux-like */
        if (a->addr->sa_family == AF_PACKET) {
          memcpy(ether_host, ((struct sockaddr_ll *)(a->addr))->sll_addr, sizeof(ether_host));
          break;
        }
#else
#warning Neither AF_LINK nor AF_PACKET address families defined
#endif
      }
    }
    pcap_freealldevs(alldevs);
  }
  pcap_handle = pcap_create(ether_ifname, errbuf);
  if (strlen(errbuf) > 0) {
    fprintf(stderr, "%s\n", errbuf);
    return;
  }
  /* set up properties on the pcap_handle and then activate it */
  pcap_rval = pcap_set_immediate_mode(pcap_handle, 1);
  pcap_rval = pcap_set_buffer_size(pcap_handle, 65536);
  pcap_rval = pcap_set_snaplen(pcap_handle, 1518);
  pcap_rval = pcap_setfilter(pcap_handle, &filter_match_none);
  pcap_rval = pcap_activate(pcap_handle);
  if (pcap_rval != 0) {
    pcap_perror(pcap_handle, "pcap_activate: ");
  }
  if (pcap_rval < 0) {
    pcap_close(pcap_handle);
    return;
  }
  snprintf(filter_exp, sizeof(filter_exp), "ether proto 0x600 and (ether multicast or ether dst %02x:%02x:%02x:%02x:%02x:%02x)",
           ether_host[0], ether_host[1], ether_host[2], ether_host[3], ether_host[4], ether_host[5]);
  pcap_rval = pcap_compile(pcap_handle, &filter_match_xns, filter_exp, 0, PCAP_NETMASK_UNKNOWN);
  if (pcap_rval == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(pcap_handle));
    pcap_close(pcap_handle);
    return;
  }
  pcap_rval = pcap_setfilter(pcap_handle, &filter_match_xns);

  pcap_rval = pcap_setnonblock(pcap_handle, 1, errbuf);
  ether_fd = pcap_get_selectable_fd(pcap_handle);
  FD_SET(ether_fd, &LispReadFds);

  printf("Ethernet starts on interface %s at %02x:%02x:%02x:%02x:%02x:%02x\n",
         ether_ifname, ether_host[0], ether_host[1], ether_host[2],
         ether_host[3],ether_host[4],ether_host[5]);
  DBPRINT(("init_ether: **** Ethernet starts ****\n"));
}
#endif /* defined(MAIKO_ENABLE_ETHERNET) && defined(USE_PCAP) */
