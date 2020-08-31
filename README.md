# Maiko 

This is the emulator for the Interlisp-D virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting witih Lisp for access to display and disk etc.

There’s a system of make files that include all the flags and
variables you have to set for each hardware/OS target.

To make the emulator, currently:

* be in the "bin" directory
* have "." on your path
* do "./makeright x"

It will detect the correct OS and then put together the makefile parts
that it needs.

It will build it in ../<osname>.<cputype>-x (for the .o files)
and ../<osname>.<cputype> for the executables.

---

See COMPILE-FLAGS.md for explanation of compile flags


---
Formerly in ChangeLog

1 Jan 02  JDS

	FILES CHANGED:  src/byteswap.c bin/makefile-tail

	Fix MAKE rules for tstsout, setsout so they use the byteswapper if
	necessary.  Changed byteswap.c to only define byte_swap_code_page
	if we're using reswapped code streams (which we aren't, as of
	now), so we don't have to drag that all in or move the function to
	someplace unrelated.

	
1 Jan 2002  JDS`

	FILES CHANGED:  bin/config.guess bin/config.sub bin/osversion,
			bin/machinetype (new), bin/makeright, bin/README

	Update config.guess to GNU latest, and make osversion use it.  Add
	a new script, machinetype, also based on config.guess to determine
	underlying hardware architecture for us.  Changed makeright to use
	machinetype.

	Added a README file, describing where to get new versions of the
	config utilities.

22 Nov 01 JDS

	FILES CHANGED:  bin/makeright, bin/makefile-linux.386-x, osversion,
	                mach

	Revamped Linux make to use "makeright" script, as other systems
	do.  Need to revamp it all to use autoconf somehow.

13 Dec 01 JDS

	FILES CHANGED:  llstk.c, xlspwin.c, dspsubrs.c, xcursor.c

	The function Set_XCursor is called two ways:  From the DSPCURSOR subr,
	and within the C code directly.  I had to fix the direct C calls
	because Set_XCursor sets Current_Hot_X to 15-y (its second arg)--
	but the function was being called with Current_Hot_Y as the 2nd
	arg.  As a result (HARDRESET) followed by ^B would bollix the
	cursor position and menu selection would be off.

	Fixed files llstk.c, xlspwin.c, dspsubrs.c to make it work.

	Added a warning comment to the function itself.

20 Dec 01 JDS

	FILES CHANGED:  xrdopt.c, main.c, lspwin.c, storage.c

	The X-emulator parser for command line options would NOT accept
	the -m flag to set max memory size (it required -memory); changed
	the -maxpages to -xpages (controlling # of pages written at once
	in sysout writing), and changed the default sysout size from 8Mb
	to 32Mb.

22 Dec 01 JDS

	FILES CHANGED:  main.c, mkvdate.c, version.h

	Changed Linux to using gettimeofday() from time(), so we get finer
	grained timings.  Changed mkvdate.c to include sys/time.h unless
	USETIMEFN rather than unless SYSVONLY.

	Need to think about using autoconf to drive make process and get
	away from the script-based maker.

	


