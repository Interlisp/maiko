/* $Id: rs232c.c,v 1.2 1999/01/03 02:07:32 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <fcntl.h>
#include <sgtty.h>
#include <stdio.h>
#include <sys/select.h>

#include "lspglob.h"
#include "rs232c.h"

/*
 * Lisp Interface
 */
static DLRS232C_HDW_CONF *HardWareConfig;
static DLRS232C_IOP_GET_FLAG *RS232CGetFlag;
static DLRS232C_IOP_PUT_FLAG *RS232CPutFlag;
static DLRS232C_IOP_MISC_CMD *RS232CMiscCommand;
static DLRS232C_PARAMETER_OUTCOME *RS232CParameterOutcome;
static DLRS232C_DEVICE_STATUS *RS232CDeviceStatus;
static DLRS232C_PARAMETER_CSB *RS232CParameterCSB;
static DLword *RS232CGetCSB, *RS232CPutCSB;

/*
 * File descriptor
 */
extern fd_set LispReadFds;
int RS232C_Fd = -1;

int RS232C_remain_data;
static char *RS232C_Dev;
static struct termios RS232C_Mode;

/*
 * Following two signal handler vector is used to deal with SIGHUP signal
 * which is sent if CLOCAL flag is not set and a modem disconnect
 * is detected.
 */

static struct sigvec rs_hup_sv;
static struct sigvec prev_hup_sv;

void rs232c_hup_handler() {
  printf("Modem disconnect is detected.\n");
  fflush(stdout);
  return;
}

rs_install_hup_handler() {
  rs_hup_sv.sv_handler = rs232c_hup_handler;
  rs_hup_sv.sv_mask = rs_hup_sv.sv_flags = 0;
  sigvec(SIGHUP, &rs_hup_sv, &prev_hup_sv);
}

rs_restore_hup_handler() { sigvec(SIGHUP, &prev_hup_sv, (struct sigvec *)NULL); }

/*
 * Fatal Error, enter URAID
 */
void rs_error(char *msg)
{ error(msg); }

/*
 * Continuable Error
 */
void rs_cerror(char *msg)
{ printf(msg); }

/*
 * Invoked at boot time
 */
void rs232c_init() {
  RS232C_Dev = "/dev/ttyb"; /*Modify for target system */
  RS232C_Fd = -1;

  /* Pointer to IOPAGE */
  HardWareConfig = (DLRS232C_HDW_CONF *)&IOPage->dliophardwareconfig;
  RS232CGetFlag = (DLRS232C_IOP_GET_FLAG *)&IOPage->dlrs232cgetflag;
  RS232CPutFlag = (DLRS232C_IOP_PUT_FLAG *)&IOPage->dlrs232cputflag;
  RS232CMiscCommand = (DLRS232C_IOP_MISC_CMD *)&IOPage->dlrs232cmisccommand;
  RS232CParameterOutcome = (DLRS232C_PARAMETER_OUTCOME *)&IOPage->dlrs232cparameteroutcome;
  RS232CDeviceStatus = (DLRS232C_DEVICE_STATUS *)&IOPage->dlrs232cdevicestatus;
  RS232CParameterCSB = (DLRS232C_PARAMETER_CSB *)&IOPage->dlrs232cparametercsblo_11;
  RS232CGetCSB = (DLword *)&IOPage->dlrs232cgetcsblo;
  RS232CPutCSB = (DLword *)&IOPage->dlrs232cputcsblo;

  HardWareConfig->rs232c_absent = 0;
  RS232CMiscCommand->busy = 0;

} /* rs232c_init end */

#define MIN_CHARS 256
#define MIN_TIME 1

/*
 * Default set up for the RS232C file descriptor
 * The value of the other parameters not listed below can be changed by user.
 */
