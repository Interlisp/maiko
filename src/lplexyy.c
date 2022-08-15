/* $Id: lplexyy.c,v 1.2 1999/01/03 02:07:18 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#ifndef BIGATOMS
#include "stdio.h"
#define U(x) x
#define NLSTATE yyprevious = YYNEWLINE
#define BEGIN yybgin = yysvec + 1 +
#define INITIAL 0
#define YYLERR yysvec
#define YYSTATE (yyestate - yysvec - 1)
#define YYOPTIM 1
#define YYLMAX 200
#define output(c) putc(c, yyout)
#define input()                                                                           \
  (((yytchar = yysptr > yysbuf ? U(*--yysptr) : getc(yyin)) == 10 ? (yylineno++, yytchar) \
                                                                  : yytchar) == EOF       \
       ? 0                                                                                \
       : yytchar)
#define unput(c)                     \
  {                                  \
    yytchar = (c);                   \
    if (yytchar == '\n') yylineno--; \
    *yysptr++ = yytchar;             \
  }
#define yymore() (yymorfg = 1)
#define ECHO fprintf(yyout, "%s", yytext)
#define REJECT         \
  {                    \
    nstr = yyreject(); \
    goto yyfussy;      \
  }
int yyleng;
extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf {
  struct yywork *yystoff;
  struct yysvf *yyother;
  int *yystops;
};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
extern int yylook();
#ifdef __cplusplus
extern "C" {
#endif
#define yywrap() (1)
extern int yylex();
extern int yyreject();
extern int yyracc(int);
extern int yyless(int);
#ifdef __cplusplus
}
#endif
#define COMMENT 2
#define YYNEWLINE 10
yylex() {
  int nstr;
  extern int yyprevious;
  while ((nstr = yylook()) >= 0)
  yyfussy:
    switch (nstr) {
      case 0:
        if (yywrap()) return (0);
        break;
      case 1: {
        BEGIN COMMENT;
      } break;
      case 2: {
        BEGIN INITIAL;
      } break;
      case 3: {
      } break;
      case 4: {
      } break;
      case 5: {
      } break;
      case 6: {
        return (COMMA);
      } break;
      case 7: {
        return (MINIMISE);
      } break;
      case 8: {
        return (MAXIMISE);
      } break;
      case 9: {
        sscanf((char *)yytext, "%lf", &f);
        return (CONS);
      } break;
      case 10: {
        Sign = 0;
        for (x = 0; x < yyleng; x++)
          if (yytext[x] == '-' || yytext[x] == '+') Sign = (Sign == (yytext[x] == '+'));
        return (SIGN);
        /* Sign is TRUE if the sign-string
           represents a '-'. Otherwise Sign
           is FALSE */
      } break;
      case 11: {
        strcpy(Last_var, (char *)yytext);
        return (VAR);
      } break;
      case 12: {
        return (COLON);
      } break;
      case 13: {
        return (AR_M_OP);
      } break;
      case 14: {
        return (RE_OP);
      } break;
      case 15: {
        return (END_C);
      } break;
      case 16: {
        fprintf(stderr, "LEX ERROR : %s lineno %d \n", yytext, yylineno);
      } break;
      case -1: break;
      default: fprintf(yyout, "bad switch yylook %d", nstr);
    }
  return (0);
}
/* end of yylex */
int yyvstop[] = {0,

                 14, 0,

                 14, 0,

                 14, 0,

                 14, 0,

                 16, 0,

                 5,  10, 16, 0,

                 5,  10, 0,

                 13, 16, 0,

                 10, 16, 0,

                 6,  16, 0,

                 16, 0,

                 16, 0,

                 9,  16, 0,

                 12, 16, 0,

                 15, 16, 0,

                 14, 16, 0,

                 14, 16, 0,

                 11, 16, 0,

                 11, 16, 0,

                 3,  0,

                 4,  0,

                 3,  0,

                 10, 0,

                 9,  0,

                 1,  0,

                 9,  0,

                 14, 0,

                 11, 0,

                 11, 0,

                 11, 0,

                 2,  0,

                 9,  0,

                 11, 0,

                 11, 0,

                 11, 0,

                 8,  0,

                 7,  0,  0};
