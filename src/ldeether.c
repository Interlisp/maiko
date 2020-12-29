/* $Id: ldeether.c,v 1.3 2001/12/24 01:09:04 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#ifdef NOETHER
/* No ethernet, so have a dummy here. */
unsigned char ether_host[6] = {0, 0, 0, 0, 0, 0}; /* 48 bit address */
int main(int argc, char *argv[]) { return (0); }
#else
/* THERE -IS- AN ETHERNET */

#include <stdio.h>
#include <string.h>

#ifdef USE_DLPI
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/pfmod.h>

#if defined(SVR4) && !defined(OS5)
char *devices[] = {"emd0", "emd1", "emd2", "emd3", "emd4", 0};
#endif

#ifdef OS5
#include <sys/bufmod.h>

char *devices[] = {"le0",   "le1",   "le2",   "le3",   "le4",   "ie0", "ie1", "ie2", "ie3",
                   "ie4",   "qe0",   "qe1",   "qe2",   "qe3",   "qe4", "qe5", "qe6", "qe7",
                   "fddi0", "fddi1", "fddi2", "fddi3", "fddi4", "bf0", "bf1", 0};
#endif
#endif /* USE_DLPI */

#ifndef NOETHER
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#ifdef OS4
#include <sys/file.h>
#endif /* OS4 */

#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#ifndef USE_DLPI
#include <net/nit.h>
#ifdef OS4
#include <stropts.h>
#include <net/nit_if.h>
#include <net/nit_pf.h>
#endif /* OS4 */
#endif /* USE_DLPI */

#include <nlist.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>

#endif /* NOETHER */

int ether_fd = -1;                                /* file descriptor for ether socket */
int ether_intf_type = 0;                          /* IF type? from DLPI */
unsigned char ether_host[6] = {0, 0, 0, 0, 0, 0}; /* 48 bit address */
char filetorun[30] = "lde";

