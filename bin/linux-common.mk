# Common Options for All Linuxes

include linux-compiler.mk

ifeq ($(USE_LIBBSD),T)
  include linux-libbsd.mk
else
  BSD_CFLAGS :=
  BSD_LDFLAGS :=
endif

ifeq ($(USE_DISPLAY),x)
  include linux-x.mk
  DEFAULT_TARGET := ../$(OSARCHNAME)/lde ../$(OSARCHNAME)/ldex
endif
ifeq ($(USE_DISPLAY),sdl)
  include linux-sdl.mk
  DEFAULT_TARGET := ../$(OSARCHNAME)/lde ../$(OSARCHNAME)/ldesdl
endif
ifeq ($(USE_DISPLAY),init)
  include linux-x.mk
  DEFAULT_TARGET := ../$(OSARCHNAME)/ldeinit
endif

OPTFLAGS ?=  -O2 -g3
DFLAGS = $(XFLAGS) -DRELEASE=$(RELEASE) $(BSD_CFLAGS) $(ADDITIONAL_DFLAGS)

LDFLAGS =  $(XLDFLAGS) -lc -lm $(BSD_LDFLAGS)
LDELDFLAGS =  $(XLDFLAGS) -lc -lm $(BSD_LDFLAGS)

OBJECTDIR = ../$(RELEASENAME)/

default: $(DEFAULT_TARGET)
