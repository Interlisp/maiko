#define CORRECT WRONG   /* don't mind this text */
#define BELOW in the file makefile in this directory.

 # You are editing the CORRECT file.
 # Read more BELOW.

 # /* When you make a compile target this file is run through
 # cpp and redirected to a file called mkfile.
 # the file mkfile is then used as the make file for the subtargets.
 # This may seem convoluted but the win is quite big. /jarl  */


 # /* The following #ifdef ... #endif selection uses the
 #  symbols kexitnown to the local icc we use to transmogrify
 #  this file with. When you port to a new arch you should
 #  a) find the unique icc macros (sparc, mips, ibm etc.)
 #  b) add or edit this to the #ifdef selection  below
 #  c) try it out by doing a make. */


 # defDEBUG -g -m
#define DEBUG -O2
#define OEXT o

 # remember   -DNOEUROKBD

#ifdef _INTELC32_       /* The cpp macro for the DOS extender */
#define SRCDIR .
#define OBJECTDIR .
#define BINDIR ../bin
#define INCDIR ../inc
#define EXTRACFLAGS -DDOS -DBYTESWAP -DKBINT -DFSERROR -DNOPIXRECT -DNOFORN -DNOETHER -DBIGATOMS -DBIGVM -DNEWCDRCODING
#define EXTRALDFLAGS graphics.lib binmode.lib mouse.lib
BINARYDIR = BINDIR
AFLAGS = /T
COLORFILES = rawcolor.obj
ARCHFILES = dosmouse.obj doskbd.obj vesafns.obj vesainit.obj vgainit.obj kbdif.obj
#define EXTRAFILES
#define EXTRALDFLAGS
#undef OEXT
#define OEXT obj

#endif  /* DOS */

#define XFLAGS
#define  XLDFLAGS

ADMINFILES = mkdos mkvdate.c optck.c

ETHERFILES = ldeether.OEXT

KEY = keytstno.OEXT

CFLAGS = -I. -DBIGATOMS -DNEW_STORAGE XFLAGS EXTRACFLAGS DEBUG

LDFLAGS = DEBUG EXTRALDFLAGS XLDFLAGS

SRCFILES = SRCDIR/conspage.c SRCDIR/gcoflow.c SRCDIR/shift.c SRCDIR/dbgtool.c SRCDIR/gcr.c\
	SRCDIR/llcolor.c SRCDIR/gcrcell.c SRCDIR/llstk.c SRCDIR/gcscan.c SRCDIR/loopsops.c\
	SRCDIR/storage.c SRCDIR/allocmds.c SRCDIR/dir.c SRCDIR/gvar2.c SRCDIR/lowlev1.c\
	SRCDIR/subr.c SRCDIR/arith2.c SRCDIR/hacks.c SRCDIR/lowlev2.c SRCDIR/subr0374.c \
	SRCDIR/arith3.c SRCDIR/doscomm.c SRCDIR/hardrtn.c SRCDIR/lsthandl.c SRCDIR/sxhash.c \
	SRCDIR/arith4.c SRCDIR/draw.c SRCDIR/main.c SRCDIR/testtool.c SRCDIR/array.c SRCDIR/dsk.c \
	SRCDIR/inet.c SRCDIR/misc7.c SRCDIR/timer.c SRCDIR/array2.c SRCDIR/dspif.c SRCDIR/initdsp.c \
	SRCDIR/miscn.c SRCDIR/typeof.c SRCDIR/array3.c SRCDIR/initkbd.c SRCDIR/ubf1.c \
	SRCDIR/array4.c SRCDIR/dspsubrs.c SRCDIR/initsout.c SRCDIR/mkatom.c SRCDIR/ubf2.c \
	SRCDIR/array5.c SRCDIR/eqf.c SRCDIR/intcall.c SRCDIR/mkcell.c SRCDIR/ubf3.c SRCDIR/array6.c \
	SRCDIR/ether.c SRCDIR/mkvdate.c SRCDIR/ufn.c SRCDIR/atom.c SRCDIR/findkey.c \
	SRCDIR/kbdsubrs.c SRCDIR/mouseif.c SRCDIR/ufs.c SRCDIR/bbtsub.c SRCDIR/foreign.c \
	SRCDIR/keyevent.c SRCDIR/unixcomm.c SRCDIR/bin.c SRCDIR/fp.c SRCDIR/keylib.c SRCDIR/binds.c \
	SRCDIR/asmbbt.c SRCDIR/fvar.c SRCDIR/mvs.c SRCDIR/unwind.c SRCDIR/bitblt.c SRCDIR/gc.c \
	SRCDIR/uraid.c SRCDIR/blt.c SRCDIR/gc2.c SRCDIR/kprint.c SRCDIR/keytstno.c SRCDIR/keytst.c\
	SRCDIR/osmsg.c usrsubr.c SRCDIR/byteswap.c SRCDIR/gcarray.c \
	SRCDIR/perrno.c SRCDIR/ldeboot.c SRCDIR/ldeether.c SRCDIR/uutils.c SRCDIR/carcdr.c SRCDIR/gccode.c \
	SRCDIR/rawcolor.c SRCDIR/vars3.c SRCDIR/gcfinal.c SRCDIR/ldsout.c SRCDIR/return.c \
	SRCDIR/vmemsave.c SRCDIR/chardev.c SRCDIR/gchtfind.c SRCDIR/lineblt8.c SRCDIR/rpc.c \
	SRCDIR/xc.c SRCDIR/common.c SRCDIR/gcmain3.c SRCDIR/lisp2c.c SRCDIR/rplcons.c SRCDIR/z2.c \
	SRCDIR/find-dsp.l SRCDIR/dsphack.l \
	SRCDIR/xmkicon.c SRCDIR/xbbt.c SRCDIR/xinit.c SRCDIR/xscroll.c SRCDIR/xcursor.c SRCDIR/xlspwin.c \
	SRCDIR/xrdopt.c SRCDIR/xwinman.c \
	SRCDIR/dosmouse.c SRCDIR/vesafns.asm SRCDIR/vesainit.c SRCDIR/vgainit.c SRCDIR/kbdif.c \
	SRCDIR/dspsparc.il SRCDIR/copyright SRCDIR/launch.asm

