/* $Id: lpytab.c,v 1.2 1999/01/03 02:07:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: lpytab.c,v 1.2 1999/01/03 02:07:21 sybalsky Exp $ Copyright (C) Venue";

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

#ifndef BIGATOMS

#define VAR 257
#define CONS 258
#define SIGN 259
#define AR_M_OP 260
#define RE_OP 261
#define END_C 262
#define COMMA 263
#define COLON 264
#define MINIMISE 265
#define MAXIMISE 266

#line 10 "lp.y"
#include "lpdefines.h"
#include "lpglobals.h"

/* globals */
char Last_var[NAMELEN];
char Constraint_name[NAMELEN];
int Lin_term_count;
double f;
int x;
int Sign;
int isign; /* internal_sign variable to make sure nothing goes wrong */
/* with lookahead */
int make_neg; /* is true after the relational operator is seen in order */
/* to remember if lin_term stands before or after re_op */
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
#define YYERRCODE 256

#line 193 "lp.y"

#include "lexyy.c"
yytabelem yyexca[] = {
    -1, 1, 0, -1, -2, 0, -1, 31, 257, 15, -2, 31,
};
#define YYNPROD 38
#define YYLAST 87
yytabelem yyact[] = {

    10, 11, 9,  34, 36, 51, 56, 57, 5,  4,  50, 52, 23, 44, 36, 24, 35, 10, 20, 18, 26, 10,
    20, 27, 31, 20, 18, 10, 11, 9,  15, 20, 18, 10, 11, 42, 40, 17, 14, 16, 19, 32, 13, 8,
    6,  8,  8,  38, 25, 21, 22, 7,  47, 37, 43, 29, 33, 30, 46, 45, 28, 12, 3,  2,  1,  0,
    39, 0,  0,  0,  0,  0,  41, 0,  0,  48, 0,  0,  0,  49, 0,  0,  0,  0,  53, 55, 54};
yytabelem yypact[] = {

    -1000, -1000, -257,  -227,  -230,  -230,  -1000, -247,  -1000, -1000, -1000, -237,
    -233,  -1000, -1000, -261,  -245,  -1000, -1000, -1000, -237,  -1000, -1000, -1000,
    -1000, -224,  -1000, -221,  -1000, -1000, -222,  -261,  -1000, -244,  -1000, -1000,
    -1000, -236,  -224,  -1000, -1000, -1000, -1000, -252,  -1000, -240,  -240,  -236,
    -1000, -1000, -1000, -1000, -251,  -1000, -255,  -1000, -1000, -1000};
yytabelem yypgo[] = {

    0, 64, 63, 62, 61, 60, 42, 38, 59, 39, 58, 57, 41, 56, 54, 37, 53, 52, 40, 51, 48, 47, 44};
yytabelem yyr1[] = {

    0, 2,  1, 4,  4, 6,  8,  6,  10, 7,  5,  5,  11, 11, 12, 13, 14, 14, 14,
    9, 16, 9, 17, 9, 15, 15, 19, 20, 19, 21, 19, 18, 18, 18, 3,  3,  3,  22};
yytabelem yyr2[] = {

    0, 1, 8, 2, 4, 2, 1, 8, 1, 11, 0, 2, 2, 4, 7, 3, 3, 5, 7,
    2, 1, 6, 1, 8, 2, 3, 2, 1, 6,  1, 8, 3, 5, 7, 5, 5, 2, 5};
yytabelem yychk[] = {

    -1000, -1,  -2,  -3,  266, 265, -22, -19, -18, 259, 257, 258, -4,  -6,  -7,
    257,   -9,  -15, 259, -18, 258, -22, -22, 259, 262, -20, 257, 260, -5,  -6,
    -11,   257, -12, -13, 264, 261, 259, -16, -21, -18, 257, -12, 257, -14, 257,
    -8,    -10, -17, -15, -18, 262, 257, 263, -7,  -9,  -15, 257, 262};
