/* $Id: timer.c,v 1.5 2001/12/26 22:17:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*				t i m e r . c				*/
/*									*/
/*	Timer handling routines, plus set-up for the other interrupts	*/
/*	Medley uses on Unix.						*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#ifdef DOS
#include <time.h>
#include <dos.h>
#include <i32.h>   /* "#pragma interrupt" & '_chain_intr'*/
#include <dos.h>   /* defines REGS & other structs       */
#include <stdio.h> /* define NULL                        */
#include <stdlib.h>
#define SIGVTALRM SIGUSR1
#define SIGIO SIGREAD
/******************************************************************************
*   Global variables
******************************************************************************/
void (*prev_int_1c)(); /* keeps address of previous 1c handlr*/
                       /* used for chaining & restore at exit*/
#pragma interrupt(DOStimer)
void DOStimer();

unsigned long tick_count = 0; /* approx 18 ticks per sec            */
#define USETIMEFN

#else
#include <time.h>
#include <sys/time.h>
#endif /* DOS */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef DOS
#include <sys/resource.h>
#endif /* DOS */

#ifdef OS5
#include <sys/times.h>
#include <stropts.h>
/* JDS 991228 removed #define USETIMEFN */
extern int ether_fd;
#endif

#ifdef LINUX
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/times.h>
#endif

#include <setjmp.h>

#include "lispemul.h"
#include "emlglob.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "arith.h"
#include "lispmap.h"
#include "stack.h"
#include "dbprint.h"

#include "timerdefs.h"
#include "commondefs.h"
#include "mkcelldefs.h"

#ifdef XWINDOW
#include "devif.h"
extern DspInterface currentdsp;
#endif /* XWINDOW */

#define LISP_UNIX_TIME_DIFF 29969152
#define LISP_ALTO_TIME_MASK 0x80000000
#define UNIX_ALTO_TIME_DIFF 2177452800U

/*	Interlisp time is signed; MIN.FIXP = "01-JAN-01 00:00:00 GMT"
 *	Interlisp 0 is at	"19-Jan-69 12:14:08 PST"
 *	Unix begins at		" 1-Jan-70  0:00:00 GMT"
 *	(CL:- (IL:IDATE	" 1-Jan-70  0:00:00 GMT")
 *	      (IL:IDATE "19-Jan-69 12:14:08 PST"))
 *	=> 29969152, amount to add to Lisp time to get Unix time
 *	Alto time is unsigned; 0 = "01-JAN-01 00:00:00 GMT"
 *      UNIX_ALTO_TIME_DIFF is amount to add to Unix time
 *	to get Alto time.
 */

int TIMEOUT_TIME; /* For file system timeout */

#ifdef XWINDOW
#define FALSE 0
#define TRUE !FALSE
int Event_Req = FALSE;
#endif /* XWINDOW */

#define ERRCHK(expr, str) \
  if (expr == -1) perror(str)

#define SIGERRCHK(set, str) \
  if (set == SIG_ERR) perror(str)

static int gettime(int casep);

/************************************************************************/
/*									*/
/*		    u p d a t e _ m i s c s t a t s			*/
/*									*/
/*	Updates counters and timers in the MISCSTATS "page".		*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void update_miscstats() {
#ifdef DOS
  struct dostime_t dtm; /* holds DOS time, so we can get .01 secs */
  _dos_gettime(&dtm);

  MiscStats->totaltime = (time(0) * 1000) + (10 * dtm.hsecond);
  MiscStats->swapwaittime = 0;
  MiscStats->pagefaults = 0;
  MiscStats->swapwrites = 0;
  MiscStats->diskiotime = 0; /* ?? not available ?? */
  MiscStats->diskops = 0;
  MiscStats->secondstmp = MiscStats->secondsclock = (time(0) + UNIX_ALTO_TIME_DIFF);