OFILES = conspage.OEXT gcoflow.OEXT shift.OEXT dbgtool.OEXT \
	gcr.OEXT llcolor.OEXT gcrcell.OEXT llstk.OEXT \
	gcscan.OEXT loopsops.OEXT storage.OEXT \
	allocmds.OEXT dir.OEXT gvar2.OEXT lowlev1.OEXT \
	subr.OEXT arith2.OEXT hacks.OEXT lowlev2.OEXT \
	subr0374.OEXT arith3.OEXT doscomm.OEXT \
	hardrtn.OEXT lsthandl.OEXT sxhash.OEXT arith4.OEXT \
	draw.OEXT main.OEXT testtool.OEXT array.OEXT \
	dsk.OEXT inet.OEXT misc7.OEXT timer.OEXT \
	array2.OEXT dspif.OEXT initdsp.OEXT miscn.OEXT \
	typeof.OEXT array3.OEXT initkbd.OEXT ubf1.OEXT \
	array4.OEXT dspsubrs.OEXT initsout.OEXT \
	mkatom.OEXT ubf2.OEXT array5.OEXT eqf.OEXT \
	intcall.OEXT mkcell.OEXT ubf3.OEXT array6.OEXT \
	ether.OEXT ufn.OEXT atom.OEXT \
	findkey.OEXT kbdsubrs.OEXT mouseif.OEXT ufs.OEXT \
	bbtsub.OEXT foreign.OEXT keyevent.OEXT \
	unixcomm.OEXT bin.OEXT fp.OEXT keylib.OEXT \
	binds.OEXT fvar.OEXT mvs.OEXT \
	unwind.OEXT bitblt.OEXT gc.OEXT \
	uraid.OEXT blt.OEXT gc2.OEXT \
	kprint.OEXT osmsg.OEXT usrsubr.OEXT byteswap.OEXT \
	gcarray.OEXT perrno.OEXT uutils.OEXT \
	carcdr.OEXT asmbbt.OEXT gccode.OEXT \
	vars3.OEXT gcfinal.OEXT ldsout.OEXT \
	return.OEXT vmemsave.OEXT chardev.OEXT \
	gchtfind.OEXT lineblt8.OEXT rpc.OEXT xc.OEXT \
	common.OEXT gcmain3.OEXT lisp2c.OEXT rplcons.OEXT \
	z2.OEXT vdate.OEXT $(KEY) $(COLORFILES) $(ARCHFILES) EXTRAFILES


