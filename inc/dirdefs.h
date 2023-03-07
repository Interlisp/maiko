#ifndef DIRDEFS_H
#define DIRDEFS_H 1

#include <sys/types.h>      // for u_short, ino_t
#include "lispemul.h"       // for LispPTR
#include "locfile.h"        // for MAXNAMLEN
/*
 * FINFO and FPROP are used to store the information of the enumerated files
 * and directories.  They are arranged in a form of linked list.  Each list is
 * corresponding to the each directory enumeration.  All of the informations
 * Lisp needs are stored in the list.  This list is in the emulator's address space
 * and can be specified by "ID" which is the interface between the emulator and Lisp
 * code.  In this implementation, ID is represented as an integer and is actually
 * an index of the array of the lists.
 *
 * To avoid the overhead of the FINFO and FPROP structure dynamic allocation and
 * deallocation, some number of their instances are pre-allocated when the emulator
 * starts and managed in a free list.  If all of the pre-allocated instances are in
 * use, new instances are allocated.  The new instances are linked to the free list
 * when it is freed.
 *
 * As described above, the linked list result of the enumeration is stored in a
 * array for the subsequent request from Lisp.  Lisp code requests the emulator to
 * release the list when it enumerated all of the entries in the list or the
 * enumerating operation is aborted.
 */

typedef struct fprop {
  unsigned length;   /* Byte length of this file. */
  unsigned wdate;    /* Written (Creation) date in Lisp sense. */
  unsigned rdate;    /* Read date in Lisp sense. */
  unsigned protect;  /* Protect mode of this file. */
  size_t au_len;     /* Byte length of author. */
  char author[256];  /* Author in Lisp sense. */
} FPROP;

/* This structure has a pointer at each end to force alignment to
   be correct when a pointer is 8 bytes long. */
typedef struct finfo {
  FPROP *prop;           /* File properties Lisp needs. */
  char lname[MAXNAMLEN + 1]; /* Name in Lisp Format. */
  char no_ver_name[MAXNAMLEN + 1]; 
  /*
   * Name in UNIX Format.  Does not
   * include Version field.
   * All lower case.
   */
  size_t lname_len;     /* Byte length of lname. */
  unsigned dirp;       /* If 1, this file is a directory. */
  unsigned version;   /* Version in Lisp sense. */
  ino_t ino;          /* I-node number of this file. */
  struct finfo *next; /* Last entry is indicated by NULL pointer. */
} FINFO;

typedef struct dfinfo {
  FINFO *head; /* Head of the linked FINFO structures. */
  FINFO *next; /* FINFO structure generated next time. */
} DFINFO;

#ifdef DOS
int make_old_version(char *old, char *file);
#endif
#ifdef FSDEBUG
void print_finfo(FINFO *fp);
#endif
int init_finfo(void);
LispPTR COM_gen_files(LispPTR *args);
LispPTR COM_next_file(LispPTR *args);
LispPTR COM_finish_finfo(LispPTR *args);
#endif
