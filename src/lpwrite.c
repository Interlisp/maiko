/* $Id: lpwrite.c,v 1.2 1999/01/03 02:07:20 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: lpwrite.c,v 1.2 1999/01/03 02:07:20 sybalsky Exp $ Copyright (C) Venue";


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"


#include "lpdefs.h"
#include "lpglobl.h"
#ifdef OS4
#include <sys/types.h>
#include <varargs.h>
#else
#include <stdarg.h>
#endif

/* this is the ansi version ... */

#ifdef OS4
print_solution(stream, sol)
FILE *stream; double *sol;
#else
void print_solution(FILE *stream, double *sol)
#endif
{
  int i;

  fprintf(stream, "Value of objective function: %16.5g\n", sol[0]);

  /* print normal variables */
  for (i = Rows + 1; i <= Sum; i++)
    if (0 != sol[i]) fprintf(stream, "%-10s%16.5g\n", Names[i], sol[i]);

  /* print dual variables */
  if(Verbose || Print_duals)
    {
      fprintf(stream, "\nValues of the dual variables:\n");
      for (i = 1; i <= Rows; i++)
	if (0 != sol[i]) fprintf(stream, "%-10s%16.5g\n", Names[i], sol[i]);
    }
} /* print_solution */


#ifdef OS4
print_indent()
#else
void print_indent(void)
#endif
{
  int i;

  fprintf(stderr, "%2d", Level);
  for(i = Level; i > 0; i--)
    fprintf(stderr, "--");
  fprintf(stderr, "> ");
} /* print_indent */


#ifdef OS4
debug_print_solution(sol)
double *sol;
#else
void debug_print_solution(double *sol)
#endif
{
  int i;

  if(Debug)
    for (i = 0; i <= Sum; i++)
      {
	print_indent();
	if (sol[i] != 0) fprintf(stderr, "%-10s%16.5g\n", Names[i], sol[i]);
      }
} /* debug_print_solution */


#ifdef OS4
debug_print_bounds(upbo, lowbo)
double *upbo, *lowbo;
#else
void debug_print_bounds(double *upbo, double *lowbo)
#endif
{
  int i;

  if(Debug)
    for(i = Rows + 1; i <= Sum; i++)
      {
	if(lowbo[i] != 0)
	  {
	    print_indent();
	    fprintf(stderr, "%s > %10.3g\n", Names[i], lowbo[i]);
	  }
	if(upbo[i] != INFINITE)
	  {
	    print_indent();
	    fprintf(stderr, "%s < %10.3g\n", Names[i], upbo[i]);
	  }
      }
} /* debug_print_bounds */


#ifdef OS4
debug_print(format)
char *format;
{ return 0; }
#else
void debug_print(char *format, ...)
{
  va_list ap;

  if(Debug)
    {
      va_start(ap, format);
      print_indent();
      vfprintf(stderr, format, ap);
      fputc('\n', stderr);
      va_end(ap);
    }
} /* debug_print */

#endif