HFILES = INCDIR/address.h INCDIR/adr68k.h INCDIR/arith.h INCDIR/cell.h INCDIR/dbprint.h INCDIR/display.h \
	INCDIR/dspif.h INCDIR/ifpage.h INCDIR/iopage.h INCDIR/lispemul.h INCDIR/lispmap.h \
	INCDIR/lsptypes.h INCDIR/miscstat.h INCDIR/lspglob.h INCDIR/array.h INCDIR/bb.h \
	INCDIR/bitblt.h INCDIR/debug.h INCDIR/devconf.h INCDIR/dspdata.h INCDIR/ether.h \
	INCDIR/fast_dsp.h INCDIR/fp.h INCDIR/gc.h INCDIR/hdw_conf.h INCDIR/initatms.h INCDIR/inlinec.h INCDIR/keyboard.h \
	INCDIR/lispver1.h INCDIR/lispver2.h INCDIR/lldsp.h INCDIR/locfile.h INCDIR/mouseif.h INCDIR/my.h \
	INCDIR/opcodes.h INCDIR/osmsg.h INCDIR/pilotbbt.h INCDIR/print.h INCDIR/profile.h \
	INCDIR/return.h INCDIR/stack.h INCDIR/stream.h INCDIR/subrs.h INCDIR/sysatms.h INCDIR/timeout.h \
	INCDIR/tos1defs.h INCDIR/tosfns.h INCDIR/tosret.h INCDIR/vmemsave.h \
	INCDIR/xdefs.h INCDIR/xbitmaps.h INCDIR/xkeymap.h



 ##############################
 ### Entry rules            ###
 ###############            ###
 ### make only one of these ###
 ### four rules on the      ###
 ### commandline            ###
 ##############################

dos4    :
	$(MAKE) -f ./mkfile $(MFLAGS) dosmkfil
	$(MAKE) -f ./dosmkfil $(MFLAGS) ../bin/medley.exe
	$(MAKE) -f ./dosmkfil $(MFLAGS) ../bin/emul.exe

 ###############################
 ### Compile rules           ###
 #################           ###
 ### Don't touch these. The  ###
 ### following rules are     ###
 ### used by the entry rules ###
 ###############################

dosmkfil : mkfile
	-copy mkfile mkfile.c
	-$(CC) /P /c mkfile.c 2> junk
	-copy mkfile.i dosmkfil

../bin/emul.exe : $(OFILES)
	@ echo $** > linkopts
	@ echo  $(CFLAGS) > copts
	$(CC) @copts @linkopts $(LDFLAGS) /e$@
	del linkopts
	del copts
	@ echo "Executable is now named '$@'"

lde     : $(OFILES) mkvdate
	$(RM) vdate.c
	mkvdate > vdate.c
	- cd ARCH;$(MAKE) $(MFLAGS) all         #/* Make the speciffic files for this target */
	$(CC) $(LDFLAGS) $(OFILES) -o $@

 ##################
 ### File rules ###
 ##################

OBJECTDIR/main.o  :   INCDIR/lispemul.h INCDIR/address.h INCDIR/lsptypes.h INCDIR/adr68k.h\
                        INCDIR/stack.h INCDIR/lspglob.h INCDIR/lispmap.h INCDIR/ifpage.h\
                        INCDIR/iopage.h INCDIR/return.h INCDIR/debug.h INCDIR/profile.h


 ######################################
 ### Architecture speciffic targets ###
 ###                                ###
 ### replaces the cruft in the      ###
 ### makefile-<OS>.<ARCH>           ###
 ######################################

#ifdef _INTELC32_       /* The cpp macro for the DOS extender */

.SUFFIXES .exe .lib .c .obj .c .asm .s .c

BINDIR/medley.exe:     OBJECTDIR/launch.obj
	TLINK launch,medley

OBJECTDIR/launch.obj:     SRCDIR/launch.asm
	copy ..\src\launch.asm launch.asm
	tasm /ml launch.asm

OBJECTDIR/xc.obj:      SRCDIR/xc.s
     tasm /ml xc.s

SRCDIR/xc.s:        SRCDIR/xc.c
     rsh sparky (cd /users/sybalsky/maiko/src ; gcc-make $* )

#endif
