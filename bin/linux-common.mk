# Common Options for All Linuxes

include linux-compiler.mk

BSD_CFLAGS :=
BSD_LDFLAGS :=
ifeq ($(USE_LIBBSD),T)
    # Use LIBBSD - but only if glibc < 2.38
    # Because we only need strlcat, strlcpy and friends from libbsd
    # and they are included in glibc from 2.38 on.
    GLIBC_VERSION := $(shell ldd --version | head -1 | sed -e "s/^.* \([0-9]\.[0-9]\+\)/\\1/")
    GLIBC_CHECK := $(shell echo "$(GLIBC_VERSION) >= 2.38" | bc)
    ifneq ($(GLIBC_CHECK),1)
      include linux-libbsd.mk
    endif
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

ifeq ($(USE_DISPLAY),x)
  LDELDFLAGS =  $(XLDFLAGS) -lc -lm $(BSD_LDFLAGS)
else
  LDELDFLAGS = -lc -lm $(BSD_LDFLAGS)
endif

OBJECTDIR = ../$(RELEASENAME)/

default: $(DEFAULT_TARGET)
