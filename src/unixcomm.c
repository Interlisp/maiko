/* %Z% %M% Version %I% (%G%). copyright venue & Fuji Xerox  */

/*

Unix Interface Communications

*/

/* Don't compile this at all under DOS. */
#ifndef DOS

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* Needed for ptsname on glibc systems. */
#endif

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 by Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "lispemul.h"

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h> /* JRB - timeout.h needs setjmp.h */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "cell.h"
#include "stack.h"
#include "arith.h"
#include "dbprint.h"
#include "timeout.h"

#include "unixcommdefs.h"
#include "byteswapdefs.h"
#include "commondefs.h"

static inline ssize_t SAFEREAD(int f, unsigned char *b, int c) {
  ssize_t res;
  do {
    res = read(f, b, c);
    if (res >= 0) return (res);
  } while (errno == EINTR || errno == EAGAIN);
  perror("reading UnixPipeIn");
  return (res);
}

#include "locfile.h" /* for LispStringToCString. */

/* JDS fixing prototypes char *malloc(size_t); */

int NPROCS = 100;

/* The following globals are used to communicate between Unix
   subprocesses and LISP */

/* One of these structures exists for every possible file descriptor */
/* type field encodes kind of stream:                                */

enum UJTYPE {
  UJUNUSED = 0,
  UJSHELL = -1,   /* PTY shell */
  UJPROCESS = -2, /* random process */
  UJSOCKET = -3,  /* socket open for connections */
  UJSOSTREAM = -4 /* connection from a UJSOCKET */
};

/* These are indexed by WRITE socket# */
struct unixjob {
  char *pathname; /* used by Lisp direct socket access subr */
  int PID;        /* process ID associated with this slot */
  int status;     /* status returned by subprocess (not shell) */
  enum UJTYPE type;
};

struct unixjob *UJ; /* allocated at run time */

long StartTime; /* Time, for creating pipe filenames */

#define valid_slot(slot) ((slot) >= 0 && (slot) < NPROCS && UJ[slot].type != UJUNUSED)

char shcom[2048]; /* Here because I'm suspicious of */
                  /* large allocations on the stack */

/************************************************************************/
/*									*/
/*		f i n d _ p r o c e s s _ s l o t			*/
/*									*/
/*	Find the slot in UJ with process id 'pid'.		        */
/*	Returns the slot #, or -1 if pid isn't found                    */
/*									*/
/*									*/
/************************************************************************/

int find_process_slot(int pid)
/* Find a slot with the specified pid */

{
  for (int slot = 0; slot < NPROCS; slot++)
    if (UJ[slot].PID == pid) {
      DBPRINT(("find_process_slot = %d.\n", slot));
      return slot;
    }
  return -1;
}

/************************************************************************/
/*									*/
/*		w a i t _ f o r _ c o m m _ p r o c e s s e s		*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void wait_for_comm_processes(void) {
  int pid;
  int slot;
  unsigned char d[6];

  memset(d, 0, sizeof(d));
  d[0] = 'W';
  write(UnixPipeOut, d, 6);
  SAFEREAD(UnixPipeIn, d, 6);

  pid = (d[0] << 8) | d[1] | (d[4] << 16) | (d[5] << 24);
  while (pid != 0) {
    slot = find_process_slot(pid);
    /* Ignore processes that we didn't start (shouldn't happen but
       occasionally does) */
    if (slot >= 0) {
      if (d[2] == 0) {
        DBPRINT(("Process %d exited status %d\n", pid, d[3]));
        UJ[slot].status = d[3];
      } else {
        DBPRINT(("Process %d terminated with signal %d\n", pid, d[2]));
        UJ[slot].status = (d[2] << 8);
      }
    }
    /* Look for another stopped process. */
    memset(d, 0, sizeof(d));
    d[0] = 'W';
    write(UnixPipeOut, d, 6);
    SAFEREAD(UnixPipeIn, d, 6);

    pid = (d[0] << 8) | d[1] | (d[4] << 16) | (d[5] << 24);
  }
}

/************************************************************************/
/*									*/
/*		b u i l d _ s o c k e t _ p a t h n a m e               */
/*									*/
/*	Returns a string which is the pathname associated with a        */
/*       socket descriptor.  Has ONE string buffer.                     */
/************************************************************************/
char *build_socket_pathname(int desc) {
  static char PathName[50];

  sprintf(PathName, "/tmp/LPU%ld-%d", StartTime, desc);
  return (PathName);
}

