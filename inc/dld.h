#ifndef DLD_H
#define DLD_H 1
/* $Id: dld.h,v 1.2 1999/01/03 02:05:57 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/* Error codes */

#define DLD_ENOFILE	1	    /* cannot open file */
#define DLD_EBADMAGIC	2	    /* bad magic number */
#define DLD_EBADHEADER	3	    /* failure reading header */
#define DLD_ENOTEXT	4	    /* premature eof in text section */
#define DLD_ENOSYMBOLS	5	    /* premature end of file in symbols */
#define DLD_ENOSTRINGS	6	    /* bad string table */
#define DLD_ENOTXTRELOC	7	    /* premature eof in text relocation */
#define DLD_ENODATA	8	    /* premature eof in data section */
#define DLD_ENODATRELOC	9	    /* premature eof in data relocation */
#define DLD_EMULTDEFS	10	    /* multiple definitions of symbol */
#define DLD_EBADLIBRARY	11	    /* malformed library archive */
#define DLD_EBADCOMMON	12	    /* common block not supported */
#define DLD_EBADOBJECT	13	    /* malformed input file (not rel or
				       archive) */
#define DLD_EBADRELOC	14	    /* bad relocation info */
#define DLD_ENOMEMORY	15	    /* virtual memory exhausted */
#define DLD_EUNDEFSYM	16	    /* undefined symbol */

extern int dld_errno;		    /* error code returned by dld */
extern int dld_undefined_sym_count; /* # of undefined global symbols */

extern int dld_init ();		    /* initialize the dld routines */
extern int dld_link ();		    /* dynamically link and load an object
				       file */
extern unsigned long
dld_get_symbol ();		    /* return the address of the named
				       identifier  */
extern unsigned long
dld_get_func ();		    /* return the address of the named
				       function */
extern unsigned long
dld_get_bare_symbol ();		    /* same as dld_get_symbol except that
				       no underscore (_) is prepended.  Use
				       to locate symbols defined by
				       assembly routines. */
extern int dld_unlink_by_file ();   /* unlink a file */
extern int dld_unlink_by_symbol (); /* unlink the module that define the
				       given symbol */

extern int
dld_function_executable_p ();	    /* return true if the named C function
				       is executable */

extern char **
dld_list_undefined_sym ();	    /* return an array of undefined symbols */

extern char *
dld_find_executable ();		    /* return the full path name of the
				       given executable file. */

extern int
dld_create_reference ();	    /* explicitly create a reference to the
				       given symbol. */

extern int dld_define_sym ();	    /* explicitly define the value for the
				       given symbol. */

extern void
dld_remove_defined_symbol ();	    /* remove a explicitly defined symbol */

extern void dld_perror ();	    /* print error messages. */
#endif /* DLD_H */
