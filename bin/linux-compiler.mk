# Select whether to use clang or gcc
# Priority
#  1.  If USE_COMPILER=gcc or USE_COMPILER=clang on make command line use the requested compiler.
#  2.  If clang is installed use it.
#  3.  Use gcc

EXISTS_GCC := $(shell /bin/sh -c command -v gcc)
EXISTS_CLANG := $(shell /bin/sh -c command -v clang)
ifeq ($(or $(EXISTS_GCC),$(EXISTS_CLANG)),)
  $(error "Cannot find compiler: neither gcc nor clang. Exiting.")
endif
COMPILER :=
ifeq ($(USE_COMPILER),clang)
  ifeq ($(EXISTS_CLANG),)
    $(error "USE_COMPILER=clang, but cannot find the clang compiler. Exiting")
  endif
  COMPILER := clang
else ifeq ($(USE_COMPILER),gcc)
  ifeq ($(EXISTS_GCC),)
    $(error "USE_COMPILER=gcc given, but cannot find the gcc compiler. Exiting")
  endif
  COMPILER := gcc
else ifneq ($(EXISTS_CLANG),)
  COMPILER := clang
else
  COMPILER := gcc
endif

ifeq ($(COMPILER),)
  $(error "Oops.  Trying to select gcc or clang but should never get here")
endif

ifeq ($(COMPILER),gcc)
  CC := gcc $(GCC_CFLAGS)
else
  CC := clang $(CLANG_CFLAGS)
endif
