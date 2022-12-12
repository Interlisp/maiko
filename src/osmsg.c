/* $Id: osmsg.c,v 1.2 1999/01/03 02:07:29 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*			O S M E S S A G E . C				*/
/*									*/
/*	Functions for handling the redirection of console and stan-	*/
/*	dard-error output so it appears in the prompt window.		*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <setjmp.h> /* Needed for jmp_buf for timeout.h */

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>


#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "arith.h"
#include "stream.h"
#include "lspglob.h"
#include "timeout.h"
#include "locfile.h"
#include "osmsg.h"
#include "dbprint.h"

#include "commondefs.h"
#include "osmsgdefs.h"

#ifdef OS4
#include <stropts.h>
#endif

#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
#define MESSAGE_BUFFER_SIZE 1024
static int cons_tty;
static int cons_pty;

static char logfile[100];
static int log_id;
static int previous_size;
static int logChanged; /* T if log file has changed since last READ */
                /* Set by flush_pty, to avoid the stat call  */
int LogFileFd = -1;
extern fd_set LispReadFds;
#endif

/************************************************************************/
/*									*/
/*			    m e s s _ i n i t				*/
/*									*/
/*	Initialize console-message handling.  Console & Log msgs	*/
/*	are redirected to a pty; when messages appear, we get the	*/
/*	interrupt, and signal a Lisp interrupt to print each msg	*/
/*	in the prompt window.  For historical reasons, the msgs 	*/
/*	are saved on the file tmp/<username>-log.			*/
/*									*/
/************************************************************************/

void mess_init(void) {
#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
  struct passwd *pwd;
  int ttyfd;
  int ptyfd, ptynum;
  char *ptyname, *ttyname;
  struct termios options;

  ptyname = "/dev/ptypx";
  ttyname = "/dev/ttypx";

  /* Get pty and tty */
  ptynum = 0;

needpty:
  while (ptynum < 16) {
    ptyname[9] = "0123456789abcdef"[ptynum];
    if ((ptyfd = open(ptyname, 2)) >= 0) goto gotpty;
    ptynum++;
  }
  return;
gotpty:
  ttyname[9] = ptyname[9];
  if ((ttyfd = open(ttyname, 2)) < 0) {
    ptynum++;
    close(ptyfd);
    goto needpty;
  }

  /* Set tty parameters same as stderr */
  tcgetattr(2, &options);
  tcsetattr(ttyfd, TCSANOW, &options);

  /* Get console IO */
  ioctl(ptyfd, FIOCLEX, 0);
  /* It isn't clear which platforms this #ifndef should
   * actually apply to. The AIX define used to apply
   * to a number of platforms, not just AIX-actual. */
#ifndef AIX
  if ((ioctl(ttyfd, TIOCCONS, 0)) == -1) {
    OSMESSAGE_PRINT(printf("TIOCCONS error\n"));
    exit(-1);
  }
#endif /* AIX */

  cons_pty = ptyfd;
  cons_tty = ttyfd;

  /* Initialize log file */
  pwd = getpwuid(getuid());
  sprintf(logfile, "/tmp/%s-lisp.log", pwd->pw_name);
  if (unlink(logfile) == -1) { /* delete old log file */
    if (errno != ENOENT) return;
  }

  if ((log_id = open(logfile, (O_RDWR | O_CREAT), 0666)) < 0) return;
#ifdef LOGINT
  LogFileFd = cons_pty; /* was kept as an fd_set, but doesn't need to be */
  fcntl(cons_pty, F_SETFL, fcntl(cons_pty, F_GETFL, 0) | O_ASYNC | O_NONBLOCK);
  if (fcntl(cons_pty, F_SETOWN, getpid()) == -1) {
#ifdef DEBUG
    perror("fcntl F_SETOWN of log PTY");
#endif
  }
  FD_SET(LogFileFd, &LispReadFds);
  flush_pty();
#endif
  previous_size = 0;
  DBPRINT(("Console logging started.\n"));
#endif /* MAIKO_HANDLE_CONSOLE_MESSAGES */
}

/************************************************************************/
/*									*/
/*			    m e s s _ r e s e t				*/
/*									*/
/*	Undo the redirection of console and standard-error outputs	*/
/*	presumably preparatory to shutting down lisp.  Closes the	*/
/*	TTY/PTY pair used for logging the messages			*/
/*									*/
/************************************************************************/

void mess_reset(void) {
#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
  int console_fd;
  close(log_id);
  close(cons_tty);
  close(cons_pty);
/* Try to make /dev/console be the real console again */
/*** This sequence sometimes causes SunOS to panic on SunOS4.0(and 4.0.1).
    TIOCCONS problem ???
    if ((console_fd = open("/dev/console", 0)) >= 0) {
          ioctl(console_fd, TIOCCONS, 0);
          close(console_fd);
        }
***/
#endif /* MAIKO_HANDLE_CONSOLE_MESSAGES */
}

/************************************************************************/
/*									*/
/*			m e s s _ r e a d p				*/
/*									*/
/*	Returns T if there are log/console messages waiting		*/
/*	to be printed.  More accurately, if the logChanged flag		*/
/*	has been set by flush_pty().					*/
/*									*/
/************************************************************************/
LispPTR mess_readp(void) {
#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
  struct stat sbuf;
  int size;
  int rval;

  /* polling pty nd flush os message to log file */
  flush_pty();

  /* * * * * * * * * * * * COMMENTED OUT * * * * * * * * *
      SETJMP(NIL);
      TIMEOUT( rval=stat(logfile, &sbuf) );

      if(rval != 0)
        {
          error("osmessage error: can not find a log file under /tmp");
          return(NIL);
        }
      if( previous_size < (int)(sbuf.st_size) ) return(ATOM_T);
  * * * * * * * * * * * * * */

  if (logChanged) return (ATOM_T);
#endif /* MAIKO_HANDLE_CONSOLE_MESSAGES */
  return (NIL);
}