#define YYTYPE char
struct yywork {
  YYTYPE verify, advance;
} yycrank[] = {
    0,  0,  0,  0,  1,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  6,  1,  7,
    9,  23, 9,  23, 23, 23, 23, 23, 0,  0,  6,  7,  6,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  23,
    1,  5,  23, 23, 0,  0,  0,  0,  6,  7,  0,  0,  0,  0,  1,  8,  1,  9,  1,  10, 4,  22, 1,  11,
    1,  12, 1,  13, 12, 25, 6,  23, 2,  8,  6,  23, 2,  10, 22, 34, 2,  11, 2,  12, 0,  0,  1,  14,
    1,  15, 1,  16, 1,  17, 0,  0,  3,  20, 16, 29, 1,  18, 1,  18, 2,  14, 2,  15, 1,  18, 2,  17,
    3,  20, 3,  21, 1,  18, 24, 28, 0,  0,  0,  0,  1,  19, 1,  18, 0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  18, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  3,  20, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  22, 3,  20, 24, 28,
    0,  0,  0,  0,  0,  0,  3,  20, 11, 24, 11, 24, 11, 24, 11, 24, 11, 24, 11, 24, 11, 24, 11, 24,
    11, 24, 11, 24, 0,  0,  3,  20, 0,  0,  0,  0,  0,  0,  0,  0,  3,  20, 3,  20, 0,  0,  0,  0,
    3,  20, 0,  0,  0,  0,  0,  0,  3,  20, 0,  0,  0,  0,  0,  0,  3,  20, 3,  20, 0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13, 26, 3,  20, 13, 27, 13, 27, 13, 27, 13, 27,
    13, 27, 13, 27, 13, 27, 13, 27, 13, 27, 13, 27, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  28, 35, 0,  0,  28, 35, 0,  0,  13, 28, 28, 36, 28, 36, 28, 36, 28, 36, 28, 36, 28, 36,
    28, 36, 28, 36, 28, 36, 28, 36, 35, 36, 35, 36, 35, 36, 35, 36, 35, 36, 35, 36, 35, 36, 35, 36,
    35, 36, 35, 36, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  13, 28, 18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,
    18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,
    18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,
    18, 30, 0,  0,  0,  0,  0,  0,  0,  0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  18, 0,  0,  0,
    0,  0,  18, 30, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  18, 0,
    18, 0,  18, 31, 18, 0,  18, 0,  18, 0,  0,  0,  18, 30, 18, 30, 0,  0,  0,  0,  18, 30, 0,  0,
    0,  0,  0,  0,  18, 30, 0,  0,  0,  0,  0,  0,  18, 30, 18, 30, 0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  18, 30, 0,  0,  0,  0,  0,  0,  18, 0,  0,  0,  0,  0,
    0,  0,  18, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  18, 0,  0,  0,  0,  0,  18, 0,  19, 0,  19, 0,  19, 0,
    19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,
    19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,
    19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  19, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19, 0,  19, 0,  0,  0,  19, 0,  19, 0,  19, 0,
    0,  0,  19, 32, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19, 33, 0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  19, 0,  0,  0,  0,  0,  0,  0,  19, 0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    19, 0,  0,  0,  0,  0,  19, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,
    30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,
    30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,
    30, 0,  30, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  30, 0,  30, 0,  30, 0,  30, 0,  30, 0,
    30, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  30, 0,  30, 0,  0,  0,  30, 0,  30, 0,  30, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  30, 0,
    0,  0,  0,  0,  0,  0,  30, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  30, 0,  0,  0,  0,  0,  30, 0,  31, 0,
    31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,
    31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,
    31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 37, 0,  0,  0,  0,
    0,  0,  0,  0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  0,  0,  0,  0,  31, 37, 0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  31, 0,  31, 0,  31, 0,  31, 0,
    31, 0,  31, 0,  0,  0,  31, 37, 31, 37, 0,  0,  0,  0,  31, 37, 0,  0,  0,  0,  0,  0,  31, 37,
    0,  0,  0,  0,  0,  0,  31, 37, 31, 37, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  31, 37, 0,  0,  0,  0,  0,  0,  31, 0,  0,  0,  0,  0,  0,  0,  31, 0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  31, 0,  0,  0,  0,  0,  31, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,
    32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,
    32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,  32, 0,
    32, 0,  32, 0,  32, 0,  32, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  32, 0,  32, 0,  32, 0,
    32, 0,  32, 0,  32, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  32, 0,  32, 0,  0,  0,  32, 0,  32, 0,  32, 0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  32, 38, 0,  0,  0,  0,
    0,  0,  32, 0,  0,  0,  0,  0,  0,  0,  32, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  32, 0,  0,  0,  0,  0,
    32, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,
    33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,
    33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  33, 0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33, 0,  33, 0,
    0,  0,  33, 0,  33, 0,  33, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33, 39, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33, 0,  0,  0,  0,  0,  0,  0,
    33, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  33, 0,  0,  0,  0,  0,  33, 0,  37, 0,  37, 0,  37, 0,  37, 0,
    37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,
    37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,
    37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  37, 0,
    37, 0,  37, 0,  37, 0,  37, 0,  37, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  37, 0,  37, 0,  37, 0,  37, 0,  37, 40, 37, 0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  37, 0,  0,  0,  0,  0,  0,  0,  37, 0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  37, 0,
    0,  0,  0,  0,  37, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,
    38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,
    38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,
    38, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,  38, 0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    38, 41, 38, 0,  0,  0,  38, 0,  38, 0,  38, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  38, 0,  0,  0,
    0,  0,  0,  0,  38, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  38, 0,  0,  0,  0,  0,  38, 0,  39, 0,  39, 0,
    39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,
    39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,
    39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  39, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  39, 42, 39, 0,  0,  0,  39, 0,  39, 0,
    39, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  39, 0,  0,  0,  0,  0,  0,  0,  39, 0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  39, 0,  0,  0,  0,  0,  39, 0,  0,  0,  0,  0,  0,  0,  0,  0};
