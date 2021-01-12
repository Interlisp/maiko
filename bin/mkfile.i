
#line 1 "d:/windows/TEMP/cbr3"
#pragma si(C:\codebldr\inc\) 

#line 1 "mkfile.c"

  
 

 

 
 
BINARYDIR = ../bin
AFLAGS = /T
COLORFILES = rawcolor.obj
ARCHFILES = dosmouse.obj doskbd.obj vesafns.obj vesainit.obj vgainit.obj kbdif.obj



ADMINFILES = mkdos mkvdate.c optck.c

ETHERFILES = ldeether.obj

KEY = keytstno.obj

CFLAGS = -I. -DDOS -DBYTESWAP -DKBINT -DNOPIXRECT -O2

LDFLAGS = -O2 graphics.lib binmode.lib mouse.lib 

SRCFILES = ./conspage.c ./gcoflow.c ./shift.c ./dbgtool.c ./gcr.c	./llcolor.c ./gcrcell.c ./llstk.c ./gcscan.c ./loopsops.c	./storage.c ./allocmds.c ./dir.c ./gvar2.c ./lowlev1.c	./subr.c ./arith2.c ./hacks.c ./lowlev2.c ./subr0374.c 	./arith3.c ./doscomm.c ./hardrtn.c ./lsthandl.c ./sxhash.c 	./arith4.c ./draw.c ./main.c ./testtool.c ./array.c ./dsk.c 	./inet.c ./misc7.c ./timer.c ./array2.c ./dspif.c ./initdsp.c 	./miscn.c ./typeof.c ./array3.c ./initkbd.c ./ubf1.c 	./array4.c ./dspsubrs.c ./initsout.c ./mkatom.c ./ubf2.c 	./array5.c ./eqf.c ./intcall.c ./mkcell.c ./ubf3.c ./array6.c 	./ether.c ./mkvdate.c ./ufn.c ./atom.c ./findkey.c 	./kbdsubrs.c ./mouseif.c ./ufs.c ./bbtsub.c ./foreign.c 	./keyevent.c ./unixcomm.c ./bin.c ./fp.c ./keylib.c ./binds.c 	./asmbbt.c ./fvar.c ./mvs.c ./unwind.c ./bitblt.c ./gc.c 	./uraid.c ./blt.c ./gc2.c ./kprint.c ./keytstno.c ./keytst.c	./osmsg.c usrsubr.c ./byteswap.c ./gcarray.c 	./perrno.c ./ldeboot.c ./ldeether.c ./uutils.c ./carcdr.c ./gccode.c 	./rawcolor.c ./vars3.c ./gcfinal.c ./ldsout.c ./return.c 	./vmemsave.c ./chardev.c ./gchtfind.c ./lineblt8.c ./rpc.c 	./xc.c ./common.c ./gcmain3.c ./lisp2c.c ./rplcons.c ./z2.c 	./find-dsp.l ./dsphack.l 	./xmkicon.c ./xbbt.c ./xinit.c ./xscroll.c ./xcursor.c ./xlspwin.c 	./xrdopt.c ./xwinman.c 	./dosmouse.c ./vesafns.asm ./vesainit.c ./vgainit.c ./kbdif.c 	./dspsparc.il ./copyright ./launch.asm

