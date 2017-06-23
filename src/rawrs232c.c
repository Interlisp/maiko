/* $Id: rawrs232c.c,v 1.2 1999/01/03 02:07:31 sybalsky Exp $ (C) Copyright Venue, All Rights
 * Reserved  */
static char *id = "$Id: rawrs232c.c,v 1.2 1999/01/03 02:07:31 sybalsky Exp $ Copyright (C) Venue";

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

#include <sys/types.h>
#include <sys/termios.h>
#include <sys/ttold.h>
#include <sys/time.h>
#include <stropts.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "debug.h"
#include "rawrs232c.h"

#define MIN_CHARS 256
#define MIN_TIME 1

/************************************************************/
/*

        raw232c.c
        Access RS device from Lisp by using FD
        This provides low-level access to RS device.

        Created:  Feb.12 1991
        By Takeshi Shimizu

        CopyRight   Fuji Xerox Co., LTD 1991


*/
/************************************************************/

LispPTR raw_rs232c_open(LispPTR portname)
{
  ONED_ARRAY *head;

  char *lispname;
  int fd;

  if (GetTypeNumber(portname) != TYPE_ONED_ARRAY) error("PORTNAME is not string");
  head = (ONED_ARRAY *)Addr68k_from_LADDR(portname);
  lispname = (char *)Addr68k_from_LADDR(head->BASE);

  if ((fd = open(lispname, O_RDWR)) < 0) {
    perror("RS open fail: ");
    return (NIL);
  }

  return (S_POSITIVE | fd);

} /* raw_rs232c_open */

LispPTR raw_rs232c_setparams(LispPTR fd, LispPTR data)
{
  RawRSParam *ndata;
  struct termios termdata;

  /* Get Current params */
  if (ioctl((fd & 0xffff), TCGETS, &termdata) < 0) {
    perror("RS get params:");
    return (NIL);
  }

  ndata = (RawRSParam *)Addr68k_from_LADDR(data);

  /* Input  */
  termdata.c_iflag |= IXON | IXOFF;
  termdata.c_iflag &= ~(INPCK);

  /*termdata.c_iflag &= ~IGNBRK;*/
  termdata.c_iflag &= ~IGNPAR;
  termdata.c_iflag &= ~IUCLC;
  if (ndata->InEOL == RAW_RS_CR) {
    termdata.c_iflag |= INLCR;
    termdata.c_iflag &= ~ICRNL;
  }

  else if (ndata->InEOL == RAW_RS_LF) {
    termdata.c_iflag &= ~INLCR;
    termdata.c_iflag |= ICRNL;
  } else if (ndata->InEOL == RAW_RS_CRLF) {
    termdata.c_iflag &= ~INLCR;
    termdata.c_iflag &= ~ICRNL;
  }
  /* Flow CNT */
  if ((ndata->FlowCnt & 0xff) == RAW_RS_XON)
    termdata.c_iflag |= IXON | IXOFF;
  else {
    termdata.c_iflag &= ~(IXON | IXOFF);
  }

  termdata.c_iflag &= ~(ISTRIP);

  /* Output   */

  termdata.c_oflag |= ~OPOST;
  termdata.c_oflag &= ~OLCUC;
  termdata.c_oflag &= ~OFILL;
  termdata.c_oflag &= ~OFDEL;
  termdata.c_oflag &= ~NLDLY;
  termdata.c_oflag &= ~CRDLY;
  termdata.c_oflag &= ~BSDLY;
  termdata.c_oflag &= ~VTDLY;
  termdata.c_oflag &= ~FFDLY;

  if (ndata->OutEOL == RAW_RS_CR) {
    termdata.c_oflag |= ONLRET;
    termdata.c_oflag &= ~OCRNL;
    termdata.c_oflag &= ~ONLCR;
  } else if (ndata->OutEOL == RAW_RS_LF) {
    termdata.c_oflag &= ~ONLRET;
    termdata.c_oflag |= OCRNL;
    termdata.c_oflag &= ~ONLCR;
  } else if (ndata->OutEOL == RAW_RS_CRLF) {
    termdata.c_oflag &= ~ONLRET;
    termdata.c_oflag &= ~OCRNL;
    termdata.c_oflag |= ONLCR;
  }

  /* LOCAL */

  termdata.c_lflag &= ~ISIG;
  termdata.c_lflag &= ~ICANON;
  termdata.c_lflag &= ~XCASE;

  if (ndata->Echo == ATOM_T)
    termdata.c_lflag |= ECHO;
  else
    termdata.c_lflag &= ~ECHO;

  termdata.c_lflag &= ~ECHOE;
  termdata.c_lflag &= ~ECHOK;
  termdata.c_lflag &= ~ECHONL;
  termdata.c_lflag &= ~NOFLSH;
  termdata.c_lflag &= ~TOSTOP;
  termdata.c_lflag &= ~ECHOCTL;
  termdata.c_lflag &= ~ECHOPRT;
  termdata.c_lflag &= ~ECHOKE;
  termdata.c_lflag &= ~FLUSHO;
  termdata.c_lflag &= ~PENDIN;

  /* Minimum and Timeout */
  termdata.c_cc[VMIN] = MIN_CHARS;
  termdata.c_cc[VTIME] = MIN_TIME;

  /*  Control */

  termdata.c_cflag = (CREAD | HUPCL);

  /* Local or DialUP */

  if (ndata->LocalLine == ATOM_T)
    termdata.c_cflag |= CLOCAL;
  else
    termdata.c_cflag &= ~CLOCAL;

  /* Bau Rate */

  if (ndata->BauRate != NIL) {
    switch (ndata->BauRate & 0xffff) {
      case 1200: termdata.c_cflag |= B1200; break;

      case 2400: termdata.c_cflag |= B2400; break;

      case 4800: termdata.c_cflag |= B4800; break;

      case 9600: termdata.c_cflag |= 96200; break;

      default: termdata.c_cflag |= B1200;
    }
  }

  /* CTS/RTS */
  if ((ndata->RTSCTSCnt & 0xff) == ATOM_T)
    termdata.c_cflag |= CRTSCTS;
  else
    termdata.c_cflag &= ~CRTSCTS;

  /* Character size */

  if ((ndata->BitsPerChar & 0xf) == 7)
    termdata.c_cflag |= CS7;
  else if ((ndata->BitsPerChar & 0xf) == 8)
    termdata.c_cflag |= CS8;
  else {
    error("RS232:Not supported char size");
    return (NIL);
  }

  /* Parity */
  if ((ndata->Parity & 0xf) == RAW_RS_NONE) {
    termdata.c_iflag &= (~(INPCK));
    termdata.c_cflag &= (~PARENB);
  } else if ((ndata->Parity & 0xf) == RAW_RS_EVEN)
    termdata.c_cflag |= PARENB;
  else if ((ndata->Parity & 0xf) == RAW_RS_ODD)
    termdata.c_cflag |= (PARENB | PARODD);

  /* Stop bits */
  if ((ndata->NoOfStopBits & 0x2) == 2)
    termdata.c_cflag |= CSTOPB;
  else
    termdata.c_cflag &= ~(CSTOPB);

  /* Set parameters */

  if (ioctl((fd & 0xffff), TCSETS, &termdata) < 0) {
    perror("RS set params");
    return (NIL);
  }

  return (ATOM_T);

} /* raw_rs232c_setparams */