/************************************************************************/
/*									*/
/*		c l o s e _ u n i x _ d e s c r i p t o r s             */
/*									*/
/*	Kill off forked PTY-shells and forked-command processes		*/
/*	Also close sockets						*/
/*									*/
/************************************************************************/

void close_unix_descriptors(void) /* Get ready to shut Maiko down */
{
  for (int slot = 0; slot < NPROCS; slot++) {
    /* If this slot has an active job */
    switch (UJ[slot].type) {
      case UJUNUSED:
        break;
      case UJSHELL:
        if (kill(UJ[slot].PID, SIGKILL) < 0) perror("Killing shell");
        UJ[slot].PID = 0;
        DBPRINT(("Kill 5 closing shell desc %d.\n", slot));
        close(slot);
        break;

      case UJPROCESS:
        if (kill(UJ[slot].PID, SIGKILL) < 0) perror("Killing process");
        UJ[slot].PID = 0;
        DBPRINT(("Kill 5 closing process desc %d.\n", slot));
        close(slot);
        break;

      case UJSOCKET:
        close(slot);
        if (UJ[slot].pathname != NULL) {
          /* socket created directly from Lisp; pathname is in .pathname */
          DBPRINT(("Closing socket %d bound to %s\n", slot, UJ[slot].pathname));
          unlink(UJ[slot].pathname);
          free(UJ[slot].pathname);
          UJ[slot].pathname = NULL;
        }
        break;

      case UJSOSTREAM: close(slot); break;
    }
    UJ[slot].type = UJUNUSED;
  }

  /* make sure everyone's really dead before proceeding */
  wait_for_comm_processes();
}

/************************************************************************/
/*								        */
/*			F i n d U n i x P i p e s		        */
/*								        */
/*   Find the file descriptors of the UnixPipe{In,Out} pipes	        */
/*    and a few other important numbers that were set originally        */
/*    before the unixcomm process was forked off; it stuck them in the  */
/*    environment so we could find them after the original lde process  */
/*    got overlaid with the real emulator			        */
/*                                                                      */
/************************************************************************/

int FindUnixPipes(void) {
  char *envtmp;
  int inttmp;
  struct unixjob cleareduj;

  DBPRINT(("Entering FindUnixPipes\n"));
  UnixPipeIn = UnixPipeOut = StartTime = UnixPID = -1;
  if ((envtmp = getenv("LDEPIPEIN"))) {
    errno = 0;
    inttmp = (int)strtol(envtmp, (char **)NULL, 10);
    if (errno == 0)
      UnixPipeIn = inttmp;
  }
  if ((envtmp = getenv("LDEPIPEOUT"))) {
    errno = 0;
    inttmp = (int)strtol(envtmp, (char **)NULL, 10);
    if (errno == 0)
      UnixPipeOut = inttmp;
  }
  if ((envtmp = getenv("LDESTARTTIME"))) {
    errno = 0;
    inttmp = (int)strtol(envtmp, (char **)NULL, 10);
    if (errno == 0)
      StartTime = inttmp;
  }
  if ((envtmp = getenv("LDEUNIXPID"))) {
    errno = 0;
    inttmp = (int)strtol(envtmp, (char **)NULL, 10);
    if (errno == 0)
      UnixPID = inttmp;
  }

/* This is a good place to initialize stuff like the UJ table */
  NPROCS = sysconf(_SC_OPEN_MAX);

  UJ = (struct unixjob *)malloc(NPROCS * sizeof(struct unixjob));
  cleareduj.status = -1;
  cleareduj.pathname = NULL;
  cleareduj.PID = 0;
  cleareduj.type = UJUNUSED;
  for (int i = 0; i < NPROCS; i++) UJ[i] = cleareduj;

  DBPRINT(("NPROCS is %d; leaving FindUnixPipes\n", NPROCS));
  return (UnixPipeIn == -1 || UnixPipeOut == -1 || StartTime == -1 || UnixPID == -1);
}

/************************************************************************/
/*									*/
/*		    F i n d A v a i l a b l e P t y			*/
/*									*/
/*	Fill string Slave with the path name to the slave		*/
/*	pseudo-terminal.						*/
/*									*/
/*	Return the fd for the master psuedo-terminal.			*/
/*									*/
/*	This uses POSIX pseudoterminals.				*/
/*									*/
/************************************************************************/

