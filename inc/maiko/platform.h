#ifndef MAIKO_PLATFORM_H
#define MAIKO_PLATFORM_H 1

/*
 * Set up various preprocessor definitions based upon
 * the platform.
 */

#if defined(__APPLE__) && defined(__MACH__)
#  define MAIKO_OS_MACOS 1
#  define MAIKO_OS_NAME "macOS"
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __CYGWIN__
#  define MAIKO_OS_CYGWIN 1
#  define MAIKO_OS_NAME "Cygwin"
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __DragonFly__
#  define MAIKO_OS_DRAGONFLYBSD 1
#  define MAIKO_OS_NAME "DragonFly BSD"
#  define MAIKO_OS_BSD_LIKE 1
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __FreeBSD__
#  define MAIKO_OS_FREEBSD 1
#  define MAIKO_OS_NAME "FreeBSD"
#  define MAIKO_OS_BSD_LIKE 1
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __linux__
#  define MAIKO_OS_LINUX 1
#  define MAIKO_OS_NAME "Linux"
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __NetBSD__
#  define MAIKO_OS_NETBSD 1
#  define MAIKO_OS_NAME "NetBSD"
#  define MAIKO_OS_BSD_LIKE 1
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#ifdef __OpenBSD__
#  define MAIKO_OS_OPENBSD 1
#  define MAIKO_OS_NAME "OpenBSD"
#  define MAIKO_OS_BSD_LIKE 1
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

/* __SVR4: Defined by clang, gcc, and Sun Studio.
 * __SVR4__ was only defined by Sun Studio. */
#if defined(__sun) && defined(__SVR4)
#  define MAIKO_OS_SOLARIS 1
#  define MAIKO_OS_NAME "Solaris"
#  define MAIKO_OS_UNIX_LIKE 1
#  define MAIKO_OS_DETECTED 1
#endif

#if defined(_WIN32) || defined(__WINDOWS__)
#  define MAIKO_OS_WINDOWS 1
#  define MAIKO_OS_NAME "Windows"
#  define MAIKO_OS_DETECTED 1
#endif

/* __x86_64__: GNU C, __x86_64: Sun Studio, _M_AMD64: Visual Studio */
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64)
#  define MAIKO_ARCH_X86_64 1
#  define MAIKO_ARCH_NAME "x86_64"
#  define MAIKO_ARCH_WORD_BITS 64
#  define MAIKO_ARCH_DETECTED 1
#endif

/* __arm__: GNU C */
#ifdef __arm__
#  define MAIKO_ARCH_ARM 1
#  define MAIKO_ARCH_NAME "arm"
#  define MAIKO_ARCH_WORD_BITS 32
#  define MAIKO_ARCH_DETECTED 1
#endif

/* __aarch64__: GNU C */
#ifdef __aarch64__
#  define MAIKO_ARCH_ARM64 1
#  define MAIKO_ARCH_NAME "arm64"
#  define MAIKO_ARCH_WORD_BITS 64
#  define MAIKO_ARCH_DETECTED 1
#endif

/* __i386: GNU C, Sun Studio, _M_IX86: Visual Studio */
#if defined(__i386) || defined(_M_IX86)
#  define MAIKO_ARCH_X86 1
#  define MAIKO_ARCH_NAME "x86"
#  define MAIKO_ARCH_WORD_BITS 32
#  define MAIKO_ARCH_DETECTED 1
#endif

#if defined(__ppc__) || defined(__ppc64__)
#  define MAIKO_ARCH_POWERPC 1
#  define MAIKO_ARCH_NAME "PowerPC"
#  ifdef __ppc64__
#    define MAIKO_ARCH_WORD_BITS 64
#  else
#    define MAIKO_ARCH_WORD_BITS 32
#  endif
#  define MAIKO_ARCH_DETECTED 1
#endif

/* Documented at https://github.com/riscv/riscv-toolchain-conventions */
#ifdef __riscv
#  define MAIKO_ARCH_RISCV 1
#  define MAIKO_ARCH_NAME "RISC-V"
#  define MAIKO_ARCH_WORD_BITS __riscv_xlen
#  define MAIKO_ARCH_DETECTED 1
#endif

/* __sparc__: GNU C, __sparc: Sun Studio */
#if defined(__sparc__) || defined(__sparc)
#  define MAIKO_ARCH_SPARC 1
#  define MAIKO_ARCH_NAME "SPARC"
#  ifdef __sparcv9
#    define MAIKO_ARCH_WORD_BITS 64
#  else
#    define MAIKO_ARCH_WORD_BITS 32
#  endif
#  define MAIKO_ARCH_DETECTED 1
#endif

#ifndef MAIKO_OS_DETECTED
#  error "Could not detect OS."
#endif

#ifndef MAIKO_ARCH_DETECTED
#  error "Could not detect system architecture."
#endif

#endif /* MAIKO_PLATFORM_H */
