# Maiko

This is the emulator for the Interlisp-D virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting witih Lisp for access to display and disk etc.

There’s a complex system of make files that include all the flags and
variables you have to set for each hardware/OS target.

You need to be in the "bin" directory, you need to have "." on your
path, you need to do "./makeright x" in order for it to detect the
correct OS and then put together the makefile parts that it needs.  It
will build it in ../<osname>.<cputype>-x (for the .o files) and
../<osname>.<cputype> for the executables.