LispPTR raw_rs232c_close(LispPTR fd)
{
  if (close(0xffff & fd) < 0) {
    perror("RS close : ");
    return (NIL);
  } else
    return (ATOM_T);
}

LispPTR raw_rs232c_write(LispPTR fd, LispPTR baseptr, LispPTR len)
{
  unsigned char *ptr;

  ptr = (unsigned char *)Addr68k_from_LADDR(baseptr);

  if (write((fd & 0xffff), ptr, (len & 0xffff)) < 0) {
    perror("RS-write :");
    return (NIL);
  }
  return (ATOM_T);
}

/* Assume numbers are SMALLPOSP */
struct timeval RS_TimeOut = {0, 0};
LispPTR raw_rs232c_read(LispPTR fd, LispPTR buff, LispPTR nbytes)
{
  unsigned char *buffptr;
  int length;
  u_int real_fd;
  u_int readfds;

  real_fd = fd & 0xffff;
  readfds = (1 << real_fd);

  select(32, &readfds, NULL, NULL, &RS_TimeOut);
  if (readfds & (1 << real_fd)) {
    buffptr = (unsigned char *)Addr68k_from_LADDR(buff);

    if ((length = read(real_fd, buffptr, (nbytes & 0xffff))) < 0) {
      perror("RS read :");
      return (NIL);
    } else {
      Irq_Stk_End = Irq_Stk_Check = 0;
      return (S_POSITIVE | length);
    }
  }

  return (S_POSITIVE); /* There is nothing to read */

} /* raw_rs232c_read  */

LispPTR raw_rs232c_setint(LispPTR fd, LispPTR onoff)
{
  extern u_int LispReadFds;
  extern u_int LispIOFds;
  u_int real_fd;

  real_fd = (fd & 0xffff);

  if (onoff == ATOM_T) {
    LispReadFds |= (1 << real_fd);
    LispIOFds |= (1 << real_fd);
    int_io_open(real_fd);
  } else {
    int_io_close(real_fd);
    LispReadFds &= ~(1 << real_fd);
    LispIOFds &= ~(1 << real_fd);
  }
  return (ATOM_T);
}
