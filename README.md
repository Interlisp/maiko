# Maiko

Maiko is the implementation of the Medley Interlisp virtual machine for a
byte-coded Lisp instruction set, and some low-level functions for
connecting Lisp to a display (via X11 or SDL), the local filesystem,
and a network subsystem.

For an overview, see [Medley Interlisp Introduction](https://interlisp.org/medley/using/docs/medley/).

See [the Medley repository](https://github.com/Interlisp/medley) for
* [Issues](https://github.com/Interlisp/medley/issues) (note that maiko issues are there too)
* [Discussions](https://github.com/Interlisp/medley/discussions) (Q&A, announcements, etc)
* [Medley's README](https://github.com/Interlisp/medley/blob/master/README.md)

Bug reports, feature requests, fixes and improvements, support for additional platforms and hardware are all welcome.

## Development Platforms

Development has been primarily on macOS, FreeBSD, and Linux, with testing on Solaris and Windows.
Processor architectures i386, x86\_64, arm64, arm7l, and SPARC.


## Building Maiko

### Building with make
Building requires a C compiler (`clang` preferred), either `make` or `CMake`, and X11 client libraries (`libx11-dev`), or SDL2. For example, using `make` and X11:

``` sh
$ sudo apt update
$ sudo apt install clang make libx11-dev
```

```
$ cd maiko/bin
$ ./makeright x
```

* The build will (attempt to) detect the OS-type and cpu-type. It will build binaries `lde` and `ldex` in `../`_`ostype.cputype`_ (with .o files in `../`_`ostype.cputype-x`_. For example, Linux on a 64-bit x86 will use `linux.x86_64`, while macOS 11 on a (new M1) Mac will use `darwin.aarch64`.
* If you prefer `gcc` over `clang`, you will need to edit the makefile fragment for your configuration (`makefile-ostype.cputype-x`) and comment out the line (with a #) that defines `CC` as `clang` and uncomment the line (delete the #) for the line that defines `CC` as `gcc`.

### Building with CMake
We provide a `CMakeLists.txt` which provides mostly matching build capabilities to the `make` setup.
CMake options are provided to control the configuration of the Maiko executables:
* MAIKO\_DISPLAY\_SDL: [OFF], 2, 3 - selects display subsystem SDL of version specified
* MAIKO\_DISPLAY\_X11: [ON], OFF - selects X11 display subsystem
* MAIKO\_NETWORK\_TYPE: [NONE], SUN\_DLPI, SUN\_NIT, NETHUB - network subsystem access
* MAIKO_RELEASE: [351], various - see `maiko/inc/version.h`

While SDL3 is selectable, the Maiko code has not yet been updated to work with the SDL3 API.

### Building For macOS

* Building/running on macOS requires either an X server and X client libraries or the SDL2 library.
An X-server for macOS (and X11 client libraries) can be freely obtained at https://www.xquartz.org/releases
The SDL library is freely available from https://libsdl.org
