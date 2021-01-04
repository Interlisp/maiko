# Maiko

This is the implementation of the Medley Interlisp virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting with Lisp for access to display and disk etc.
See [the main Medley repository](https://github.org/Interlisp/medley) for
Issues, Discussions, documents and more context.)

There are make file fragments that include all the flags and
variables you have to set for each hardware/OS target.

- cd to the "bin" directory
- do `./makeright x`

It will (attempt to) detect the OS-type and cpu-type, and put
together the makefile parts that it needs. 
It will build in ../ostype.cputype-x (for the .o files) and
../ostype.cputype for the executables.

## Development Platforms

We are developing on FreeBSD, Linux, macOS, and Solaris currently
on arm, arm64, PowerPC, SPARC, x86, and x86_64 hardware.

We believe it will work on these platforms.

Fixes and improvements for additional platforms and hardware is
welcome. Work is underway to run better on Windows.

In the past, Maiko ran on DOS. This may or may not still work.
