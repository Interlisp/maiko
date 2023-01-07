/* $Id: ether.c,v 1.4 2001/12/24 01:09:02 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1996 inclusive Venue. All Rights Reserved.	*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <unistd.h>
#include <sys/types.h>

#include "lsptypes.h"
#include "lspglob.h"
#include "adr68k.h"
#include "etherdefs.h"

/*
 * global variables exported to ether_*.c and possibly others
 */

extern int     ether_fd;
extern u_char  ether_host[6];
extern const u_char  broadcast[6];
extern int     ether_bsize;
extern u_char *ether_buf;
extern int     ETHEREventCount;

int     ether_fd      = -1;    /* file descriptor for ether socket */

u_char  ether_host[6] = {0, 0, 0, 0, 0, 0}; /* 48 bit address of this node */
const u_char  broadcast[6]  = {255, 255, 255, 255, 255, 255};

int     ether_bsize   = 0;     /* if nonzero then a receive is pending */
u_char *ether_buf     = NULL;  /* address of receive buffer */

int     ETHEREventCount = 0;


/*
 * public procedures
 */

/************************************************************************/
/*              i n i t _ i f p a g e _ e t h e r                       */
/*                                                                      */
/*      sets Lisp's idea of \my.nsaddress. Clears it if ether not       */
/*      enabled                                                         */
/*                                                                      */
/************************************************************************/

void init_ifpage_ether(void) {
  InterfacePage->nshost0 = (DLword)((ether_host[0] << 8) + ether_host[1]);
  InterfacePage->nshost1 = (DLword)((ether_host[2] << 8) + ether_host[3]);
  InterfacePage->nshost2 = (DLword)((ether_host[4] << 8) + ether_host[5]);
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
  LispPTR checksum;
  DLword *address;
  int nwords;

  address = (DLword *)NativeAligned2FromLAddr(*args++);
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


/*
 * dummy implementation of SUBRs if none of the networking options is compiled in
 */


#if !defined(MAIKO_ENABLE_ETHERNET) && !defined(MAIKO_ENABLE_NETHUB)


/************************************************************************/
/*                                                                      */
/*                      e t h e r _ s u s p e n d                       */
/*                                                                      */
/*      Suspend receiving packets.                                      */
/*      175/70/0                                                        */
/*                                                                      */
/************************************************************************/
LispPTR ether_suspend(LispPTR args[])
{
  return (ATOM_T);
}


/************************************************************************/
/*                                                                      */
/*                      e t h e r _ r e s u m e                         */
/*                                                                      */
/*      resume nit socket to receive all types of packets 175/71/0      */
/*                                                                      */
/************************************************************************/
LispPTR ether_resume(LispPTR args[])
{
  return (ATOM_T);
}

/************************************************************************/
/*                                                                      */
/*                      e t h e r _ c t r l r                           */
/*                                                                      */
/*      return T if ether controller is available 175/72/0              */
/*                                                                      */
/************************************************************************/
LispPTR ether_ctrlr(LispPTR args[])
{
  return (NIL);
}
/**********************************************************************
 *      ether_reset(args) 175/73/0
 *      reset ether controller and disable receipt of packets
 **********************************************************************/
LispPTR ether_reset(LispPTR args[])
{
  return (NIL);
}
/************************************************************************/
/*                                                                      */
/*                        e t h e r _ g e t (175/74/2)                  */
/*                                                                      */
/*      Set up the Ethernet driver to receive a packet.  The driver     */
/*      first tries to read any pending packet from the net, and if     */
/*      there is one, ether_get returns T.  If there is no pending      */
/*      packet, the failing read sets us up to get an interrupt when    */
/*      a packet DOES arrive, and ether_get returns NIL.                */
/*                                                                      */
/*              args[0] Length of the buffer we're passed               */
/*              args[1] LISP address of a packet buffer                 */
/*                                                                      */
/*      sets ether_buf to the buffer address, for check_ether's use     */
/*      sets ether_bsize to the buffer size.  ether_bsize>0 means       */
/*      it's OK to read packets from the network on interrupt.          */
/*                                                                      */
/************************************************************************/
LispPTR ether_get(LispPTR args[])
{
  return (NIL);
}

/**********************************************************************
 *      ether_send(args) 175/75/2 max_words,buffer_addr
 *      send a packet
 **********************************************************************/
LispPTR ether_send(LispPTR args[])
{
  return (NIL);
}

/**********************************************************************
 *      ether_setfilter(args) 175/76/1 filterbits
 *      check whether a packet has come. if does, notify iocb
 **********************************************************************/
LispPTR ether_setfilter(LispPTR args[])
{
  return (NIL);
}


/**********************************************************************
 *      check_ether() 175/77/0
 *      checks an incoming packet
 **********************************************************************/
LispPTR check_ether(void)
{
  return (NIL);
}

#endif
