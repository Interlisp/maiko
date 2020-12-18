/* $Id: cdrom.c,v 1.3 1999/05/31 23:35:26 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***********************************

        file: cdrom.c

***********************************/

#include <stdio.h>
#include <strings.h>
#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "arith.h"
#include "cdrom.h"

int cdrom(LispPTR *args) {
  int request;
  int res;

  N_GETNUMBER(args[0], request, BAD_ARG);

#ifdef DEBUG
  printf("CD-ROM function called. function = %d\n", request);
#endif

  switch (request) {
    case CDROM_INIT_DRV: res = cdrom_init_drv(args); break;
    case CDROM_KEN_INT: res = cdrom_ken_int(args); break;
    case CDROM_CHOSAKU: res = cdrom_chosaku(args); break;
    case CDROM_MIKANA: res = cdrom_mikana(args); break;
    case CDROM_MIKANAT: res = cdrom_mikanat(args); break;
    case CDROM_SYURYO: res = cdrom_syuryo(args); break;
    default: return (NIL); break;
  }
#ifdef DEBUG
  printf("Result = %d\n", res);
#endif
  if (res == 0) {
    return (ATOM_T);
  } else {
    return (GetSmallp(res));
  }

BAD_ARG:
  return (NIL);
}

static int cdrom_init_drv(LispPTR *args)

/*
        args[0] function number
        args[1] pointer to buffer
        args[2] device name
*/
{
  int res;
  char *buff;
  char drive[80];
  LispPTR *naddress;
  char *base;
  int offset;

  if (GetTypeNumber(args[1]) == TYPE_ONED_ARRAY) {
    naddress = (LispPTR *)(Addr68k_from_LADDR(args[1]));
    base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));
    offset = (int)(((OneDArray *)naddress)->offset);
    buff = &base[offset];
  } else
    return (1);
  LispStringToCString(args[2], drive, 80);

  return (init_drv(buff, drive));
}

static int cdrom_ken_int(LispPTR *args)

/*
        args[0] function number
        args[1] shoseki number
        args[2] flag for index and sort
*/
{
  int bunno;
  int flg;

  N_GETNUMBER(args[1], bunno, BAD_ARG);
  N_GETNUMBER(args[2], flg, BAD_ARG);
#ifdef DEBUG
  printf("bunno = %x\n", bunno);
  printf("flg = %d\n", flg);
#endif
  return (ken_int(bunno, flg));

BAD_ARG:
  return (1);
}

static int cdrom_chosaku(LispPTR *args)

/*
        args[0] function number
        args[1] buffer for copy right data
        args[2] real data size
*/
{
  char *buff;
  int size;
  int *data_size;
  LispPTR *naddress;
  char *base;
  int offset;

  if (GetTypeNumber(args[1]) == TYPE_ONED_ARRAY) {
    naddress = (LispPTR *)(Addr68k_from_LADDR(args[1]));
    base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));
    offset = (int)(((OneDArray *)naddress)->offset);
    buff = &base[offset];
  } else
    return (1);
  size = (int)(((OneDArray *)naddress)->totalsize);
#ifdef DEBUG
  printf("size = %d\n", size);
#endif
  data_size = (int *)(Addr68k_from_LADDR(args[2]));
  return (chosaku(buff, size, data_size));
}

static int cdrom_mikana(LispPTR *args)

/*
        args[0] function number
        args[1] relative id number
        args[2] pointer to retrieve data
        args[3] size of retrieve data
*/
{
  int hyono;
  char *buff;
  int buff_size;
  int *data_size;
  LispPTR *naddress;
  char *base;
  int offset;

  N_GETNUMBER(args[1], hyono, BAD_ARG);
  if (GetTypeNumber(args[2]) == TYPE_ONED_ARRAY) {
    naddress = (LispPTR *)(Addr68k_from_LADDR(args[2]));
    base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));
    offset = (int)(((OneDArray *)naddress)->offset);
    buff = &base[offset];
  } else
    return (1);
  buff_size = (int)(((OneDArray *)naddress)->totalsize);
  data_size = (int *)(Addr68k_from_LADDR(args[3]));
  return (mikana(hyono, buff, buff_size, data_size));

BAD_ARG:
  return (1);
}

static int cdrom_mikanat(LispPTR *args)

/*
        args[0] function number
        args[1] pointer to a search key
        args[2] pointer to buffer
        args[3] size of searched data
        args[4] nuber of items matched to the search key
*/
{
  char key[65];
  char *buff;
  int buff_size;
  int *data_size;
  int *hitn;
  LispPTR *naddress;
  char *base;
  int offset;

  LispStringToCString2(args[1], key, 65);
  if (GetTypeNumber(args[2]) == TYPE_ONED_ARRAY) {
    naddress = (LispPTR *)(Addr68k_from_LADDR(args[2]));
    base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));
    offset = (int)(((OneDArray *)naddress)->offset);
    buff = &base[offset];
  } else
    return (1);
  buff_size = (((OneDArray *)naddress)->totalsize);
  data_size = (int *)(Addr68k_from_LADDR(args[3]));
  hitn = (int *)(Addr68k_from_LADDR(args[4]));

  return (mikanat(key, buff, buff_size, data_size, hitn));
}

static int cdrom_syuryo(LispPTR *args)

/*
        args[0] function number
*/
{
  return (syuryo());
}