OFILES = conspage.obj gcoflow.obj shift.obj dbgtool.obj 	gcr.obj llcolor.obj gcrcell.obj llstk.obj 	gcscan.obj loopsops.obj storage.obj 	allocmds.obj dir.obj gvar2.obj lowlev1.obj 	subr.obj arith2.obj hacks.obj lowlev2.obj 	subr0374.obj arith3.obj doscomm.obj 	hardrtn.obj lsthandl.obj sxhash.obj arith4.obj 	draw.obj main.obj testtool.obj array.obj 	dsk.obj inet.obj misc7.obj timer.obj 	array2.obj dspif.obj initdsp.obj miscn.obj 	typeof.obj array3.obj initkbd.obj ubf1.obj 	array4.obj dspsubrs.obj initsout.obj 	mkatom.obj ubf2.obj array5.obj eqf.obj 	intcall.obj mkcell.obj ubf3.obj array6.obj 	ether.obj ufn.obj atom.obj 	findkey.obj kbdsubrs.obj mouseif.obj ufs.obj 	bbtsub.obj foreign.obj keyevent.obj 	unixcomm.obj bin.obj fp.obj keylib.obj 	binds.obj fvar.obj mvs.obj 	unwind.obj bitblt.obj gc.obj 	uraid.obj blt.obj gc2.obj 	kprint.obj osmsg.obj usrsubr.obj byteswap.obj 	gcarray.obj perrno.obj uutils.obj 	carcdr.obj asmbbt.obj gccode.obj 	vars3.obj gcfinal.obj ldsout.obj 	return.obj vmemsave.obj chardev.obj 	gchtfind.obj lineblt8.obj rpc.obj xc.obj 	common.obj gcmain3.obj lisp2c.obj rplcons.obj 	z2.obj vdate.obj $(KEY) $(COLORFILES) $(ARCHFILES) 


HFILES = ../inc/address.h ../inc/adr68k.h ../inc/arith.h ../inc/cell.h ../inc/dbprint.h ../inc/display.h 	../inc/dspif.h ../inc/ifpage.h ../inc/iopage.h ../inc/lispemul.h ../inc/lispmap.h 	../inc/lsptypes.h ../inc/miscstat.h ../inc/lspglob.h ../inc/array.h ../inc/bb.h 	../inc/bitblt.h ../inc/debug.h ../inc/devconf.h ../inc/dspdata.h ../inc/ether.h 	../inc/fast_dsp.h ../inc/gcdata.h ../inc/hdw_conf.h ../inc/initatms.h ../inc/inlinec.h ../inc/keyboard.h 	../inc/lispver1.h ../inc/lispver2.h ../inc/lldsp.h ../inc/locfile.h ../inc/medleyfp.h ../inc/mouseif.h ../inc/my.h 	../inc/opcodes.h ../inc/osmsg.h ../inc/pilotbbt.h ../inc/print.h ../inc/profile.h 	../inc/return.h ../inc/stack.h ../inc/stream.h ../inc/subrs.h ../inc/timeout.h 	../inc/tos1defs.h ../inc/tosfns.h ../inc/tosret.h ../inc/vmemsave.h 	../inc/xdefs.h ../inc/xbitmaps.h ../inc/xkeymap.h



       
dos4    :
	$(MAKE) -f ./mkfile $(MFLAGS) dosmkfil
	$(MAKE) -f ./dosmkfil $(MFLAGS) ../bin/medley.exe
	$(MAKE) -f ./dosmkfil $(MFLAGS) ../bin/emul.exe

       
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
	- cd ARCH;$(MAKE) $(MFLAGS) all         #/* Make the specific files for this target */
	$(CC) $(LDFLAGS) $(OFILES) -o $@

   
./main.o  :   ../inc/lispemul.h ../inc/address.h ../inc/lsptypes.h ../inc/adr68k.h                        ../inc/stack.h ../inc/lspglob.h ../inc/lispmap.h ../inc/ifpage.h                        ../inc/iopage.h ../inc/return.h ../inc/debug.h ../inc/profile.h


      

.SUFFIXES .exe .lib .c .obj .c .asm .s .c

../bin/medley.exe:     ./launch.obj
	TLINK launch,medley

./launch.obj:     ./launch.asm
	copy ..\src\launch.asm launch.asm
	tasm /ml launch.asm

./xc.obj:      ./xc.s
     tasm /ml xc.s

./xc.s:        ./xc.c
     rsh sparky (cd /users/sybalsky/maiko/src ; gcc-make $* )

