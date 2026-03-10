# Common Options for All Linuxes

CC = gcc $(GCC_CFLAGS)
# CC = clang $(CLANG_CFLAGS)

include linux-libbsd.mk

# OPTFLAGS is normally -O2.
OPTFLAGS =  -O2 -g3
DFLAGS = $(XFLAGS) -DRELEASE=$(RELEASE) $(BSD_CFLAGS) $(ADDITIONAL_DFLAGS)

LDFLAGS =  $(XLDFLAGS) -lc -lm $(BSD_LDFLAGS)
LDELDFLAGS =  $(XLDFLAGS) -lc -lm $(BSD_LDFLAGS)

OBJECTDIR = ../$(RELEASENAME)/
