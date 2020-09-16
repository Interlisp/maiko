# Maiko

This is the implementation of the Medley Interlisp virtual machine, for a
byte-coded Lisp instruction set and some low-level functions for
connecting witih Lisp for access to display and disk etc.

There are make file fragments that include all the flags and
variables you have to set for each hardware/OS target.

- cd to the "bin" directory
- have "." on your PATH
- do "./makeright x" 

It will (attempt to) detect the OS-type and cpu-type, and put
together the makefile parts that it needs. 
It will build in ../ostype.cputype-x (for the .o files) and
../ostype.cputype for the executables.