static int FindAvailablePty(char *Slave) {
  int res;

  res = posix_openpt(O_RDWR);
  if (res < 0) {
    perror("open_pt failed");
    return (-1);
  }
  grantpt(res);
  unlockpt(res);
  strcpy(Slave, ptsname(res));
  DBPRINT(("slave pty name is %s.\n", Slave));

  if (res != -1) {
    fcntl(res, F_SETFL, fcntl(res, F_GETFL, 0) | O_NONBLOCK);
    return (res);
  }
  return (-1);
}

/************************************************************************/
/*                                                                      */
/*  U n i x _ h a n d l e c o m m                                       */
/*                                                                      */
/*	LISP subr to talk to the forked "Unix process".                     */
/*                                                                      */
/*	The first argument (Arg[0]) is the command number.                  */
/*	Second argument (Arg[1]) is the Job # (except as indicated).        */
/*                                                                      */
/*	Commands are:                                                       */
/*                                                                      */
/*		0 Fork Pipe, Arg1 is a string for system();                     */
/*		     => Job # or NIL                                            */
/*		1 Write Byte, Arg2 is Byte;                                     */
/*		     => 1 (success), NIL (fail)                                 */
/*		2 Read Byte => Byte, NIL (no data), or T (EOF)                  */
/*		3 Kill Job => Status or T                                       */
/*		4 Fork PTY to Shell (no args) => Job # or NIL                   */
/*		5 Kill All (no args) => T                                       */
/*		6 Close (EOF)                                                   */
/*		7 Job status => T or status                                     */
/*		8 => the largest supported command # (15 now)                   */
/*		9 Read Buffer, Arg1 = vmempage (512 byte buffer)                */
/*		     => byte count (<= 512), NIL (no data), or T (EOF)          */
/*	   10 Set Window Size, Arg2 = rows, Arg3 = columns                  */
/*	   11 Fork PTY to Shell (obsoletes command 4)                       */
/*        Arg1 = termtype, Arg2 = shell command string                    */
/*		     => Job # or NIL                                            */
/*     12 Create Unix Socket                                            */
/*        Arg1 = pathname to bind socket to (string)                    */
/*           => Socket # or NIL                                         */
/*     13 Try to accept on unix socket                                  */
/*           => Accepted socket #, NIL (fail) or T (try again)          */
/*     14 Query job type                                                */
/*           => type number or NIL if not a job                         */
/*     15 Write Buffer, Arg1 = Job #, Arg2 = vmempage,                  */
/*           Arg3 = # of bytes to write from buffer                     */
/*           => # of bytes written or NIL (failed)                      */
/*                                                                      */
/************************************************************************/

