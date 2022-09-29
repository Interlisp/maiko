/* $Id: chardev.c,v 1.2 1999/01/03 02:06:50 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*                                                                      */
/*            C H A R A C T E R - D E V I C E   S U P P O R T           */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifndef DOS
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#endif /* DOS */

#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "arith.h"
#include "timeout.h"
#include "locfile.h"
#include "osmsg.h"
#include "dbprint.h"

#include "chardevdefs.h"
#include "byteswapdefs.h"
#include "commondefs.h"
#include "perrnodefs.h"


extern int *Lisp_errno;
extern int Dummy_errno;

/************************************************************************/
/*                                                                      */
/*                        C H A R _ o p e n f i l e                     */
/*                                                                      */
/*      Given the arg vector                                            */
/*              args[0] Lisp string full Unix file-name to open                 */
/*              args[1] Access to open it for (INPUT, OUTPUT, BOTH)     */
/*              args[2] a FIXP cell to hold any Unix error number       */
/*                                                                      */
/*      Open the file named, and return the SMALLP descriptor.  If      */
/*      the open fails, return NIL, and put the Unix error number       */
/*      into the FIXP cell provided, for Lisp to look at.               */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_openfile(LispPTR *args)
/* args[0]            fullname */
/* args[1]            access */
/* args[2]            errno */
{
#ifndef DOS
  int fd;    /* return value  of open system call. */
  int flags; /* open system call's argument */
  /* struct stat statbuf; */
  char pathname[MAXPATHLEN];

  Lisp_errno = (int *)NativeAligned4FromLAddr(args[2]);

  LispStringToCString(args[0], pathname, MAXPATHLEN);
  flags = O_NONBLOCK;
  ERRSETJMP(NIL);
  /*    TIMEOUT( rval=stat(pathname, &statbuf) );
      if(rval == 0){      } */
  switch (args[1]) {
    case ACCESS_INPUT: flags |= O_RDONLY; break;
    case ACCESS_OUTPUT: flags |= (O_WRONLY | O_CREAT); break;
    case ACCESS_APPEND: flags |= (O_APPEND | O_RDWR | O_CREAT); break;
    case ACCESS_BOTH: flags |= (O_RDWR | O_CREAT); break;
    default: return (NIL);
  }
  TIMEOUT(fd = open(pathname, flags));
  if (fd == -1) {
    err_mess("open", errno);
    *Lisp_errno = errno;
    return (NIL);
  }
  /* Prevent I/O requests from blocking -- make them error */
  /* if no char is available, or there's no room in pipe.  */
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

  return (GetSmallp(fd));
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                      C H A R _ c l o s e f i l e                     */
/*                                                                      */
/*      Given the arg vector:                                           */
/*              args[0] The SMALLP file descriptor as returned by OPEN  */
/*              args[1] a FIXP cell to hold any Unix error number       */
/*                                                                      */
/*      Close the file identified by the descriptor.  If the            */
/*      close succeeds, return T.  Otherwise, return NIL, and put       */
/*      the Unix error number in the FIXP cell, for Lisp to see.        */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_closefile(LispPTR *args)
/* args[0]            fd      */
/* args[1]            errno   */
{
#ifndef DOS
  int fd; /* file descriptor */
  int rval;
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[1]);
  fd = LispNumToCInt(args[0]);
  ERRSETJMP(NIL);
  TIMEOUT(rval = close(fd));
  if (rval == -1) {
    /** This if is a patch for an apparent problem **/
    /** in SunOS 4 that causes a close on /dev/ttya **/
    /** to error with 'not owner' **/
    if (errno == 1) {
      DBPRINT(("Got errno 1 on a CLOSE!"));
      return (ATOM_T);
    }
    DBPRINT(("Closing char device descriptor #%d.\n", fd));
    err_mess("close", errno);
    *Lisp_errno = errno;
    return (NIL);
  }
  return (ATOM_T);
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                          C H A R _ i o c t l                                 */
/*                                                                      */
/*      Given the arg vector:                                           */
/*              args[0] the file descriptor to be acted on.             */
/*              args[1] the IOCTL request code.                                 */
/*              args[2] auxiliary data structure passed to IOCTL        */
/*              args[3] a FIXP cell to contain any Unix error number    */
/*                                                                      */
/*      Perform the IOCTL system call on the given file descriptor,     */
/*      passing in the request code and auxiliary structure given.      */
/*      If the IOCTL succeeds, return T (and the aux structure may      */
/*      be side-effected).  Otherwise, return NIL, and put the Unix     */
/*      error number in the FIXP cell for Lisp to look at.              */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_ioctl(LispPTR *args)
{
#ifndef DOS
  int fd, request;
  void *data;
  int rval;
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[3]);
  fd = LispNumToCInt(args[0]);
  request = LispNumToCInt(args[1]);
  data = NativeAligned4FromLAddr(args[2]);
  ERRSETJMP(NIL);
  TIMEOUT(rval = ioctl(fd, request, data));
  if (rval != 0) {
    err_mess("ioctl", errno);
    *Lisp_errno = errno;
    return (NIL);
  }
  return (ATOM_T);
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                         C H A R _ b i n                              */
/*                                                                      */
/*      Reads one character from the character file descriptor,         */
/*      and returns the value.  If no character is available,           */
/*      or an error happens, returns NIL and sets the errno FIXP        */
/*      cell to the Unix error number.                                  */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_bin(int fd, LispPTR errn)
{
#ifndef DOS
  ssize_t rval;
  unsigned char ch[4];
  Lisp_errno = (int *)NativeAligned4FromLAddr(errn);
  ERRSETJMP(NIL);
  fd = LispNumToCInt(fd);

  TIMEOUT(rval = read(fd, ch, 1));
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }
  return (GetPosSmallp(ch[0]));
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                          C H A R _ b o u t                           */
/*                                                                      */
/*      Write character ch to the character file descriptor id.  If     */
/*      the write works, return T; else return NIL and sets the FIXP    */
/*      cell at errno to contain the Unix error number.                         */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_bout(int fd, LispPTR ch, LispPTR errn)
{
#ifndef DOS
  ssize_t rval;
  char buf[4];
  Lisp_errno = (int *)NativeAligned4FromLAddr(errn);
  ERRSETJMP(NIL);
  fd = LispNumToCInt(fd);
  buf[0] = LispNumToCInt(ch);

  TIMEOUT(rval = write(fd, buf, 1));

  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  return (ATOM_T);
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                         C H A R _ b i n s                            */
/*                                                                      */
/*      Given the argument vector:                                      */
/*      args[0] the file id to read bytes from                          */
/*      args[1] the base address of the buffer to read into             */
/*      args[2] starting offset within the buffer to put bytes at       */
/*      args[3] the number of bytes desired to read, maximum            */
/*      args[4] a FIXP cell to hold the errno, if an error occurs       */
/*                                                                      */
/*      Read up to the specified number of bytes into the buffer,       */
/*      starting at the offset given.  Return the number of bytes       */
/*      actually read; will return if fewer bytes than desired are      */
/*      read.  If an error occurs in reading, return NIL, and put       */
/*      the Unix errno into the FIXP cell given.  EWOULDBLOCK is an     */
/*      error that can occur--and bins returns NIL, so Lisp code has    */
/*      to handle that case itself.                                     */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_bins(LispPTR *args)
{
#ifndef DOS
  int fd;
  ssize_t rval;
  char *buffer;
  int nbytes;
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[4]);
  ERRSETJMP(NIL);
  fd = LispNumToCInt(args[0]);
  buffer = ((char *)(NativeAligned2FromLAddr(args[1]))) + LispNumToCInt(args[2]);
  nbytes = LispNumToCInt(args[3]);
  /* Read PAGE_SIZE bytes file contents from filepointer. */
  TIMEOUT(rval = read(fd, buffer, nbytes));
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

#ifdef BYTESWAP
  word_swap_page((unsigned short *)buffer, (nbytes + 3) >> 2);
#endif /* BYTESWAP */

  return (GetPosSmallp(rval));
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                         C H A R _ b o u t s                          */
/*                                                                      */
/*      Given the argument vector:                                      */
/*      args[0] the file descriptor to write bytes to                   */
/*      args[1] the base address of the buffer to write from            */
/*      args[2] starting offset within the buffer to gt bytes from      */
/*      args[3] the number of bytes desired to write, maximum           */
/*      args[4] a FIXP cell to hold the errno, if an error occurs       */
/*                                                                      */
/*      write up to the specified number of bytes from the buffer,      */
/*      starting at the offset given.  Return the number of bytes       */
/*      actually written; will return if fewer bytes than desired are   */
/*      written.  If an error occurs in writing, return NIL, and put    */
/*      the Unix errno into the FIXP cell given.  EWOULDBLOCK is an     */
/*      error that can occur--and bins returns NIL, so Lisp code has    */
/*      to handle that case itself.                                     */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_bouts(LispPTR *args)
{
#ifndef DOS
  int fd;
  ssize_t rval;
  char *buffer;
  int nbytes;
  Lisp_errno = (int *)NativeAligned4FromLAddr(args[4]);
  ERRSETJMP(NIL);
  fd = LispNumToCInt(args[0]);
  buffer = ((char *)(NativeAligned2FromLAddr(args[1]))) + LispNumToCInt(args[2]);
  nbytes = LispNumToCInt(args[3]);
/* Write PAGE_SIZE bytes file contents from filepointer. */
#ifdef BYTESWAP
  word_swap_page((unsigned short *)buffer, (nbytes + 3) >> 2);
#endif /* BYTESWAP */

  TIMEOUT(rval = write(fd, buffer, nbytes));
#ifdef BYTESWAP
  word_swap_page((unsigned short *)buffer, (nbytes + 3) >> 2);
#endif /* BYTESWAP */

  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  return (GetSmallp(rval));
#endif /* DOS */
}