int rs232_fd_init(int fd)
{
  struct termios tio;

  if (ioctl(fd, TCGETS, &tio) < 0) {
    rs_error("RS232C: rs232c_fd_init: cannot get status");
    return 0;
  } else {
    /* Input Mode */
    tio.c_iflag &= ~IGNBRK;
    tio.c_iflag &= ~IGNPAR;
    tio.c_iflag &= ~IUCLC;

    /* Output Mode */
    tio.c_oflag |= OPOST;
    tio.c_oflag &= ~OLCUC;
    tio.c_oflag &= ~OFILL;
    tio.c_oflag &= ~OFDEL;
    tio.c_oflag &= ~NLDLY;
    tio.c_oflag &= ~CRDLY;
    tio.c_oflag &= ~BSDLY;
    tio.c_oflag &= ~VTDLY;
    tio.c_oflag &= ~FFDLY;

    /* Control Mode */
    tio.c_cflag |= CREAD;
    tio.c_cflag |= HUPCL;
    tio.c_cflag &= ~CIBAUD; /* Input Baudrate == Output Baudrate*/

    /* Local Modes */
    tio.c_lflag &= ~ISIG;
    tio.c_lflag &= ~ICANON;
    tio.c_lflag &= ~XCASE;
    tio.c_lflag &= ~ECHO;
    tio.c_lflag &= ~ECHOE;
    tio.c_lflag &= ~ECHOK;
    tio.c_lflag &= ~ECHONL;
    tio.c_lflag &= ~NOFLSH;
    tio.c_lflag &= ~TOSTOP;
    tio.c_lflag &= ~ECHOCTL;
    tio.c_lflag &= ~ECHOPRT;
    tio.c_lflag &= ~ECHOKE;
    tio.c_lflag &= ~FLUSHO;
    tio.c_lflag &= ~PENDIN;

    /* Minimum and Timeout */
    tio.c_cc[VMIN] = MIN_CHARS;
    tio.c_cc[VTIME] = MIN_TIME;

    RS232C_Mode = tio;

    if (ioctl(fd, TCSETS, &tio) < 0)
      rs_error("RS232C: rs232c_fd_init: cannot set status");
    else
      return 1;
  }
}

rs232c_open() {
  if (RS232C_Fd < 0) {
    if ((RS232C_Fd = open(RS232C_Dev, O_RDWR)) < 0)
      rs_error("RS232C: rs232c_open: cannot open");
    else {
      rs232_fd_init();

      /* Receive SIGIO on the descriptor */
      if (ioctl(RS232C_Fd, I_SETSIG, S_INPUT) < 0)
        rs_error("RS232C: rs232c_open: cannot set signal");
      else {
        rs_install_hup_handler();
        FD_SET(RS232C_Fd, &LispReadFds);
        RS232C_remain_data = 0;
      }
    }
  }
}

rs232c_close() {
  if (RS232C_Fd >= 0) {
    rs232c_abortinput();
    rs232c_abortoutput();

    rs_restore_hup_handler();

    FD_CLR(RS232C_Fd, &LispReadFds);

    /*
                    if (close(RS232C_Fd) < 0)
                      rs_error("RS232C: rs232c_close: cannot close");
    */
    close(RS232C_Fd);
    RS232C_Fd = -1;
  }
}

RS232C_readinit() {
  if (RS232C_remain_data) {
    /*
     * There are other data which we have not read yet.
     * Signaling SIGIO invokes rs232c_read.
     */
    kill(getpid(), SIGIO);
  }
}

rs232c_lisp_is_ready() { return (RS232CGetFlag->busy); }

static struct timeval sel_tv = {0, 0};

