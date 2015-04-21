##########################################################################
# This is the user installation makefile, it does not make the "source"
# files in this directory, i.e. it uses the objectfiles :
# ldesingle.o, ldemulti.o, ldex.o
##########################################################################

default	: ldesingle ldemulti ldex ldeether

#### The native translator needs the symbol table in 'ldesingle' or 'ldemulti',
#### so don't 'strip'.

ldesingle :  ldesingle.o usersubrs.o
	cc ldesingle.o usersubrs.o -lsuntool -lsunwindow -lpixrect -lc -lm -o ldesingle

ldemulti :  ldemulti.o usersubrs.o
	cc ldemulti.o usersubrs.o -lsuntool -lsunwindow -lpixrect -lc -lm -o ldemulti

ldex :  ldex.o usersubrs.o
	cc ldex.o usersubrs.o -lX11 -lpixrect -lc -lm -o ldex

ldeether : ldeether.c
	cc ldeether.c -DOS4 -o ldeether
	@echo ""
	@echo "The 'ldeether' executable must be made setuid root to allow"
	@echo "PUP/XNS Ethernet access.  Feel free to examine the source"
	@echo "of 'ldeether' before doing this."


#### user ops is placeholder for user subrs.  needs work.

usersubrs.o :    usersubrs.c
	cc -c -O usersubrs.c 

#### gets rid of user-created files.

cleanup :
	rm -f ldesingle ldemulti ldex ldeether runlisp usersubrs.o


