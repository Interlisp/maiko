# Common Options for Linux with X Windows

XFILES = $(OBJECTDIR)xmkicon.o \
	$(OBJECTDIR)xbbt.o \
	$(OBJECTDIR)dspif.o \
	$(OBJECTDIR)xinit.o \
	$(OBJECTDIR)xscroll.o \
	$(OBJECTDIR)xcursor.o \
	$(OBJECTDIR)xlspwin.o \
	$(OBJECTDIR)xrdopt.o \
	$(OBJECTDIR)xwinman.o


XFLAGS = -DXWINDOW

XLDFLAGS = -L/usr/X11/lib -lX11