rs232c_read() {
  register DLRS232C_IOCB *iocb;
  register int count;
  fd_set readfds;

  if (RS232C_Fd >= 0) {
    if (RS232CGetFlag->busy) {
      iocb =
          (DLRS232C_IOCB *)Addr68k_from_LADDR(((*(RS232CGetCSB + 1) & 0xff) << 16) + *RS232CGetCSB);

      if ((count =
               read(RS232C_Fd, (char *)Addr68k_from_LADDR(((iocb->block_pointer_hi & 0xff) << 16) +
                                                          iocb->block_pointer_lo),
                    iocb->byte_count)) < 0) {
        ((DLRS232C_IOCB_TRANSFER_STATUS *)(&iocb->transfer_status))->success = 0;
        RS232CGetFlag->busy = 0;
        return;
      }

      ((DLRS232C_IOCB_TRANSFER_STATUS *)(&(iocb->transfer_status)))->success = 1;
      iocb->returned_byte_count = count;
      RS232CGetFlag->busy = 0;

      /*
       * We want to check if the some other data
       * remaining or not.
       */
      FD_ZERO(&readfds);
      FD_SET(RS232C_Fd, &readfds);
      if (select(RS232C_Fd + 1, &readfds, NULL, NULL, &sel_tv) > 0) {
        if (FD_ISSET(RS232C_Fd, &readfds))
          RS232C_remain_data = 1;
        else
          RS232C_remain_data = 0;
      } else {
        RS232C_remain_data = 0;
      }
    } else {
      /*
       * SIGIO handler getsignaldata and the successive
       * rs232c_read has been called before Lisp prepares
       * the next buffer.  Turn on RS232C_remain_data to
       * specify to read the remaining data after.
       */
      RS232C_remain_data = 1;
    }
  }
}

#define MAX_WRITE_TRY 5

RS232C_write() {
  register int size, count, trynum;
  register char *buf;
  register DLRS232C_IOCB *iocb;

  iocb = (DLRS232C_IOCB *)Addr68k_from_LADDR(((*(RS232CPutCSB + 1) & 0xff) << 16) + *RS232CPutCSB);

  if (RS232CPutFlag->busy && RS232C_Fd >= 0) {
    if (iocb->put) {
      for (size = iocb->byte_count,
          buf = (char *)Addr68k_from_LADDR(((iocb->block_pointer_hi & 0xff) << 16) +
                                           iocb->block_pointer_lo);
           size > 0; size -= count, buf += count) {
        trynum = 0;
        while (trynum < MAX_WRITE_TRY) {
          count = write(RS232C_Fd, buf, size);
          if (count >= 0) break;
          trynum++;
        }

        if (count < 0) {
          ((DLRS232C_IOCB_TRANSFER_STATUS *)(&(iocb->transfer_status)))->success = 0;
          RS232CPutFlag->busy = 0;
          return;
        }
      }

      ((DLRS232C_IOCB_TRANSFER_STATUS *)(&(iocb->transfer_status)))->success = 1;
      RS232CPutFlag->busy = 0;
    } else {
      ((DLRS232C_IOCB_TRANSFER_STATUS *)(&(iocb->transfer_status)))->success = 1;
      RS232CPutFlag->busy = 0;
    }
  } else {
    ((DLRS232C_IOCB_TRANSFER_STATUS *)(&(iocb->transfer_status)))->success = 1;
    RS232CPutFlag->busy = 0;
  }
}

RS232C_cmd() {
  if (RS232CMiscCommand->busy) {
    switch (RS232CMiscCommand->command) {
      case ON: rs232c_open(); break;
      case OFF: rs232c_close(); break;
      case BREAK_ON: rs232c_breakon(); break;
      case BREAK_OFF: rs232c_breakoff(); break;
      case ABORT_INPUT: rs232c_abortinput(); break;
      case ABORT_OUTPUT: rs232c_abortoutput(); break;
      case GET_STATUS: rs232c_getstatus(); break;
      case MAJOR_SET_PARAMETERS: rs232c_majorparam(); break;
      case MINOR_SET_PARAMETERS: rs232c_minorparam(); break;
      default: rs_error("RS232C : RS232C_cmd");
    }
    RS232CMiscCommand->busy = 0;
  } else {
    rs_cerror("RS232C : RS232C_cmd: device busy.");
  }
}

rs232c_breakon() {
  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TIOCSBRK, 0) < 0) rs_cerror("RS232C: rs232c_breakon");
  }
}

rs232c_breakoff() {
  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TIOCCBRK, 0) < 0) rs_cerror("RS232C: rs232c_breakoff");
  }
}