/************************************************************************/
/*									*/
/*			   m e s s _ r e a d				*/
/*									*/
/*	Read up to 1024 chars of log and console message(s)		*/
/*	that are waiting to be printed, into a buffer supplied		*/
/*	by the caller.  If all pending message text gets read in	*/
/*	this call, reset the logChanged flag; Otherwise, leave		*/
/*	it set, so Lisp knows there is more to read.			*/
/*									*/
/************************************************************************/

LispPTR mess_read(LispPTR *args)
/* args[0]		buffer	*/
{
#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
  struct stat sbuf;
  int size, save_size;
  char *base;
  LispPTR *naddress;
  int i;
  static char temp_buf[MESSAGE_BUFFER_SIZE];

  SETJMP(NIL);

  /* Get buff address from LISP */
  naddress = (LispPTR *)(NativeAligned4FromLAddr(args[0]));
  base = (char *)(NativeAligned2FromLAddr(((OneDArray *)naddress)->base));

  close(log_id);
  TIMEOUT(log_id = open(logfile, O_RDONLY));
  if (log_id == -1) return (NIL);
  TIMEOUT(i = fstat(log_id, &sbuf));
  if (i != 0) {
    OSMESSAGE_PRINT(printf("stat err\n"));
    return (NIL);
  }
  save_size = (int)(sbuf.st_size);
  size = save_size - previous_size;
  if (size > MESSAGE_BUFFER_SIZE)
    size = MESSAGE_BUFFER_SIZE;
  else
    logChanged = 0; /* only reset msg-pending flag if we cleaned it out! */
  TIMEOUT(i = lseek(log_id, previous_size, SEEK_SET));
  if (i == -1) {
    OSMESSAGE_PRINT(printf("seek err\n"));
    return (NIL);
  }

  /* Now, read "console output" */
  TIMEOUT(size = read(log_id, temp_buf, size));
  if (size == -1) {
    OSMESSAGE_PRINT(printf("read err\n"));
    return (NIL);
  }
  TIMEOUT(i = lseek(log_id, save_size, SEEK_SET));
  if (i == -1) {
    OSMESSAGE_PRINT(printf("seek err\n"));
    return (NIL);
  }
  /*
      TIMEOUT( close(id) );
  */
  previous_size += size;

  for (i = 0; i < size; ++i) {
    if (temp_buf[i] == '\n') temp_buf[i] = '\000';
  }
  /* COPY actual Lisp Buffer(for BYTESWAP magic) */
  StrNCpyFromCToLisp(base, temp_buf, size);

  return (GetSmallp(size));
#else
  return (NIL);
#endif /* MAIKO_HANDLE_CONSOLE_MESSAGES */
}

/************************************************************************/
/*									*/
/*			f l u s h _ p t y				*/
/*									*/
/*	Called whenever the console/standard-error PTY signals (IO)	*/
/*	that there's a message waiting to be printed.  Copies the	*/
/*	msg to the log file, sets the "msg waiting" semaphore, and	*/
/*	signals the log-message-waiting Lisp interrupt.			*/
/*									*/
/************************************************************************/

LispPTR flush_pty(void) {
#ifdef MAIKO_HANDLE_CONSOLE_MESSAGES
  struct stat sbuf;
  char buf[MESSAGE_BUFFER_SIZE]; /* Buffer between pty and log file */
  int size;
  static fd_set rfds;
  int rval;
  struct statvfs vfsbuf;
#ifndef LOGINT
  struct timeval selecttimeout = {0, 0};
#endif

  SETJMP(NIL);
  DBPRINT(("flush_pty() called.\n"));
/* polling pty nd flush os message to log file */
#ifndef LOGINT
  FD_SET(cons_pty, &rfds);
  if (select(32, &rfds, NULL, NULL, &selecttimeout) <= 0) return (NIL);

  if ((cons_pty >= 0) && FD_ISSET(cons_pty, &rfds))
#else /* LOGINT */

  if ((cons_pty >= 0) && ((size = read(cons_pty, buf, MESSAGE_BUFFER_SIZE - 1)) > 0))
#endif
  { /* There are messages to log in the file. */
    DBPRINT(("Log msgs being printed...\n"));
    close(log_id);
    TIMEOUT(log_id = open(logfile, O_WRONLY | O_APPEND, 0666));
    if (log_id == -1) return (NIL);
#ifndef LOGINT
    size = read(cons_pty, buf, MESSAGE_BUFFER_SIZE - 1);
#endif
    if (size == -1) return (NIL);

    /* Check free space to avoid printing System Error Message
       to /dev/console */
    TIMEOUT(rval = statvfs("/tmp", &vfsbuf));
    if (rval != 0) return (NIL);

    if (vfsbuf.f_bavail <= (long)0) {
      /* No Free Space */
      error("osmessage error: No free space on file system (/tmp).");
      return (NIL);
    }
    logChanged = 1; /* Note the change, for READP */
    TIMEOUT(rval = write(log_id, buf, size));
    if (rval == -1) {
      if (errno == ENOSPC) /* == 28 on Sun, ibm */
      {
        /* No free space, but it's too late to avoid
           print system Error Message. */
        error("osmessage error: No free space on file system (/tmp).");
        return (NIL);
      } else {
        error("osmessage error: cannot write to log file (/tmp)");
        return (NIL);
      }
    } else {
      /**
                  close(id);
      **/
      return (ATOM_T);
    }
  }
  return (NIL);
#else
  return (NIL);
#endif /* MAIKO_HANDLE_CONSOLE_MESSAGES */
}
