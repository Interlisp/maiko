/* $Id: keytst.c,v 1.3 1999/05/31 23:35:36 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/* =========================================================================
        The functions defined in this file are used to validate the copyright
        protection keys for NewCo's Maiko software. The main function is
        'keytester', which takes an input key string and returns a status
        code after processing the keys.

        The external functions called were stored in file 'keylib.o'.

        Creation date: May, 1988
  ====================================================================== */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "keylibdefs.h"

#define GOLDEN_RATIO_HACK -478700649
#define floadbyte(number, pos) ((number >> pos) & 0xFFFF)
#define hash_unhash(number, hashkey) \
  (number ^ (GOLDEN_RATIO_HACK * (floadbyte(hashkey, 16) + floadbyte(hashkey, 0))))
#define KEYNUMBERS 3
#define RC0 0

/*   meaning of symbolic constants used:

                FAILURE2	invalid date
                FAILURE3	invalid key
                FAILURE4	key expired
                FAILURE99	invalid date  (this shouldn't happen unless string format returned
   by ctime got changed) */

#define FAILURE2 -2
#define FAILURE3 -3
#define FAILURE4 -4
#define FAILURE99 -99

/* =====================================================================
        KEYTESTER checks the input key string.
     It returns 0 if the keys are valid, otherwise it returns non-0.
  ====================================================================== */
int keytester(char *keystring) {
  unsigned long keyarray[KEYNUMBERS]; /* array which holds numeric keys */
  unsigned long hostid;               /* 32-bit unique identifier of the current host  */
  unsigned long hashedword;
  int rc; /* return code */

  /* check the keys and convert them from hexdecimal strings to numbers  */
  if (keystring == NULL) return FAILURE3;
  if (read_hex(keystring, keyarray) == FAILURE3) return FAILURE3;

/*  get machines host id  */
  hostid = gethostid();
  printf("hostid = 0x%x\n", hostid);

  hostid = modify(hostid);

  /*  generate hashword */
  hashedword = hash_unhash(keyarray[1], hostid);

  /*  validate keys	*/
  if (keyarray[0] != hash_unhash(hostid, hostid)) return FAILURE3;
  if ((rc = ok_date(floadbyte(hashedword, 16))) != RC0) return rc;
  if (keyarray[2] != make_verification(keyarray[0], keyarray[1])) return FAILURE3;

  return 0;
}

/* =====================================================================
        READ_HEX reads in keys from the input string , validates them, then
     stores them in the input array.
  ====================================================================== */

int read_hex(char *s1, long unsigned int *array) {
  char *s2 = {" "};
  char *ptr;
  char *hexdigits = {"0123456789abcdefABCDEF"};
  int i;

  for (i = 0; (i < KEYNUMBERS) && ((ptr = strtok(s1, s2)) != NULL); ++i) {
    /* make sure the key contains only hexadecimal characters */
    if ((strspn(ptr, hexdigits)) != strlen(ptr)) return FAILURE3;

/* convert key to numeric format*/
    *(array + i) = strtol(ptr, NULL, 16); /* On suns, this works OK */
    printf("0x%x ", *(array + i));
    fflush(stdout);
    /* continue search the next one */
    s1 = NULL;
  };

  if (i == KEYNUMBERS)
    return RC0;
  else
    return FAILURE3;
}

/* ============================================
        OK_DATE checks the expiration of the key
   ============================================ */

int ok_date(long unsigned int date) {
  char current_date[30];
  char *mptr, *dptr, *yptr, *str;
  long realtime, *clock;

  /* first check if the expiration date is set to indefinite */

  printf("\narg date: %ld  %x\n", date, date);
  printf("\narg date ?????: %ld  %x\n", date_integer16("29-DEC-77"), date_integer16("29-DEC-77"));

  if (date == date_integer16("29-DEC-77")) return RC0;

  /* next check if current date is less than or equal to the expiration date */
  /* get the current date  string */
  realtime = time(0);
  clock = &realtime;
  str = (char *)ctime(clock);

  /* delete day and time info and rearrange the string format to be dd-mmm-yy */
  mptr = strtok(str, " ");
  mptr = strtok(NULL, " ");
  dptr = strtok(NULL, " ");
  yptr = strtok(NULL, " ");
  yptr = strtok(NULL, " \n"); /* watch out for newline char */
  current_date[0] = '\0';
  strcat(current_date, dptr);
  strcat(current_date, "-");
  strcat(current_date, mptr);
  strcat(current_date, "-");
  strcat(current_date, yptr);

  /* check the date */
  if (idate(current_date) == FAILURE2) return FAILURE99;

  printf("*current date*: %ld %x\n", date_integer16(current_date), date_integer16(current_date));

  return (date_integer16(current_date) <= date) ? RC0 : FAILURE4;
}
