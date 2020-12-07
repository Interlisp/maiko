/* $Id: keymaker.c,v 1.3 1999/05/31 23:35:35 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static const char *id = "$Id: keymaker.c,v 1.3 1999/05/31 23:35:35 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/* =====================================================================
        This function is used to generate copyright protection keys for
        Venue's Medley software.  It prompts for a machine's host id and
        the software's expiration date before generating a set of 3 keys.

        The external functions called were stored in file 'keylib.o'.

        Creation Date: May, 1988
   ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "keylibdefs.h"

#define GOLDEN_RATIO_HACK -478700649
#define floadbyte(number, pos) ((number >> pos) & 0xFFFF)
#define hash_unhash(number, hashkey) \
  (number ^ (GOLDEN_RATIO_HACK * (floadbyte(hashkey, 16) + floadbyte(hashkey, 0))))

/*   meaning of symbolic constants used:

                FAILURE1	invalid hostid
                FAILURE2	invalid date entered from terminal
                FAILURE3	can't open logfile in the command line */

#define FAILURE1 -1
#define FAILURE2 -2
#define FAILURE3 -3

void writeresults(FILE *fp, char *host, char *expdate,
		  unsigned long key1, unsigned long key2, unsigned long key3,
		  char *info);
/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int main(int argc, char **argv) {
  int logfile = 0; /* set to 1 if logfile on command line */
  FILE *fp;        /* file pointer for the logfile */
  unsigned long hostid;
  unsigned long keys[3];
  char s[50], expdate[30], saveexpdate[30];
  char infostring[500];
  char *sptr;
  char *eptr;
  int i, c;
  int commandlineargs;

  commandlineargs = (argc > 2);
  /* If we have more than one argv we assume this is used as a filter */

  /* == check if user enters logfile name in the command line == */

  if ((argc > 1) & !commandlineargs) {
    if ((fp = fopen(*++argv, "a")) == NULL) {
      printf("\n Can't open %s \n", *argv);
      exit(FAILURE3);
    };
    logfile = 1;
  };

  /* == prompt for machine's host id, verify, then do some modification == */

  if (commandlineargs) {
    /* assume that the second argument is hex-hostid */
    sptr = *++argv;
    hostid = strtoul(sptr, &eptr, 16);
  } else {
    printf("Enter Host ID (starts with 0x if the number is hexadecimal): ");
    sptr = fgets(s, sizeof(s), stdin);
    hostid = strtoul(s, &eptr, 0);

    /* look for syntax error */
    if (*eptr != '\n') {
      printf("\nInvalid Host ID\n");
      exit(FAILURE1);
    } else {
      /* trim off the trailing newline */
      *eptr = '\0';
    }

    /* make sure Host ID is less than 32 bits */
    /* XXX: why?, is 32-bits not OK -- compare to original code */
    if ((hostid & 0x7FFFFFFF) != hostid) {
      printf("\nInvalid Host ID\n");
      exit(FAILURE1);
    }
  }

  hostid = modify(hostid);

  /* == prompt for the expiration date and validate it == */

  if (!commandlineargs) {
    /* assume that info is not needed when we use argc,argv */
    printf("Enter information string (one line only, below)\n:");
    fgets(infostring, sizeof(infostring), stdin);
    infostring[strlen(infostring) - 1] = '\0';
  }
  /* == prompt for the expiration date and validate it == */

  if (commandlineargs) {
    strcpy(expdate, *++argv);
  } else {
    printf("Enter Software Expiration Date (dd-mmm-yy or never): ");
    fgets(expdate, sizeof(expdate), stdin);
    expdate[strlen(expdate) - 1] = '\0';
  }
  strcpy(saveexpdate, expdate);
  /* check for 'never' entry */
  if ((expdate[0] == 'N') || (expdate[0] == 'n')) {
    for (i = 0; (c = expdate[i]) != '\0'; ++i)
      if (islower(c)) expdate[i] = toupper(c);

    if (strcmp(expdate, "NEVER") == 0)
      strcpy(expdate, "29-DEC-77");
    else {
      printf("\nInvalid Software Expiration Date\n");
      exit(FAILURE2);
    };
  };

  /* validate the date */
  if (idate(expdate) == FAILURE2) {
    printf("\nInvalid Software Expiration Date\n");
    exit(FAILURE2);
  };

  /* == generate 3 keys == */
  keys[0] = hash_unhash(hostid, hostid);
  keys[1] = hash_unhash(((date_integer16(expdate) << 16) | 0), hostid);
  keys[2] = make_verification(keys[0], keys[1]);

  /* == report the results == */

  if (commandlineargs) {
    printf("%8lx %8lx %8lx\n", keys[0], keys[1], keys[2]);
    exit(1);
  } else {
    /* if logfile is open, append the results to the end of the file */
    if (logfile == 1) {
      if (argc > 2) {
        fprintf(fp, "\n%s", *++argv);
        printf("\n%s", *argv);
      }
      writeresults(fp, sptr, saveexpdate, keys[0], keys[1], keys[2], infostring);
      fclose(fp);
    };
    /* display the results on the terminal */
    writeresults(stdout, sptr, saveexpdate, keys[0], keys[1], keys[2], infostring);
  };
  exit(0);
}

/************************************************************************/
/*									*/
/*			w r i t e r e s u l t s				*/
/*									*/
/*	Prints the newly-generated key, along with identifying info	*/
/*									*/
/************************************************************************/

void writeresults(FILE *fp, char *host, char *expdate,
		  unsigned long key1, unsigned long key2, unsigned long key3,
		  char *info) {
  fprintf(fp, "Host ID: %-14s Expiration: %-9s", host, expdate);
  fprintf(fp, " Key: %8lx %8lx %8lx", key1, key2, key3);
  fprintf(fp, " Doc: %s\n", info);
}
