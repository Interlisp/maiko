/* dld.h -- dynamic link editor library interface header.
   Copyright (C) 1996 Free Software Foundation, Inc.
   Copyright (C) 1990 by W. Wilson Ho.
   This file is part of the GNU Dld Library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Written by W. Wilson Ho <how@sgi.com> */

#include <ansidecl.h>

/* Error codes */

#define DLD_ENOFILE	1	/* Cannot open file. */
#define DLD_EBADMAGIC	2	/* Bad magic number. */
#define DLD_EBADHEADER	3	/* Failure reading header. */
#define DLD_ENOTEXT	4	/* Premature end of file in text section. */
#define DLD_ENOSYMBOLS	5	/* Premature EOF in symbol section. */
#define DLD_ENOSTRINGS	6	/* Bad string table. */
#define DLD_ENOTXTRELOC	7	/* Premature EOF in text relocation. */
#define DLD_ENODATA	8	/* Premature EOF in data section. */
#define DLD_ENODATRELOC	9	/* Premature EOF in data relocation. */
#define DLD_EMULTDEFS	10	/* A symbol was multiply defined. */
#define DLD_EBADLIBRARY	11	/* Malformed library archive. */
#define DLD_EBADCOMMON	12	/* Common block not supported. */
#define DLD_EBADOBJECT	13	/* Malformed input file
				   (not relocatable or archive). */
#define DLD_EBADRELOC	14	/* Bad relocation info. */
#define DLD_ENOMEMORY	15	/* Virtual memory exhausted. */
#define DLD_EUNDEFSYM	16	/* Undefined symbol. */

#define DLD_ELAST	16	/* Must equal largest errno. */

/* Most recent error code returned by dld. */
extern int dld_errno;

/* The symbol name that was multiply defined.  Valid only if DLD_ERRNO is 
   DLD_EMULTDEFS. */
extern char *dld_errname;

/* The number of undefined global symbols. */
extern int dld_undefined_sym_count;

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the dld routines. PROGNAME is the name of the currently
   running program, as given by argv[0].  If PROGNAME is not an
   absolute path, then dld_init will automatically invoke
   dld_find_program (PROGNAME). */
extern int dld_init PARAMS ((CONST char *progname));

/* Dynamically load and link the object file named FILE into the
   current process. */
extern int dld_link PARAMS ((CONST char *file));

/* Return the address of the identifier named ID.  This function
   automatically prepends an underscore to the identifier, if C
   identifiers usually use them. */
extern PTR dld_get_symbol PARAMS ((CONST char *id));

/* Return the address of the identifier named ID.  Use this function
   to locate symbols defined by assembly routines, since it doesn't
   prepend and underscore to the symbol name. */
extern PTR dld_get_bare_symbol PARAMS ((CONST char *id));

/* Return the address of the function named FUNC. */
extern PTR dld_get_func PARAMS ((CONST char *func));

/* Unlink the module in the current process defined by the file FILE.
   If FORCE is nonzero, then the module is removed from memory even if
   it is referenced by other modules. */
extern int dld_unlink_by_file PARAMS ((CONST char *file, int force));

/* Unlink the module in the current process that defines the symbol
   ID.  If FORCE is nonzero, then the module is removed from memory
   even if it is referenced by other modules. */
extern int dld_unlink_by_symbol PARAMS ((CONST char *id, int force));

/* Explicitly create a reference to the symbol named ID. */
extern int dld_create_reference PARAMS ((CONST char *id));

/* Explicitly define the symbol named ID, of SIZE bytes. */
extern int dld_define_sym PARAMS ((CONST char *id, int size));

/* Remove the explicitly defined symbol named ID. */
extern int dld_remove_defined_symbol PARAMS ((CONST char *id));

/* Return an array that lists the undefined symbols in the current
   process, or NULL if there are none.  The caller is responsible for
   freeing this array. */
extern char **dld_list_undefined_sym PARAMS ((NOARGS));

/* Return true (nonzero) if the function named FUNC may be safely
   executed. */
extern int dld_function_executable_p PARAMS ((CONST char *func));

/* Return the absolute name of the program named NAME.  This function
   searches the directories in the PATH environment variable if PROG
   has no directory components. */
extern char *dld_find_executable PARAMS ((CONST char *name));

/* Return the absolute name of the shared library named NAME.  This
   function searches:
     a) in an absolute location, if NAME is an absolute path.
     b) relative to the current working directory, if NAME is a relative path.
     c) in the current directory
     d) if all the above fail, then look for a file named "libNAME.so"
        (depending on the system):
       i)  in the directories named in the LIBPATH array, if it is non-NULL
       ii) in the directories named in the LD_LIBRARY_PATH (or equivalent)
           environment variable.

   If all the above fails, then dld_find_library returns the NULL pointer.

   See the dld documentation for a description of MAJOR, DELTA, and
   REVISION version numbers, or set them all to zero if the shared library
   was not built by libtool. */
extern char *dld_find_library PARAMS ((CONST char *name, CONST char **libpath,
				      int major, int delta, int revision));

/* Print MSG followed by a colon and dld_strerror (DLD_ERRNO).  May be
   used as a simple way to report errors. */
extern void dld_perror PARAMS ((CONST char *MSG));

/* Return the string corresponding to the error code CODE. */
extern CONST char *dld_strerror PARAMS ((int code));

#if 0
/* FIXME: these functions have been removed from dld due to legal problems. */

/* A replacement for dld_link that invokes g++ constructors. */
extern void dyn_load PARAMS ((CONST char *name));

/* A replacement for dld_unlink that invoke g++ destructors. */
extern void dyn_unload PARAMS ((CONST char *name));

/* A list of libraries to be loaded by dyn_load. */
extern char *dyn_libraries[];
#endif

#ifdef __cplusplus
}
#endif