yytabelem yydef[] = {

    1,  -2, 0,  0,  0,  0, 36, 0, 26, 27, 31, 0,  10, 3, 5, 31, 0,  19, 20, 24,
    25, 34, 35, 29, 37, 0, 32, 0, 2,  4,  11, -2, 12, 0, 6, 8,  22, 0,  0,  28,
    33, 13, 15, 0,  16, 0, 0,  0, 21, 30, 14, 17, 0,  7, 0, 23, 18, 9};
typedef struct {
  char *t_name;
  int t_val;
} yytoktype;
#ifndef YYDEBUG
#define YYDEBUG 0 /* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] = {
    "VAR",   257, "CONS",  258, "SIGN",     259, "AR_M_OP",  260, "RE_OP",     261, "END_C", 262,
    "COMMA", 263, "COLON", 264, "MINIMISE", 265, "MAXIMISE", 266, "-unknown-", -1 /* ends search */
};

char *yyreds[] = {
    "-no such reduction-",
    "inputfile : /* empty */",
    "inputfile : objective_function constraints int_declarations",
    "constraints : constraint",
    "constraints : constraints constraint",
    "constraint : real_constraint",
    "constraint : VAR COLON",
    "constraint : VAR COLON real_constraint",
    "real_constraint : x_lineair_sum RE_OP",
    "real_constraint : x_lineair_sum RE_OP x_lineair_sum END_C",
    "int_declarations : /* empty */",
    "int_declarations : real_int_decls",
    "real_int_decls : int_declaration",
    "real_int_decls : real_int_decls int_declaration",
    "int_declaration : int_declarator vars END_C",
    "int_declarator : VAR",
    "vars : VAR",
    "vars : vars VAR",
    "vars : vars COMMA VAR",
    "x_lineair_sum : x_lineair_term",
    "x_lineair_sum : SIGN",
    "x_lineair_sum : SIGN x_lineair_term",
    "x_lineair_sum : x_lineair_sum SIGN",
    "x_lineair_sum : x_lineair_sum SIGN x_lineair_term",
    "x_lineair_term : lineair_term",
    "x_lineair_term : CONS",
    "lineair_sum : lineair_term",
    "lineair_sum : SIGN",
    "lineair_sum : SIGN lineair_term",
    "lineair_sum : lineair_sum SIGN",
    "lineair_sum : lineair_sum SIGN lineair_term",
    "lineair_term : VAR",
    "lineair_term : CONS VAR",
    "lineair_term : CONS AR_M_OP VAR",
    "objective_function : MAXIMISE real_of",
    "objective_function : MINIMISE real_of",
    "objective_function : real_of",
    "real_of : lineair_sum END_C",
};
#endif /* YYDEBUG */
/*
 *      Copyright 1987 Silicon Graphics, Inc. - All Rights Reserved
 */

#ident "$Revision: 1.2 $"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR goto yyerrlab
#define YYACCEPT return (0)
#define YYABORT return (1)
#define YYBACKUP(newtoken, newvalue)              \
  \
{                                            \
    if (yychar >= 0 || (yyr2[yytmp] >> 1) != 1) { \
      yyerror("syntax error - cannot backup");    \
      goto yyerrlab;                              \
    }                                             \
    yychar = newtoken;                            \
    yystate = *yyps;                              \
    yylval = newvalue;                            \
    goto yynewstate;                              \
  \
}
#define YYRECOVERING() (!!yyerrflag)
#ifndef YYDEBUG
#define YYDEBUG 1 /* make debugging available */
#endif

/*
** user known globals
*/
int yydebug; /* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG (-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[YYMAXDEPTH]; /* value stack */
int yys[YYMAXDEPTH];     /* state stack */

YYSTYPE *yypv; /* top of value stack */
int *yyps;     /* top of state stack */

int yystate; /* current state */
int yytmp;   /* extra var (lasts between blocks) */

