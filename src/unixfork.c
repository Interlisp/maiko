/* $Id: unixfork.c,v 1.6 2001/12/26 22:17:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: unixfork.c,v 1.6 2001/12/26 22:17:05 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*		Code to fork a subprocess for Unix communication	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1998 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef OS5
#include <sys/stropts.h>
#define FULLSLAVENAME
#endif

#include "dbprint.h"
#include "unixfork.h"

#ifdef DEBUG
/* required by DBPRINT from dbprint.h */
extern int flushing = 0;
#endif

/* The following globals are used to communicate between Unix
   subprocesses and LISP */

long StartTime; /* Time, for creating pipe filenames */

char shcom[512]; /* Here because I'm suspicious of */
                 /* large allocations on the stack */


static __inline__ ssize_t
SAFEREAD(int f, char *b, int c)
{
  ssize_t res;
loop:
  res = read(f, b, c);
  if ((res < 0)) {
    if (errno == EINTR || errno == EAGAIN) goto loop;
    perror("reading UnixPipeIn");
  }
  return (res);
}

/************************************************************************/
/*									*/
/*			F o r k U n i x S h e l l			*/
/*									*/
/*	Fork a PTY connection to a C-shell process.			*/
/*	Returns PID of process, or -1 if something failed		*/
/*									*/
/*									*/
/************************************************************************/

/* Creates a PTY connection to a csh */

#ifdef FULLSLAVENAME
int ForkUnixShell(int slot, char *PtySlave, char *termtype, char *shellarg)
#else
int ForkUnixShell(int slot, char ltr, char numb, char *termtype, char *shellarg)
#endif
{
#ifndef FULLSLAVENAME
  char PtySlave[20];
#endif
  int res, PID, SlaveFD;
  struct termios tio;

  PID = fork();

  if (PID == 0) {
    char envstring[64];
    char *argvec[4];

#ifndef SYSVONLY
    /* Divorce ourselves from /dev/tty */
    res = open("/dev/tty", O_RDWR);
    if (res >= 0) {
      (void)ioctl(res, TIOCNOTTY, (char *)0);
      (void)close(res);
    } else {
      perror("Slave TTY");
      exit(0);
    }
#else
    if (0 > setsid()) /* create us a new session for tty purposes */
      perror("setsid");
#endif

/* Open the slave side */
#ifndef FULLSLAVENAME
    sprintf(PtySlave, "/dev/tty%c%c", ltr, numb);
#endif
    SlaveFD = open(PtySlave, O_RDWR);
    if (SlaveFD == -1) {
      perror("Slave Open");
      perror(PtySlave);
      exit(0);
    }

#ifdef OS5
    ioctl(SlaveFD, I_PUSH, "ptem");
    ioctl(SlaveFD, I_PUSH, "ldterm");
#endif /* OS5 */

    /* Set up as basic display terminal: canonical erase,
       kill processing, echo, backspace to erase, echo ctrl
       chars as ^x, kill line by backspacing */
    tcgetattr(SlaveFD, &tio);
    tio.c_lflag |= ICANON | ECHO | ECHOE | ECHOCTL | ECHOKE;
    tcsetattr(SlaveFD, TCSANOW, &tio);

    (void)dup2(SlaveFD, 0);
    (void)dup2(SlaveFD, 1);
    (void)dup2(SlaveFD, 2);
    (void)close(SlaveFD);

    /* set the LDESHELL variable so the underlying .cshrc can see it and
       configure the shell appropriately, though this may not be so important any more */
    setenv("LDESHELL", "YES", 1);

    if ((termtype[0] != 0) && (strlen(termtype) < 59)) { /* set the TERM environment var */
      setenv("TERM", termtype, 1);
    }
    /* Start up csh */
    argvec[0] = "csh";
    if (shellarg[0] != 0) { /* setup to run command */
      argvec[1] = "-c";     /* read commands from next arg */
      argvec[2] = shellarg;
      argvec[3] = (char *)0;
    } else
      argvec[1] = (char *)0;

    execv("/bin/csh", argvec);

    /* Should never get here */
    perror("execv");
    exit(0);
  } else { /* not the forked process. */
    if (shellarg != shcom) free(shellarg);
  }

  /* Set the process group so all the kids get the bullet too
  if (setpgrp(PID, PID) != 0)
    perror("setpgrp"); */

  return (PID);
}