rs232c_abortinput() {
  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TCFLSH, 0) < 0)
      rs_cerror("RS232C: rs232c_abortinput");
    else
      RS232C_remain_data = 0;
  }
}

rs232c_abortoutput() {
  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TCFLSH, 1) < 0) rs_cerror("RS232C: rs232c_abortoutput");
  }
}

rs232c_getstatus() {
  int status;

  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TIOCMGET, &status) < 0)
      rs_error("rs232c_getstatus : cannot get status");
    else {
      RS232CDeviceStatus->power_indication = ((status & TIOCM_LE) == TIOCM_LE);
      ((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->data_terminal_ready =
          ((status & TIOCM_DTR) == TIOCM_DTR);
      ((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->request_to_send =
          ((status & TIOCM_RTS) == TIOCM_RTS);
      RS232CDeviceStatus->clear_to_send = ((status & TIOCM_CTS) == TIOCM_CTS);
      RS232CDeviceStatus->carrier_detect = ((status & TIOCM_CAR) == TIOCM_CAR);
      RS232CDeviceStatus->ring_indicator = ((status & TIOCM_RNG) == TIOCM_RNG);
      RS232CDeviceStatus->data_set_ready = ((status & TIOCM_DSR) == TIOCM_DSR);
    }
  }
}

void rs_baud(u_int baud)
{
  switch (baud) {
    case 0: return B50;
    case 1: return B75;
    case 2: return B110;
    case 3: return B134;
    case 4: return B150;
    case 5: return B200;
    case 6: return B300;
    case 7: return B600;
    case 8: return B1200;
    case 9: return B1800;
    case 10: return B2400;
    case 11: return B4800;
    case 12: return B9600;
    case 13: return B19200;
    case 14: return B38400;
    default: rs_error("rs_baud: illegal baud rate.");
  }
}

void rs_csize(u_int csize)
{
  switch (csize) {
    case 0: return CS5;
    case 1: return CS6;
    case 2: return CS7;
    case 3: return CS8;
    default: rs_error("rs_csize: illegal character size");
  }
}

void rs_sbit(u_int sbit)
{
  switch (sbit) {
    case 0: return 0;
    case 1: return CSTOPB;
    default: rs_error("rs_bit: illegal stop bit");
  }
}

void rs232c_majorparam() {
  register int baud, csize, sbit;

  if (RS232C_Fd >= 0) {
    baud = rs_baud((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->line_speed);
    RS232C_Mode.c_cflag &= ~CBAUD;
    RS232C_Mode.c_cflag |= baud;

    csize = rs_csize((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->char_length);
    RS232C_Mode.c_cflag &= ~CSIZE;
    RS232C_Mode.c_cflag |= csize;
    if (csize == CS8)
      RS232C_Mode.c_iflag &= ~ISTRIP;
    else
      RS232C_Mode.c_iflag |= ISTRIP;

    switch ((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->parity) {
      case ODD:
        RS232C_Mode.c_iflag |= INPCK;
        RS232C_Mode.c_cflag |= PARENB;
        RS232C_Mode.c_cflag |= PARODD;
        break;

      case EVEN:
        RS232C_Mode.c_iflag |= INPCK;
        RS232C_Mode.c_cflag |= PARENB;
        RS232C_Mode.c_cflag &= ~PARODD;
        break;

      default: RS232C_Mode.c_iflag &= ~INPCK; RS232C_Mode.c_cflag &= ~PARENB;
    }

    sbit = rs_sbit((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->stop_bits);
    RS232C_Mode.c_cflag &= ~CSTOPB;
    RS232C_Mode.c_cflag |= sbit;

    if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->flowcontrol_on) {
      RS232C_Mode.c_iflag |= IXON;
      RS232C_Mode.c_iflag |= IXOFF;
      RS232C_Mode.c_cc[VSTART] =
          (char)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->flowcontrol_xon_char;
      RS232C_Mode.c_cc[VSTOP] =
          (char)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->flowcontrol_xoff_char;
    } else {
      RS232C_Mode.c_iflag &= ~IXON;
      RS232C_Mode.c_iflag &= ~IXOFF;
    }

    switch ((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->in_eol) {
      case CR:
        RS232C_Mode.c_iflag &= ~INLCR;
        RS232C_Mode.c_iflag &= ~IGNCR;
        RS232C_Mode.c_iflag &= ~ICRNL;
        break;

      case LF:
        RS232C_Mode.c_iflag |= INLCR;
        RS232C_Mode.c_iflag &= ~IGNCR;
        RS232C_Mode.c_iflag &= ~ICRNL;
        break;

      case CRLF:
      default: RS232C_Mode.c_iflag |= INLCR; RS232C_Mode.c_iflag |= IGNCR;
    }

    switch ((u_int)((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->out_eol) {
      case LF:
        RS232C_Mode.c_oflag &= ~ONLCR;
        RS232C_Mode.c_oflag &= ~OCRNL;
        RS232C_Mode.c_oflag &= ~ONOCR;
        RS232C_Mode.c_oflag &= ~ONLRET;
        break;

      case CRLF:
        RS232C_Mode.c_oflag |= ONLCR;
        RS232C_Mode.c_oflag &= ~OCRNL;
        RS232C_Mode.c_oflag &= ~ONOCR;
        RS232C_Mode.c_oflag &= ~ONLRET;
        break;

      case CR:
      default:
        RS232C_Mode.c_oflag &= ~ONLCR;
        RS232C_Mode.c_oflag &= ~OCRNL;
        RS232C_Mode.c_oflag &= ~ONOCR;
        RS232C_Mode.c_oflag |= ONLRET;
    }

    if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->input_max_bell)
      RS232C_Mode.c_iflag |= IMAXBEL;
    else
      RS232C_Mode.c_iflag &= ~IMAXBEL;

    if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->tab_expand)
      RS232C_Mode.c_oflag |= XTABS;
    else
      RS232C_Mode.c_oflag &= ~XTABS;

    if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->modem_status_line)
      RS232C_Mode.c_cflag &= ~CLOCAL;
    else
      RS232C_Mode.c_cflag |= CLOCAL;

    if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->rts_cts_control)
      RS232C_Mode.c_cflag |= CRTSCTS;
    else
      RS232C_Mode.c_cflag &= ~CRTSCTS;

    if (ioctl(RS232C_Fd, TCSETS, &RS232C_Mode) < 0)
      rs_error("rs232c_majorparam: cannot set params");
    else
      RS232CParameterOutcome->success = 1;
  }
}

void rs232c_minorparam() {
  int status;

  if (RS232C_Fd >= 0) {
    if (ioctl(RS232C_Fd, TIOCMGET, &status) < 0)
      rs_error("rs_minorparam: cannot get status");
    else {
      if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->data_terminal_ready)
        status |= TIOCM_DTR;
      else
        status &= ~TIOCM_DTR;

      if (((DLRS232C_PARAMETER_CSB *)RS232CParameterCSB)->request_to_send)
        status |= TIOCM_RTS;
      else
        status &= ~TIOCM_RTS;

      if (ioctl(RS232C_Fd, TIOCMSET, &status) < 0)
        rs_error("rs_minorparam: cannot set status");
      else
        RS232CParameterOutcome->success = 1;
    }
  }
}

/*
 * Following functions named as check_XXX are used for debug.
 */

void check_params(int fd)
{
  struct termios tos;

  if (ioctl(fd, TCGETS, &tos) < 0) {
    perror("IOCTL");
    exit(1);
  }

  check_brate(tos.c_cflag);
  check_csize(tos.c_cflag);
  check_sbit(tos.c_cflag);
  check_parity(tos.c_cflag);
  check_oflag(tos.c_oflag);

  check_canon(&tos);
}

void check_brate(u_long cf)
{
  int b;
  printf("Baud rate : ");
  switch (cf & CBAUD) {
    case B0: b = 0; break;
    case B50: b = 50; break;
    case B75: b = 75; break;
    case B110: b = 110; break;
    case B134: b = 134; break;
    case B200: b = 200; break;
    case B300: b = 300; break;
    case B600: b = 600; break;
    case B1200: b = 1200; break;
    case B1800: b = 1800; break;
    case B2400: b = 2400; break;
    case B4800: b = 4800; break;
    case B9600: b = 9600; break;
    case B19200: b = 19200; break;
    case B38400: b = 384000; break;
    default: printf("Illegal.\n");
  }
  printf("%d bps.\n", b);
}

void check_csize(u_long cf)
{
  int s;
  printf("Char size : ");
  switch (cf & CSIZE) {
    case CS5: s = 5; break;
    case CS6: s = 6; break;
    case CS7: s = 7; break;
    case CS8: s = 8; break;
    default: printf("Illegal.\n");
  }
  printf("%d chars.\n", s);
}

void check_sbit(u_long cf)
{
  int s;
  printf("Stop bit : ");
  if (cf & CSTOPB)
    printf("2 bits.\n");
  else
    printf("1 bit.\n");
}

void check_parity(u_long cf)
{
  if (cf & PARENB) {
    printf("Parity Enabled : ");
    if (cf & PARODD)
      printf("Odd.\n");
    else
      printf("Even.\n");
  } else {
    printf("Parity Disabled.\n");
  }
}

void check_canon(struct termios *tos)
{
  u_long lf;

  lf = tos->c_lflag;

  if (lf & ICANON) {
    printf("Canonical.\n");
  } else {
    printf("Non-Canonical.\n");
    printf("MIN : %d  ,  TIME : %d\n", (tos->c_cc)[VMIN], (tos->c_cc)[VTIME]);
  }
}

void check_oflag(u_long of)
{
  if ((OPOST & of) == OPOST) {
    printf("OPOST : O\n");
  } else {
    printf("OPOST : X\n");
  }

  if ((ONLCR & of) == ONLCR) {
    printf("ONLCR : O\n");
  } else {
    printf("ONLCR : X\n");
  }
  if ((OCRNL & of) == OCRNL) {
    printf("OCRNL : O\n");
  } else {
    printf("OCRNL : X\n");
  }

  if ((ONOCR & of) == ONOCR) {
    printf("ONOCR : O\n");
  } else {
    printf("ONOCR : X\n");
  }
  if ((ONLRET & of) == ONLRET) {
    printf("ONLRET : O\n");
  } else {
    printf("ONLRET : X\n");
  }
  if ((OFILL & of) == OFILL) {
    printf("OFILL : O\n");
  } else {
    printf("OFILL : X\n");
  }
  if ((OFDEL & of) == OFDEL) {
    printf("OFDEL : O\n");
  } else {
    printf("OFDEL : X\n");
  }
  switch (NLDLY & of) {
    case NL0: printf("NL0 \n"); break;
    case NL1: printf("NL1\n"); break;
  }
  switch (CRDLY & of) {
    case CR0: printf("NL0 \n"); break;
    case CR1: printf("CR1\n"); break;
    case CR2: printf("CR2\n"); break;
    case CR3: printf("CR3\n"); break;
  }
  switch (TABDLY & of) {
    case TAB0: printf("NL0 \n"); break;
    case TAB1: printf("TAB1\n"); break;
    case TAB2: printf("TAB2\n"); break;
    case XTABS: printf("XTABS\n"); break;
  }
  switch (BSDLY & of) {
    case BS0: printf("NL0 \n"); break;
    case BS1: printf("BS1\n"); break;
  }
  switch (VTDLY & of) {
    case VT0: printf("NL0 \n"); break;
    case VT1: printf("VT1\n"); break;
  }
  switch (FFDLY & of) {
    case FF0: printf("NL0 \n"); break;
    case FF1: printf("FF1\n"); break;
  }
}

/*
 * In dbx, "call rsc()".
 */
void rsc() {
  if (RS232C_Fd >= 0) check_params(RS232C_Fd);
}

/* Old debug function: not updated.

rs232c_debug( name, sw )
        char *name;
        int  sw;
{
        struct sgttyb mode;


        if ( sw == 1 ) {

        printf("DEBUG: %s\n",name);

        printf("DEBUG: \t\tRS232C_Dev = %s\n", RS232C_Dev);
        printf("DEBUG: \t\tRS232C_Fd  = %d\n", RS232C_Fd);

        if ( RS232C_Fd >= 0 ) {
                ioctl( RS232C_Fd, TIOCGETP, &mode );

        printf("DEBUG: \t\tRS232C_Mode.sg_ispeed = %#x\n", mode.sg_ispeed );
        printf("DEBUG: \t\tRS232C_Mode.sg_ospeed = %#x\n", mode.sg_ospeed );
        printf("DEBUG: \t\tRS232C_Mode.sg_erase  = %#x\n", mode.sg_erase  );
        printf("DEBUG: \t\tRS232C_Mode.sg_kill   = %#x\n", mode.sg_kill   );
        printf("DEBUG: \t\tRS232C_Mode.sg_flags  = %#x\n", mode.sg_flags  );

        } /+ if(RS232C_Fd) end +/

        } /+ if(sw) end +/

        if ( sw == 2 ) {

        printf("DEBUG: %s\n",name);

        printf("DEBUG:\n");
        printf("DEBUG: \t\tSymbol             Address    Contents\n");
        printf("DEBUG: \t\tIOPAGE             %#x\n",Addr68k_from_LADDR(IOPAGE_OFFSET));
        printf("DEBUG: \t\tHardWareConfig     %#x    %#x\n", HardWareConfig, *(DLword*)
HardWareConfig);
        printf("DEBUG: \t\tRS232CGetFlag      %#x    %#x\n", RS232CGetFlag, *(DLword*)
RS232CGetFlag);
        printf("DEBUG: \t\tRS232CPutFlag      %#x    %#x\n", RS232CPutFlag, *(DLword*)
RS232CPutFlag);
        printf("DEBUG: \t\tRS232CMiscCommand  %#x    %#x\n", RS232CMiscCommand, *(DLword*)
RS232CMiscCommand);
        printf("DEBUG: \t\tRS232CGetCSB       %#x    %#x\n", RS232CGetCSB, (LispPTR)
(((*(RS232CGetCSB+1) & 0xff)<<16) + *RS232CGetCSB ));
        printf("DEBUG: \t\tRS232CPutCSB       %#x    %#x\n", RS232CPutCSB, (LispPTR)
(((*(RS232CPutCSB+1) & 0xff)<<16) + *RS232CPutCSB ));
        printf("DEBUG: \t\tRS232CParameterCSB %#x    %#x\n", RS232CParameterCSB, *(DLword*)
RS232CParameterCSB);
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+1));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+2));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+3));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+4));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+5));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+6));
        printf("DEBUG: \t\t                              %#x\n", *(DLword*) (RS232CParameterCSB+7));
        if( RS232C_WrIOCB != NULL )
        printf("DEBUG: \t\tRS232C_WrIOCB      %#x    %#x\n", RS232C_WrIOCB,
*((LispPTR*)RS232C_WrIOCB));
        if( RS232C_RdIOCB != NULL )
        printf("DEBUG: \t\tRS232C_RdIOCB      %#x    %#x\n", RS232C_RdIOCB,
*((LispPTR*)RS232C_RdIOCB));

        /+
        if( RS232C_WrBufAddr != NULL )
        printf("DEBUG: \t\tRS232C_WrBufAddr   %s\n" , RS232C_WrBufAddr);
        if( RS232C_WrBufAddr != NULL )
        printf("DEBUG: \t\tRS232C_RdBufAddr   %s\n" , RS232C_RdBufAddr);
        +/

        } /+ if(sw) end +/

 } /+ rs232c_debug end +/

*/
