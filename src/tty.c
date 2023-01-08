/* $Id: tty.c,v 1.2 1999/01/03 02:07:39 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "lispemul.h"
#include "lispmap.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lspglob.h"
#include "commondefs.h"
#include "tty.h"
#include "ttydefs.h"

static DLTTY_OUT_COMMAND *DLTTYPortCmd;
static DLTTY_IN_CSB *DLTTYIn;
static DLTTY_OUT_CSB *DLTTYOut;

static char *TTY_Dev;
static int TTY_Fd = -1;

extern fd_set LispReadFds;

void tty_init(void)
{
  TTY_Dev = "/dev/ttyb"; /* Modify device name */
  TTY_Fd = (-1);

  DLTTYPortCmd = (DLTTY_OUT_COMMAND *)NativeAligned2FromLAddr(IOPAGE_OFFSET + 20);
  DLTTYIn = (DLTTY_IN_CSB *)NativeAligned2FromLAddr(IOPAGE_OFFSET + 36);
  DLTTYOut = (DLTTY_OUT_CSB *)NativeAligned2FromLAddr(IOPAGE_OFFSET + 34);
}

void tty_open(void)
{
  struct termios options;

  if (TTY_Fd < 0) {
    if ((TTY_Fd = open(TTY_Dev, O_RDWR)) >= 0) {
      tcgetattr(TTY_Fd, &options);
      options.c_iflag &= ~(IMAXBEL|IXOFF|INPCK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IGNPAR);
      options.c_iflag |= IGNBRK;
      options.c_oflag &= ~OPOST;
      options.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|ISIG|IEXTEN|NOFLSH|TOSTOP);
      options.c_cflag &= ~(CSIZE|PARENB);
      options.c_cflag |= CS8|CREAD;
      options.c_cc[VMIN] = 1;
      options.c_cc[VTIME] = 0;
      tcsetattr(TTY_Fd, TCSANOW, &options);

      FD_SET(TTY_Fd, &LispReadFds);
#ifdef TTYINT
      int_io_open(TTY_Fd);
#endif

    } else {
      error("TTY: tty_open");
    }
  }
}

void tty_close(void)
{
  if (TTY_Fd >= 0) {
    FD_CLR(TTY_Fd, &LispReadFds);
    close(TTY_Fd);
#ifdef TTYINT
    int_io_close(TTY_Fd);
#endif
    TTY_Fd = (-1);
  }
}

void tty_get(void)
{
  char indata[256];

  if ((TTY_Fd >= 0) && !DLTTYIn->state) {
    DLTTYIn->in_data = '\0'; /* Clear Previous Data */

    if (read(TTY_Fd, indata, 1) == 1) {
      DLTTYIn->in_data = indata[0];
      DLTTYIn->state = 1;
    } else {
      error("TTY: tty_get");
    }
  }
}

void tty_put(void)
{
  char c;

  if (TTY_Fd >= 0) {
    c = DLTTYPortCmd->outdata;
    if (write(TTY_Fd, &c, 1) != 1) { error("TTY: tty_put()"); }
  }
}

static speed_t tty_baudtosymbol(short aBaud)
{
  /* This matches the constants in DLTTY where possible. */
  if (aBaud == 0) return (B50);
  if (aBaud == 1) return (B75);
  if (aBaud == 2) return (B110);
  if (aBaud == 3) return (B134);
  if (aBaud == 4) return (B150);
  if (aBaud == 5) return (B300);
  if (aBaud == 6) return (B600);
  if (aBaud == 7) return (B1200);
  if (aBaud == 8) return (B1800);
  /* 9 is defined to be 2000, not in POSIX */
  if (aBaud == 10) return (B2400);
  /* 11 is defined to be 3600, not in POSIX */
  if (aBaud == 12) return (B4800);
  /* 13 is defined to be 7200, not in POSIX */
  if (aBaud == 14) return (B9600);
  return (-1);
}

void tty_setbaudrate(void)
{
  speed_t baudrate;

  if (TTY_Fd >= 0) {
    if ((baudrate = tty_baudtosymbol(DLTTYOut->line_speed)) != -1) {
        struct termios options;
        tcgetattr(TTY_Fd, &options);
        cfsetispeed(&options, baudrate);
        cfsetospeed(&options, baudrate);
        tcsetattr(TTY_Fd, TCSANOW, &options);
    } else {
      error("TTY: tty_setbaudrate");
    }
  }
}

void tty_setparam(void)
{
  if (DLTTYPortCmd->outdata & SET_BAUD_RATE) tty_setbaudrate();
}

void tty_breakon(void)
{
  if (TTY_Fd >= 0) { ioctl(TTY_Fd, TIOCSBRK, 0); }
}

void tty_breakoff(void)
{
  if (TTY_Fd >= 0) { ioctl(TTY_Fd, TIOCCBRK, 0); }
}

void tty_cmd(void)
{
  if (DLTTYPortCmd->command >= PUT_CHAR) {
    if (DLTTYPortCmd->command == PUT_CHAR)
      tty_put();
    else if (DLTTYPortCmd->command == SET_PARAM)
      tty_setparam();
    else if (DLTTYPortCmd->command == TTY_ON)
      tty_open();
    else if (DLTTYPortCmd->command == TTY_OFF)
      tty_close();
    else if (DLTTYPortCmd->command == TTY_BREAK_ON)
      tty_breakon();
    else if (DLTTYPortCmd->command == TTY_BREAK_OFF)
      tty_breakoff();
    else
      error("TTY: tty_cmd");

    DLTTYPortCmd->command &= ~PUT_CHAR;
  }
}

void tty_debug(const char *name)
{
  printf("DEBUG: %s\n", name);
  printf("DEBUG: \t\tTTY_Dev            = \"%s\"\n", TTY_Dev);
  printf("DEBUG: \t\tTTY_Fd             = %d\n", TTY_Fd);

  if (TTY_Fd >= 0) {
    struct termios attr;
    tcgetattr(TTY_Fd, &attr);
    printf("DEBUG: \t\tTTY_Mode.sg_ispeed = %#x\n", (unsigned int)cfgetispeed(&attr));
    printf("DEBUG: \t\tTTY_Mode.sg_ospeed = %#x\n", (unsigned int)cfgetospeed(&attr));
  }

  printf("DEBUG:\n");
  printf("DEBUG: \t\tSymbol       Address        Contents\n");
  printf("DEBUG: \t\tIOPAGE       %p\n", (void *)NativeAligned2FromLAddr(IOPAGE_OFFSET));
  /* In the future, we could print out the various fields of DLTTYOut, DLTTYIn, and DLTTYPortCmd */
}