/* fork_Unix is the secondary process spawned right after LISP is
   started, to avoid having TWO 8 mbyte images sitting around. It listens
   to the pipe LispToUnix waiting for requests, and responds on UnixToLisp.
   The data passed through this pipe is in 4 byte packets, of the form:

   Byte 0:   Command character, one of:
                   S: Fork PTY (shell) process. This is used for CHAT windows.
                   P: New version of S, takes 2 string args.
                   F: Fork piped shell, takes 1 string arg.
                   K: Kill process
                   E: Exit (kill all subprocesses)
                   C: Close stdin to subprocess
                   W: call WAIT3 & get one process's close info.
                   O: Fork OCR process.
   Byte 1:   Process number (0 to NPROCS - 1)
             Not used for S, F, and E commands
             [For S&P, pty letter]
             [For F, process # for pipe naming]
   Byte 2:   Value, used as follows:
             Only used for W command, contains byte to write
             [For S&P, pty number]
   Byte 3:   Slot number.

In the case of F & P commands, additional data follows the 4 byte packet.
This consists of 2 bytes representing the length of the shell command
string, and the string itself.

fork_Unix will return another 4 byte packet. The bytes are the same as those
of the packet received except:

   F:        Byte 2 is job number
             Byte 3 is 1 if successful, 0 if not
   S:	     Byte 2 is job number
             Byte 3 is 1 if successful, 0 if not
   R:        Byte 2 is value of byte read from stdin, if any
             Byte 3 is 1 if successful, 2 if EOF, 0 if nothing waiting
   W:        Bytes 0 & 1 are the Process ID of the terminated process
             Bytes 2 & 3 are the high & low bytes of the exit status.
   K:        Bytes 1 and 2 are the high and low bytes of the exit status
             of the process.
             Byte 3 is 1 if an exit status was available.
   E:        Always the same
   C:        Always the same
   O:	     Byte 3 is 1 if successful, 0 if not
             Byte 1 and Byte 2 are the process ID of OCR process

*/

