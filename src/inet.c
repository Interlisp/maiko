/* $Id: inet.c,v 1.3 2001/12/24 01:09:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h> /* for mem... fns */

#ifndef DOS
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h> /* for FD_ fns */
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/* if using inet_ntop you must #include <arpa/inet.h> */
#endif /* DOS */

#if (defined(OS5) || defined(__CYGWIN__)) && !defined(O_ASYNC)
/* Cygwin and Solaris don't define O_ASYNC, yet still define FASYNC. */
#define O_ASYNC FASYNC
#endif

#include "lispemul.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "arith.h"
#include "emlglob.h"
#include "lspglob.h"
#include "adr68k.h"
#include "ether.h"
#include "dbprint.h"
#include "locfile.h"

#include "inetdefs.h"
#include "byteswapdefs.h"
#include "commondefs.h"
#include "mkcelldefs.h"


#define TCPhostlookup 0
#define TCPservicelookup 1
#define TCPsocket 2
#define TCPclose 3
#define TCPconnect 4
#define TCPsend 5
#define TCPrecv 6
#define TCPListen 7
#define TCPAccept 8

#define INETpeername 64 /* (socket buf) => buf has name, returns len */
#define INETpeeraddr 65
#define INETgetname 66 /* Address to name translation */

#define UDPListen 128
#define UDPConnect 129
#define UDPSendto 130
#define UDPRecvfrom 131

extern int *Lisp_errno;
extern fd_set LispReadFds;
fd_set LispIOFds;

