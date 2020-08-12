/* $Id: cdaudio.c,v 1.3 1999/05/31 23:35:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: cdaudio.c,v 1.3 1999/05/31 23:35:25 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***********************************

        file: cdaudio.c

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

int cdaudio(LispPTR *args) {
  int request;
  int res;

  N_GETNUMBER(args[0], request, BAD_ARG);

#ifdef DEBUG
  printf("CD-ROM function called. function = %d\n", request);
#endif

  switch (request) {
    case CD_OPEN: res = cd_open(args); break;
    case CD_CLOSE: res = cd_close(args); break;
    case CD_READ: res = cd_read(args); break;
    case CD_DISK_INFO: res = cd_disk_info(args); break;
    case CD_TRACK_INFO: res = cd_track_info(args); break;
    case CD_START: res = cd_start(args); break;
    case CD_STOP: res = cd_stop(args); break;
    case CD_PLAY: res = cd_play(args); break;
    case CD_Q_READ: res = cd_q_read(args); break;
    case CD_PAUSE: res = cd_pause(args); break;
    case CD_RESUME: res = cd_resume(args); break;
    case CD_VOLUME: res = cd_volume(args); break;
    case CD_EJECT: res = cd_eject(args); break;
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

int cd_open(LispPTR *args)

/*
        args[0] function number
        args[1] CD-ROM drive path name string
*/
{
  char drive[80];

  LispStringToCString(args[1], drive, 80);
#ifdef DEBUG
  printf("cd_open called. drive = %s\n", drive);
#endif
  return (CDopen(drive));
}

int cd_close(LispPTR *args)

/*
        args[0] function number
*/
{
#ifdef DEBUG
  printf("cd_close is called\n");
#endif
  return (CDclose());
}

int cd_read(LispPTR *args)

/*
        args[0] function number
        args[1] Logical block number
        args[2] number of blocks to read
        args[3] data buffer
*/
{
  DWORD blk;
  int num;
  BYTE *buf;
  LispPTR *naddress;
  char *base;
  int offset;

  switch (GetTypeNumber(args[1])) {
    case TYPE_SMALLP: N_GETNUMBER(args[1], blk, BAD_ARG); break;
    case TYPE_FIXP: blk = *(DWORD *)(Addr68k_from_LADDR(args[1])); break;
    default: return (1); break;
  }
  switch (GetTypeNumber(args[2])) {
    case TYPE_SMALLP: N_GETNUMBER(args[2], num, BAD_ARG); break;
    case TYPE_FIXP: num = *(DWORD *)(Addr68k_from_LADDR(args[2])); break;
    default: return (1); break;
  }
  if (GetTypeNumber(args[3]) == TYPE_ONED_ARRAY) {
    naddress = (LispPTR *)(Addr68k_from_LADDR(args[3]));
    base = (char *)(Addr68k_from_LADDR(((OneDArray *)naddress)->base));
    offset = (int)(((OneDArray *)naddress)->offset);
    buf = base + offset;
  } else
    return (1);
#ifdef DEBUG
  printf("call CDread()\n");
  printf("blk = %d, num = %d buff = %d\n", blk, num, buf);
#endif
  return (CDread(blk, num, buf));
BAD_ARG:
  return (1);
}

int cd_disk_info(LispPTR *args)

/*
        args[0] function number
        args[1] min tune number
        args[2] max tune number
*/
{
  BYTE min_no, max_no;
  int res;

  res = CDdisk_info(&min_no, &max_no);
#ifdef DEBUG
  printf("min = %d, max = %d", min_no, max_no);
#endif
  *(int *)(Addr68k_from_LADDR(args[1])) = (int)min_no;
  *(int *)(Addr68k_from_LADDR(args[2])) = (int)max_no;
  return (res);
}

int cd_track_info(LispPTR *args)

/*
        args[0] function number (in: smallp)
        args[1] tune number (in: fixp)
        args[2] start address(LBN) (out: fixp)
        args[3] control data (out: fixp)
*/
{
  int tno;
  DWORD blk;
  BYTE cntl;
  int res;

  switch (GetTypeNumber(args[1])) {
    case TYPE_SMALLP: N_GETNUMBER(args[1], tno, BAD_ARG); break;
    case TYPE_FIXP: tno = *(DWORD *)(Addr68k_from_LADDR(args[1])); break;
  }
  res = CDtrack_info(tno, &blk, &cntl);
  *(int *)(Addr68k_from_LADDR(args[2])) = (int)blk;
  *(int *)(Addr68k_from_LADDR(args[3])) = (int)cntl;
  return (res);

BAD_ARG:
  return (-1);
}

int cd_start(LispPTR *args)

/*
        args[0] function number
*/
{
  return (CDstart());
}

int cd_stop(LispPTR *args)

/*
        args[0] function number
*/
{
  return (CDstop());
}

int cd_play(LispPTR *args)

/*
        args[0] function number
        args[1] play start address(LBN)
        args[2] play end address(LBN)
*/
{
  DWORD sblk, eblk;

  switch (GetTypeNumber(args[1])) {
    case TYPE_SMALLP: N_GETNUMBER(args[1], sblk, BAD_ARG); break;
    case TYPE_FIXP: sblk = *(DWORD *)(Addr68k_from_LADDR(args[1])); break;
  }
  switch (GetTypeNumber(args[2])) {
    case TYPE_SMALLP: N_GETNUMBER(args[2], eblk, BAD_ARG); break;
    case TYPE_FIXP: eblk = *(DWORD *)(Addr68k_from_LADDR(args[2])); break;
  }
  return (CDplay(sblk, eblk));

BAD_ARG:
  return (1);
}

int cd_q_read(LispPTR *args)

/*
        args[0] function number
        args[1] audio status
        args[2] tune number
        args[3] current position min
        args[4] current position sec
        args[5] current position frame
*/
{
  BYTE ast, tno, mm, ss, ff;
  int res;

  res = CDqread(&ast, &tno, &mm, &ss, &ff);
  if (!res) {
    *(int *)(Addr68k_from_LADDR(args[1])) = (int)ast;
    *(int *)(Addr68k_from_LADDR(args[2])) = (int)tno;
    *(int *)(Addr68k_from_LADDR(args[3])) = (int)mm;
    *(int *)(Addr68k_from_LADDR(args[4])) = (int)ss;
    *(int *)(Addr68k_from_LADDR(args[5])) = (int)ff;
  }
  return (res);
}

int cd_pause(LispPTR *args)

/*
        args[0] function number
*/
{
  return (CDpause());
}

int cd_resume(LispPTR *args)

/*
        args[0] function number
*/
{
  return (CDresume());
}

int cd_volume(LispPTR *args)

/*
        args[0] function number
        args[1] right volume
        args[2] left volume
*/
{
  int right, left;

  switch (GetTypeNumber(args[1])) {
    case TYPE_SMALLP: N_GETNUMBER(args[1], right, BAD_ARG); break;
    case TYPE_FIXP: right = *(DWORD *)(Addr68k_from_LADDR(args[1])); break;
  }
  switch (GetTypeNumber(args[2])) {
    case TYPE_SMALLP: N_GETNUMBER(args[2], left, BAD_ARG); break;
    case TYPE_FIXP: left = *(DWORD *)(Addr68k_from_LADDR(args[2])); break;
  }
  return (CDvolume(right, left));

BAD_ARG:
  return (1);
}

int cd_eject(LispPTR *args)

/*
        args[0] function number
*/
{
#ifdef DEBUG
  printf("cd_eject called.\n");
#endif
  return (CDeject());
}
