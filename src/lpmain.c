/* $Id: lpmain.c,v 1.2 1999/01/03 02:07:19 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: lpmain.c,v 1.2 1999/01/03 02:07:19 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.     */
/*          All Rights Reserved.                */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/************************************************************************/

#include "version.h"

#include <signal.h>
#include "lpkit.h"
#include "lpglob.h"
#include "lppatch.h"
#include <setjmp.h>
#include <malloc.h>

#ifdef OS4
#include <sys/types.h>
#endif /* OS4 */

#include "lispemul.h"
#include "lspglob.h"
#include "lispmap.h"
#include "adr68k.h"
#include "arith.h"

#define CFREE(x)     \
  if (x) {           \
    free(x);         \
    (x) = (void *)0; \
  }

lprec *Medley_lp = NULL; /* The lp description for us */
int SolveCount = 0;      /* Counter for interrupting periodically */
jmp_buf LP_jmpbuf;

static lprec *initmilp(lprec *lp) {
  int i, j, rownr;
  int *num, *rownum;
  sstate *st; /* for saving state at timeouts */

  if (!lp) CALLOC(lp, 1, lprec);

  /* Done already in lisp. Rows--; */
  Sum = Rows + Columns;

  strcpy(lp->lp_name, "LOTS-TP integer program");

  lp->active = FALSE;
  lp->verbose = FALSE;
  lp->print_duals = FALSE;
  lp->print_sol = FALSE;
  lp->debug = FALSE;
  lp->print_at_invert = FALSE;
  lp->trace = FALSE;

  lp->rows = Rows;
  lp->columns = Columns;
  lp->sum = Rows + Columns;
  lp->rows_alloc = Rows;
  lp->columns_alloc = Columns;
  lp->sum_alloc = lp->sum;

  lp->names_used = FALSE;

  lp->obj_bound = DEF_INFINITE;
  lp->bb_rule = FIRST_NI;
  lp->break_at_int = FALSE;
  lp->infinite = DEF_INFINITE;
  lp->epsilon = DEF_EPSILON;
  lp->epsb = DEF_EPSB;
  lp->epsd = DEF_EPSD;
  lp->epsel = DEF_EPSEL;
  lp->non_zeros = Non_zeros;
  lp->mat_alloc = Non_zeros;
  lp->row_end_valid = FALSE;

  CFREE(lp->mat);
  MALLOC(lp->mat, Non_zeros, matrec);

  CFREE(lp->col_no);
  CALLOC(lp->col_no, Non_zeros, int);

  CFREE(lp->col_end);
  CALLOC(lp->col_end, Columns + 1, int);

  CFREE(lp->row_end);
  CALLOC(lp->row_end, Rows + 1, int);

  CFREE(lp->orig_rh);
  CALLOC(lp->orig_rh, Rows + 1, REAL);

  CFREE(lp->rh);
  CALLOC(lp->rh, Rows + 1, REAL);

  CFREE(lp->rhs);
  CALLOC(lp->rhs, Rows + 1, REAL);

  CFREE(lp->must_be_int);
  CALLOC(lp->must_be_int, Sum + 1, short);

  CFREE(lp->orig_upbo);
  MALLOC(lp->orig_upbo, Sum + 1, REAL);

  CFREE(lp->upbo);
  CALLOC(lp->upbo, Sum + 1, REAL);

  CFREE(lp->orig_lowbo);
  CALLOC(lp->orig_lowbo, Sum + 1, REAL);

  CFREE(lp->lowbo);
  CALLOC(lp->lowbo, Sum + 1, REAL);

  for (i = 0; i <= Sum; i++) {
    lp->orig_upbo[i] = lp->infinite;
    lp->orig_lowbo[i] = 0;
  }
  lp->basis_valid = TRUE;

  CFREE(lp->bas);
  CALLOC(lp->bas, Rows + 1, int);

  CFREE(lp->basis);
  CALLOC(lp->basis, Sum + 1, short);

  CFREE(lp->lower);
  CALLOC(lp->lower, Sum + 1, short);

  for (i = 0; i <= Rows; i++) {
    lp->bas[i] = i;
    lp->basis[i] = TRUE;
  }
  for (i = Rows + 1; i <= Sum; i++) lp->basis[i] = FALSE;
  for (i = 0; i <= Sum; i++) lp->lower[i] = TRUE;

  lp->eta_valid = TRUE;
  lp->eta_size = 0;
  lp->eta_alloc = 10000;
  lp->max_num_inv = DEFNUMINV;

  CFREE(lp->eta_value);
  CALLOC(lp->eta_value, 10000, REAL);

  CFREE(lp->eta_row_nr);
  CALLOC(lp->eta_row_nr, 10000, int);

  CFREE(lp->eta_col_end);
  CALLOC(lp->eta_col_end, Rows + lp->max_num_inv + 1, int);

  lp->iter = 0;
  lp->total_iter = 0;

  CFREE(lp->solution);
  CALLOC(lp->solution, Sum + 1, REAL);

  CFREE(lp->best_solution);
  CALLOC(lp->best_solution, Sum + 1, REAL);

  CFREE(lp->duals);
  CALLOC(lp->duals, Rows + 1, REAL);

  lp->maximise = FALSE;
  lp->floor_first = TRUE;

  lp->scaling_used = FALSE;

  CALLOC(lp->ch_sign, Rows + 1, short);

  for (i = 0; i <= Rows; i++) lp->ch_sign[i] = FALSE;

  st = lp->solve_states = NULL;
  for (i = 0; i < Sum + 2; i++) {
    if (st) /* There's an old state-saver to use; re-use it. */
    {
      st->saved = 0;  /* clear the in-use field */
      st->notint = 0; /* And the not-integer field */
      st = st->next;
    } else {
      st = (sstate *)malloc(sizeof(sstate));
      if (!st) ERROR(ERR_NOMEM); /* Tell the guy there's no memory */
      st->next = lp->solve_states;
      st->saved = 0;
      st->notint = 0; /* And the not-integer field */
      lp->solve_states = st;
      st = (sstate *)NULL;
    }
  }

  lp->valid = FALSE;

  return (lp);
}