struct yysvf yysvec[] = {0,
                         0,
                         0,
                         yycrank + -1,
                         0,
                         yyvstop + 1,
                         yycrank + -10,
                         yysvec + 1,
                         yyvstop + 3,
                         yycrank + -63,
                         0,
                         yyvstop + 5,
                         yycrank + -4,
                         yysvec + 3,
                         yyvstop + 7,
                         yycrank + 0,
                         0,
                         yyvstop + 9,
                         yycrank + 8,
                         0,
                         yyvstop + 11,
                         yycrank + 0,
                         yysvec + 6,
                         yyvstop + 15,
                         yycrank + 0,
                         0,
                         yyvstop + 18,
                         yycrank + 3,
                         yysvec + 6,
                         yyvstop + 21,
                         yycrank + 0,
                         0,
                         yyvstop + 24,
                         yycrank + 64,
                         0,
                         yyvstop + 27,
                         yycrank + 8,
                         0,
                         yyvstop + 29,
                         yycrank + 104,
                         0,
                         yyvstop + 31,
                         yycrank + 0,
                         0,
                         yyvstop + 34,
                         yycrank + 0,
                         0,
                         yyvstop + 37,
                         yycrank + 4,
                         0,
                         yyvstop + 40,
                         yycrank + 0,
                         0,
                         yyvstop + 43,
                         yycrank + -205,
                         0,
                         yyvstop + 46,
                         yycrank + -332,
                         yysvec + 18,
                         yyvstop + 49,
                         yycrank + 0,
                         0,
                         yyvstop + 52,
                         yycrank + 0,
                         0,
                         yyvstop + 54,
                         yycrank + 8,
                         0,
                         yyvstop + 56,
                         yycrank + 5,
                         yysvec + 6,
                         yyvstop + 58,
                         yycrank + 6,
                         yysvec + 11,
                         yyvstop + 60,
                         yycrank + 0,
                         0,
                         yyvstop + 62,
                         yycrank + 0,
                         yysvec + 11,
                         0,
                         yycrank + 0,
                         yysvec + 13,
                         yyvstop + 64,
                         yycrank + 126,
                         0,
                         0,
                         yycrank + 0,
                         0,
                         yyvstop + 66,
                         yycrank + -459,
                         yysvec + 18,
                         yyvstop + 68,
                         yycrank + -586,
                         0,
                         0,
                         yycrank + -713,
                         yysvec + 18,
                         yyvstop + 70,
                         yycrank + -840,
                         yysvec + 18,
                         yyvstop + 72,
                         yycrank + 0,
                         0,
                         yyvstop + 74,
                         yycrank + 136,
                         0,
                         0,
                         yycrank + 0,
                         yysvec + 35,
                         yyvstop + 76,
                         yycrank + -967,
                         yysvec + 31,
                         0,
                         yycrank + -1094,
                         yysvec + 18,
                         yyvstop + 78,
                         yycrank + -1221,
                         yysvec + 18,
                         yyvstop + 80,
                         yycrank + 0,
                         0,
                         yyvstop + 82,
                         yycrank + 0,
                         0,
                         yyvstop + 84,
                         yycrank + 0,
                         0,
                         yyvstop + 86,
                         0,
                         0,
                         0};