LispPTR subr_TCP_ops(int op, LispPTR nameConn, LispPTR proto, LispPTR length, LispPTR bufaddr, LispPTR maxlen)
{
#ifndef DOS
  int sock, len, buflen, res;
  unsigned ures;
  char namestring[100];
  char servstring[50];
  struct sockaddr_in addr;
  struct hostent *host;
  struct servent *service;
  struct sockaddr_in farend;
  int addr_class, protocol;
  DLword *buffer;
  int result;

  switch (op & 0xFFFF) {
    case TCPhostlookup:
      LispStringToCString(nameConn, namestring, 100);
      host = gethostbyname(namestring);
      if (!host) return (NIL);
      res = ntohl(*(in_addr_t *)host->h_addr);
      N_ARITH_SWITCH(res);
      break;

    case TCPservicelookup:
      LispStringToCString(nameConn, namestring, 100);
      LispStringToCString(proto, servstring, 50);
      service = getservbyname(namestring, servstring);
      if (!service) return (NIL);
      return (GetSmallp(ntohs(service->s_port)));

    case TCPsocket:
      addr_class = LispNumToCInt(nameConn);
      protocol = LispNumToCInt(proto);
      result = socket(addr_class, protocol, 0);
#ifndef MAIKO_OS_HAIKU
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_ASYNC | O_NONBLOCK);
#else
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
#endif
#ifdef F_SETOWN
      fcntl(result, F_SETOWN, getpid());
#endif

      return (GetSmallp(result));
    case TCPconnect: /* args: hostname or (fixp)address, socket# */
      memset(&farend, 0, sizeof farend);
      N_GETNUMBER(nameConn, res, string_host);
      farend.sin_addr.s_addr = htonl(res);
      goto host_ok;
    string_host:
      LispStringToCString(nameConn, namestring, 100);
      host = gethostbyname(namestring);
      if (!host) return (NIL);
      memcpy((char *)&farend.sin_addr, (char *)host->h_addr, host->h_length);
    host_ok:
      sock = LispNumToCInt(proto);
      result = socket(AF_INET, SOCK_STREAM, 0);
      farend.sin_family = AF_INET;
      farend.sin_port = htons(sock);
      if (connect(result, (struct sockaddr *)&farend, sizeof farend) < 0) {
        perror("TCP connect");
        return (NIL);
      }
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
#ifdef F_SETOWN
      fcntl(result, F_SETOWN, getpid());
#endif

      return (GetSmallp(result));

    case TCPsend: /* args: conn, buffer, len */
      sock = LispNumToCInt(nameConn);
      buffer = NativeAligned2FromLAddr(proto);
      len = LispNumToCInt(length);
      DBPRINT(("sock: %d, len %d.\n", sock, len));

#ifdef BYTESWAP
      word_swap_page(buffer, (len + 3) >> 2);
#endif /* BYTESWAP */

      result = send(sock, buffer, len, 0);

#ifdef BYTESWAP
      word_swap_page(buffer, (len + 3) >> 2);
#endif /* BYTESWAP */

      if (result < 0) {
        perror("TCP send");
        return (NIL);
      }
      return (GetSmallp(result));

    case TCPrecv: /* args: conn, buffer, maxlen */
      sock = LispNumToCInt(nameConn);
      buffer = NativeAligned2FromLAddr(proto);
      len = LispNumToCInt(length);
      result = read(sock, buffer, len);
      if (result < 0) {
        if (errno == EWOULDBLOCK) return (ATOM_T);
        perror("TCP read");
        *Lisp_errno = errno;
        return (NIL);
      }
#ifdef BYTESWAP
      word_swap_page(buffer, (result + 3) >> 2);
#endif /* BYTESWAP */

      return (GetSmallp(result));

    case TCPclose:
      sock = LispNumToCInt(nameConn);
      FD_CLR(sock, &LispIOFds);
      FD_CLR(sock, &LispReadFds);
      shutdown(sock, 2);
      close(sock);
      return (ATOM_T);

    case TCPListen: /* socket# to listen on */
      sock = LispNumToCInt(nameConn);
      result = socket(AF_INET, SOCK_STREAM, 0);
      farend.sin_family = AF_INET;
      farend.sin_port = htons(sock);
      farend.sin_addr.s_addr = INADDR_ANY;
      if (bind(result, (struct sockaddr *)&farend, sizeof(farend)) < 0) {
        perror("TCP bind");
        close(result);
        return (NIL);
      }
      { /* Do this without taking IO interrupts */
        sigset_t signals;

        sigemptyset(&signals);
#ifndef MAIKO_OS_HAIKU
        sigaddset(&signals, SIGIO);
#endif
        sigprocmask(SIG_BLOCK, &signals, NULL);

#ifndef MAIKO_OS_HAIKU
        fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_ASYNC | O_NONBLOCK);
#else
        fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
#endif

#ifdef F_SETOWN
	fcntl(result, F_SETOWN, getpid());
#endif
        if (listen(result, 5) == -1) {
          perror("TCP Listen");
          close(result);
          sigprocmask(SIG_UNBLOCK, &signals, NULL);
          return (NIL);
        }
        sigprocmask(SIG_UNBLOCK, &signals, NULL);
      }
      FD_SET(result, &LispIOFds);  /* so we get interrupts */
      FD_SET(result, &LispReadFds);
      DBPRINT(("LispIOFds = %p\n", (void *)&LispIOFds));
      return (GetSmallp(result));

    case TCPAccept: /* Socket we're listening on */
      sock = LispNumToCInt(nameConn);
      result = accept(sock, NULL, 0);
      if (result < 0) {
        if (errno != EWOULDBLOCK) perror("TCP Accept");
        return (NIL);
      }
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
#ifdef F_SETOWN
      fcntl(result, F_SETOWN, getpid());