#elif !defined(USETIMEFN)
  struct timeval timev;
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);

  MiscStats->totaltime = ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000;
  MiscStats->swapwaittime = ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000;
  MiscStats->pagefaults = ru.ru_minflt + ru.ru_majflt;
  MiscStats->swapwrites = ru.ru_majflt;
  MiscStats->diskiotime = 0; /* ?? not available ?? */
  MiscStats->diskops = ru.ru_inblock
      /* ?? this doesn't work ???
     + ru.ru_outblock   */
      ;
  gettimeofday(&timev, NULL);
  MiscStats->secondstmp = MiscStats->secondsclock = (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
#else
  struct tms ru;

  times(&ru); /* Get system time used */

  MiscStats->totaltime =
      ru.tms_utime * 10 + ru.tms_stime * 10 + ru.tms_cutime * 10 + ru.tms_cstime * 10;
  MiscStats->swapwaittime = ru.tms_stime * 10 + ru.tms_cstime * 10;
  MiscStats->pagefaults = 0; /* can't tell this on ISC */
  MiscStats->swapwrites = 0;
  MiscStats->diskiotime = 0; /* ?? not available ?? */
  MiscStats->diskops = 0;
  MiscStats->secondstmp = MiscStats->secondsclock = (time(0) + UNIX_ALTO_TIME_DIFF);

#endif /* DOS */
}

/************************************************************************/
/*									*/
/*			i n i t _ m i s c s t a t s			*/
/*									*/
/*	Called at initialization time to set miscstats words.		*/
/*	?? and to periodically update them ?? [JDS 11/22/89]		*/
/*									*/
/*									*/
/************************************************************************/

void init_miscstats() {
  MiscStats->starttime = gettime(0);
  MiscStats->gctime = 0;
  update_miscstats();
}

/************************************************************************/
/*									*/
/*			s u b r _ g e t t i m e				*/
/*									*/
/*	Handler for Lisps GETTIME subr call, dispatched thru		*/
/*	subr.c/miscn.c sub-dispatch.					*/
/*									*/
/*	Calls gettime, and returns the result to Lisp as a SMALLP	*/
/*	or FIXP, as appropriate.					*/
/*									*/
/************************************************************************/

DLword *createcell68k();

LispPTR subr_gettime(LispPTR args[])
{
  int result;
  result = gettime(args[0] & 0xffff);
  if (args[1]) {
    *((int *)Addr68k_from_LADDR(args[1])) = result;
    return (args[1]);
  } else
    N_ARITH_SWITCH(result);
}

/************************************************************************/
/*									*/
/*		    		g e t t i m e				*/
/*									*/
/*	Get the value of one of the various time counters, as		*/
/*	specified by the argument casep.  casep's values & meanings:	*/
/*									*/
/*	0 elapsed time, in milliseconds.				*/
/*	1 start of elapsed-time period, in milliseconds			*/
/*	2 this process's run time, in milliseconds			*/
/*	3 total GC time, in milliseconds				*/
/*	4 current time-of-day, in ALTO format				*/
/*	5 current time-of-day, in Interlisp format			*/
/*	6 start of daylight-savings, as day-in-year			*/
/*	7 end of daylight-savings, as day-in-year			*/
/*	8 time zone, as hours of offset from GMT (whole hours only)	*/
/*									*/
/************************************************************************/