struct yywork *yytop = yycrank + 1348;
struct yysvf *yybgin = yysvec + 1;
char yymatch[] = {
    00,  01,  01,  01,  01,  01,  01,  01,  01,  011, 012, 01,  01,  01,  01,  01,  01,  01,  01,
    01,  01,  01,  01,  01,  01,  01,  01,  01,  01,  01,  01,  01,  011, 01,  01,  '#', '#', '#',
    '#', '#', 01,  01,  01,  '+', 01,  '+', '#', '#', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', 01,  01,  '<', 01,  '<', 01,  '#', 'A', 'B', 'B', 'B', 'E', 'B', 'B', 'B', 'I', 'B', 'B',
    'B', 'M', 'N', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'X', 'B', 'B', '#', 01,  '#', '#',
    '#', 01,  'A', 'B', 'B', 'B', 'E', 'B', 'B', 'B', 'I', 'B', 'B', 'B', 'M', 'N', 'B', 'B', 'B',
    'B', 'B', 'B', 'B', 'B', 'B', 'X', 'B', 'B', '#', 01,  '#', '#', 01,  0};
char yyextra[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#ident "$Header: /disk/disk3/cvsroot/medley/src/lplexyy.c,v 1.2 1999/01/03 02:07:18 sybalsky Exp $"
int yylineno = 1;
#define YYU(x) x
#define NLSTATE yyprevious = YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate[YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
int yyback(int *p, int m) {
  if (p == 0) return (0);
  while (*p) {
    if (*p++ == m) return (1);
  }
  return (0);
}
/* the following are only used in the lex library */
int yyinput() { return (input()); }
void yyoutput(int c) { output(c); }
void yyunput(int c) { unput(c); }

int yylook() {
  struct yysvf *yystate, **lsp;
  struct yywork *yyt;
  struct yysvf *yyz;
  int yych, yyfirst;
  struct yywork *yyr;
#ifdef LEXDEBUG
  int debug;
#endif
  char *yylastch;
/* start off machines */
#ifdef LEXDEBUG
  debug = 0;
#endif
  yyfirst = 1;
  if (!yymorfg)
    yylastch = yytext;
  else {
    yymorfg = 0;
    yylastch = yytext + yyleng;
  }
  for (;;) {
    lsp = yylstate;
    yyestate = yystate = yybgin;
    if (yyprevious == YYNEWLINE) yystate++;
    for (;;) {
#ifdef LEXDEBUG
      if (debug) fprintf(yyout, "state %d\n", yystate - yysvec - 1);
#endif
      yyt = yystate->yystoff;
      if (yyt == yycrank && !yyfirst) { /* may not be any transitions */
        yyz = yystate->yyother;
        if (yyz == 0) break;
        if (yyz->yystoff == yycrank) break;
      }
      *yylastch++ = yych = input();
      yyfirst = 0;
    tryagain:
#ifdef LEXDEBUG
      if (debug) {
        fprintf(yyout, "char ");
        allprint(yych);
        putchar('\n');
      }
#endif
      yyr = yyt;
      if ((int)yyt > (int)yycrank) {
        yyt = yyr + yych;
        if (yyt <= yytop && yyt->verify + yysvec == yystate) {
          if (yyt->advance + yysvec == YYLERR) /* error transitions */
          {
            unput(*--yylastch);
            break;
          }
          *lsp++ = yystate = yyt->advance + yysvec;
          goto contin;
        }
      }
#ifdef YYOPTIM
      else if ((int)yyt < (int)yycrank) { /* r < yycrank */
        yyt = yyr = yycrank + (yycrank - yyt);
#ifdef LEXDEBUG
        if (debug) fprintf(yyout, "compressed state\n");
#endif
        yyt = yyt + yych;
        if (yyt <= yytop && yyt->verify + yysvec == yystate) {
          if (yyt->advance + yysvec == YYLERR) /* error transitions */
          {
            unput(*--yylastch);
            break;
          }
          *lsp++ = yystate = yyt->advance + yysvec;
          goto contin;
        }
        yyt = yyr + YYU(yymatch[yych]);
#ifdef LEXDEBUG
        if (debug) {
          fprintf(yyout, "try fall back character ");
          allprint(YYU(yymatch[yych]));
          putchar('\n');
        }
#endif
        if (yyt <= yytop && yyt->verify + yysvec == yystate) {
          if (yyt->advance + yysvec == YYLERR) /* error transition */
          {
            unput(*--yylastch);
            break;
          }
          *lsp++ = yystate = yyt->advance + yysvec;
          goto contin;
        }
      }
      if ((yystate = yystate->yyother) && (yyt = yystate->yystoff) != yycrank) {
#ifdef LEXDEBUG
        if (debug) fprintf(yyout, "fall back to state %d\n", yystate - yysvec - 1);
#endif
        goto tryagain;
      }
#endif
      else {
        unput(*--yylastch);
        break;
      }
    contin:
#ifdef LEXDEBUG
      if (debug) {
        fprintf(yyout, "state %d char ", yystate - yysvec - 1);
        allprint(yych);
        putchar('\n');
      }
#endif
      ;
    }
#ifdef LEXDEBUG
    if (debug) {
      fprintf(yyout, "stopped at %d with ", *(lsp - 1) - yysvec - 1);
      allprint(yych);
      putchar('\n');
    }
#endif
    while (lsp-- > yylstate) {
      *yylastch-- = 0;
      if (*lsp != 0 && (yyfnd = (*lsp)->yystops) && *yyfnd > 0) {
        yyolsp = lsp;
        if (yyextra[*yyfnd]) { /* must backup */
          while (yyback((*lsp)->yystops, -*yyfnd) != 1 && lsp > yylstate) {
            lsp--;
            unput(*yylastch--);
          }
        }
        yyprevious = YYU(*yylastch);
        yylsp = lsp;
        yyleng = yylastch - yytext + 1;
        yytext[yyleng] = 0;
#ifdef LEXDEBUG
        if (debug) {
          fprintf(yyout, "\nmatch ");
          sprint(yytext);
          fprintf(yyout, " action %d\n", *yyfnd);
        }
#endif
        return (*yyfnd++);
      }
      unput(*yylastch);
    }
    if (yytext[0] == 0 /* && feof(yyin) */) {
      yysptr = yysbuf;
      return (0);
    }
    yyprevious = yytext[0] = input();
    if (yyprevious > 0) output(yyprevious);
    yylastch = yytext;
#ifdef LEXDEBUG
    if (debug) putchar('\n');
#endif
  }
}
#endif
