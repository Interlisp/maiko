# Maiko

Maiko is the implementation of the Medley Interlisp virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting with Lisp for access to display (via X11) and disk etc.

For an overview, see [Medley Interlisp Introduction](https://interlisp.org/medley/using/docs/medley/).

See [the Medley repository](https://github.com/Interlisp/medley) for
* [Issues](https://github.com/Interlisp/medley/issues) (note that maiko issues are there too)
* [Discussions](https://github.com/Interlisp/medley/discussions) (Q&A, announcements, etc)
* [Medley's README](https://github.com/Interlisp/medley/blob/master/README.md)

Bug reports, feature requests, fixes and improvements, support for additional platforms and hardware are all welcome.

## Development Platforms

We are developing on FreeBSD, Linux, macOS, and Solaris currently
on arm7l, arm64, PowerPC, SPARC, i386, and x86_64 hardware.


## Building Maiko

Building requires `clang`, `make`, X11 client libraries (`libx11-dev`). For example, 

``` sh
$ sudo apt update
$ sudo apt install clang make libx11-dev
```

```
$ cd maiko/bin
$ ./makeright x
```

* The build will (attempt to) detect the OS-type and cpu-type. It will build binaries `lde` and `ldex` in `../ostype.cputype` (with .o files in `..ostype.cputype-x`. For example, Linux on a 64-bit x86 will use `linux.x86_64`, while macOS 11 on a (new M1) Mac will use `darwin.aarch64`.
* If you prefer using `gcc` over `clang`, you will need to edit the makefile fragment for your configuration (`makefile-ostype.cputype-x`) and comment out the line (with a #) that defines `CC` for `clang` and uncomment the line (delete the #) for the line that defines `CC` for `gcc`.
* There is a cmake configuration (TBD To Be Described here).

### Building For macOS

* Running on macOS requires an X server, and building on a Mac requires X client libraries.
An X-server for macOS (and X11 client libraries) can be freely obtained at https://www.xquartz.org/releases

### Building for Windows 10

Windows 10 currently requires "Docker for Desktop" or WSL2 and a (Windows X-server).
See [Medley's README](https://github.com/Interlisp/medley/blob/master/README.md) for more.