#endif
      return (GetSmallp(result));

    case INETpeername: /* socket#, buffer for name string */
      sock = LispNumToCInt(nameConn);
      buffer = NativeAligned2FromLAddr(proto);
      ures = sizeof(addr);
      getpeername(sock, (struct sockaddr *)&addr, &ures);
      host = gethostbyaddr((const char *)&addr, ures, AF_INET);
      strcpy((char *)buffer, host->h_name);
      return (GetSmallp(strlen(host->h_name)));

    case INETgetname: /* host addr, buffer for name string */
      sock = LispNumToCInt(nameConn);
      buffer = NativeAligned2FromLAddr(proto);
      ures = sizeof(addr);
      addr.sin_addr.s_addr = htonl(sock);
      host = gethostbyaddr((const char *)&addr, ures, 0);
      if (!host) return (GetSmallp(0));
      strcpy((char *)buffer, host->h_name);
      return (GetSmallp(strlen(host->h_name)));

    case UDPListen: /* socket# to listen on */
      sock = LispNumToCInt(nameConn);
      result = socket(AF_INET, SOCK_DGRAM, 0);
      farend.sin_family = AF_INET;
      farend.sin_port = htons(sock);
      farend.sin_addr.s_addr = INADDR_ANY;
      if (bind(result, (struct sockaddr *)&farend, sizeof(farend)) < 0) {
        perror("UDP bind");
        close(result);
        return (NIL);
      }
#ifndef MAIKO_OS_HAIKU
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_ASYNC | O_NONBLOCK);
#else
      fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
#endif
#ifdef F_SETOWN
      fcntl(result, F_SETOWN, getpid());
#endif
      FD_SET(result, &LispIOFds);  /* so we get interrupts */
      FD_SET(result, &LispReadFds);
      DBPRINT(("LispIOFds = %p\n", (void *)&LispIOFds));
      return (GetSmallp(result));

    case UDPSendto: /* fd-socket# addr remote-socket buffer len*/
      sock = LispNumToCInt(nameConn);
      farend.sin_family = AF_INET;
      farend.sin_port = htons(LispNumToCInt(length));
      farend.sin_addr.s_addr = htonl(LispNumToCInt(proto));
      buffer = NativeAligned2FromLAddr(bufaddr);
      buflen = LispNumToCInt(maxlen);

      DBPRINT(("UDP send:  socket = %d, remote-port = %d.\n", sock, farend.sin_port));
      DBPRINT(("           remote-addr = 0x%x, buflen = %d.\n", farend.sin_addr.s_addr, buflen));

#ifdef BYTESWAP
      word_swap_page(buffer, (buflen + 3) >> 2);
#endif /* BYTESWAP */
      result = sendto(sock, buffer, buflen, 0, (struct sockaddr *)&farend, sizeof farend);
#ifdef BYTESWAP
        word_swap_page(buffer, (buflen + 3) >> 2);
#endif /* BYTESWAP */
      if (result < 0) {
        perror("UDP Send");
        printf(" fd = %d, addr = 0x%x.\n", sock, farend.sin_addr.s_addr);
        return (NIL);
      }
      return (GetSmallp(result));

    case UDPRecvfrom: /* fd-socket# buffer len addr-cell port-cell*/
      sock = LispNumToCInt(nameConn);
      buffer = NativeAligned2FromLAddr(proto);
      buflen = LispNumToCInt(length);
      ures = sizeof farend;
      if ((result = recvfrom(sock, buffer, buflen, 0, (struct sockaddr *)&farend, &ures)) < 0) {
        perror("UDP Recv");
        return (NIL);
      }

      DBPRINT(("UDP recv:  socket = %d, len = %d.\n", sock, result));
      DBPRINT(("           remote-addr = 0x%x, remote-port = %d.\n", ntohl(farend.sin_addr.s_addr),
               ntohs(farend.sin_port)));
      DBPRINT(("           bufsize = %d, addrcell = 0x%x, portcell = 0x%x.\n", buflen, bufaddr,
               maxlen));

      /* XXX NBriggs: 12 Aug 2020 -- WHAT IS GOING ON HERE? */
      *((int *)NativeAligned4FromLAddr(bufaddr)) = (int)farend.sin_addr.s_addr;
      *((int *)NativeAligned4FromLAddr(maxlen)) = (int)farend.sin_port;

#ifdef BYTESWAP
      word_swap_page(buffer, (result + 3) >> 2);
#endif /* BYTESWAP */

      return (GetSmallp(result));

    default: return (NIL);
  }
#endif /* DOS */
}
