# Maiko

This is the implementation of the Medley Interlisp virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting with Lisp for access to display (via X11) and disk etc.
See:
* [the Medley repository](https://github.org/Interlisp/medley) for
[Issues](https://github.com/Interlisp/medley/issues), [Discussions](https://github.com/Interlisp/medley/discussions), documents and more context.
* [Building](https://github.com/Interlisp/medley/wiki/Building-Medley-Interlisp) 
* [Running](https://github.com/Interlisp/medley/wiki/Running-Medley-Interlisp)
* [Using](https://github.com/Interlisp/medley/wiki/Using-Medley-Interlisp)


More on what's here:

There are make file fragments that include all the flags and
variables you have to set for each hardware/OS target.

The [build](https://github.com/Interlisp/medley/wiki/Building-Medley-Interlisp) will (attempt to) detect the OS-type and cpu-type, and put together the makefile parts that it needs. 
It will build in ../ostype.cputype-x (for the .o files) and
../ostype.cputype for the executables.

## Development Platforms

We are developing on FreeBSD, Linux, macOS, and Solaris currently
on arm7l, arm64, PowerPC, SPARC, x86, and x86_64 hardware.

Windows 10 currently requires "Docker for Desktop" or WSL2 (See [Running on Windows10](https://github.com/Interlisp/medley/wiki/Running-Medley-Interlisp-on-Windows10)
* [Running Medley Interlisp](https://github.com/Interlisp/medley/wiki/Running-Medley-Interlisp)
* [Using Medley Interlisp](https://github.com/Interlisp/medley/wiki/Using-Medley-Interlisp)

Bug reports, feature requests, fixes and improvements, support for additional platforms and hardware are all welcome.