LispPTR Unix_handlecomm(LispPTR *args) {
  int command, dest, slot;
  unsigned char d[6];
  unsigned char ch;
  unsigned char buf[1];

  /* Get command */
  N_GETNUMBER(args[0], command, bad);
  DBPRINT(("\nUnix_handlecomm: command %d\n", command));

  switch (command) {
    case 0: /* Fork pipe process */
    {
      char *PipeName;
      int PipeFD, sockFD;

      /* First create the socket */
      struct sockaddr_un sock;
      sockFD = socket(AF_UNIX, SOCK_STREAM, 0);
      if (sockFD < 0) {
        perror("socket open");
        return (NIL);
      }

      /* then bind it to a canonical pathname */
      PipeName = build_socket_pathname(sockFD);
      memset(&sock, 0, sizeof(sock));
      sock.sun_family = AF_UNIX;
      strcpy(sock.sun_path, PipeName);
      if (bind(sockFD, (struct sockaddr *)&sock, sizeof(struct sockaddr_un)) < 0) {
        close(sockFD);
        perror("binding sockets");
        unlink(PipeName);
        return (NIL);
      }

      DBPRINT(("Socket %d bound to name %s.\n", sockFD, PipeName));

      if (listen(sockFD, 1) < 0) perror("Listen");

      memset(d, 0, sizeof(d));
      d[0] = 'F';
      d[3] = sockFD;
      write(UnixPipeOut, d, 6);
      WriteLispStringToPipe(args[1]);

      DBPRINT(("Sending cmd string: %s\n", shcom));

      /* Get status */
      SAFEREAD(UnixPipeIn, d, 6);

      /* If it worked, return job # */
      if (d[3] == 1) {
      case0_lp:
        TIMEOUT(PipeFD = accept(sockFD, NULL, NULL));
        if (PipeFD < 0) {
          if (errno == EINTR) goto case0_lp;
          perror("Accept.");
          close(sockFD);
          if (unlink(PipeName) < 0) perror("Unlink");
          return (NIL);
        }
        if (fcntl(PipeFD, F_SETFL, fcntl(PipeFD, F_GETFL, 0) | O_NONBLOCK) == -1) {
          perror("setting up fifo to nodelay");
          return (NIL);
        }
        UJ[PipeFD].type = UJPROCESS;
        UJ[PipeFD].status = -1;
        UJ[PipeFD].PID = (d[1] << 8) | d[2] | (d[4] << 16) | (d[5] << 24);
        close(sockFD);
        unlink(PipeName);
        DBPRINT(("New process: slot/PipeFD %d PID %d\n", PipeFD, UJ[PipeFD].PID));
        return (GetSmallp(PipeFD));
      } else {
        DBPRINT(("Fork request failed."));
        close(sockFD);
        unlink(PipeName);
        return (NIL);
      }
    }

    case 1: /* Write byte */
      /* Get job #, Byte */
      N_GETNUMBER(args[1], slot, bad);
      N_GETNUMBER(args[2], dest, bad);
      ch = dest; /* ch is a char */

      if (valid_slot(slot) && (UJ[slot].status == -1)) switch (UJ[slot].type) {
          case UJPROCESS:
          case UJSHELL:
          case UJSOSTREAM:
            dest = write(slot, &ch, 1);
            if (dest == 0) {
              wait_for_comm_processes();
              return (NIL);
            } else
              return (GetSmallp(1));

	  case UJSOCKET:
	  case UJUNUSED:
	    return (NIL);
        }
      break;

    case 2: /* Read byte */
      /**********************************************************/
      /* 							    */
      /* NB that it is possible for the other end of the stream */
      /* to have terminated, and hence status != -1.	    */
      /* EVEN IF THERE ARE STILL CHARACTERS TO READ.	    */
      /* 							    */
      /**********************************************************/

      N_GETNUMBER(args[1], slot, bad); /* Get job # */

      if (!valid_slot(slot)) return (NIL); /* No fd open; punt the read */
      switch (UJ[slot].type) {
        case UJPROCESS:
        case UJSHELL:
        case UJSOSTREAM:
          TIMEOUT(dest = read(slot, buf, 1));
          if (dest > 0) return (GetSmallp(buf[0]));
          /* Something's amiss; check our process status */
          wait_for_comm_processes();
          if ((dest == 0) &&
              (UJ[slot].status == -1)) { /* No available chars, but other guy still running */
            DBPRINT(("dest = 0, status still -1\n"));
            return (ATOM_T);
          }
          if ((UJ[slot].status == -1) &&
              ((errno == EWOULDBLOCK) ||
               (errno == EAGAIN))) { /* No available chars, but other guy still running */
            DBPRINT((" dest<0, EWOULDBLOCK\n"));
            return (ATOM_T);
          }
          /* At this point, we either got an I/O error, or there */
          /* were no chars available and the other end has terminated. */
          /* Either way, signal EOF. */
          DBPRINT(("Indicating EOF from PTY desc %d.\n", slot));
          return (NIL);

	case UJSOCKET:
	case UJUNUSED:
	    return (NIL);
      }

    case 3: /* Kill process */
            /* Maiko uses this as CLOSEF, so "process" is a misnomer */

      N_GETNUMBER(args[1], slot, bad);

      DBPRINT(("Terminating process in slot %d.\n", slot));
      if (!valid_slot(slot)) return (ATOM_T);
      /* in all cases we need to close() the file descriptor */
      if (slot == 0) DBPRINT(("ZERO SLOT\n"));
      close(slot);
      switch (UJ[slot].type) {
      case UJSHELL:
      case UJPROCESS:
        /* wait for up to 0.1s for it to exit on its own after the close() */
        for (int i = 0; i < 10; i++) {
          wait_for_comm_processes();
          if (UJ[slot].status != -1) break;
          usleep(10000);
        }
        /* check again before we terminate it */
        if (UJ[slot].status != -1) break;
        kill(UJ[slot].PID, SIGKILL);
        for (int i = 0; i < 10; i++) {
          /* Waiting for the process to exit is possibly risky.
             Sending SIGKILL is always supposed to kill
             a process, but on very rare occurrences this doesn't
             happen because of a Unix kernel bug, usually a user-
             written device driver which hasn't been fully
             debugged.  So we time it out just be safe. */
          wait_for_comm_processes();
          usleep(10000);
          if (UJ[slot].status != -1) break;
        }
        break;
      case UJSOCKET:
        if (UJ[slot].pathname) {
          DBPRINT(("Unlinking %s\n", UJ[slot].pathname));
          if (unlink(UJ[slot].pathname) < 0) perror("Kill 3 unlink");
          free(UJ[slot].pathname);
          UJ[slot].pathname = NULL;
        }
        break;
      case UJSOSTREAM:
      case UJUNUSED:
	break;
      }
      UJ[slot].type = UJUNUSED;
      UJ[slot].PID = 0;
      UJ[slot].pathname = NULL;

      /* If status available, return it, otherwise T */
      return (GetSmallp(UJ[slot].status));

    case 4:
    case 11: /* Fork PTY process */
    {
      char SlavePTY[32];
      int Master;
      unsigned short len;

      Master = FindAvailablePty(SlavePTY);
      DBPRINT(("Fork Shell; Master PTY = %d. Slave=%c%c.\n", Master, SlavePTY[0], SlavePTY[1]));
      if (Master < 0) {
        printf("Open of lisp side of PTY failed.\n");
        fflush(stdout);
        return (NIL);
      }

      d[0] = (command == 4) ? 'S' : 'P';
      d[1] = SlavePTY[0];
      d[2] = SlavePTY[1];
      d[3] = Master;
      d[4] = '\0';
      d[5] = '\0';
      write(UnixPipeOut, d, 6);

      len = strlen(SlavePTY) + 1;
      write(UnixPipeOut, &len, 2);
      write(UnixPipeOut, SlavePTY, len);

      if (command != 4) { /* New style has arg1 = termtype, arg2 = command */
        WriteLispStringToPipe(args[1]);
        WriteLispStringToPipe(args[2]);
      }

      /* Get status */
      SAFEREAD(UnixPipeIn, d, 6);

      /* If successful, return job # */
      DBPRINT(("Pipe/fork result = %d.\n", d[3]));
      if (d[3] == 1) {
        /* Set up the IO not to block */
        fcntl(Master, F_SETFL, fcntl(Master, F_GETFL, 0) | O_NONBLOCK);

        UJ[Master].type = UJSHELL; /* so we can find them */
        UJ[Master].PID = (d[1] << 8) | d[2] | (d[4] << 16) | (d[5] << 24);
        printf("Shell job %d, PID = %d\n", Master, UJ[Master].PID);
        UJ[Master].status = -1;
        DBPRINT(("Forked pty in slot %d.\n", Master));
        return (GetSmallp(Master));
      } else {
        printf("Fork failed.\n");
        fflush(stdout);
        printf("d = %d, %d, %d, %d, %d, %d\n", d[0], d[1], d[2], d[3], d[4], d[5]);
        close(Master);
        return (NIL);
      }
    }

    case 5: /* Kill all the subprocesses */ close_unix_descriptors(); return (ATOM_T);

    case 6: /* Kill this subprocess */
      memset(d, 0, sizeof(d));
      d[0] = 'C';

      /* Get job # */
      N_GETNUMBER(args[1], dest, bad);
      d[1] = dest;

      d[3] = 1;
      write(UnixPipeOut, d, 6);

      /* Get status */
      SAFEREAD(UnixPipeIn, d, 6);

      switch (UJ[dest].type) {
        case UJUNUSED:
          break;

        case UJSHELL:
          DBPRINT(("Kill 5 closing shell desc %d.\n", dest));
          close(dest);
          break;

        case UJPROCESS:
          DBPRINT(("Kill 5 closing process desc %d.\n", dest));
          close(dest);
          break;

        case UJSOCKET:
          /* close a socket; be sure and unlink the file handle */
          DBPRINT(("Kill 5 closing raw socket desc %d.\n", dest));
          close(dest);
          if (UJ[dest].pathname != NULL) {
            unlink(UJ[dest].pathname);
            free(UJ[dest].pathname);
            UJ[dest].pathname = NULL;
          } /* else return an error somehow... */
          break;

        case UJSOSTREAM:
          DBPRINT(("Kill 5 closing socket stream %d.\n", dest));
          close(dest);
          break;
      }

      UJ[dest].type = UJUNUSED;
      UJ[dest].PID = 0;
      return (ATOM_T);
    /* break; */

    case 7: /* Current job status */

      N_GETNUMBER(args[1], slot, bad); /* Get job # */
      wait_for_comm_processes();       /* Make sure we're up to date */

      if (UJ[slot].status == -1)
        return (ATOM_T);
      else
        return (GetSmallp(UJ[slot].status));

    case 8: /* Return largest supported command */ return (GetSmallp(15));

    case 9: /* Read buffer */
      /**********************************************************/
      /* 							    */
      /* NB that it is possible for the other end of the stream */
      /* to have terminated, and hence ForkedStatus != -1.	    */
      /* EVEN IF THERE ARE STILL CHARACTERS TO READ.	    */
      /* 							    */
      /**********************************************************/

      {
        DLword *bufp;
        int terno; /* holds errno thru sys calls after I/O fails */

        N_GETNUMBER(args[1], slot, bad);     /* Get job # */
        if (!valid_slot(slot)) return (NIL); /* No fd open; punt the read */

        bufp = (NativeAligned2FromLAddr(args[2])); /* User buffer */
        DBPRINT(("Read buffer slot %d, type is %d buffer LAddr 0x%x (native %p)\n", slot, UJ[slot].type, args[2], bufp));

        switch (UJ[slot].type) {
          case UJSHELL:
          case UJPROCESS:
          case UJSOSTREAM: dest = read(slot, bufp, 512);
#ifdef BYTESWAP
            word_swap_page(bufp, 128);
#endif /* BYTESWAP */

            if (dest > 0) { /* Got characters.  If debugging, print len &c */
              /* printf("got %d chars\n", dest); */
              return (GetSmallp(dest));
            }

            /* Something's amiss; update process status */
            DBPRINT(("Problem: Got status %d from read, errno %d.\n", dest, errno));
            wait_for_comm_processes(); /* make sure we're up to date */
            if (((dest == 0) || (errno == EINTR) || (errno == 0) || (errno == EAGAIN) ||
                 (errno == EWOULDBLOCK)) &&
                (UJ[slot].status == -1))
              /* No available chars, but other guy still running */
              return (ATOM_T);

            /* At this point, we either got an I/O error, or there */
            /* were no chars available and the other end has terminated. */
            /* Either way, signal EOF. */
            DBPRINT(("read failed; dest = %d, errno = %d, status = %d\n", dest, terno,
                     UJ[slot].status));
            DBPRINT(("Indicating EOF from PTY desc %d.\n", slot));
            return (NIL);

          case UJSOCKET:
          case UJUNUSED:
            return (NIL);
        }
      }

    case 10: /* Change window */
    {
      int rows, cols, pgrp, pty;
      struct winsize w;

      /* Get job #, rows, columns */
      N_GETNUMBER(args[1], slot, bad);
      N_GETNUMBER(args[2], rows, bad);
      N_GETNUMBER(args[3], cols, bad);

      if (valid_slot(slot) && (UJ[slot].type == UJSHELL) && (UJ[slot].status == -1)) {
        w.ws_row = rows;
        w.ws_col = cols;
        w.ws_xpixel = 0; /* not used */
        w.ws_ypixel = 0;
        pty = slot;
        /* Change window size, then
           notify process group of the change */
        if ((ioctl(pty, TIOCSWINSZ, &w) >= 0) &&
            ((pgrp = tcgetpgrp(pty)) >= 0) &&
            (killpg(pgrp, SIGWINCH) >= 0))
          return (ATOM_T);
        return (GetSmallp(errno));
      }

      return (NIL);
    }

    case 12: /* create Unix socket */

    {
      int sockFD;
      struct sockaddr_un sock;

      /* First open the socket */
      sockFD = socket(AF_UNIX, SOCK_STREAM, 0);
      if (sockFD < 0) {
        perror("socket open");
        return (NIL);
      }
      /* Then get a process slot and blit the pathname of the
         socket into it */
      /* need to type-check the string here */
      LispStringToCString(args[1], shcom, 2048);
      UJ[sockFD].pathname = malloc(strlen(shcom) + 1);
      strcpy(UJ[sockFD].pathname, shcom);
      /* Then bind it to the pathname, and get it	listening properly */

      sock.sun_family = AF_UNIX;
      strcpy(sock.sun_path, shcom);
      if (bind(sockFD, (struct sockaddr *)&sock, sizeof(struct sockaddr_un)) < 0) {
        close(sockFD);
        free(UJ[sockFD].pathname);
        UJ[sockFD].type = UJUNUSED;
        perror("binding Lisp sockets");
        return (NIL);
      }
      DBPRINT(("Socket %d bound to name %s.\n", sockFD, shcom));
      if (listen(sockFD, 1) < 0) perror("Listen");
      /* Set up the IO not to block */
      fcntl(sockFD, F_SETFL, fcntl(sockFD, F_GETFL, 0) | O_NONBLOCK);

      /* things seem sane, fill out the rest of the UJ slot and return */
      UJ[sockFD].status = -1;
      UJ[sockFD].PID = -1;
      UJ[sockFD].type = UJSOCKET;

      return (GetSmallp(sockFD));
    }

    case 13: /* try to accept */
    {
      /* returns file descriptor if successful,
         -1 if no connection available,
         NIL if failure */
      int sockFD, newFD;

      N_GETNUMBER(args[1], sockFD, bad);
      if (UJ[sockFD].type == UJSOCKET && UJ[sockFD].pathname != NULL) {
      /* sockFD SHOULD be non-blocking;
         but I'll time this out just in case */
      case13_lp:
        TIMEOUT(newFD = accept(sockFD, NULL, NULL));
        if (newFD < 0)
          if (errno == EINTR)
            goto case13_lp;
          else if (errno == EWOULDBLOCK)
            return (GetSmallp(-1));
          else {
            perror("Lisp socket accept");
            return (NIL);
          }
        else {
          UJ[newFD].status = -1;
          UJ[newFD].PID = -1;
          UJ[newFD].type = UJSOSTREAM;
          return (GetSmallp(newFD));
        }
      } else
        return (NIL);
    }

    case 14: /* return type of socket */
    {
      int streamFD;

      N_GETNUMBER(args[1], streamFD, bad);
      if (valid_slot(streamFD))
        return GetSmallp(UJ[streamFD].type);
      else
        return NIL;
    }

    case 15: /* Write buffer */
    {
      DLword *bufp;
      int i;
      N_GETNUMBER(args[1], slot, bad);              /* Get job # */
      bufp = (NativeAligned2FromLAddr(args[2])); /* User buffer */
      N_GETNUMBER(args[3], i, bad);                 /* # to write */
      DBPRINT(("Write buffer, type is %d\n", UJ[slot].type));

      switch (UJ[slot].type) {
        case UJSHELL:
        case UJPROCESS:
        case UJSOSTREAM:
#ifdef BYTESWAP
          word_swap_page(bufp, (i + 3) >> 2);
#endif /* BYTESWAP */

          dest = write(slot, bufp, i);
#ifdef BYTESWAP
          word_swap_page(bufp, (i + 3) >> 2);
#endif /* BYTESWAP */

          if (dest > 0) return (GetSmallp(dest));
          /* Something's amiss; update process status */
          wait_for_comm_processes(); /* make sure we're up to date */
          if (((dest == 0) || (errno == EWOULDBLOCK)) && (UJ[slot].status == -1))
            /* No room to write, but other guy still running */
            return (ATOM_T);
          /* At this point, we either got an I/O error, or there */
          /* were no chars available and the other end has terminated. */
          /* Either way, signal EOF. */
          DBPRINT(("Indicating write failure from PTY desc %d.\n", slot));
          return (NIL);

        case UJUNUSED:
        case UJSOCKET:
          return (NIL);
      }
    }

    default: return (NIL);
  }

bad:
  DBPRINT(("Bad input value."));
  return (NIL);
}

/************************************************************************/
/*									*/
/*		W r i t e L i s p S t r i n g T o P i p e		*/
/*									*/
/*	Convert a lisp string to a C string (both format and byte-	*/
/*	order), write 2 bytes of length and the string			*/
/*									*/
/*									*/
/************************************************************************/

void WriteLispStringToPipe(LispPTR lispstr) {
  unsigned short len;
  LispStringToCString(lispstr, shcom, 2048);
  /* Write string length, then string */
  len = strlen(shcom) + 1;
  write(UnixPipeOut, &len, 2);
  write(UnixPipeOut, shcom, len);
}

#endif /* DOS */