int fork_Unix() {
  int LispToUnix[2], /* Incoming pipe from LISP */
      UnixToLisp[2], /* Outgoing pipe to LISP */
      UnixPID, LispPipeIn, LispPipeOut, res, slot;
  pid_t pid;

  char IOBuf[4];
  unsigned short tmp;
  char *cmdstring;

  /* Pipes between LISP subr and process */
  if (pipe(LispToUnix) == -1) {
    perror("pipe");
    exit(-1);
  }
  if (pipe(UnixToLisp) == -1) {
    perror("pipe");
    exit(-1);
  }

  StartTime = time(0);   /* Save the time, to create filenames with */
  StartTime &= 0xFFFFFF; /* as a positive number! */

/* interrupts need to be blocked here so subprocess won't see them */
#ifdef SYSVSIGNALS
  sighold(SIGVTALRM);
  sighold(SIGIO);
  sighold(SIGALRM);
  sighold(SIGXFSZ);
  sighold(SIGFPE);
#else
  sigblock(sigmask(SIGVTALRM) | sigmask(SIGIO) | sigmask(SIGALRM)
           | sigmask(SIGXFSZ)
           | sigmask(SIGFPE));
#endif /* SYSVSIGNALS */

  if ((UnixPID = fork()) == -1) { /* Fork off small version of the emulator */
    perror("fork");
    exit(-1);
  }

  if (UnixPID != 0) {
    /* JRB - fork_Unix is now called in ldeboot; leave UnixPipe{In,Out} open
       and put their numbers in the environment so parent can find them */
    /* JDS - NB that sprintf doesn't always return a string! */

    char *tempstring;

    tempstring = (char *)malloc(30);
    sprintf(tempstring, "%d", UnixToLisp[0]);
    setenv("LDEPIPEINE", tempstring, 1);

    tempstring = (char *)malloc(30);
    sprintf(tempstring, "%d", LispToUnix[1]);
    setenv("LDEPIPEOUT", tempstring, 1);

    tempstring = (char *)malloc(30);
    sprintf(tempstring, "%ld", StartTime);
    setenv("LDESTARTTIME", tempstring, 1);

    tempstring = (char *)malloc(30);
    sprintf(tempstring, "%d", UnixPID);
    setenv("LDEUNIXPID", tempstring, 1);

    close(LispToUnix[0]);
    close(UnixToLisp[1]);
    return (1);
  }

  LispPipeIn = LispToUnix[0];
  LispPipeOut = UnixToLisp[1];
  close(LispToUnix[1]);
  close(UnixToLisp[0]);

  res = fcntl(LispPipeIn, F_GETFL, 0);
  res &= (65535 - FNDELAY);
  res = fcntl(LispPipeIn, F_SETFL, res);

  while (1) {
    ssize_t len;
    len = 0;
    while (len != 4) {
      if ((len = SAFEREAD(LispPipeIn, IOBuf, 4)) < 0) { /* Get packet */
        perror("Packet read by slave");
        /*      kill_comm_processes(); */
        exit(0);
      }
      if (len != 4) {
        DBPRINT(("Input packet wrong length:  %d.\n", len));
        exit(0);
      }
    }
    slot = IOBuf[3];
    IOBuf[3] = 1; /* Start by signalling success in return-code */

    switch (IOBuf[0]) {
      case 'S':
      case 'P':          /* Fork PTY shell */
        if (slot >= 0) { /* Found a free slot */
          char termtype[32];
#ifdef FULLSLAVENAME
          char slavepty[32]; /* For slave pty name */

          if (SAFEREAD(LispPipeIn, (char *)&tmp, 2) < 0) perror("Slave reading slave pty len");
          if (SAFEREAD(LispPipeIn, slavepty, tmp) < 0) perror("Slave reading slave pty id");
#endif /* FULLSLAVENAME */

          if (IOBuf[0] == 'P') { /* The new style, which takes term type & command to csh */
            if (SAFEREAD(LispPipeIn, (char *)&tmp, 2) < 0) perror("Slave reading cmd length");
            if (SAFEREAD(LispPipeIn, termtype, tmp) < 0) perror("Slave reading termtype");
            if (SAFEREAD(LispPipeIn, (char *)&tmp, 2) < 0) perror("Slave reading cmd length");
            if (tmp > 510)
              cmdstring = (char *)malloc(tmp + 5);
            else
              cmdstring = shcom;

            if (SAFEREAD(LispPipeIn, cmdstring, tmp) < 0) perror("Slave reading shcom");
          } else /* old style, no args */
          {
            termtype[0] = 0;
            cmdstring = shcom;
            cmdstring[0] = 0;
          }

/* Alloc a PTY and fork  */
#ifdef FULLSLAVENAME
          pid = ForkUnixShell(slot, slavepty, termtype, cmdstring);
#else
          pid = ForkUnixShell(slot, IOBuf[1], IOBuf[2], termtype, cmdstring);
#endif

          if (pid == -1) {
            printf("Impossible failure from ForkUnixShell??\n");
            fflush(stdout);
            IOBuf[3] = 0;
          } else {
            /* ForkUnixShell sets the pid and standard in/out variables */
            IOBuf[1] = (pid >> 8) & 0xFF;
            IOBuf[2] = pid & 0xFF;
          }
        } else {
          printf("Can't get process slot for PTY shell.\n");
          fflush(stdout);
          IOBuf[3] = 0;
        }
        break;

      case 'F': /* Fork pipe command */
        if (slot >= 0) {
          /* Read in the length of the shell command, and then the command */
          if (SAFEREAD(LispPipeIn, (char *)&tmp, 2) < 0) perror("Slave reading cmd length");
          if (tmp > 510)
            cmdstring = (char *)malloc(tmp + 5);
          else
            cmdstring = shcom;
          if (SAFEREAD(LispPipeIn, cmdstring, tmp) < 0) perror("Slave reading cmd");
          DBPRINT(("Cmd len = %d.\n", tmp));
          DBPRINT(("Rev'd cmd string: %s\n", cmdstring));
          pid = fork(); /* Fork */

          if (pid == 0) {
            int i;
            int status, sock;
            struct sockaddr_un addr;
            char PipeName[40];
            sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock < 0) {
              perror("slave socket");
              exit(0);
            }
            sprintf(PipeName, "/tmp/LPU%ld-%d", StartTime, slot);
            memset(&addr, 0, sizeof(struct sockaddr_un));
            addr.sun_family = AF_UNIX;
            strcpy(addr.sun_path, PipeName);
            status =
                connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
            if (status < 0) {
              perror("slave connect");
              printf("Name = %s.\n", PipeName);
              fflush(stdout);
              exit(0);
            } else {
              DBPRINT(("Slave connected on %s.\n", PipeName));
            }

            /* Copy the pipes onto stdin, stdout, and stderr */
            dup2(sock, 0);
            dup2(sock, 1);
            dup2(sock, 2);

            /* Make sure everything else is closed. */
            for (i = 3; i < sysconf(_SC_OPEN_MAX); i++) close(i);

            /* Run the shell command and get the result */
            status = system(cmdstring);
            if (cmdstring != shcom) free(cmdstring);
            /* Comment out to fix USAR 11302 (FXAR 320)
            unlink(PipeName);
            */
            _exit((status & ~0xff) ? (status >> 8) : status);
          }

          /* Check for error doing the fork */
          if (pid == (pid_t)-1) {
            perror("unixcomm: fork");
            IOBuf[3] = 0;
          } else {
            IOBuf[1] = (pid >> 8) & 0xFF;
            IOBuf[2] = pid & 0xFF;
          }
        } else {
          printf("No process slots available.\n");
          IOBuf[3] = 0; /* Couldn't get a process slot */
        }
        break;

      case 'W': /* Wait for a process to die. */
      {
        int status;

#ifdef OCR
        int slot;
#endif

        status = 0;

        IOBuf[0] = 0;
        IOBuf[1] = 0;
        DBPRINT(("About to wait for processes.\n"));
      retry1:
        pid = waitpid(-1, &status, WNOHANG);
        if (pid == -1 && errno == EINTR) goto retry1;
        if (pid > 0) {
          /* Ignore processes which are suspended but haven't exited
             (this shouldn't happen) */
          if (WIFSTOPPED(status)) break;
          IOBuf[3] = status >> 8;
          IOBuf[2] = status & 0xFF;
          IOBuf[1] = pid & 0xFF;
          IOBuf[0] = (pid >> 8) & 0xFF;
        }
        DBPRINT(("wait3 returned pid = %d.\n", pid));
      }

      break;

      case 'C': /* Close stdin to subprocess */ break;

      case 'K': /* Kill subprocess */ break;

#ifdef OCR
      case 'w': /* Wait paticular process to die */
      {
        int pid, res, status;

        pid = IOBuf[1] << 8 | IOBuf[2];

      retry:
        res = waitpid(pid, &status, WNOHANG);
        if (res == -1 && errno == EINTR) goto retry;

        if (res == pid) {
          IOBuf[0] = res >> 24 & 0xFF;
          IOBuf[1] = res >> 16 & 0xFF;
          IOBuf[2] = res >> 8 & 0xFF;
          IOBuf[3] = res & 0xFF;
        } else {
          IOBuf[0] = IOBuf[1] = IOBuf[2] = IOBuf[3] = 0;
        }
      } break;

      case 'O': /* Fork OCR process */
        if (slot >= 0) {
          pid_t ppid;
          ppid = getppid();
          pid = fork();
          if (pid == 0) {
            int i;
            int status, len;
            struct sockaddr_un addr;
            char PipeName[40];
            extern int OCR_sv;

            OCR_sv = socket(AF_UNIX, SOCK_STREAM, 0);
            if (OCR_sv < 0) {
              perror("slave socket");
              exit(0);
            }
            sprintf(PipeName, "/tmp/LispPipe%d-%d", StartTime, slot);
            addr.sun_family = AF_UNIX;
            strcpy(addr.sun_path, PipeName);
            len = strlen(PipeName) + sizeof(addr.sun_family);
            status = connect(OCR_sv, &addr, len);
            if (status < 0) {
              perror("OCR slave connect");
              OCR_sv = -1;
              exit(0);
            }

            (void)ocr_proc(ppid);
            OCR_sv = -1;
            exit(1);
          }

          if (pid == -1) {
            perror("unixcomm: fork OCR");
            IOBuf[3] = 0;
          } else {
            IOBuf[1] = (pid >> 8) & 0xFF;
            IOBuf[2] = pid & 0xFF;
          }
        } else
          IOBuf[3] = 0;
        break;
#endif /* OCR */

    } /* End of switch */

    /* Return the status/data packet */
    write(LispPipeOut, IOBuf, 4);
  }
}

