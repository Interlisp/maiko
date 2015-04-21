/* $Id: lpglobl.h,v 1.2 1999/01/03 02:06:14 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*                                                                      */
/*      (C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.     */
/*          All Rights Reserved.                */
/*      Manufactured in the United States of America.                   */
/*                                                                      */
/*      The contents of this file are proprietary information           */
/*      belonging to Venue, and are provided to you under license.      */
/*      They may not be further distributed or disclosed to third       */
/*      parties without the specific permission of Venue.               */
/*                                                                      */
/************************************************************************/

/* function declarations */
#include "lpproto.h"

/* global variables */
extern int        Rows, Columns, Nonnuls, Sum;
extern double     Extrad;
extern double     *Pcol; /* [Rows+1] */
extern nstring    Probname;
extern int        Totnum, Classnr, Linenr;
extern short      Bounds, Ranges, Verbose, Debug, Show_results, Print_duals;
extern unsigned   Cur_eta_size;
extern double     *Eta_value;
extern int        *Eta_rownr;
extern constraint_name *First_constraint_name;

extern matrec     *Mat;
extern double     *Upbo, *Lowbo;
extern nstring    *Names;
extern int        *Cend;
extern double     *Rh;
extern short      *Relat;

extern short      *Chsign;
extern int        *Endetacol;
extern int        *Rend, *Bas;
extern double     *Rhs; 
extern int        *Colno;
extern short      *Basis, *Lower;
extern short      Maximise;

extern double     *Solution, *Best_solution, *Orig_rh, *Orig_upbo, *Orig_lowbo;
extern short      *Must_be_int;
extern short      Having_ints;
extern matrec     *Orig_mat;
extern int        Level;
extern short      Floorfirst;

extern intrec     *First_int;

   /* external variables for yy_trans and the yacc parser */
extern int        yylineno;
extern int        yyleng;
extern int        Lin_term_count;
extern int        Sign;


/* I hate #ifdefs, but there seems to be no "standard" way to do this */
#if defined(__hpux) || defined(__apollo)
/* for HP and Apollo (and possibly others) */
extern unsigned char       yytext[];
#else
/* For other computers */
extern char    yytext[];
#endif

/*extern hashelem   *Hash_tab[HASH_SIZE];*/
extern rside      *First_rside;
extern short      Ignore_decl;

extern tmp_store_struct tmp_store;

extern int SolveCount;
extern jmp_buf LP_jmpbuf;