int yynerrs;   /* number of errors */
int yyerrflag; /* error recovery flag */
int yychar;    /* current input token number */

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int yyparse() {
  register YYSTYPE *yypvt; /* top of value stack for $vars */

  /*
  ** Initialize externals - yyparse may be called more than once
  */
  yypv = &yyv[-1];
  yyps = &yys[-1];
  yystate = 0;
  yytmp = 0;
  yynerrs = 0;
  yyerrflag = 0;
  yychar = -1;

  goto yystack;
  {
    register YYSTYPE *yy_pv; /* top of value stack */
    register int *yy_ps;     /* top of state stack */
    register int yy_state;   /* current state */
    register int yy_n;       /* internal state number info */

  /*
  ** get globals into registers.
  ** branch to here only if YYBACKUP was called.
  */
  yynewstate:
    yy_pv = yypv;
    yy_ps = yyps;
    yy_state = yystate;
    goto yy_newstate;

  /*
  ** get globals into registers.
  ** either we just started, or we just finished a reduction
  */
  yystack:
    yy_pv = yypv;
    yy_ps = yyps;
    yy_state = yystate;

  /*
  ** top of for (;;) loop while no reductions done
  */
  yy_stack:
/*
** put a state and value onto the stacks
*/
#if YYDEBUG
    /*
    ** if debugging, look up token value in list of value vs.
    ** name pairs.  0 and negative (-1) are special values.
    ** Note: linear search is used since time is not a real
    ** consideration while debugging.
    */
    if (yydebug) {
      register int yy_i;

      printf("State %d, token ", yy_state);
      if (yychar == 0)
        printf("end-of-file\n");
      else if (yychar < 0)
        printf("-none-\n");
      else {
        for (yy_i = 0; yytoks[yy_i].t_val >= 0; yy_i++) {
          if (yytoks[yy_i].t_val == yychar) break;
        }
        printf("%s\n", yytoks[yy_i].t_name);
      }
    }
#endif                               /* YYDEBUG */
    if (++yy_ps >= &yys[YYMAXDEPTH]) /* room on stack? */
    {
      yyerror("yacc stack overflow");
      YYABORT;
    }
    *yy_ps = yy_state;
    *++yy_pv = yyval;

  /*
  ** we have a new state - find out what to do
  */
  yy_newstate:
    if ((yy_n = yypact[yy_state]) <= YYFLAG) goto yydefault; /* simple state */
#if YYDEBUG
    /*
    ** if debugging, need to mark whether new token grabbed
    */
    yytmp = yychar < 0;
#endif
    if ((yychar < 0) && ((yychar = yylex()) < 0)) yychar = 0; /* reached EOF */
#if YYDEBUG
    if (yydebug && yytmp) {
      register int yy_i;

      printf("Received token ");
      if (yychar == 0)
        printf("end-of-file\n");
      else if (yychar < 0)
        printf("-none-\n");
      else {
        for (yy_i = 0; yytoks[yy_i].t_val >= 0; yy_i++) {
          if (yytoks[yy_i].t_val == yychar) break;
        }
        printf("%s\n", yytoks[yy_i].t_name);
      }
    }
#endif /* YYDEBUG */
    if (((yy_n += yychar) < 0) || (yy_n >= YYLAST)) goto yydefault;
    if (yychk[yy_n = yyact[yy_n]] == yychar) /*valid shift*/
    {
      yychar = -1;
      yyval = yylval;
      yy_state = yy_n;
      if (yyerrflag > 0) yyerrflag--;
      goto yy_stack;
    }

  yydefault:
    if ((yy_n = yydef[yy_state]) == -2) {
#if YYDEBUG
      yytmp = yychar < 0;
#endif
      if ((yychar < 0) && ((yychar = yylex()) < 0)) yychar = 0; /* reached EOF */
#if YYDEBUG
      if (yydebug && yytmp) {
        register int yy_i;

        printf("Received token ");
        if (yychar == 0)
          printf("end-of-file\n");
        else if (yychar < 0)
          printf("-none-\n");
        else {
          for (yy_i = 0; yytoks[yy_i].t_val >= 0; yy_i++) {
            if (yytoks[yy_i].t_val == yychar) { break; }
          }
          printf("%s\n", yytoks[yy_i].t_name);
        }
      }
#endif /* YYDEBUG */
      /*
      ** look through exception table
      */
      {
        register int *yyxi = yyexca;

        while ((*yyxi != -1) || (yyxi[1] != yy_state)) { yyxi += 2; }
        while ((*(yyxi += 2) >= 0) && (*yyxi != yychar))
          ;
        if ((yy_n = yyxi[1]) < 0) YYACCEPT;
      }
    }

    /*
    ** check for syntax error
    */
    if (yy_n == 0) /* have an error */
    {
      /* no worry about speed here! */
      switch (yyerrflag) {
        case 0: /* new error */
          yyerror("syntax error");
          goto skip_init;
        yyerrlab:
          /*
          ** get globals into registers.
          ** we have a user generated syntax type error
          */
          yy_pv = yypv;
          yy_ps = yyps;
          yy_state = yystate;
          yynerrs++;
        skip_init:
        case 1:
        case 2: /* incompletely recovered error */
                /* try again... */
          yyerrflag = 3;
          /*
          ** find state where "error" is a legal
          ** shift action
          */
          while (yy_ps >= yys) {
            yy_n = yypact[*yy_ps] + YYERRCODE;
            if (yy_n >= 0 && yy_n < YYLAST && yychk[yyact[yy_n]] == YYERRCODE) {
              /*
              ** simulate shift of "error"
              */
              yy_state = yyact[yy_n];
              goto yy_stack;
            }
/*
** current state has no shift on
** "error", pop stack
*/
#if YYDEBUG
#define _POP_ "Error recovery pops state %d, uncovers state %d\n"
            if (yydebug) printf(_POP_, *yy_ps, yy_ps[-1]);
#undef _POP_
#endif
            yy_ps--;
            yy_pv--;
          }
          /*
          ** there is no state on stack with "error" as
          ** a valid shift.  give up.
          */
          YYABORT;
        case 3: /* no shift yet; eat a token */
#if YYDEBUG
          /*
          ** if debugging, look up token in list of
          ** pairs.  0 and negative shouldn't occur,
          ** but since timing doesn't matter when
          ** debugging, it doesn't hurt to leave the
          ** tests here.
          */
          if (yydebug) {
            register int yy_i;

            printf("Error recovery discards ");
            if (yychar == 0)
              printf("token end-of-file\n");
            else if (yychar < 0)
              printf("token -none-\n");
            else {
              for (yy_i = 0; yytoks[yy_i].t_val >= 0; yy_i++) {
                if (yytoks[yy_i].t_val == yychar) { break; }
              }
              printf("token %s\n", yytoks[yy_i].t_name);
            }
          }
#endif                     /* YYDEBUG */
          if (yychar == 0) /* reached EOF. quit */
            YYABORT;
          yychar = -1;
          goto yy_newstate;
      }
    } /* end if ( yy_n == 0 ) */
/*
** reduction by production yy_n
** put stack tops, etc. so things right after switch
*/
#if YYDEBUG
    /*
    ** if debugging, print the string that is the user's
    ** specification of the reduction which is just about
    ** to be done.
    */
    if (yydebug) printf("Reduce by (%d) \"%s\"\n", yy_n, yyreds[yy_n]);
#endif
    yytmp = yy_n;  /* value to switch over */
    yypvt = yy_pv; /* $vars top of value stack */
    /*
    ** Look in goto table for next state
    ** Sorry about using yy_state here as temporary
    ** register variable, but why not, if it works...
    ** If yyr2[ yy_n ] doesn't have the low order bit
    ** set, then there is no action to be done for
    ** this reduction.  So, no saving & unsaving of
    ** registers done.  The only difference between the
    ** code just after the if and the body of the if is
    ** the goto yy_stack in the body.  This way the test
    ** can be made before the choice of what to do is needed.
    */
    {
      /* length of production doubled with extra bit */
      register int yy_len = yyr2[yy_n];

      if (!(yy_len & 01)) {
        yy_len >>= 1;
        yyval = (yy_pv -= yy_len)[1]; /* $$ = $1 */
        yy_state = yypgo[yy_n = yyr1[yy_n]] + *(yy_ps -= yy_len) + 1;
        if (yy_state >= YYLAST || yychk[yy_state = yyact[yy_state]] != -yy_n) {
          yy_state = yyact[yypgo[yy_n]];
        }
        goto yy_stack;
      }
      yy_len >>= 1;
      yyval = (yy_pv -= yy_len)[1]; /* $$ = $1 */
      yy_state = yypgo[yy_n = yyr1[yy_n]] + *(yy_ps -= yy_len) + 1;
      if (yy_state >= YYLAST || yychk[yy_state = yyact[yy_state]] != -yy_n) {
        yy_state = yyact[yypgo[yy_n]];
      }
    }
    /* save until reenter driver code */
    yystate = yy_state;
    yyps = yy_ps;
    yypv = yy_pv;
  }
  /*
  ** code supplied by user is placed in this switch
  */
  switch (yytmp) {
    case 1:
#line 32 "lp.y"
    {
      init_read();
      isign = 0;
      make_neg = 0;
    } break;
    case 6:
#line 49 "lp.y"
    {
      add_constraint_name(Last_var, Rows);
    } break;
    case 8:
#line 58 "lp.y"
    {
      store_re_op();
      make_neg = 1;
    } break;
    case 9:
#line 64 "lp.y"
    {
      if (Lin_term_count == 0) {
        fprintf(stderr, "WARNING line %d: constraint contains no variables\n", yylineno);
        null_tmp_store();
      }

      if (Lin_term_count > 1) Rows++;

      if (Lin_term_count == 1) store_bounds();

      Lin_term_count = 0;
      isign = 0;
      make_neg = 0;
      Constraint_name[0] = '\0';
    } break;
    case 14:
#line 93 "lp.y"
    {
      Having_ints = 1;
    } break;
    case 15:
#line 96 "lp.y"
    { /* check_decl(yytext);*/
    } break;
    case 16:
#line 99 "lp.y"
    {
      add_int_var((char *)yytext);
    } break;
    case 17:
#line 100 "lp.y"
    {
      add_int_var((char *)yytext);
    } break;
    case 18:
#line 101 "lp.y"
    {
      add_int_var((char *)yytext);
    } break;
    case 20:
#line 106 "lp.y"
    {
      isign = Sign;
    } break;
    case 22:
#line 112 "lp.y"
    {
      isign = Sign;
    } break;
    case 25:
#line 120 "lp.y"
    {
      if ((isign || !make_neg) && !(isign && !make_neg)) /* but not both! */
        f = -f;
      rhs_store(f);
      isign = 0;
    } break;
    case 27:
#line 131 "lp.y"
    {
      isign = Sign;
    } break;
    case 29:
#line 137 "lp.y"
    {
      isign = Sign;
    } break;
    case 31:
#line 144 "lp.y"
    {
      if ((isign || make_neg) && !(isign && make_neg)) /* but not both! */
        var_store(Last_var, Rows, (double)-1);
      else
        var_store(Last_var, Rows, (double)1);
      isign = 0;
    } break;
    case 32:
#line 154 "lp.y"
    {
      if ((isign || make_neg) && !(isign && make_neg)) /* but not both! */
        f = -f;
      var_store(Last_var, Rows, f);
      isign = 0;
    } break;
    case 33:
#line 164 "lp.y"
    {
      if ((isign || make_neg) && !(isign && make_neg)) /* but not both! */
        f = -f;
      var_store(Last_var, Rows, f);
      isign = 0;
    } break;
    case 34:
#line 174 "lp.y"
    {
      Maximise = TRUE;
    } break;
    case 35:
#line 178 "lp.y"
    {
      Maximise = FALSE;
    } break;
    case 37:
#line 186 "lp.y"
    {
      Rows++;
      Lin_term_count = 0;
      isign = 0;
      make_neg = 0;
    } break;
  }
  goto yystack; /* reset registers in driver code */
}
#endif