int lpmain(LispPTR lispresults) {
  int i, failure;
  float *results = (float *)Addr68k_from_LADDR(lispresults);

  /* solve it */

  SolveCount = 0; /* reset the "timer", to limit length of a run */

  if (failure = setjmp(LP_jmpbuf)) return (S_POSITIVE | failure);

  /* solve it */

  Level = 0;

  failure = milpsolve(Medley_lp->solve_states, Orig_upbo, Orig_lowbo, Basis, Lower, Bas);

  Medley_lp->eta_size = Eta_size;
  Medley_lp->eta_alloc = Eta_alloc;
  Medley_lp->num_inv = Num_inv;

  if ((failure == INT_SOLN) || (failure == OPTIMAL))
    for (i = 0; i < Sum + 1; i++) results[i] = Best_solution[i];

  return (S_POSITIVE | failure); /* Tell him what happened so far. */

} /* main */

int lpsetup(int rows, int cols, int nonnuls, int rhs, int relns, int cend, int mat, int ints,
            int lowbo, int upbo, int objbound)
{
  double obj_bound = -Infinite;
  int failure, i, autoscale;

  if (failure = setjmp(LP_jmpbuf)) return (S_POSITIVE | failure);

  /* mallopt(M_DEBUG, 1);*/ /*debugging!!!!*/

  Rows = rows & 0xFFFF;
  Columns = cols & 0xFFFF;
  Non_zeros = nonnuls & 0xFFFF;

  printf("Rows = %d.  Cols = %d.  Non-zeros = %d.\n", Rows, Columns, Non_zeros);

  Medley_lp = initmilp(Medley_lp);
  readlispinput(Medley_lp, Addr68k_from_LADDR(rhs), Addr68k_from_LADDR(relns),
                Addr68k_from_LADDR(cend), Addr68k_from_LADDR(mat), Addr68k_from_LADDR(ints),
                Addr68k_from_LADDR(lowbo), Addr68k_from_LADDR(upbo));

  auto_scale(Medley_lp); /* Scale values */

  Medley_lp->total_iter = 0;
  Medley_lp->max_level = 1;
  Medley_lp->total_nodes = 0;

  set_globals(Medley_lp); /* Set global vars for the run */
  if (Isvalid(Medley_lp)) {
    if (objbound) {
      Medley_lp->obj_bound = Best_solution[0] = *((float *)Addr68k_from_LADDR(objbound));
    } else if (Maximise && Medley_lp->obj_bound == Infinite)
      Best_solution[0] = -Infinite;
    else if (!Maximise && Medley_lp->obj_bound == -Infinite)
      Best_solution[0] = Infinite;
    else
      Best_solution[0] = Medley_lp->obj_bound;

    Level = 0;

    if (!Medley_lp->basis_valid) {
      for (i = 0; i <= Medley_lp->rows; i++) {
        Medley_lp->basis[i] = TRUE;
        Medley_lp->bas[i] = i;
      }
      for (i = Medley_lp->rows + 1; i <= Medley_lp->sum; i++) Medley_lp->basis[i] = FALSE;
      for (i = 0; i <= Medley_lp->sum; i++) Medley_lp->lower[i] = TRUE;
      Medley_lp->basis_valid = TRUE;
    }

    Medley_lp->eta_valid = FALSE;
  }
  return (0);
} /* lpsetup */
