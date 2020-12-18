/* $Id: keylib.c,v 1.4 2001/12/24 01:09:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1999 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/* ==============================================================================
        The functions defined in this file were called by KEYMAKER.C & KEYTESTER.C

        Creation Date:  May, 1988
   ============================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "keylibdefs.h"

#define GOLDEN_RATIO_HACK -478700649
#define ULONG_MAX 4294967295

/*   meaning of symbolic constant used:

        FAILURE2	invalid date  */

#define FAILURE2 -2

/* ===========================================================================
        IMOD64BIT computs (IMOD X Y).
        X is a 64-bit integer; x1 is the higher 32-bit while x0 is the lower 32-bit.
        Y is  less than 65535.
        (((ULONG_MAX % y) + 1 ) % y) is equivalent to (expt 2 32) % y
 =============================================================================*/

int imod64bit(unsigned long x1, unsigned long x0, unsigned long y) {
  /* JDS 990601 ansi    return (((x0 % y) + ((x1 % y) * (((ULONG_MAX % y) + 1 ) % y)  )) % y); */
  return (((x0 % y) + ((x1 % y) * ((y + 1) % y))) % y);
}

/* ===========================================================================
        MAKE_VERIFICATION forms a new 32-bit integer by messaging the two input
        long integers x and y.  The number was created from the lower bits toward
        higher bits.
 =============================================================================*/

unsigned long make_verification(long unsigned int x, long unsigned int y) {
  return (imod64bit(x, y, 4) | (imod64bit(x, y, 6) << 3) | (imod64bit(x, y, 13) << 6) |
          (imod64bit(x, y, 25) << 10) | (imod64bit(x, y, 30) << 15) | (imod64bit(x, y, 63) << 20) |
          (imod64bit(x, y, 5) << 26) | (imod64bit(x, y, 7) << 29));
}

/* ============================================================
        DATE_INTEGER16 takes a date string  and return an intger
   =============================================================*/

unsigned long date_integer16(const char *date) {
  unsigned long const1, const2;

  const1 = 17152;
  const2 = 86400;
  return ((idate(date) - const1) / const2);
}

/* ===================================================================
        IDATE takes a date string (dd-mmm-yy or dd mmm yy) and return an
        integer.
                        dd -  date in decimal digits
                        mmm - month name in alphabetic form or decimal digits
                        yy  - year in decimal digits ; can be 2 or 4 digits
        If date1 is before date2 then (idate date1) < (idate date2).
        If date string is not valid, FAILURE2 is returned.
  =====================================================================*/

unsigned long idate(const char *str) {
  char ds[50];
  char *decdigits = {"0123456789"};
  char *tokendelimiters = {" -"};
  char *yptr, *mptr, *dptr, *ptr1;

  int day, month, year, maxday;
  int i, len, s;
  unsigned long days_since_day0, sec, hour;

  static char *month_name[13] = {"",    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                 "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

  static int days_each_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  static int days_since_jan1st[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  /* copy input string to a local one */
  ds[0] = '\0';
  strcpy(ds, str);

  /* get day, month, and year string tokens */
  if ((dptr = strtok(ds, tokendelimiters)) == NULL) return FAILURE2;
  if ((mptr = strtok(NULL, tokendelimiters)) == NULL) return FAILURE2;
  if ((yptr = strtok(NULL, tokendelimiters)) == NULL) return FAILURE2;

  /* validate year and convert it to numeric form 19xx */
  if (strspn(yptr, decdigits) != strlen(yptr)) return FAILURE2;
  if ((year = atoi(yptr)) <= 100) year += 1900;
  if (year < 1970 || year > 2050) return FAILURE2;

  /* validate month and convert it to numeric form if necessary */
  if (strspn(mptr, decdigits) == (len = (strlen(mptr)))) {
    /* input month is in numeric form */
    if ((month = atoi(mptr)) < 1 || month > 12) return FAILURE2;
  } else {
    /* input month is in alphabetic  form */

    /* we're only interested in the first 3 chars */
    if (len > 3) *(mptr + 3) = '\0';

    ptr1 = mptr;

    /* make sure month name is in upper case letters */
    for (i = 0; (s = *(ptr1 + i)) != '\0'; ++i)
      if (islower(s)) *(ptr1 + i) = toupper(s);

    /* find out its corresponding numeric value */
    for (i = 1; i <= 12; ++i)
      if (strcmp(*(month_name + i), mptr) == 0) {
        month = i;
        break;
      };
    if (i == 13) return FAILURE2;
  };

  /* set up maxday to be the number of days of that month */
  maxday = days_each_month[month];

  /* watch out for leap year */
  if (month == 2 && (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))) ++maxday;

  /* validate day */
  if (strspn(dptr, decdigits) != strlen(dptr) || (day = atoi(dptr)) < 1 || maxday < day)
    return FAILURE2;

  year -= 1901;
  --month;
  days_since_day0 = days_since_jan1st[month] + (day - 1) + (365 * year) + (year / 4);

  if (month > 1 && year % 4 == 3 && year != -1) ++days_since_day0;

  hour = (24 * days_since_day0) + 7;
  /* JDS 990601 ANSI cleanup	sec = (60 * (60 * hour)) ^ -2147483648L ; */
  sec = (60 * (60 * hour)) ^ 0x80000000;
  /* printf("\n days_since_day0: %ld  hour: %ld  sec: %ld\n", days_since_day0 , hour ,sec); */
  return sec;
}

/* ============================================================
        MODIFY  turns on/off certain bits of the passed
        in 32-bit hostid, depending on the value of (hostid % 16).
  =============================================================*/

unsigned long modify(unsigned long hostid) {
  unsigned long dividor;

#ifdef xBIGVM
  hostid ^= 0xa3f5069e;
#endif /* xBIGVM */

  dividor = 16;
  switch (hostid % dividor) {
    case 0: hostid = hostid | 0x48004802; break;
    case 1: hostid = hostid ^ 0xfedc5793; break;
    case 2: hostid = hostid ^ 0x9981afe3; break;
    case 3: hostid = hostid & 0xffcb6ffd; break;
    case 4: hostid = hostid ^ 0x3489dfa8; break;
    case 5: hostid = hostid | 0x77012877; break;
    case 6: hostid = hostid & 0xefc9fffd; break;
    case 7: hostid = hostid | 0x22311300; break;
    case 8: hostid = hostid & 0xdadf8871; break;
    case 9: hostid = hostid ^ 0xe8c948ad; break;
    case 10: hostid = hostid | 0x12488412; break;
    case 11: hostid = hostid & 0xeffdea87; break;
    case 12: hostid = hostid ^ 0xad8ce639; break;
    case 13: hostid = hostid | 0x88221144; break;
    case 14: hostid = hostid & 0xfffb6edd; break;
    default: hostid = hostid ^ 0xfffddeee; break;
  };
  /*  printf("\n ** new host id:  result: %x  ", hostid); */
  return hostid;
}
