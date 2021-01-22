# Maiko

Maiko is the implementation of the Medley Interlisp virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting with Lisp for access to display (via X11) and disk etc.

Newcomers should check out the [Medley Interlisp Introduction](https://github.com/Interlisp/medley/wiki/Medley-Interlisp-Introduction).

See [the Medley repository](https://github.org/Interlisp/medley) for
* [Issues](https://github.com/Interlisp/medley/issues) (note that maiko issues are there too)
* [Discussions](https://github.com/Interlisp/medley/discussions) (Q&A, announcements, etc)
* [Medley's README](https://github.com/Interlisp/medley/blob/master/README.md)

Bug reports, feature requests, fixes and improvements, support for additional platforms and hardware are all welcome.

## ## Development Platforms

We are developing on FreeBSD, Linux, MacOS, and Solaris currently
on arm7l, arm64, PowerPC, SPARC, i386, and x86_64 hardware.


## Building Maiko
Building requires `clang`, `make`, X11 client libraries (`libx11-dev`). For example, 

``` sh
$ sudo apt update
$ sudo apt install clang make x11dev
```

```
$ cd maiko/bin
$ ./makeright x
```

* The build will (attempt to) detect the OS-type and cpu-type. It will build binaries `lde` and `ldex` in `../ostype.cputype` (with .o files in `..ostype.cputype-x`. For example, Linux on a 64-bit x86 will use `linux.x86_64`, while MacOS 11 on (new) Mac will use `darwin.arm64`.
* If you prefer using `gcc` over `clang`, you will need to edit the makefile fragment for your configuration (`makefile-ostype.cputype-x1) and comment out the line (with a #) that defines `CC` for `clang` and uncomment the line (delete the #) for the line that defines `CC` for `gcc`.
* There is a cmake configuration (TBD)

### Building For MacOS

* Running on MacOS requires an X server, and building on a Mac requires X client libraries.
An X-server for x86 MacOS (and X11 client libraries) can be freely obtained at https://www.xquartz.org/
For the new arm64 MacOS 11, you'll need https://x.org which you can get via MacPorts or Brew.

### Building for Windows 10

Windows 10 currently requires "Docker for Desktop" or WSL2 -- basically a Linux virtual machine -- and and a (Windows X-server).
See [Medley's README](https://github.com/Interlisp/medley/blob/master/README.md) for more.