int main(int argc, char *argv[]) {
  char Earg[30], Ename[30], **newargv;
  int i;
#ifdef USE_DLPI
  static struct packetfilt pf = {0, 1, {ENF_PUSHZERO}};
  struct strioctl si;
#endif /* USE_DLPI */

#ifndef NOETHER

  /* Only do the ether kick=-start if we've got to. */

  /* Kickstart program for the Lisp Development Environment (LDE).
          Run this as setuid root to open the LDE ether socket.
          Passes all arguments through to LDE plus -E <ether-info>
          to communicate open ether socket.

          <ether-info> looks like this:
          <descriptor-number>:<b1>:<b2>:<b3>:<b4>:<b5>:<b6>:<name>

          where <descriptor-number> is the number of the open
          socket (decimal), and <b1>-<b6> are hex of the socket's
          48-bit Ethernet address, and <name> is the name of the
          Ethernet device as found by SIOCGIFCONF.
  */

  if (!geteuid()) {
#ifdef USE_DLPI
    /* Use DLPI to connect to the ethernet.  This code is stolen
       from NFSWATCH4.3
    */
    char *etherdev = getenv("LDEETHERDEV");
    ether_fd = setup_dlpi_dev(etherdev);
    if (ether_fd >= 0) { /* Open an ether interface */
      ether_intf_type = dlpi_devtype(ether_fd);
      printf("opened ldeether fd %d.\n", ether_fd);
      /* first and foremost, get the packet filter module attached
             (used for ether_suspend and ether_resume) */

      if (ioctl(ether_fd, I_PUSH, "pfmod") < 0) {
        perror("IOCTL push of pf lost");
        close(ether_fd);
        goto I_Give_Up;
      }

      si.ic_cmd = PFIOCSETF;
      si.ic_timout = -1;
      si.ic_len = sizeof(pf);
      si.ic_dp = (char *)&pf;

      if (ioctl(ether_fd, I_STR, &si) < 0) {
        perror("ioctl: I_STR PFIOCSETF");
        return (-1);
      }

      fcntl(ether_fd, F_SETFL, fcntl(ether_fd, F_GETFL, 0) | O_NONBLOCK);

#else
/*    N O T   D L P I   C O D E   */

#ifndef OS4
    if ((ether_fd = socket(AF_NIT, SOCK_RAW, NITPROTO_RAW)) >= 0) {
#else  /* OS4 */

    if ((ether_fd = open("/dev/nit", O_RDWR)) >= 0) {
#endif /* OS4 */

      /* it's open, now query it and find out its name and address */
      /* JRB - must document that LDE uses the first net board as found
      by SIOCGIFCONF (see if(4)).  Maybe we need an option to specify
      which net board (suspect more than one net board on an LDE machine
      will be rare, but...).
      */
      struct ifconf if_data;
      struct ifreq ifbuf[20];

#ifdef OS4
      /* first and foremost, get the packet filter module attached
              (used for ether_suspend and ether_resume) */

      if (ioctl(ether_fd, I_PUSH, "pf") < 0) {
        perror("IOCTL push of pf lost");
        close(ether_fd);
        goto I_Give_Up;
      }
#endif /* OS4 */

      if_data.ifc_len = sizeof(ifbuf);
      if_data.ifc_req = ifbuf;
#ifndef OS4
      if (ioctl(ether_fd, SIOCGIFCONF, &if_data) < 0) {
        perror("Couldn't GIFCONF socket; Net is off");
#else  /* OS4 */

      memset(ifbuf, 0, sizeof(ifbuf));
      {
        /* we have to get the interface name from another socket, since
        /dev/nit doesn't know anything until it gets bound, and we
        can't bind it without knowing the interface name... */
        int s;

        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
          perror("No socket for interface name");
          close(s);
          close(ether_fd);
          ether_fd = -1;
          goto I_Give_Up;
        }
        if (ioctl(s, SIOCGIFCONF, (char *)&if_data) < 0) {
          perror("Couldn't get interface name from socket");
          close(s);
          close(ether_fd);
          ether_fd = -1;
          goto I_Give_Up;
        }
        (void)close(s);
      }
      if (ioctl(ether_fd, NIOCBIND, &if_data.ifc_req[0]) < 0) {
        perror("Couldn't NIOCBIND socket: Net is off");
#endif /* OS4 */

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
      strcpy(Ename, if_data.ifc_req[0].ifr_name);

      fcntl(ether_fd, F_SETFL, fcntl(ether_fd, F_GETFL, 0) | O_ASYNC | O_NONBLOCK);

#endif /* USE_DLPI  */
#ifdef DEBUG
      printf("init_ether: **** Ethernet starts ****\n");
#endif
    } else {
    I_Give_Up:
      perror("init_ether: can't open NIT socket\n");
      ether_fd = -1;
      /*	exit();	*/
    }
    seteuid(getuid());
  }

#endif /* NOETHER */

  /* OK, right here do other stuff like scan args */
  /* finally crank up LDE; first copy the original args */

  newargv = (char **)malloc((argc + 1 + (ether_fd > 0) * 2) * sizeof(char **));
  newargv[0] = filetorun; /* or whatever... */
  for (i = 1; i < argc; i++) newargv[i] = argv[i];

  /* then if the net is active, spit out the ether info */
  if (ether_fd > 0) {
    newargv[i++] = "-E";
#ifdef USE_DLPI
    sprintf(Earg, "%d:%x:%x:%x:%x:%x:%x", ether_fd, ether_host[0], ether_host[1], ether_host[2],
            ether_host[3], ether_host[4], ether_host[5]);
#else
    sprintf(Earg, "%d:%x:%x:%x:%x:%x:%x:%s", ether_fd, ether_host[0], ether_host[1], ether_host[2],
            ether_host[3], ether_host[4], ether_host[5], Ename);
#endif /* USE_DLPI */
    newargv[i++] = Earg;
    printf("ether = %d:%x:%x:%x:%x:%x:%x:%s", ether_fd, ether_host[0], ether_host[1], ether_host[2],
           ether_host[3], ether_host[4], ether_host[5], Ename);
  }
  newargv[i] = 0;

  /* then execve the LDE executable */
  execvp(filetorun, newargv);
  perror("failed to exec lde");
  return (1);
}

#endif /* NOETHER */
