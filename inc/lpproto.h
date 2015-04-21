/* $Id: lpproto.h,v 1.2 1999/01/03 02:06:15 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
/* dual.c */
void rowdual(int *rownr);
short coldual(int rownr, int *colnr, short minit, REAL *prow, REAL *drow);

/* lp.c */
int yyparse(void);

/* read.c */
void yyerror(char *string);
void check_decl(char *str);
void add_int_var(char *name);
void init_read(void);
void null_tmp_store(void);
void store_re_op(void);
void rhs_store(REAL value);
void var_store(char *var, int row, REAL value);
int store_bounds(void);
void add_constraint_name(char *name, int row);
void readinput(int *cend, REAL *rh, short *relat, REAL *lowbo, REAL *upbo, matrec *mat, nstring *names);

/* tran.c */
void ftran(int start, int end, REAL *pcol);
void btran(int numc, REAL *row);

/* main.c */

/* solve.c */
int solve(REAL *upbo, REAL *lowbo, short *sbasis, short *slower, int *sbas,
          sstate *t);

/* write.c */
void print_solution(FILE *stream, REAL *sol, REAL *duals);
void debug_print_solution(REAL *sol);
void debug_print_bounds(REAL *upbo, REAL *lowbo);
void debug_print(char *format, ...);
