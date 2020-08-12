/* $Id: lpdefs.h,v 1.2 1999/01/03 02:06:13 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.     */
/*          All Rights Reserved.                */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#ifndef OS4
#include <stdlib.h>
#else
#include <sys/types.h>
#endif /* OS4 */
#include <string.h>
#include <setjmp.h>


#define HASHSIZE  10007 /* prime number is better, MB */
#define ETA_START_SIZE 10000 /* start size of array Eta. Realloced if needed */
#define FNAMLEN 64
#define NAMELEN 25
#define MAXSTRL (NAMELEN-1)
#define STD_ROW_NAME_PREFIX "r_"

#define FALSE   0
#define TRUE    1

#define LE      0
#define EQ      1
#define GE      2
#define OF      3

#define OPTIMAL    0
#define MILP_FAIL  1
#define INFEASIBLE 2
#define UNBOUNDED  3
#define TIMEOUT    4
#define INT_SOLN   5

#define my_abs(x)       ((x) < 0 ? -(x) : (x))
#define my_min(x, y)    ((x) < (y) ? (x) : (y))
#define my_max(x, y)    ((x) > (y) ? (x) : (y))

#define CALLOC(ptr, nr, type) if(!(ptr = calloc((size_t)(nr),\
  sizeof(type)))) { fprintf(stderr, "calloc failed\n"); ERROR(ERR_NOMEM); }

#define MALLOC(ptr, nr, type) if(!(ptr = malloc((size_t)((nr) * \
  sizeof(type))))) { fprintf(stderr, "malloc failed\n"); ERROR(ERR_NOMEM); }

#define DEFAULT_INFINITE  1.0e24 /* limit for dynamic range */
#define DEFAULT_EPSB      0.0001 /* for rounding RHS values to 0 */
#define DEFAULT_EPSEL     1.0e-8 /* for rounding other values to 0 */
#define DEFAULT_EPSD      0.0001 /* ?? MB */
#define DEFAULT_EPSILON   1e-6   /* to determine if a float value is integer */

#define INVITER 50 /* number of iterations between inversions */

#ifndef REAL /* to allow -DREAL=<float type> while compiling */
#define REAL double
#endif

typedef char    nstring[NAMELEN];

typedef struct _column
{
  int            row;
  float          value;
  struct _column *next ;
} column;

typedef struct _constraint_name
{
  char                    name[NAMELEN];
  int                     row;
  struct _constraint_name *next;
} constraint_name;

typedef struct _bound
{
  REAL          upbo;
  REAL          lowbo;
} bound;

typedef struct _hashelem
{
  nstring          colname;
  struct _hashelem *next;
  struct _column   *col;
  struct _bound    *bnd;
  int              must_be_int;
} hashelem;

typedef struct _rside /* contains relational operator and rhs value */
{
  REAL          value;
  struct _rside *next;
  short         relat;
} rside;

/* structure or final data-storage */

typedef struct  _matrec 
{
  int    rownr;
  REAL    value;
} matrec;

typedef struct _lispmr
  {
	int rownr;
	float value;
  } lispmr;



typedef struct _tmp_store_struct
{
  nstring name;
  int     row;
  REAL    value;
  REAL    rhs_value;
  short   relat;
} tmp_store_struct;

typedef struct _intrec
{
  int             varnr;
  struct _intrec  *next;
}
intrec;


/************************************************************************/
/*                                                                      */
/*      S T A T E - S A V I N G   F O R   T I M E - O U T S             */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

typedef struct solve_state
  {
	int    saved;                           /* 0 = not used, else states below */
	int    notint;                          /* variable # of the non-integer var chosen */
	int    bound;                           /* integer lower bound for variable notint */
	int    res1, res2;
	struct solve_state *next;       /* The next state holder in the chain */
  } sstate;

#define ST_LO 1 /* We got to before calling solve on the lower bound side */
#define ST_HI 2 /* We got to before calling solve on the upper bound side */
#define ST_SOLN 3 /* We got past calling solve, not to analyzing */



/*************************************************************************/
/*                                                                       */
/*                E R R O R   H A N D L I N G   F O R   L I B R A R Y    */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#define ERROR(x) longjmp(LP_jmpbuf,256|x)

#define ERR_NOMEM      0x10       /* Any out-of-memory problem */
#define ERR_ST         0x11       /* ran out allocating State-infos */

#define ERR_NUM        0x20       /* Any numeric stability problem */

#define ERR_BUG        0x40       /* Any lp_solve-bug exit */
#define ERR_BUG_CONDCOL 0x41      /* Condensecol out-of-bounds */








