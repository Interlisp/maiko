/* $Id: lpdual.c,v 1.2 1999/01/03 02:07:17 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: lpdual.c,v 1.2 1999/01/03 02:07:17 sybalsky Exp $ Copyright (C) Venue";


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

#ifdef alliant
#pragma global safe (Eta_rownr, Eta_value)
#pragma global assoc
#endif

#ifdef OS4
rowdual(rownr)
int *rownr;
#else
void  rowdual(int *rownr)
#endif
{
  int    i;
  double f, g, minrhs;
  short  artifs;

#ifdef alliant
#pragma safe (Rhs, Upbo, Bas)
#endif
  
  if (Verbose)
    printf("rowdual\n");
  (*rownr) = 0;
  minrhs = -EPSB;
  i = 0;
  artifs = FALSE;
  while (i < Rows && !artifs)
    {
      i++;
      f = Upbo[Bas[i]];
      if (f == 0 && Rhs[i] != 0)
	{
	  artifs = TRUE;
	  (*rownr) = i;
	}
      else
	{
	  if (Rhs[i] < f - Rhs[i])
	    g = Rhs[i];
	  else
	    g = f - Rhs[i];
	  if (g < minrhs)
	    {
	      minrhs = g;
	      (*rownr) = i;
	    }
	}
    }
} /* rowdual */

#ifdef OS4
short coldual(numeta, rownr, colnr, minit, prow, drow)
 int *numeta, *rownr, *colnr;
 short *minit;
 double *prow, *drow;
#else
short  coldual(int *numeta,
	       int *rownr,
	       int *colnr,
	       short *minit,
	       double *prow,
	       double *drow)
#endif
{
  int    i, j, r, varnr;
  double theta, quot, pivot, d, f, g;
  
#ifdef alliant
#pragma safe (Rhs, Upbo, Bas, Cend, Endetacol, prow, drow, Basis, Lower)
#endif

  if (Verbose)
    printf("coldual\n");
  if (!(*minit))
    {
      for (i = 0; i <= Rows; i++)
	{
	  prow[i] = 0;
	  drow[i] = 0;
	}
      drow[0] = 1;
      prow[(*rownr)] = 1;
      for (i = (*numeta); i >= 1; i--)
	{
	  d = 0;
	  f = 0;
	  r = Eta_rownr[Endetacol[i] - 1];
	  for (j = Endetacol[i - 1]; j < Endetacol[i]; j++)
	    {
	      /* this is where the program consumes most cpu time */
	      f = f + prow[Eta_rownr[j]] * Eta_value[j];
	      d = d + drow[Eta_rownr[j]] * Eta_value[j];
	    }
	  if (abs(f) < EPSEL)
	    prow[r] = 0;
	  else
	    prow[r] = f;
	  if (abs(d) < EPSEL)
	    drow[r] = 0;
	  else
	    drow[r] = d;
	}
      for (i = 1; i <= Columns; i++)
	{
	  varnr = Rows + i;
	  if (!Basis[varnr])
	    {
	      d = -Extrad * drow[0];
	      f = 0;
	      for (j = Cend[i - 1]; j < Cend[i]; j++)
		{
		  d += drow[Mat[j].rownr] * Mat[j].value;
		  f += prow[Mat[j].rownr] * Mat[j].value;
		}
	      drow[varnr] = d;
	      prow[varnr] = f;
	    }
	}

#ifdef alliant
#pragma loop novector
#endif
      
      for (i = 0; i <= Sum; i++)
	{
	  if (abs(prow[i]) < EPSEL)
	    prow[i] = 0;
	  if (abs(drow[i]) < EPSD)
	    drow[i] = 0;
	}
    }
  if (Rhs[(*rownr)] > Upbo[Bas[(*rownr)]])
    g = -1;
  else
    g = 1;
  pivot = 0;
  (*colnr) = 0;
  theta = INFINITE;
  for (i = 1; i <= Sum; i++)
    {
      if (Lower[i])
	d = prow[i] * g;
      else
	d = -prow[i] * g;
      if (d < 0)
	if (!Basis[i])
	  if (Upbo[i] > 0)
	    {
	      if (Lower[i])
		quot = -drow[i] / (double) d;
	      else
		quot = drow[i] / (double) d;
	      if (quot < theta)
		{
		  theta = quot;
		  pivot = d;
		  (*colnr) = i;
		}
	      else
		if (quot == theta)
		  if (abs(d) > abs(pivot))
		    {
		      pivot = d;
		      (*colnr) = i;
		    }
	    }
    }
  return ((*colnr) > 0);
} /* coldual */
