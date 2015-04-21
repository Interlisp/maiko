/* $Id: oldeether.c,v 1.2 1999/01/03 02:07:28 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: oldeether.c,v 1.2 1999/01/03 02:07:28 sybalsky Exp $ Copyright (C) Venue";







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


#ifdef NOETHER
main (argc, argv, argp) int argc; char **argv, **argp;
{
}
#else

#include <stdio.h>
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
#include <net/nit.h>
#include <sys/ioctl.h>
#ifdef OS4
#include <stropts.h>
#include <net/nit_if.h>
#include <net/nit_pf.h>
#endif /* OS4 */

#include <nlist.h>
#include <fcntl.h>
#include <errno.h>

int ether_fd = -1;	/* file descriptor for ether socket */
unsigned char ether_host[6] = {0,0,0,0,0,0};	/* 48 bit address */
char filetorun[30] = "lde";

main(argc, argv, envp) int argc; char **argv, **envp;
{
	char	Earg[30], Ename[30], **newargv;
	int i;
	int flags;
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

if (!geteuid()){
#ifndef OS4
	if ((ether_fd = socket(AF_NIT, SOCK_RAW, NITPROTO_RAW)) >= 0) {
#else /* OS4 */

	if ((ether_fd =  open("/dev/nit", O_RDWR)) >= 0) {
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

	if(ioctl(ether_fd, I_PUSH, "pf") < 0) {
		perror("IOCTL push of pf lost");
		close(ether_fd);
		goto I_Give_Up;
	}
#endif /* OS4 */

	if_data.ifc_len = sizeof(ifbuf);
	if_data.ifc_req = ifbuf;
#ifndef OS4
	if(ioctl(ether_fd, SIOCGIFCONF, &if_data) < 0) {
		perror("Couldn't GIFCONF socket; Net is off");
#else /* OS4 */

	bzero(ifbuf, sizeof(ifbuf));
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
		(void) close(s);
	}
	if(ioctl(ether_fd, NIOCBIND, &if_data.ifc_req[0]) < 0) {
		perror("Couldn't NIOCBIND socket: Net is off");
#endif /* OS4 */

		close(ether_fd);
		ether_fd = -1;
		goto I_Give_Up;
	}
	/* now for the address */
	if(ioctl(ether_fd, SIOCGIFADDR, &if_data.ifc_req[0]) < 0) {
		perror("Couldn't GIFADDR socket: Net is off");
		close(ether_fd);
		ether_fd = -1;
		goto I_Give_Up;
	}
	bcopy(if_data.ifc_req[0].ifr_addr.sa_data, ether_host, 6);
	strcpy(Ename, if_data.ifc_req[0].ifr_name);

	flags = fcntl(ether_fd, F_GETFL, 0);
	flags = fcntl(ether_fd, F_SETFL, flags | FASYNC | FNDELAY);


#ifdef  DEBUG
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

/* OK, right here do other stuff like scan args */
/* finally crank up LDE; first copy the original args */

newargv = (char **) malloc((argc + 1 + (ether_fd > 0)*2) * sizeof (char **));
newargv[0] = filetorun;	/* or whatever... */
for(i=1; i<argc; i++) newargv[i] = argv[i];

/* then if the net is active, spit out the ether info */
if(ether_fd > 0) {
	newargv[i++] = "-E";
	sprintf(Earg, "%d:%x:%x:%x:%x:%x:%x:%s", ether_fd,
		ether_host[0], ether_host[1], ether_host[2],
		ether_host[3], ether_host[4], ether_host[5], Ename);
	newargv[i++] = Earg;
}
newargv[i] = 0;

/* then execve the LDE executable */
execvp(filetorun, newargv);
perror(filetorun);
exit(1);
}

#endif /* NOETHER */

