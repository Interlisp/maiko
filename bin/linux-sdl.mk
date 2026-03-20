# Common Options for Linux using SDL instead of X Windows

XFILES = $(OBJECTDIR)sdl.o

#
#  For SDL version 2
#      -DSDL=2 in XFLAGS and -lSDL2 in LDFLAGS
#  For SDL version 3
#      -DSDL=3 in XFLAGS and -lSDL3 in LDFLAGS
#
XFLAGS = -DSDL=2

XLDFLAGS ?= -lSDL2

