/* $Id: keymaker.c,v 1.3 1999/05/31 23:35:35 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: keymaker.c,v 1.3 1999/05/31 23:35:35 sybalsky Exp $ Copyright (C) Venue";


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"





/* =====================================================================
	This function is used to generate copyright protection keys for
	enue's Medley software.  It prompts for a machine's host id and
	the software's expiration date before generating a set of 3 keys.

	The external functions called were stored in file 'keylib.o'.

	Creation Date: May, 1988
   ===================================================================== */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define	GOLDEN_RATIO_HACK			-478700649
#define	floadbyte(number,pos)		((number >> pos) & 0xFFFF)
#define	hash_unhash(number,hashkey)	(number ^ (GOLDEN_RATIO_HACK * (floadbyte(hashkey,16) + floadbyte(hashkey,0)) ))
#define	KEYNUMBERS					3


/*   meaning of symbolic constants used:

		FAILURE1	invalid hostid
		FAILURE2	invalid date entered from terminal
		FAILURE3	can't open logfile in the command line */

#define	FAILURE1					-1
#define	FAILURE2					-2
#define	FAILURE3					-3


unsigned long make_verification(long unsigned int x, long unsigned int y);
unsigned long date_integer16(char *date);
unsigned long idate(char *str);
unsigned long modify(long unsigned int hostid);



/************************************************************************/
/*									*/
/*			w r i t e r e s u l t s				*/
/*									*/
/*	Prints the newly-generated key, along with identifying info	*/
/*									*/
/************************************************************************/

writeresults(FILE *fp, char *host, char *expdate, int key1, int key2, int key3, char *info)
{
    fprintf(fp, "Host ID: %-14s Expiration: %-9s",host, expdate);
    fprintf(fp, " Key: %8x %8x %8x", key1, key2, key3);
    fprintf(fp, " Doc: %s\n", info);
  }



/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

main(int argc, char **argv)
{
    int  logfile = 0;	/* set to 1 if logfile on command line */
    FILE *fp;		/* file pointer for the logfile */
    unsigned long hostid;
    long keyarray[KEYNUMBERS];
       char *hexdigits = {"-0123456789abcdefABCDEF"};
    char s[50], hstr[50], expdate[30], saveexpdate[30], cc;
    char infostring[500];
    char *sptr, *ptr, *digitstring;
    char *hptr = {" "};
    char **hhptr = &hptr;
    int  base = 10;
    int  i,j,c;
    int commandlineargs;

    commandlineargs = (argc > 2);
    /* If we have more than one argv we assume this is used as a filter */

   /* == check if user enters logfile name in the command line == */

    if ((argc > 1) & !commandlineargs)
      {
	if ((fp = fopen(*++argv,"a")) == NULL)
	  {
	    printf("\n Can't open %s \n", *argv);
	    exit(FAILURE3);
	  };
	logfile = 1;
      };

   /* == prompt for machine's host id, verify, then do some modification == */

    if (commandlineargs)
      {
	/* assume that the second argument is hex-hostid */
	sptr = *++argv;
	hostid = strtol(sptr,hhptr,16);

      }
    else
      {
	printf("\n\nEnter Host ID (starts with 0x if the number is hexidecimal): ");
	gets(s);
	sptr = strtok(s," ");

	    /* decide the base */
	if (((ptr = strchr(sptr,'0')) != NULL) &&
		(*(ptr+1) == 'x' || *(ptr+1) == 'X')) base = 16;
	    
#ifdef INDIGO
	hostid = strtoul(sptr,hhptr,base);
#elif defined(OS5)
	hostid = strtoul(sptr,hhptr,base);
#else
	hostid = strtol(sptr,hhptr,base);
#endif
	    
	    /* look for syntax error */
	if (**hhptr != '\0')
	  {
		printf("\nInvalid Host ID \n");
		exit(FAILURE1);
	  };

	    /* make sure Host ID  is less than 32 bits */
	if (base == 16)
	  {	sprintf(hstr, "%x", hostid);
		for (i = 0 ; (cc = *(sptr+i)) != '\0' ; ++i)
		  if (isupper(cc)) *(sptr+i) = tolower(cc);
		for (i = 0 ; (cc = *(hstr+i)) != '\0' ; ++i)
		  if (isupper(cc)) *(hstr+i) = tolower(cc);
		digitstring = "123456789abcdef";
	  }
	else
	  {    sprintf(hstr, "%u", hostid);
		   digitstring = "123456789";
	  };

	ptr = strpbrk(sptr,digitstring);
	if ((ptr == NULL && *sptr != '0' && *hstr != '0') ||
		(ptr != NULL && strcmp(ptr,hstr) != 0))
	      {
		printf("\nInvalid Host ID \n");
		exit(FAILURE1);
	      }
      };
    hostid = modify(hostid);

   /* == prompt for the expiration date and validate it == */

    if (!commandlineargs)
	  {
	    /* assume that info is not needed when we use argc,argv */
	    printf("\n\nEnter information string (one line only, below)\n:");
	    gets(infostring);
	  }
   /* == prompt for the expiration date and validate it == */

    if (commandlineargs)
	  {
	    
	    strcpy(expdate,*++argv);
	    
	  } else {
	    
	    printf("Enter Software Expiration Date (dd-mmm-yy or never): ");
	    gets(expdate);
	    
	  }
    strcpy(saveexpdate,expdate);
    /* check for 'never' entry */
    if ((expdate[0] == 'N') || (expdate[0] == 'n'))
	  {
	    for (i = 0 ; (c = expdate[i]) != '\0' ; ++i)
	      if (islower(c)) expdate[i] = toupper(c);
	    
	    if ( strcmp(expdate, "NEVER") == 0)
	      strcpy(expdate, "29-DEC-77");
	    else {
	      printf("\nInvalid Software Expiration Date\n");
	      exit(FAILURE2);
	    };
	  };
	
	/* validate the date */
    if ( idate(expdate) == FAILURE2)
	  {
	    printf("\nInvalid Software Expiration Date\n");
	    exit(FAILURE2);
	  };

 /* == generate 3 keys == */
    keyarray[0] = hash_unhash(hostid, hostid);
    keyarray[1] = hash_unhash( ((date_integer16(expdate) << 16) | 0), hostid);
    keyarray[2] = make_verification(keyarray[0], keyarray[1]);


  /* == report the results == */

    if (commandlineargs)
      {
	printf("%8x %8x %8x\n", *keyarray, *(keyarray+1), *(keyarray+2));
      }
    else
      {
	/* if logfile is open, append the results to the end of the file */
	if (logfile == 1)
	  {
	    if (argc > 2)
	      {
		fprintf(fp,"\n%s",*++argv);
		printf("\n%s",*argv);
	      }
	    writeresults( fp, sptr, saveexpdate,
			  *keyarray, *(keyarray+1), *(keyarray+2),
				infostring);
	    fclose(fp);
	  };
	writeresults( stdout, sptr, saveexpdate,
			*keyarray, *(keyarray+1), *(keyarray+2),
			infostring);
	/* display the results on the terminal */
      };
    exit(0);
}
