/* $Id: ejlisp.c,v 1.2 1999/01/03 02:06:58 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: ejlisp.c,v 1.2 1999/01/03 02:06:58 sybalsky Exp $ Copyright (C) Venue";

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

#include <stdio.h>
#include <sys/param.h>

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "arith.h"
#include "locfile.h"

#ifdef FPO
#define WCHAR_T
#endif

#include "jllib.h"
#include "jslib.h"

#include "timeout.h"

#define CHAR_MAXLEN 512
#define TABLE_MAX 100
#define CHAR_SIZE CHAR_MAXLEN *(sizeof(char) / sizeof(int))

#define LStringToCString(Lisp, C, MaxLen, Len)                                                     \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int i, j;                                                                                      \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((unsigned int)Lisp));                                \
    Len = min(MaxLen, arrayp->totalsize);                                                          \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base =                                                                                     \
            ((char *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset));  \
        for (i = 0; i < Len; i++) { C[i] = base[i]; }                                              \
        C[Len] = '\0';                                                                             \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase =                                                                                    \
            ((short *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (i = 0; i < Len * 2; i++) C[i] = base[i];                                              \
        C[Len * 2] = '\0';                                                                         \
        break;                                                                                     \
                                                                                                   \
      default: error("LStringToCString can not handle\n");                                         \
    }                                                                                              \
  }

#define CStringToLString(C, Lisp, Len)                                                             \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int id;                                                                                        \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((unsigned int)Lisp));                                \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base =                                                                                     \
            ((char *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset));  \
        for (id = 0; id < Len; id++) base[id] = C[id];                                             \
        arrayp->fillpointer = Len;                                                                 \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase =                                                                                    \
            ((short *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (id = 0; id < Len * 2; id++) base[id] = C[id];                                         \
        arrayp->fillpointer = Len;                                                                 \
        break;                                                                                     \
                                                                                                   \
      default: error("CStringToLString can not handle\n");                                         \
    }                                                                                              \
  }

#define IntToFixp(C, Lisp)                                \
  {                                                       \
    int *base;                                            \
                                                          \
    base = (int *)Addr68k_from_LADDR((unsigned int)Lisp); \
    *base = C;                                            \
  }

#define ZeroFix(C)                                       \
  {                                                      \
    int k, j;                                            \
                                                         \
    if (C[0] == 0 && C[1] != 0) {                        \
      for (k = j = 0; C[k] != 0 || C[k + 1] != 0; k++) { \
        if (C[k] != 0) C[j++] = C[k];                    \
      }                                                  \
      C[j] = 0;                                          \
    }                                                    \
  }

#define EJLISP_SETJMP(x)       \
  {                            \
    if (setjmp(jmpbuf) != 0) { \
      ejlisp_buf = NULL;       \
      return (x);              \
    }                          \
  }

typedef struct first_array {
  unsigned int orig : 1;
  unsigned int nil1 : 1;
  unsigned int rdonly : 1;
  unsigned int nil2 : 1;
  unsigned int type : 4;
  unsigned int base : 24;
  unsigned short length;
  unsigned short offset;
} FirstArray;

struct wnn_buf *ejlisp_buf = NULL;
struct wnn_jdata *ejlisp_jdp;
int ejlisp_jdp_num;
int ejlisp_kouho_max;
int ejlisp_kouho_num;
int ejlisp_kouho_next;

#define WNN_OPEN 1
#define WNN_CLOSE 2
#define WNN_CONV 3
#define WNN_RECONV 4
#define WNN_SELECT_BUNSETSU 5
#define WNN_GET_KOUHO 6
#define WNN_SELECT_KOUHO 7
#define WNN_GET_YOMI 8
#define WNN_CONV_END 9
#define WNN_ADD_USERDIC 10
#define WNN_DEL_USERDIC 11
#define WNN_SEARCH_USERDIC 12
#define WNN_GET_USERDIC 13
#define WNN_GET_USERDIC_NAME 14
#define WNN_BUNSETSU_KANJI 15
#define WNN_BUNSETSU_YOMI 16

ejlisp(int args[])
{
  int result;
  int i, j, length;

  int c_number1, c_number2, c_number3, c_number4;
  int c_number5[TABLE_MAX], c_number6[TABLE_MAX];

  unsigned char c_char1[(CHAR_MAXLEN + 1) * 2];
  unsigned char c_char2[(CHAR_MAXLEN + 1) * 2];
  unsigned char c_char3[TABLE_MAX][(CHAR_MAXLEN + 1) * 2];
  unsigned char tmp[(CHAR_MAXLEN + 1) * 2];

  FirstArray *a_ptr;
  unsigned int *base1;
  unsigned int *base2;
  unsigned int *base3;

  N_GETNUMBER(args[0], c_number1, ERROR);

#ifdef DEBUG
  printf("ejlisp start\n");
  printf("case = %d\n", c_number1);
#endif

  switch (c_number1) {
    case WNN_OPEN:
      LStringToCString(args[1], c_char1, CHAR_MAXLEN, length);
      N_GETNUMBER(args[2], c_number1, ERROR);
      LStringToCString(args[3], c_char2, CHAR_MAXLEN, length);
      result = ejlisp_open(c_char1, c_number1, c_char2);
      break;

    case WNN_CLOSE: result = ejlisp_close(); break;

    case WNN_CONV:
      LStringToCString(args[1], tmp, CHAR_MAXLEN, length);
      if (strlen(tmp) == length) {
        for (i = length - 1; i >= 0; i--) {
          tmp[i * 2 + 1] = tmp[i];
          tmp[i * 2] = 0;
        }
        tmp[length * 2] = '\0';
      }
      length = FatcharNStoEUC(tmp, length, c_char1);
      for (i = length; i < (CHAR_MAXLEN + 1) * 2; i++) c_char1[i] = NULL;

      result = ejlisp_conv(c_char1, c_char2, &c_number1, &c_number2);
      if (result != 0) break;

      ZeroFix(c_char2);
      length = EUCtoFatcharNS(c_char2, tmp);
      CStringToLString(tmp, args[2], length);

      IntToFixp(c_number1, args[3]);
      IntToFixp(c_number2, args[4]);

      break;

    case WNN_RECONV:
      N_GETNUMBER(args[1], c_number1, ERROR);
      N_GETNUMBER(args[2], c_number2, ERROR);

      result = ejlisp_reconv(c_number1, c_number2, c_char1, &c_number3, &c_number4);
      if (result != 0) break;

      ZeroFix(c_char1);
      length = EUCtoFatcharNS(c_char1, tmp);
      CStringToLString(tmp, args[3], length);

      IntToFixp(c_number3, args[4]);
      IntToFixp(c_number4, args[5]);

      break;

    case WNN_SELECT_BUNSETSU:
      N_GETNUMBER(args[1], c_number1, ERROR);

      result = ejlisp_select_bunsetsu(c_number1, &c_number2);
      if (result != 0) break;

      IntToFixp(c_number2, args[2]);

      break;

    case WNN_GET_KOUHO:
      N_GETNUMBER(args[1], c_number1, ERROR);

      result = ejlisp_get_kouho(c_number1, c_char3, &c_number2);
      if (result != 0) break;

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[2]));
      base1 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      for (i = 0; i < ejlisp_kouho_max; i++, base1++, base2++) {
        ZeroFix(c_char3[i]);
        length = EUCtoFatcharNS(c_char3[i], tmp);
        CStringToLString(tmp, *base1, length);
      }

      IntToFixp(c_number2, args[3]);

      break;

    case WNN_SELECT_KOUHO:
      N_GETNUMBER(args[1], c_number1, ERROR);

      result = ejlisp_select_kouho(c_number1, c_char1);
      if (result != 0) break;

      ZeroFix(c_char1);
      length = EUCtoFatcharNS(c_char1, tmp);
      CStringToLString(tmp, args[2], length);

      IntToFixp(length, args[3]);

      break;

    case WNN_GET_YOMI:
      N_GETNUMBER(args[1], c_number1, ERROR);

      result = ejlisp_get_yomi(c_number1, c_char1, &c_number2);
      if (result != 0) break;

      ZeroFix(c_char1);
      length = EUCtoFatcharNS(c_char1, tmp);
      CStringToLString(tmp, args[2], length);

      IntToFixp(c_number2, args[3]);

      break;

    case WNN_CONV_END:
      result = ejlisp_conv_end();
      if (result != 0) break;
      break;

    case WNN_ADD_USERDIC:
      N_GETNUMBER(args[1], c_number1, ERROR);

      LStringToCString(args[2], tmp, CHAR_MAXLEN, length);
      if (strlen(tmp) == length) {
        for (i = length - 1; i >= 0; i--) {
          tmp[i * 2 + 1] = tmp[i];
          tmp[i * 2] = 0;
        }
        tmp[length * 2] = '\0';
      }
      length = FatcharNStoEUC(tmp, length, c_char1);
      for (i = length; i < (CHAR_MAXLEN + 1) * 2; i++) c_char1[i] = NULL;

      LStringToCString(args[3], tmp, CHAR_MAXLEN, length);
      if (strlen(tmp) == length) {
        for (i = length - 1; i >= 0; i--) {
          tmp[i * 2 + 1] = tmp[i];
          tmp[i * 2] = 0;
        }
        tmp[length * 2] = '\0';
      }
      length = FatcharNStoEUC(tmp, length, c_char2);
      for (i = length; i < (CHAR_MAXLEN + 1) * 2; i++) c_char2[i] = NULL;

      N_GETNUMBER(args[4], c_number2, ERROR);

      result = ejlisp_add_userdic(c_number1, c_char1, c_char2, c_number2);
      if (result != 0) break;

      break;

    case WNN_DEL_USERDIC:
      N_GETNUMBER(args[1], c_number1, ERROR);
      N_GETNUMBER(args[2], c_number2, ERROR);

      result = ejlisp_del_userdic(c_number1, c_number2);
      if (result != 0) break;

      break;

    case WNN_SEARCH_USERDIC:
      N_GETNUMBER(args[1], c_number1, ERROR);

      LStringToCString(args[2], tmp, CHAR_MAXLEN, length);
      if (strlen(tmp) == length) {
        for (i = length - 1; i >= 0; i--) {
          tmp[i * 2 + 1] = tmp[i];
          tmp[i * 2] = 0;
        }
        tmp[length * 2] = '\0';
      }
      length = FatcharNStoEUC(tmp, length, c_char1);
      for (i = length; i < (CHAR_MAXLEN + 1) * 2; i++) c_char1[i] = NULL;

      result = ejlisp_search_userdic(c_number1, c_char1);
      if (result != 0) break;

      IntToFixp(ejlisp_jdp_num, args[3]);

      break;

    case WNN_GET_USERDIC:

      result = ejlisp_get_userdic(c_char3, c_number5, c_number6);
      if (result != 0) break;

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[1]));
      base1 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[2]));
      base2 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[3]));
      base3 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      for (i = 0; i < ejlisp_jdp_num; i++, base1++, base2++, base3++) {
        ZeroFix(c_char3[i]);
        length = EUCtoFatcharNS(c_char3[i], tmp);
        CStringToLString(tmp, *base1, length);
        *base2 = length;
        *base3 = c_number6[i];
      }

      break;

    case WNN_GET_USERDIC_NAME:

      result = ejlisp_get_userdic_name(c_char3, c_number5, c_number6);
      if (result != 0) break;

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[1]));
      base1 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[2]));
      base2 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      a_ptr = (FirstArray *)(Addr68k_from_LADDR((unsigned int)args[3]));
      base3 =
          ((unsigned int *)(Addr68k_from_LADDR((unsigned int)a_ptr->base))) + (int)(a_ptr->offset);

      for (i = 0; c_char3[i][0] != NULL; i++, base1++, base2++, base3++) {
        ZeroFix(c_char3[i]);
        length = EUCtoFatcharNS(c_char3[i], tmp);
        CStringToLString(tmp, *base1, length);
        *base2 = c_number5[i];
        *base3 = c_number6[i];
      }

      break;

    case WNN_BUNSETSU_KANJI:
      N_GETNUMBER(args[1], c_number1, ERROR);
      N_GETNUMBER(args[2], c_number2, ERROR);

      result = ejlisp_bunsetsu_kanji(c_number1, c_number2, &c_number3);
      if (result != 0) break;

      IntToFixp(c_number3, args[3]);
      break;

    case WNN_BUNSETSU_YOMI:
      N_GETNUMBER(args[1], c_number1, ERROR);
      N_GETNUMBER(args[2], c_number2, ERROR);

      result = ejlisp_bunsetsu_yomi(c_number1, c_number2, &c_number3);
      if (result != 0) break;

      IntToFixp(c_number3, args[3]);
      break;

    ERROR:
      result = 9999;
      break;
  }

#ifdef DEBUG
  printf("ejlisp end\n");
  printf("result = %d\n", result);
#endif

  return (GetSmallp(result));
}

int ejlisp_add_userdic(int dic_no, unsigned short *kanji, unsigned short *yomi, int hinshi)
{
  int result;
#ifdef DEBUG
  printf("ejlisp_add_userdic start\n");
  printf("dic_no = %d,kanji = %s,yomi = %s,hinshi = %d\n", dic_no, kanji, yomi, hinshi);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(result = jl_word_add(ejlisp_buf, dic_no, yomi, kanji, NULL, hinshi, 0));
  if (result == 0) { TIMEOUT(jl_dic_save(ejlisp_buf, dic_no)); }

#ifdef DEBUG
  printf("ejlisp_add_userdic end\n");
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_bunsetsu_kanji(int bunsetsu_no, int bunsetsu_no2, int *kanji_len)
{
#ifdef DEBUG
  printf("ejlisp_bunsetsu_kanji start\n");
  printf("bunsetsu_no = %d\n", bunsetsu_no);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*kanji_len = jl_kanji_len(ejlisp_buf, bunsetsu_no, bunsetsu_no2));

#ifdef DEBUG
  printf("ejlisp_bunsetsu_kanji end\n");
  if (wnn_errorno == 0) printf("kanji_len = %d\n", *kanji_len);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_bunsetsu_yomi(int bunsetsu_no, int bunsetsu_no2, int *yomi_len)
{
#ifdef DEBUG
  printf("ejlisp_bunsetsu_yomi start\n");
  printf("bunsetsu_no = %d\n", bunsetsu_no);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*yomi_len = jl_yomi_len(ejlisp_buf, bunsetsu_no, bunsetsu_no2));

#ifdef DEBUG
  printf("ejlisp_bunsetsu_yomi end\n");
  printf("yomi_len = %d\n", *yomi_len);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_close() {
#ifdef DEBUG
  printf("ejlisp_close start\n");
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(jl_close(ejlisp_buf));
  ejlisp_buf = NULL;

#ifdef DEBUG
  printf("ejlisp_close end\n");
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_conv(unsigned short *yomi, unsigned short *kanji, int *kanji_len, int *bunsetsu_suu)
{
#ifdef DEBUG
  printf("ejlisp_conv start\n");
  printf("yomi = %s\n", yomi);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*bunsetsu_suu = jl_ren_conv(ejlisp_buf, yomi, 0, -1, WNN_USE_MAE));
  if (*bunsetsu_suu != -1) { TIMEOUT(*kanji_len = jl_get_kanji(ejlisp_buf, 0, -1, kanji)); }

#ifdef DEBUG
  printf("ejlisp_conv end\n");
  if (wnn_errorno == 0)
    printf("kanji = %s, kanji_len = %d, bunsetsu_suu = %d\n", kanji, *kanji_len, *bunsetsu_suu);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_conv_end() {
#ifdef DEBUG
  printf("ejlisp_conv_end start\n");
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(jl_update_hindo(ejlisp_buf, 0, 99));
  if (wnn_errorno == 0) { TIMEOUT(jl_dic_save_all(ejlisp_buf)); }

#ifdef DEBUG
  printf("ejlisp_conv_end end\n");
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_del_userdic(int dic_no, int entry) {
#ifdef DEBUG
  printf("ejlisp_del_userdic start\n");
  printf("dic_no = %d,entry = %d\n", dic_no, entry);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(jl_word_delete(ejlisp_buf, dic_no, entry));
  if (wnn_errorno == 0) { TIMEOUT(jl_dic_save(ejlisp_buf, dic_no)); }

#ifdef DEBUG
  printf("ejlisp_del_userdic end\n");
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_get_kouho(int pos, unsigned short kouho_arrey[TABLE_MAX][CHAR_MAXLEN + 1],
                 int *kouho_num)
{
  int i, j;

#ifdef DEBUG
  int num;

  printf("ejlisp_get_kouho start\n");
  printf("pos = %d\n", pos);
  num = ejlisp_kouho_next;

#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  i = ejlisp_kouho_next;
  if (pos < 0) {
    if ((i -= ejlisp_kouho_max) < 0) i = ejlisp_kouho_num - (ejlisp_kouho_num % ejlisp_kouho_max);
    if ((i -= ejlisp_kouho_max) < 0) i = ejlisp_kouho_num - (ejlisp_kouho_num % ejlisp_kouho_max);
  }

  for (j = 0; j < ejlisp_kouho_max && i < ejlisp_kouho_num; i++, j++) {
    TIMEOUT(jl_get_zenkouho_kanji(ejlisp_buf, i, &kouho_arrey[j][0]));
    if (wnn_errorno != 0) break;
  }
  if (wnn_errorno == 0) {
    *kouho_num = j;
    for (; j < ejlisp_kouho_max; j++) {
      kouho_arrey[j][0] = NULL;
      kouho_arrey[j][1] = NULL;
    }

    if (i >= ejlisp_kouho_num)
      ejlisp_kouho_next = 0;
    else
      ejlisp_kouho_next = i;
  }

#ifdef DEBUG
  printf("ejlisp_get_kouho end\n");
  if (wnn_errorno == 0) {
    for (i = 0; i < j; i++) printf("kouho[%d] = %s\n", i + num, kouho_arrey[i]);
    printf("kouho_num = %d\n", *kouho_num);
  }
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_get_userdic(unsigned short kouho_arrey[TABLE_MAX][CHAR_MAXLEN + 1], int kouho_len_arrey[TABLE_MAX], int kouho_entry_arrey[TABLE_MAX])
{
  int i;

#ifdef DEBUG
  printf("ejlisp_get_userdic start\n");
#endif

  if (ejlisp_buf == NULL) return (72);

  for (i = 0; i < ejlisp_jdp_num; i++, ejlisp_jdp++) {
    strcpy(&kouho_arrey[i][0], ejlisp_jdp->kanji);
    kouho_len_arrey[i] = strlen(kouho_arrey[i]) / 2;
    kouho_entry_arrey[i] = ejlisp_jdp->serial;
  }

#ifdef DEBUG
  printf("ejlisp_get_userdic end\n");
  for (i = 0; i < ejlisp_jdp_num; i++)
    printf("kouho[%d] = %d,%s,%d\n", i, kouho_len_arrey[i], kouho_arrey[i], kouho_entry_arrey[i]);
#endif

  return (0);
}

int ejlisp_get_userdic_name(unsigned short dic_name_array[TABLE_MAX][CHAR_MAXLEN + 1], int dic_name_len_array[TABLE_MAX], int dic_no_array[TABLE_MAX])
{
  WNN_DIC_INFO *dip;
  int i, j;
  int dic_num;

#ifdef DEBUG
  printf("ejlisp_get_userdic_name start\n");
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(dic_num = jl_dic_list(ejlisp_buf, &dip));
  if (wnn_errorno == 0) {
    for (i = j = 0; i < dic_num; i++, dip++) {
      if (dip->fname[0] == '!') {
        strcpy(&dic_name_array[j][0], &(dip->fname[1]));
        dic_name_len_array[j] = strlen(&dic_name_array[j][0]);
        dic_no_array[j] = dip->dic_no;
        j++;
      }
    }
    dic_name_array[j][0] = NULL;
  }

#ifdef DEBUG
  printf("ejlisp_get_userdic_name end\n");
  if (wnn_errorno == 0) {
    for (i = 0; i < j; i++) {
      printf("dic_no_array = %d,dic_name_len_array = %d,dic_name_array = %s\n", dic_no_array[i],
             dic_name_len_array[i], &dic_name_array[i][0]);
    }
  }
  printf("wnn_errorno = %d", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_get_yomi(int bunsetsu_no, char *yomi, int *yomi_len)
{
#ifdef DEBUG
  printf("ejlisp_get_yomi start\n");
  printf("bunsetsu_no = %d\n", bunsetsu_no);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*yomi_len = jl_get_yomi(ejlisp_buf, bunsetsu_no, bunsetsu_no + 1, yomi));

#ifdef DEBUG
  printf("ejlisp_get_yomi end\n");
  if (wnn_errorno == 0) printf("yomi = %s,yomi_len = %d\n", yomi, *yomi_len);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_open(char *wnn_env, int kouho_max, char *server_name)
{
  char username[L_cuserid];
  char *env_name;

#ifdef DEBUG
  printf("ejlisp_open start\n");
  printf("wnn_env = %s, kouho_max = %d,server_name = %s\n", wnn_env, kouho_max, server_name);
#endif

  EJLISP_SETJMP(-1);

  wnn_errorno = 0;

  if (ejlisp_buf != NULL) {
    TIMEOUT(jl_close(ejlisp_buf));
    ejlisp_buf = NULL;
  }

  if (wnn_errorno == 0) {
    env_name = cuserid(username);
    TIMEOUT(ejlisp_buf = jl_open(env_name, server_name, wnn_env, WNN_CREATE, NULL, NULL, 20));

    if (ejlisp_buf != NULL) { ejlisp_kouho_max = kouho_max; }
  }

#ifdef DEBUG
  printf("ejlisp_open end\n");
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_reconv(int bunsetsu_no, int bunsetsu_len, unsigned short kanji[], int *kanji_len, int *bunsetsu_suu)
{
#ifdef DEBUG
  printf("ejlisp_reconv start\n");
  printf("bunsetsu_no = %d, bunsetsu_len = %d\n", bunsetsu_no, bunsetsu_len);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*bunsetsu_suu =
              jl_nobi_conv(ejlisp_buf, bunsetsu_no, bunsetsu_len, -1, WNN_USE_MAE, WNN_SHO));
  if (wnn_errorno == 0) { TIMEOUT(*kanji_len = jl_get_kanji(ejlisp_buf, 0, -1, kanji)); }

#ifdef DEBUG
  printf("ejlisp_reconv end\n");
  if (wnn_errorno == 0)
    printf("kanji = %s, kanji_len = %d, bunsetsu_suu = %d\n", kanji, *kanji_len, *bunsetsu_suu);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_search_userdic(int dic_no, unsigned short *yomi)
{
#ifdef DEBUG
  printf("ejlisp_search_userdic start\n");
  printf("dic_no = %d,yomi = %s\n", dic_no, yomi);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(ejlisp_jdp_num = jl_word_search(ejlisp_buf, dic_no, yomi, &ejlisp_jdp));

#ifdef DEBUG
  printf("ejlisp_search_userdic end\n");
  if (wnn_errorno == 0) printf("kouho_no = %d\n", ejlisp_jdp_num);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_select_bunsetsu(int bunsetsu_no, int *bunsetsu_len)
{
#ifdef DEBUG
  printf("ejlisp_select_bunsetsu start\n");
  printf("bunsetsu_no = %d\n", bunsetsu_no);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  wnn_errorno = 0;
  TIMEOUT(*bunsetsu_len = jl_kanji_len(ejlisp_buf, bunsetsu_no, bunsetsu_no + 1));
  if (wnn_errorno == 0) { TIMEOUT(jl_zenkouho(ejlisp_buf, bunsetsu_no, WNN_USE_MAE, WNN_NO_UNIQ)); }
  if (wnn_errorno == 0) {
    TIMEOUT(ejlisp_kouho_num = jl_zenkouho_suu(ejlisp_buf));
    ejlisp_kouho_next = 0;
  }

#ifdef DEBUG
  printf("ejlisp_select_bunsetsu end\n");
  if (wnn_errorno == 0) printf("bunsetsu_len = %d\n", *bunsetsu_len);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}

int ejlisp_select_kouho(int kouho_no, unsigned short *kanji)
{
  int no, i, c;

#ifdef DEBUG
  printf("ejlisp_select_kouho start\n");
  printf("kouho_no = %d\n", kouho_no);
#endif

  if (ejlisp_buf == NULL) return (72);
  EJLISP_SETJMP(-1);

  if (kouho_no != 0) {
    if (kouho_no < 0)
      no = ejlisp_kouho_num + kouho_no;
    else if (ejlisp_kouho_num <= ejlisp_kouho_max)
      no = kouho_no - 1;
    else if (ejlisp_kouho_next == 0)
      no = ejlisp_kouho_num - (ejlisp_kouho_num % ejlisp_kouho_max) + kouho_no - 1;
    else
      no = ejlisp_kouho_next - ejlisp_kouho_max + kouho_no - 1;
    wnn_errorno = 0;
    TIMEOUT(jl_set_jikouho(ejlisp_buf, no));
  }
  if (wnn_errorno == 0) { TIMEOUT(jl_get_zenkouho_kanji(ejlisp_buf, no, kanji)); }

#ifdef DEBUG
  printf("ejlisp_select_kouho end\n");
  if (wnn_errorno == 0) printf("kanji = %s\n", kanji);
  printf("wnn_errorno = %d\n", wnn_errorno);
#endif

  return (wnn_errorno);
}
