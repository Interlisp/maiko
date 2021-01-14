/* $Id: lpread.c,v 1.2 1999/01/03 02:07:19 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1994, 1995 Venue.				     */
/*          All Rights Reserved.                */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/************************************************************************/

#include "version.h"

/*
  ============================================================================
  NAME    : read.c
  PURPOSE : translation of lp-problem and storage in sparse matrix
  SHORT   : Subroutines for yacc program to store the input in an intermediate
  data-structure. The yacc and lex programs translate the input.
  First the problemsize is determined and the date is read into
  an intermediate structure, then readinput fills the sparse matrix.
  USAGE   : call yyparse(); to start reading the input.
  call readinput(); to fill the sparse matrix.
  ============================================================================
  Rows : contains the amount of rows + 1
  Rows-1 is the amount of constraints (no bounds)
  Rows   also contains the rownr 0 which is the objectfunction
  Columns : contains the amount of columns (different variable names
  found in the constraints)
  Nonnuls : contains the amount of nonnuls = sum of different entries
  of all columns in the constraints and in the objectfunction
  Hash_tab : contains all columnnames on the first level of the structure
  the row information is kept under each column structure
  in a linked list (also the objext function is in this structure)
  Bound information is also stored under under the column name
  First_rside : points to a linked list containing all relational operators
  and the righthandside values of the constraints
  the linked list is in reversed order with respect to the
  rownumbers
  ============================================================================
  */
#include "lpkit.h"
#include "lpglob.h"

extern REAL Infinite;

/*
 * transport the data from the intermediate structure to the sparse matrix
 * and free the intermediate structure
 */

void readlispinput(lprec *lp, float *lisprhs, short *lisprelns, int *lispcend, lispmr *lispmat,
                   short *lispints, float *lisplowbo, float *lispupbo)
{
  int i, j, k, index, nn_ind;
  int x;

  /* initialize lower and upper bound arrays */
  for (i = 0; i <= Sum; i++) {
    lp->orig_lowbo[i] = lisplowbo[i];

    if (lispupbo[i] > 1.0e20)
      lp->orig_upbo[i] = Infinite;
    else
      lp->orig_upbo[i] = lispupbo[i];
  }

  for (i = Rows; i >= 0; i--) { lp->orig_rh[i] = lisprhs[i]; }

  memcpy(lp->col_end, lispcend, (Columns + 1) * sizeof(int));
#ifdef BYTESWAP
  for (i = 0; i < Sum + 1; i++) lp->must_be_int[i] = lispints[i ^ 1];
#else
  memcpy(lp->must_be_int, lispints, (Sum + 1) * sizeof(short));
#endif /* BYTESWAP */

  for (i = 0; i < Non_zeros; i++) {
    lp->mat[i].row_nr = lispmat[i].rownr;
    lp->mat[i].value = lispmat[i].value;
  }

  set_maxim(lp); /* We always maximize. */

  for (i = Rows; i > 0; i--) {
#ifdef BYTESWAP
    set_constr_type(lp, i, lisprelns[i ^ 1]);
#else
    set_constr_type(lp, i, lisprelns[i]);
#endif /* BYTESWAP */
  }

} /* readlispinput */

/* ===================== END OF read.c ===================================== */
