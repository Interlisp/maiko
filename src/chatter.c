/* $Id: chatter.c,v 1.2 1999/01/03 02:06:51 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <sys/fcntlcom.h>
#include <sys/termios.h>
#include <sys/ttydev.h>
#include <stdio.h>

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "arith.h"
#include "locfile.h"

#define CHAR_MAXLEN 512
#define BYTESIZE 256

int chatter_fd;

#define LStringToCString(Lisp, C, MaxLen, Len)                                                     \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int i;                                                                                         \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((UNSIGNED)Lisp));                                    \
    Len = min(MaxLen, arrayp->totalsize);                                                          \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base = ((char *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset));   \
        for (i = 0; i < Len; i++) C[i] = base[i];                                                  \
        C[Len] = '\0';                                                                             \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase = ((short *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (i = 0; i < Len * 2; i++) C[i] = base[i];                                              \
        C[Len * 2] = '\0';                                                                         \
        break;                                                                                     \
                                                                                                   \
      default: error("LStringToCString can not handle\n");                                         \
    }                                                                                              \
  }

#define CStringToLString(C, Lisp, Len)                                                             \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int idx;                                                                                       \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((UNSIGNED)Lisp));                                    \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base = ((char *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset));   \
        for (idx = 0; idx < Len; idx++) base[idx] = C[idx];                                        \
        arrayp->fillpointer = Len;                                                                 \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase = ((short *)(Addr68k_from_LADDR((UNSIGNED)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (idx = 0; idx < Len * 2; idx++) base[idx] = C[idx];                                    \
        arrayp->fillpointer = Len;                                                                 \
        break;                                                                                     \
                                                                                                   \
      default: error("CStringToLString can not handle\n");                                         \
    }                                                                                              \
  }

#define IntToFixp(C, Lisp)                            \
  {                                                   \
    int *base;                                        \
                                                      \
    base = (int *)Addr68k_from_LADDR((UNSIGNED)Lisp); \
    *base = C;                                        \
  }

chatter(LispPTR *args)
{
  int result;
  int length;
  int number;
  unsigned char data[(CHAR_MAXLEN + 1) * 2];
  unsigned char tmp[(CHAR_MAXLEN + 1) * 2];

  int i;

  N_GETNUMBER(args[0], number, ERROR);
  switch (number) {
    case 1:
      LStringToCString(args[1], data, CHAR_MAXLEN, length);
      result = chatter_open(data);
      break;

    case 2: result = chatter_close(); break;

    case 3:
      LStringToCString(args[1], data, CHAR_MAXLEN, length);
      result = chatter_write_string(data, length);
      break;

    case 4:
      result = chatter_read(data, 1);
      number = data[0];
      IntToFixp(number, args[1]);
      break;

    case 5:
      N_GETNUMBER(args[1], number, ERROR);
      data[0] = number & 0xff;
      result = chatter_write_code(data[0]);
      break;

    ERROR:
      result = 9999;
      break;
  }

  if (result == 0)
    return (ATOM_T);
  else
    return (NIL);
}

chatter_open(char *dev)
{
  struct termios termdata;

  if ((chatter_fd = open(dev, O_RDWR)) < 0) {
    perror("CHATTER OPEN ERROR");
    return (-1);
  }
  if (ioctl(chatter_fd, TCGETS, &termdata) < 0) {
    perror("CHATTER GET PARAMS ERROR");
    return (-1);
  }
  termdata.c_iflag &= ~IXON;
  termdata.c_iflag &= ~IXANY;
  termdata.c_iflag &= ~IXOFF;

  termdata.c_cflag &= ~CBAUD;
  termdata.c_cflag |= B4800;
  termdata.c_cflag &= ~CSIZE;
  termdata.c_cflag |= CS8;
  termdata.c_cflag &= ~CSTOPB;
  termdata.c_cflag &= ~PARODD;
  termdata.c_cflag &= ~CIBAUD;

  termdata.c_lflag = 0;

  termdata.c_cc[VMIN] = 256;
  termdata.c_cc[VTIME] = 1;

  if (ioctl(chatter_fd, TCSETS, &termdata) < 0) {
    perror("CHATTER SET PARAMS ERROR:");
    return (-1);
  }
  return (0);
}

chatter_close() {
  if (close(chatter_fd) < 0) {
    perror("CHATTER CLOSE ERROR");
    return (-1);
  }
  return (0);
}

chatter_write_string(char *data, int len)
{
  if (write(chatter_fd, data, len) < 0) {
    perror("WRITE ERROR");
    return (-1);
  }
  return (0);
}

chatter_write_code(unsigned char code)
{
  if (write(chatter_fd, &code, 1) < 0) {
    perror("WRITE ERROR");
    return (-1);
  }
  return (0);
}

chatter_read(unsigned char *data, int len)
{
  if (read(chatter_fd, data, len) < 0) {
    perror("READ ERROR");
    return (-1);
  }
  return (0);
}