static int gettime(int casep)
{
#ifndef USETIMEFN
  struct timeval timev;
#elif DOS
  struct dostime_t dtm; /* for hundredths of secs */
#endif /* USETIMEFN */
  switch (casep) {
    case 0: /* elapsed time in alto milliseconds */
#ifdef USETIMEFN
#ifdef DOS
      _dos_gettime(&dtm);
      return ((time(0) + UNIX_ALTO_TIME_DIFF) * 1000) + (10 * dtm.hsecond);
#else  /* DOS */
      return ((time(0) + UNIX_ALTO_TIME_DIFF)) * 1000;
#endif /* DOS */
#else  /* USETIMEFN */
      gettimeofday(&timev, NULL);
      return ((timev.tv_sec + UNIX_ALTO_TIME_DIFF) * 1000 + timev.tv_usec / 1000);
#endif /* USETIMEFN */

    case 1: /* starting elapsed time in milliseconds */ return (MiscStats->starttime);

    case 2: /* run time, this process, in milliseconds */
      update_miscstats();
      return (MiscStats->totaltime);

    case 3: /* total GC time in milliseconds */ return (MiscStats->gctime);

    case 4: /* current time of day in Alto format */
#ifdef USETIMEFN
      return (time(0) + UNIX_ALTO_TIME_DIFF);
#else
      gettimeofday(&timev, NULL);
      return (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
#endif

    case 5: /* current time of day in Interlisp format */
#ifdef USETIMEFN
      return (time(0) + LISP_UNIX_TIME_DIFF);
#else
      gettimeofday(&timev, NULL);
      return (timev.tv_sec + LISP_UNIX_TIME_DIFF);
#endif

    case 6:
      return (98); /* this is wrong, only works in PST */

    case 7:
      return (305); /* this is wrong, only works in PST */

    case 8:
      /* other methods of determining timezone offset are unreliable and/or deprecated */
      /* timezone is declared in <time.h>; the time mechanisms must be initialized     */
      /* Unfortunately, FreeBSD does not support the timezone external variable, nor   */
      /* does gettimeofday() seem to produce the correct timezone values.	       */
      tzset();
#if defined(FREEBSD)
      time_t tv = time(NULL);
      struct tm *tm = localtime(&tv);
      return (tm->tm_gmtoff / -3600);
#else
      return (timezone / 3600); /* timezone, extern, is #secs diff GMT to local. */
#endif
    default: return (0);
  }
}

/************************************************************************/
/*									*/
/*			s u b r _ s e t t i m e				*/
/*									*/
/*	Converts its argument, a time in ALTO seconds, to the		*/
/*	UNIX time format, and sets the UNIX clock.  You must be		*/
/*	the super-user for this to work.				*/
/*									*/
/*	Implements the SETTIME subr call, sub-dispatched from subr.c	*/
/*									*/
/************************************************************************/

void subr_settime(LispPTR args[])
{
#ifdef DOS
  struct dostime_t dostime;
  struct dosdate_t dosday;
  struct tm uxtime;

  uxtime = *localtime((time_t *)(*((int *)Addr68k_from_LADDR(args[0])) - UNIX_ALTO_TIME_DIFF));
  dostime.hsecond = 0;
  dostime.second = uxtime.tm_sec;
  dostime.minute = uxtime.tm_min;
  dostime.hour = uxtime.tm_hour;
  _dos_settime(&dostime);

  dosday.day = uxtime.tm_mday;
  dosday.month = uxtime.tm_mon;
  dosday.year = uxtime.tm_year;
  dosday.dayofweek = uxtime.tm_wday;
  _dos_setdate(&dosday);
#else
  struct timeval timev;
  timev.tv_sec = *((int *)Addr68k_from_LADDR(args[0])) - UNIX_ALTO_TIME_DIFF;
  timev.tv_usec = 0;
  settimeofday(&timev, NULL);
#endif
} /* end subr_settime */

/************************************************************************/
/*									*/
/*		    s u b r _ c o p y t i m e s t a t s			*/
/*									*/
/*	Given source and destination MISCSTATS structure pointers,	*/
/*	copy the contents of the source structure into the dest.	*/
/*									*/
/*	Also calls update_miscstats, to keep stats current.		*/
/*									*/
/************************************************************************/

void subr_copytimestats(LispPTR args[])
{
  MISCSTATS *source;
  MISCSTATS *dest;
  source = (MISCSTATS *)Addr68k_from_LADDR(args[0]);
  dest = (MISCSTATS *)Addr68k_from_LADDR(args[1]);
  update_miscstats();
  *dest = *source;
}

/************************************************************************/
/*									*/
/*			     N _ O P _ r c l k				*/
/*									*/
/*	Get the current time in UNIX format, convert it to micro-	*/
/*	seconds in ALTO format, and store the low 32 bits into		*/
/*	the FIXP cell passed in to us on the top of stack.		*/
/*									*/
/************************************************************************/

LispPTR N_OP_rclk(LispPTR tos)
{
  unsigned int usec;
#ifdef DOS
  struct dostime_t dtm;
#endif /* DOS */

#ifdef USETIMEFN
#ifdef DOS
  _dos_gettime(&dtm);
  usec = (time(0) * 1000000) + (10000 * dtm.hsecond);
#else
  usec = time(0) * 1000000;
#endif /* DOS */
#else
  struct timeval timev;

  gettimeofday(&timev, NULL);
  usec = (timev.tv_sec * 1000000UL) + timev.tv_usec;
#endif /* USETIMEFN */
  *((unsigned int *)(Addr68k_from_LADDR(tos))) = usec;
  return (tos);
} /* end N_OP_rclk */

/**********************************************************************/
/* update_timer called periodically */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void update_timer() {
#ifdef USETIMEFN
  MiscStats->secondstmp = MiscStats->secondsclock = time(0) + UNIX_ALTO_TIME_DIFF;
#else
  struct timeval timev;
  gettimeofday(&timev, NIL);
  MiscStats->secondstmp = MiscStats->secondsclock = (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
#endif /* USETIMEFN */
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/**********************************************************************/
/* timer interrupt handling system

        int_init() should be called before first entering dispatch loop.
        int_timer_init() is called by int_init() and arms the timer interrupt.
        int_io_init() is called by int_init() and arms the I/O interrupt.
        int_timer_service() catches the timer signal and sets
                Irq_Stk_Check & Irq_Stk_End to 0
                so the rest of the system will see it and respond.
        int_block() and int_unblock() block timer interrupts  and release them.
        int_io_open(fd) should be called whenever a file that should interrupt
                us is opened; it enables the interrupt on that fd.
        int_io_close(fd) should be called whenever a file that should interrupt
                us is closed; it disables the interrupt on that fd.
*/

/* TIMER_INTERVAL usec ~ 20  per second.  This should live in some
        machine-configuration
        file somewhere - it can be changed as the -t parameter to lisp*/

#ifdef sparc
int TIMER_INTERVAL = 100000;
#else
int TIMER_INTERVAL = 25000;
#endif

int FileIOFlag = 0;
int TimerFlag = 0;
extern int LispWindowFd;

/************************************************************************/
/*									*/
/*		    i n t _ t i m e r _ s e r v i c e			*/
/*									*/
/*	Handle the virtual-time alarm signal VTALRM.			*/
/*									*/
/*									*/
/************************************************************************/

#ifndef SYSVSIGNALS
static struct sigvec timerv;
#endif /* SYSVSIGNALS */

static void int_timer_service(int sig, int code, void *scp)
{
  /* this may have to do more in the future, like check for nested interrupts,
          etc... */

  Irq_Stk_Check = 0;
  Irq_Stk_End = 0;
  TimerFlag = 1;

#ifdef XWINDOW
  Event_Req = TRUE;
#endif

#ifdef SYSVSIGNALS
/*    sigset(SIGVTALRM, int_timer_service); */
#endif /* SYSVSIGNALS */
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

static void int_timer_init()

{
#ifdef DOS
  /******************************************************************************
  *  All code and data touched during the processing of an interrupt should
  *  locked prior to receiving any interrupts.  This prevents the Timer
  *  function from being swapped out during an interrupt.
  ******************************************************************************/
  _dpmi_lockregion((void *)TimerFlag, sizeof(TimerFlag));
  _dpmi_lockregion((void *)Irq_Stk_End, sizeof(Irq_Stk_End));
  _dpmi_lockregion((void *)Irq_Stk_Check, sizeof(Irq_Stk_Check));
  _dpmi_lockregion((void *)tick_count, sizeof(tick_count));
  _dpmi_lockregion((void *)&DOStimer, 4096);
  _dpmi_lockregion((void *)prev_int_1c, sizeof(prev_int_1c));

  /* Set up the DOS time handler. */
  prev_int_1c = _dos_getvect(0x1c); /* get addr of currnt 1c hndlr, */
                                    /* if any*/
  _dos_setvect(0x1c, DOStimer);     /* hook our int handler to timer int */

#elif SIGVTALRM
  struct itimerval timert, tmpt;

#ifdef SYSVSIGNALS
  SIGERRCHK(sigset(SIGVTALRM, int_timer_service), "sigset vtalrm");
#else
  /* first set up the signal handler */
  timerv.sv_handler = int_timer_service;
  timerv.sv_mask = timerv.sv_flags = 0;
  sigvec(SIGVTALRM, &timerv, 0);
#endif /* SYSVSIGNALS */

/* then attach a timer to it and turn it loose */
  timert.it_interval.tv_sec = timert.it_value.tv_sec = 0;
  timert.it_interval.tv_usec = timert.it_value.tv_usec = TIMER_INTERVAL;

  timerclear(&tmpt.it_value);
  timerclear(&tmpt.it_interval);
  setitimer(ITIMER_VIRTUAL, &timert, &tmpt);
  getitimer(ITIMER_VIRTUAL, &tmpt);

  DBPRINT(("Timer interval set to %d usec\n", timert.it_value.tv_usec));
#endif /* DOS or SIGVTALRM */
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void int_io_open(int fd)
{
#ifdef DOS
/* would turn on DOS kbd signal handler here */
#elif KBINT

  DBPRINT(("int_io_opening %d\n", fd));
  if (fcntl(fd, F_SETOWN, getpid()) == -1) {
#ifdef DEBUG
    perror("fcntl F_SETOWN ERROR");
#endif
  };
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | FASYNC) == -1) perror("fcntl F_SETFL error");
#endif
}

void int_io_close(int fd)
{
#ifdef DOS
/* Turn off signaller here */
#elif KBINT
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~FASYNC);
#endif
}

/************************************************************************/
/*									*/
/*			i n t _ i o _ i n i t				*/
/*									*/
/*	Set up handling for the SIGIO and SIGPOLL signals, in		*/
/*	support of keyboard event handling and ethernet incoming-	*/
/*	packet handling.						*/
/*									*/
/*									*/
/************************************************************************/

static void int_io_init() {
#ifndef SYSVSIGNALS
  static struct sigvec timerv;
  static struct sigvec poll_timerv;
#endif /* SYSVSIGNALS */
  extern void getsignaldata();

/* first set up the signal handler */

#ifndef SYSVSIGNALS
#ifdef KBINT
  timerv.sv_handler = getsignaldata;
  timerv.sv_mask = timerv.sv_flags = 0;
  sigvec(SIGIO, &timerv, 0);

  DBPRINT(("I/O interrupts enabled\n"));
#endif /* KBINT */
#else  /* SYSVSIGNALS in effect... */
#ifndef DOS
  SIGERRCHK(sigset(SIGIO, getsignaldata), "sigset io");
#ifdef XWINDOW
#ifndef __CYGWIN__
#ifdef LINUX
  if (fcntl(ConnectionNumber(currentdsp->display_id), F_SETSIG, 0) < 0)
    perror("fcntl on X fd - SETSIG for input handling failed");
#else
  if (ioctl(ConnectionNumber(currentdsp->display_id), I_SETSIG, S_INPUT) < 0)
    perror("ioctl on X fd - SETSIG for input handling failed");
#endif
#endif /* __CYGWIN__ */
#endif /* XWINDOW */

#ifdef USE_DLPI
  DBPRINT(("INIT ETHER:  Doing I_SETSIG.\n"));
  if (ether_fd > 0)
    if (ioctl(ether_fd, I_SETSIG, S_INPUT) != 0) {
      perror("init_ether: I_SETSIG failed:\n");
      close(ether_fd);
      ether_fd = -1;
      return;
    }
#endif /* USE_DLPI */

#endif /* DOS */
#endif /* SYSVSIGNALS */
}

int oldmask = 0;

/************************************************************************/
/*									*/
/*			   i n t _ b l o c k				*/
/*									*/
/*	Temporarily turn off interrupts.				*/
/*									*/
/*	NOTE that these interrupts must also be turned off in ldeboot's	*/
/*	forking code; if you change these, go fix that one too		*/
/*									*/
/************************************************************************/

void int_block() {
/* temporarily turn off interrupts */
#ifdef DOS
  _dos_setvect(0x1c, prev_int_1c);
#else /* DOS */
#ifdef SYSVSIGNALS
#ifdef SIGVTALRM
  sighold(SIGVTALRM);
#endif /* SIGVTALRM */
  sighold(SIGIO);
  sighold(SIGALRM);
#ifdef SIGXFSZ
  sighold(SIGXFSZ);
#endif /* SIGXFSZ */
#else
  oldmask = sigblock(sigmask(SIGVTALRM) | sigmask(SIGIO) | sigmask(SIGALRM)
                     | sigmask(SIGXFSZ)
#ifdef FLTINT
                     | sigmask(SIGFPE)
#endif
                         );
#endif /* SYSVSIGNALS */
#endif /* DOS */
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void int_unblock() {
#ifdef DOS
  _dos_setvect(0x1c, DOStimer);
#else /* DOS */
#ifdef SYSVSIGNALS
#ifdef SIGVTALRM
  ERRCHK(sigrelse(SIGVTALRM), "sigrelse vtalrm");
#endif /* SIGVTALRM */
  ERRCHK(sigrelse(SIGIO), "sigrelse io");
  ERRCHK(sigrelse(SIGALRM), "sigrelse alrm");
#ifdef SIGXFSZ
  ERRCHK(sigrelse(SIGXFSZ), "sigrelse XFSZ");
#endif /* SIGXFSZ */
#else
  sigsetmask(oldmask);
#endif /* SYSVSIGNALS */
#endif /* DOS */
}

void int_timer_on() { int_unblock(); }
void int_timer_off() { int_block(); }

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef NEVER

void int_timer_off() {
#ifdef SYSVSIGNALS
  sigignore(SIGVTALRM);
#else
  struct sigvec tmpv, timeroffv;

  timeroffv.sv_handler = SIG_IGN;
  timeroffv.sv_mask = timeroffv.sv_flags = 0;
  sigvec(SIGVTALRM, &timeroffv, &tmpv);
#endif /* SYSVSIGNALS */
}

void int_timer_on() {
#ifdef SYSVSIGNALS
#ifdef SIGVTALRM
  SIGERRCHK(sigset(SIGVTALRM, int_timer_service), "sigset vtalrm");
#endif /* SIGVTALRM */

#else
  struct sigvec tmpv;

  sigvec(SIGVTALRM, &timerv, &tmpv);
#endif /* SYSVSIGNALS */
}

#endif /* NEVER */

#ifdef FLTINT
/************************************************************************/
/*									*/
/*  F L O A T I N G - P O I N T   I N T E R R U P T   H A N D L I N G	*/
/*									*/
/*	This is the handler for the SIGFPE signal, to catch floating-	*/
/*	point exceptions.  Sets the global 'FP_error' to the error	*/
/*	code passed in by the signal; FP_error is checked by the	*/
/*	Lisp emulator FP code to make sure everything is OK.		*/
/*									*/
/************************************************************************/

/* The global used to signal floating-point errors */
int FP_error = 0;

void int_fp_service(int sig, int code, struct sigcontext *scp)
{
  switch (code) {
    case FPE_FLTDIV_TRAP:
    case FPE_FLTUND_TRAP:
    case FPE_FLTOVF_TRAP:
    case FPE_FLTOPERR_TRAP:

      FP_error = code;
      break;
    default: {
#ifdef DEBUG
      char stuff[100];
      sprintf(stuff, "Unexpected FP error signal code: %d", code);
      perror(stuff);
#else
      FP_error = code;
#endif
    }
  }
#ifdef SYSVSIGNALS
  sigset(SIGFPE, int_fp_service);
#endif /* SYSVSIGNALS */
}

void int_fp_init() { /* first set up the signal handler */
  if (sigset(SIGFPE, int_fp_service))

  perror("Sigset for FPE failed");
  DBPRINT(("FP interrupts enabled\n"));
}

#endif /* FLTINT */

/************************************************************************/
/*									*/
/*			t i m e o u t _ e r r o r			*/
/*									*/
/*	Error handling routine for SIGALRM.  Called when any		*/
/*	TIMEOUT(...) forms spend more than TIMEOUT_TIME (normally	*/
/*	10 sec.) trying to do an I/O operation.				*/
/*									*/
/*									*/
/************************************************************************/

jmp_buf jmpbuf;
static void timeout_error() {
/*
 * Following printf changes the contents of jmpbuf!
 * This would lead to horrible segmentation violation.
 */
/*  printf("File access timed out.\n"); */
#ifdef SYSVSIGNALS
#ifdef SIGALRM
  sigset(SIGALRM, timeout_error);
#endif
#endif /* SYSVSIGNALS */

  longjmp(jmpbuf, 1);
}

/************************************************************************/
/*									*/
/*			i n t _ f i l e _ i n i t			*/
/*									*/
/*	Set up the signal handler for SIGALRM, to catch TIMEOUTs:	*/
/*	TIMEOUT(...) forms spend more than TIMEOUT_TIME (normally	*/
/*	10 sec.) trying to do an I/O operation.				*/
/*									*/
/*									*/
/************************************************************************/

static void int_file_init() {
#ifndef SYSVSIGNALS
  static struct sigvec timerv;
#endif /* SYSVSIGNALS */

  char *envtime;
  int timeout_time;

/* first set up the signal handler */
#ifndef SYSVSIGNALS
  timerv.sv_handler = timeout_error;
  timerv.sv_mask = timerv.sv_flags = 0;
  sigvec(SIGALRM, &timerv, 0);
#else
#ifdef SIGALRM
  sigset(SIGALRM, timeout_error);
#endif /* SIGALRM */
#endif /* SYSVSIGNALS */

  /* Set Timeout period */
  if ((envtime = getenv("LDEFILETIMEOUT")) == NULL) {
    TIMEOUT_TIME = 10;
  } else {
    if ((timeout_time = atoi(envtime)) > 0)
      TIMEOUT_TIME = timeout_time;
    else
      TIMEOUT_TIME = 10;
  }
  DBPRINT(("File timeout interrupts enabled\n"));
}

/************************************************************************/
/*                                                                      */
/*                         p a n i c u r a i d                          */
/*                                                                      */
/*   Most of the unused process-killing interrupts end up here; you     */
/*      can't do a whole lot safely here but dump your sysout for       */
/*      post-mortem analysis, but you MIGHT be able to get a clue       */
/*      about what killed you.                                          */
/*                                                                      */
/************************************************************************/

/* 2017-06-22 NBriggs -- it's not clear that this definition has the correct
   parameters for any known signal handler

      Superceded by sigaction on FreeBSD, Solaris 11, Linux
      Unavailable on macOS
      SVr4, POSIX.1-2001, POSIX.1-2008 (marked obsolete; use sigaction)

      void (*)(int) sigset(int, void (*disp)(int));


     OK on FreeBSD, Solaris 11, macOS, Linux

     struct  sigaction {
             void    (*sa_handler)(int);
             void    (*sa_sigaction)(int, siginfo_t *, void *);
             int     sa_flags;                see signal options below
	     sigset_t sa_mask;                signal mask to apply
     };

     int
     sigaction(int sig, const struct sigaction * restrict act,
               struct sigaction * restrict oact);
 */

void panicuraid(int sig, int code, void *scp, void *addr)
{
  static char errormsg[200];
  static char *stdmsg =
      "Please record the signal and code information\n\
and do a 'v' before trying anything else.";
  int i;

  for (i = 0; i < 200; i++) errormsg[i] = 0;

  switch (sig) {
#ifdef SIGBUS
    case SIGBUS:
      sprintf(errormsg, "BUS error (code %d) at address %p.\n%s", code, addr, stdmsg);
      break;
#endif /* SIGBUS */
    case SIGSEGV:
      sprintf(errormsg, "SEGV error (code %d) at address %p.\n%s", code, addr, stdmsg);
      break;
    case SIGILL:
      sprintf(errormsg, "Illegal instruction (code %d) at address %p.\n%s", code, addr, stdmsg);
      break;
#ifdef SIGPIPE
    case SIGPIPE:
      sprintf(errormsg, "Broken PIPE (code %d) at address %p.\n%s", code, addr, stdmsg);
      break;
#endif /* SGPIPE */
#ifdef SIGHUP
    case SIGHUP:
      sprintf(errormsg, "HANGUP signalled (code %d) at address %p.\n%s", code, addr, stdmsg);
/* Assume that a user tried to exit UNIX shell */
#ifdef SYSVONLY
      kill(0, SIGKILL);
      exit(0);
#else
      killpg(getpgrp(), SIGKILL);
      exit(0);
#endif /* SYSVONLY */
      break;
#endif /* SIGHUP */
    case SIGFPE:
      sprintf(errormsg, "FP error (code %d) at address %p.\n%s", code, addr, stdmsg);
      break;
    default: sprintf(errormsg, "Uncaught SIGNAL %d (code %d).\n%s", sig, code, stdmsg);
  }

  error(errormsg);
}

/************************************************************************/
/*                                                                      */
/*                     i n t _ p a n i c _ i n i t                      */
/*                                                                      */
/*  A catch for all the deadly interupts (but KILL, of course)          */
/*  Dumps you into uraid; you probably can't get back from it,          */
/*     but there is hope that you will be able to poke around with      */
/*     uraid and get a clue about why you're dying.                     */
/*                                                                      */
/************************************************************************/
static void int_panic_init() {
#ifdef SYSVSIGNALS
#ifndef DOS
  sigset(SIGHUP, panicuraid);
  sigset(SIGQUIT, panicuraid);
  sigset(SIGILL, panicuraid);
#ifdef SIGEMT
  sigset(SIGEMT, panicuraid);
#endif
  sigset(SIGBUS, panicuraid);
  sigset(SIGSEGV, panicuraid);
#ifdef SIGSYS
  sigset(SIGSYS, panicuraid);
#endif
  sigset(SIGTERM, panicuraid);
#endif
#else
  static struct sigvec panicv;
  static struct sigvec ignorev;

  /* first set up the signal handlers: */

  panicv.sv_handler = panicuraid;
  panicv.sv_mask = panicv.sv_flags = 0;
  ignorev.sv_handler = SIG_IGN;
  ignorev.sv_mask = ignorev.sv_flags = 0;

  /* Now arrange for signals to be handled properly: */

  sigvec(SIGHUP, &panicv, 0);
  /*  sigvec(SIGINT,  &panicv, 0); */
  sigvec(SIGQUIT, &panicv, 0);
  sigvec(SIGILL, &panicv, 0);
/*  sigvec(SIGTRAP, &panicv, 0); */
#ifdef OS4
  sigvec(SIGABRT, &panicv, 0);
#endif /* OS4 */
#ifdef SIGEMT
  sigvec(SIGEMT, &panicv, 0);
#endif
  sigvec(SIGBUS, &panicv, 0);
  sigvec(SIGSEGV, &panicv, 0);
#ifdef SIGSYS
  sigvec(SIGSYS, &panicv, 0);
#endif
  /*  sigvec(SIGPIPE, &panicv, 0);  Caused trouble with TCP; now ignored: */
  sigvec(SIGPIPE, &ignorev, 0);
  sigvec(SIGTERM, &panicv, 0);
#ifdef OS4
  sigvec(SIGXCPU, &panicv, 0);
  sigvec(SIGLOST, &panicv, 0);
#endif /* OS4 */

  sigvec(SIGUSR1, &panicv, 0);
  sigvec(SIGUSR2, &panicv, 0);
#endif /* SYSVSIGNALS */

  DBPRINT(("Panic interrupts enabled\n"));
}

/************************************************************************/
/*									*/
/*			    i n t _ i n i t				*/
/*									*/
/*	Initialize all the interrupts for Lisp & the emulator.		*/
/*									*/
/************************************************************************/

void int_init() {
  int_timer_init(); /* periodic interrupt timer */
  int_io_init();    /* SIGIO and SIGPOLL async I/O handlers */
  int_file_init();  /* file-io TIMEOUT support */
  int_panic_init(); /* catch for all other dangerous interrupts */

#ifdef FLTINT
  int_fp_init(); /* Floating-point exception handler */
#endif

  int_unblock(); /* Turn on interrupts */
}

#ifdef DOS
/******************************************************************************
*  DOStimer()
*
*  The interrupt 0x1c handler.  This routine must be declared using the
*  '#pragma interrupt()' statement to ensure that all registers are preserved.
*  It is also needed to ensure the proper functioning of '_chain_intr()'.
*
*  The timer interrupt (normally) occurs 18.2 times per second.  This routine
*  waits one extra tick every 91 ticks (18.2*5).
*
*  Before this interrupt was installed, 'prev_int_1c' was set to the current
*  0x1c interrupt. 'DOStimer()' chains to this interrupt using '_chain_intr()',
*  rather than returning back to the caller.
*
*  Note that as little as possible should be done within a timer interrupt,
*  since further clock ticks are disabled until the interrupt returns.
******************************************************************************/
void DOStimer() {
  /* if (--tick_count == 0) { */
  Irq_Stk_Check = 0;
  Irq_Stk_End = 0;
  TimerFlag = 1;
  /*   _dos_setvect(0x1c, prev_int_1c);
   } else if (tick_count <= 0) { */
  /* I'm dead, uninstal me */
  /*   _dos_setvect(0x1c, prev_int_1c);
     tick_count = 0;
   } */
  _chain_intr(prev_int_1c); /* call previous int 1c handlr, if any*/
  /* (pts to 'ret' if no prev installed)*/
}

void alarm(unsigned long sec)
{
  /* tick_count = sec * 18;
  _dos_setvect(0x1c, DOStimer); */
}
#endif /* DOS */
