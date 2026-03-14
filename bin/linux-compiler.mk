# Select whether to use clang or gcc
# Priority
#  1.  If -DUSE_GCC or -DUSE_CLANG on command line (but not both) use the requested compiler.
#  2.  If one compiler is installed but not the other,  use the installed compiler.
#  3.  Use clang

EXISTS_GCC := $(shell command -v gcc)
EXISTS_CLANG := $(shell command -v clang)
ifeq ($(or $(EXISTS_GCC),$(EXISTS_CLANG)),)
  $(error "Cannot find compiler: neither gcc nor clang. Exiting.")
endif
ifneq ($(and $(USE_CLANG),$(USE_GCC)),)
  $(error "Cannot use both USE_CLANG=T and USE_GCC=T.  Exiting.")
endif
COMPILER :=
ifdef USE_CLANG
  ifeq ($(EXISTS_CLANG),)
    $(error "USE_CLANG=T given, but cannot find the clang compiler. Exiting")
  endif
  COMPILER := clang
endif
ifdef USE_GCC
  ifeq ($(EXISTS_GCC),)
    $(error "USE_GCC=T given, but cannot find the gcc compiler. Exiting")
  endif
  COMPILER := gcc
endif
ifeq ($(COMPILER),)
  ifneq ($(EXISTS_CLANG),)
    COMPILER := clang
  else
    COMPILER := gcc
  endif
endif

ifeq ($(COMPILER),gcc)
  CC := gcc $(GCC_CFLAGS)
else
  CC := clang $(CLANG_CFLAGS)
endif
