/* $Id: tty.c,v 1.2 1999/01/03 02:07:39 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: tty.c,v 1.2 1999/01/03 02:07:39 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <sys/select.h>
#include "tty.h"

DLTTY_OUT_COMMAND *DLTTYPortCmd;
DLTTY_IN_CSB *DLTTYIn;
DLTTY_OUT_CSB *DLTTYOut;

char *TTY_Dev;
int TTY_Fd = -1;
extern fd_set LispReadFds;
struct sgttyb TTY_Mode;

void tty_init() {
#ifdef TRACE
  printf("TRACE: tty_init()\n");
#endif

  TTY_Dev = "/dev/ttyb"; /* Modify device name */
  TTY_Fd = (-1);

  DLTTYPortCmd = (DLTTY_OUT_COMMAND *)Addr68k_from_LADDR(IOPAGE_OFFSET + 20);
  DLTTYIn = (DLTTY_IN_CSB *)Addr68k_from_LADDR(IOPAGE_OFFSET + 36);
  DLTTYOut = (DLTTY_OUT_CSB *)Addr68k_from_LADDR(IOPAGE_OFFSET + 34);
} /* tty_init end */

void tty_open() {
  int stat;

#ifdef TRACE
  printf("TRACE: tty_open()\n");
#endif

  if (TTY_Fd < 0) {
    if ((TTY_Fd = open(TTY_Dev, O_RDWR)) >= 0) {
      stat = ioctl(TTY_Fd, TIOCGETP, &TTY_Mode);
      TTY_Mode.sg_flags = RAW;
      stat = ioctl(TTY_Fd, TIOCSETP, &TTY_Mode);

      FD_SET(TTY_Fd, &LispReadFds);
#ifdef TTYINT
      int_io_open(TTY_Fd);
#endif

    } else {
      error("TTY: tty_open");
    }
  }
} /* tty_open end */

void tty_close() {
  int stat;

#ifdef TRACE
  printf("TRACE: tty_close()\n");
#endif

  if (TTY_Fd >= 0) {
    FD_CLR(TTY_Fd, &LispReadFds);
    stat = close(TTY_Fd);
#ifdef TTYINT
    int_io_close(TTY_Fd);
#endif
    TTY_Fd = (-1);
  }
} /* tty_close end */

void TTY_get() {
  char indata[256];
  int count;

#ifdef TRACE
  printf("TRACE: tty_get()\n");
#endif

  if ((TTY_Fd >= 0) && !DLTTYIn->state) {
    DLTTYIn->in_data = '\0'; /* Clear Previous Data */

    if ((count = read(TTY_Fd, indata, 1)) == 1) {
      DLTTYIn->in_data = indata[0];
      DLTTYIn->state = 1;
    } else {
      error("TTY: tty_get");
    }
  }
} /* TTY_get end */

void tty_put() {
  int count;
  char c;

#ifdef TRACE
  printf("TRACE: tty_put()\n");
#endif

  if (TTY_Fd >= 0) {
    c = DLTTYPortCmd->outdata;
    if ((count = write(TTY_Fd, &c, 1)) != 1) { error("TTY: tty_put()"); }
  }
} /* tty_put end */

void tty_breakon() {
  int stat;

#ifdef TRACE
  printf("TRACE: tty_breakon()\n");
#endif

  if (TTY_Fd >= 0) { stat = ioctl(TTY_Fd, TIOCSBRK, 0); }
} /* tty_breakon end */

void tty_breakoff() {
  int stat;

#ifdef TRACE
  printf("TRACE: tty_breakoff()\n");
#endif

  if (TTY_Fd >= 0) { stat = ioctl(TTY_Fd, TIOCCBRK, 0); }
} /* tty_breakoff end */

void TTY_cmd() {
#ifdef TRACE
  printf("TRACE: tty_cmd()\n");
#endif

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
} /* TTY_cmd end */

void tty_setparam() {
#ifdef TRACE
  printf("TRACE: tty_setpram()\n");
#endif

  if (DLTTYPortCmd->outdata & SET_BAUD_RATE) tty_setbaudrate();

} /* tty_setpram end */

void tty_setbaudrate() {
  char baudrate;
  int stat;

#ifdef TRACE
  printf("TRACE: tty_setbaudrete()\n");
#endif

  if (TTY_Fd >= 0) {
    if ((baudrate = tty_baudtosymbol(DLTTYOut->line_speed)) != -1) {
      TTY_Mode.sg_ispeed = baudrate;
      TTY_Mode.sg_ospeed = baudrate;
      stat = ioctl(TTY_Fd, TIOCSETP, &TTY_Mode);
    } else {
      error("TTY: tty_setbaudrate");
    }
  }
} /* tty_setbaudrate end */

int tty_baudtosymbol(short aBaud)
{
#ifdef TRACE
  printf("TRASE: tty_baudtosymbol(%x)\n", aBaud);
#endif

  if (aBaud == 0) return (B50);
  if (aBaud == 1) return (B75);
  if (aBaud == 2) return (B110);
  if (aBaud == 3) return (B134);
  if (aBaud == 4) return (B150);
  if (aBaud == 5) return (B300);
  if (aBaud == 6) return (B600);
  if (aBaud == 7) return (B1200);
  if (aBaud == 10) return (B2400);
  if (aBaud == 12) return (B4800);
  if (aBaud == 14) return (B9600);
  if (aBaud == 15) return (EXTA);
  return (-1);

} /* tty_baudtosymbol */

void tty_debug(char *name)
{
  int stat;
  struct sgttyb mode;

  printf("DEBUG: %s\n", name);
  printf("DEBUG: \t\tTTY_Dev            = \"%s\"\n", TTY_Dev);
  printf("DEBUG: \t\tTTY_Fd             = %d\n", TTY_Fd);

  if (TTY_Fd >= 0) {
    stat = ioctl(TTY_Fd, TIOCGETP, &mode);
    printf("DEBUG: \t\tTTY_Mode.sg_ispeed = %#x\n", mode.sg_ispeed);
    printf("DEBUG: \t\tTTY_Mode.sg_ospeed = %#x\n", mode.sg_ospeed);
    printf("DEBUG: \t\tTTY_Mode.sg_erase  = %#x\n", mode.sg_erase);
    printf("DEBUG: \t\tTTY_Mode.sg_kill   = %#x\n", mode.sg_kill);
    printf("DEBUG: \t\tTTY_Mode.sg_flags  = %#x\n", mode.sg_flags);
  }

  printf("DEBUG:\n");
  printf("DEBUG: \t\tSymbol       Address        Contents\n");
  printf("DEBUG: \t\tIOPAGE       %#x\n", Addr68k_from_LADDR(IOPAGE_OFFSET));
  printf("DEBUG: \t\tDLTTYPortCmd %#x        %#x\n", DLTTYPortCmd, *(DLword *)DLTTYPortCmd);
  printf("DEBUG: \t\tDLTTYOut     %#x        %#x\n", DLTTYOut, *(DLword *)DLTTYOut);
  printf("DEBUG: \t\t                            %#x\n", *(DLword *)(DLTTYOut + 1));
  printf("DEBUG: \t\tDLTTYIn      %#x        %#x\n", DLTTYIn, *(DLword *)DLTTYIn);
  printf("DEBUG: \t\t                            %#x\n", *(DLword *)(DLTTYIn + 1));

} /* tty_debug end */
