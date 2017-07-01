/* $Id: chardev.c,v 1.2 1999/01/03 02:06:50 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: chardev.c,v 1.2 1999/01/03 02:06:50 sybalsky Exp $ Copyright (C) Venue";

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

/************************************************************************/
/*                                                                      */
/*            C H A R A C T E R - D E V I C E   S U P P O R T           */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifndef DOS
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/time.h>
#ifndef OS5
#ifndef FREEBSD
#include <sys/dir.h>
#endif /* FREEBSD */
#endif /* OS5 */
#ifndef HPUX
#ifndef OS5
#include <strings.h>
#endif /* OS5 */
#endif /* HPUX */
#include <sys/ioctl.h>
#else /* DOS */
#include <string.h>
#endif /* DOS */

#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "arith.h"
#include "timeout.h"
#include "locfile.h"
#include "osmsg.h"
#include "dbprint.h"
#include "chardev.h"

#if defined(ISC) || defined(FREEBSD)
#include <dirent.h>
#endif

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
  register int id;    /* return value  of open system call. */
  register int flags; /* open system call's argument */
  register int rval;
  register int linkflag = 0;
  register int *bufp;
  struct stat statbuf;
  char pathname[MAXPATHLEN];

#if (defined(RS6000) || defined(HPUX))
  static int one = 1; /* Used in charopenfile, etc. */
#endif

  Lisp_errno = (int *)(Addr68k_from_LADDR(args[2]));

  LispStringToCString(args[0], pathname, MAXPATHLEN);
  flags = O_NDELAY;
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
  TIMEOUT(id = open(pathname, flags));
  if (id == -1) {
    err_mess("open", errno);
    *Lisp_errno = errno;
    return (NIL);
  }
/* Prevent I/O requests from blocking -- make them error */
/* if no char is available, or there's no room in pipe.  */
#ifdef RS6000
  ioctl(id, FIONBIO, &one);
  fcntl(id, F_SETOWN, getpid());
#else
#ifdef HPUX
  ioctl(id, FIOSNBIO, &one);
#else
  rval = fcntl(id, F_GETFL, 0);
  rval |= FNDELAY;
  rval = fcntl(id, F_SETFL, rval);
#endif /* HPUX */

#endif /* RS6000 */

  return (GetSmallp(id));
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
/* args[0]            id      */
/* args[1]            errno   */
{
#ifndef DOS
  register int id; /* FileID */
  register int rval;
  char pathname[MAXPATHLEN];
  Lisp_errno = (int *)(Addr68k_from_LADDR(args[1]));
  id = LispNumToCInt(args[0]);
  ERRSETJMP(NIL);
  TIMEOUT(rval = close(id));
  if (rval == -1) {
    /** This if is a patch for an apparent problem **/
    /** in SunOS 4 that causes a close on /dev/ttya **/
    /** to error with 'not owner' **/
    if (errno == 1) {
      DBPRINT(("Got errno 1 on a CLOSE!"));
      return (ATOM_T);
    }
    DBPRINT(("Closing char device descriptor #%d.\n", id));
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
  int id, request, data;
  register int rval;
  char *base;
  struct stat sbuf;
  Lisp_errno = (int *)(Addr68k_from_LADDR(args[3]));
  id = LispNumToCInt(args[0]);
  request = LispNumToCInt(args[1]);
  data = (int)(Addr68k_from_LADDR(args[2]));
  ERRSETJMP(NIL);
  TIMEOUT(rval = ioctl(id, request, data));
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
/*      Reads one character from the character file descriptor          */
/*      id, and returns the value.  If no character is available,       */
/*      or an error happens, returns NIL and sets the errno FIXP        */
/*      cell to the Unix error number.                                  */
/*                                                                      */
/************************************************************************/

LispPTR CHAR_bin(int id, LispPTR errn)
{
#ifndef DOS
  register int rval, size;
  unsigned char ch[4];
  Lisp_errno = (int *)(Addr68k_from_LADDR(errn));
  ERRSETJMP(NIL);
  id = LispNumToCInt(id);
  /* Read PAGE_SIZE bytes file contents from filepointer. */
  TIMEOUT(rval = read(id, ch, 1));
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }
  return (GetSmallp(ch[0]));
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

LispPTR CHAR_bout(int id, LispPTR ch, LispPTR errn)
{
#ifndef DOS
  register int rval;
  char buf[4];
  Lisp_errno = (int *)(Addr68k_from_LADDR(errn));
  ERRSETJMP(NIL);
  id = LispNumToCInt(id);
  buf[0] = LispNumToCInt(ch);
  /* Write PAGE_SIZE bytes file contents from filepointer. */

  TIMEOUT(rval = write(id, buf, 1));

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
  register int id, rval;
  char *buffer;
  int offset, nbytes;
  Lisp_errno = (int *)(Addr68k_from_LADDR(args[4]));
  ERRSETJMP(NIL);
  id = LispNumToCInt(args[0]);
  buffer = ((char *)(Addr68k_from_LADDR(args[1]))) + LispNumToCInt(args[2]);
  nbytes = LispNumToCInt(args[3]);
  /* Read PAGE_SIZE bytes file contents from filepointer. */
  TIMEOUT(rval = read(id, buffer, nbytes));
  if (rval == 0) {
    *Lisp_errno = EWOULDBLOCK;
    return (NIL);
  }
  if (rval == -1) {
    *Lisp_errno = errno;
    return (NIL);
  }

#ifdef BYTESWAP
  word_swap_page(buffer, (nbytes + 3) >> 2);
#endif /* BYTESWAP */

  return (GetSmallp(rval));
#endif /* DOS */
}

/************************************************************************/
/*                                                                      */
/*                         C H A R _ b o u t s                          */
/*                                                                      */
/*      Given the argument vector:                                      */
/*      args[0] the file id to write bytes to                           */
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
  register int id, rval;
  char *buffer;
  int nbytes, offset;
  Lisp_errno = (int *)(Addr68k_from_LADDR(args[4]));
  ERRSETJMP(NIL);
  id = LispNumToCInt(args[0]);
  buffer = ((char *)(Addr68k_from_LADDR(args[1]))) + LispNumToCInt(args[2]);
  nbytes = LispNumToCInt(args[3]);
/* Write PAGE_SIZE bytes file contents from filepointer. */
#ifdef BYTESWAP
  word_swap_page(buffer, (nbytes + 3) >> 2);
#endif /* BYTESWAP */

  TIMEOUT(rval = write(id, buffer, nbytes));
#ifdef BYTESWAP
  word_swap_page(buffer, (nbytes + 3) >> 2);
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
