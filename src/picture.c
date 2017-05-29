/* $Id: picture.c,v 1.2 1999/01/03 02:07:30 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: picture.c,v 1.2 1999/01/03 02:07:30 sybalsky Exp $ Copyright (C) Venue";

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
#include <pixrect/pixrect_hs.h>

#include "lispemul.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "arith.h"
#include "adr68k.h"
#include "lspglob.h"

#define FALSE 0
#define TRUE !FALSE

#include "picture.h"

#define min(x, y) (((x) > (y)) ? (y) : (x))
#define max(x, y) (((x) > (y)) ? (x) : (y))

#define LStringToCString(Lisp, C, MaxLen, Len)                                                     \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int i;                                                                                         \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((unsigned int)Lisp));                                \
    Len = min(MaxLen, arrayp->totalsize);                                                          \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base =                                                                                     \
            ((char *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset));  \
        for (i = 0; i < Len; i++) C[i] = base[i];                                                  \
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

#define PICT_CREATE 0
#define PICT_FREE 1
#define PICT_GETVALUE 2
#define PICT_SETVALUE 3
#define PICT_GET 4
#define PICT_PUT 5
#define PICT_BITBLT 6
#define PICT_BLTSHADE 7

#define VIDEOFILE_OPEN 64
#define VIDEOFILE_CLOSE 65
#define VIDEOFILE_READ 66
#define VIDEOFILE_WRITE 67
#define VIDEOFILE_POSITION 68

extern DLword REPLACE_atom, INVERT_atom, PAINT_atom, ERASE_atom;

#ifdef VIDEO
extern int Video_OnOff_Flg;
#endif /* VIDEO */

LispPTR Picture_Op(args) LispPTR *args;
{
  int op;
  LispPTR ret_value;

  op = (DLword)args[0];

  switch (op) {
    case PICT_CREATE: ret_value = (LispPTR)Picture_Create(args); break;
    case PICT_FREE: ret_value = Picture_Free(args); break;
    case PICT_GETVALUE: ret_value = Picture_GetValue(args); break;
    case PICT_SETVALUE: ret_value = Picture_SetValue(args); break;
    case PICT_GET: ret_value = Picture_Get(args); break;
    case PICT_PUT: ret_value = Picture_Put(args); break;
    case PICT_BITBLT: ret_value = Picture_Bitblt(args); break;
    case PICT_BLTSHADE: ret_value = Picture_Bltshade(args); break;
    case VIDEOFILE_OPEN: ret_value = VideoFile_Open(args); break;
    case VIDEOFILE_CLOSE: ret_value = VideoFile_Close(args); break;
    case VIDEOFILE_READ: ret_value = VideoFile_Read(args); break;
    case VIDEOFILE_WRITE: ret_value = VideoFile_Write(args); break;
    case VIDEOFILE_POSITION:
      ret_value = VideoFile_Position(args);
      break;
    defaults:
      ret_value = NIL;
      break;
  } /* end switch( op ) */

  return (ret_value);
}

int Picture_Create(args) LispPTR *args;
{
  int width, height, bitsperpixel;
  Pixrect *storage;
  unsigned int *cell;

  width = (DLword)args[1];
  height = (DLword)args[2];
  bitsperpixel = (DLword)args[3];

  if ((bitsperpixel != 1) && (bitsperpixel != 8) && (bitsperpixel != 24)) {
    return (NULL);
  } /* end if( bitsperpixel ) */

  if (bitsperpixel == 24) bitsperpixel = 32;

  storage = mem_create(width, height, bitsperpixel);
  /* if storage is NULL, memory pixrect could not allocated. */
  /* Checking error must be done in Lisp. */

  /* Initial picture data is set. (may be white) */
  if ((bitsperpixel == 8) || (bitsperpixel == 24)) {
    pr_rop(storage, 0, 0, storage->pr_width, storage->pr_height, PIX_SET, 0, 0, 0);
  } /* end if( bitsprepixel ) */

  cell = (unsigned int *)createcell68k(TYPE_FIXP);
  *cell = (unsigned int)storage;
  return (LADDR_from_68k(cell));

} /* end Picture_Create */

extern Pixrect *TrueColorFb;

int Picture_Free(args) LispPTR *args;
{
  LispPicture *n_picture;
  Pixrect *pict;

  n_picture = (LispPicture *)Addr68k_from_LADDR(args[1]);
  pict = (Pixrect *)n_picture->storage;

  /* TrueColorFb should not be destory. */
  if (pict != TrueColorFb) {
    pr_destroy(pict);
    n_picture->storage = NULL;
  } /* end if( pict ) */

  return (T);
} /* end Picture_Free */

int Picture_GetValue(args) LispPTR *args;
{
  LispPTR picture;
  int x, y, value;
  LispPicture *n_picture;
  Pixrect *pict;
  unsigned int *cell;

  n_picture = (LispPicture *)Addr68k_from_LADDR(args[1]);
  x = (DLword)args[2];
  y = (DLword)args[3];

  pict = (Pixrect *)n_picture->storage;
  value = pr_get(pict, x, y);

  cell = (unsigned int *)createcell68k(TYPE_FIXP);
  *cell = (unsigned int)value;
  return (LADDR_from_68k(cell));

} /* end Picture_GetValue */

int Picture_SetValue(args) LispPTR *args;
{
  int x, y, value;
  LispPicture *n_picture;
  struct pixrect *pict;

  n_picture = (LispPicture *)Addr68k_from_LADDR(args[1]);
  x = (DLword)args[2];
  y = (DLword)args[3];
  N_GETNUMBER(args[4], value, bad_arg);

  pict = (Pixrect *)n_picture->storage;
  pr_put(pict, x, y, value);
  return (T);

bad_arg:
  return (NIL);

} /* end Picture_SetValue */

#define MAX_NAME_LEN 512
static char file_name[MAX_NAME_LEN];

int Picture_Get(args) LispPTR *args;
{
  LispPicture *n_picture;
  int length;

  n_picture = (LispPicture *)Addr68k_from_LADDR(args[1]);
  LStringToCString(args[2], file_name, MAX_NAME_LEN, length);

  if ((n_picture->storage = (unsigned int)File_to_Pixrect(file_name)) != NULL) {
    n_picture->width = (DLword)(((Pixrect *)n_picture->storage)->pr_width);
    n_picture->height = (DLword)(((Pixrect *)n_picture->storage)->pr_height);
    n_picture->bitsperpixel = (DLword)(((Pixrect *)n_picture->storage)->pr_depth);
    if (n_picture->bitsperpixel == 32) n_picture->bitsperpixel = 24;
    return (T);
  } else {
    return (NIL);
  } /* end if */
} /* end Picture_Get */

int Picture_Put(args) LispPTR *args;
{
  LispPicture *n_picture;
  char *name;
  int length;

  n_picture = (LispPicture *)Addr68k_from_LADDR(args[1]);
  LStringToCString(args[2], file_name, MAX_NAME_LEN, length);

  Pixrect_to_File((Pixrect *)n_picture->storage, file_name);

  return (T);
} /*end Picture_Put */

Picture_Bitblt(args) LispPTR *args;
{
  LispPicture *src, *dst;
  int src_left, src_bottom, dst_left, dst_bottom, top, bottom, left, right, width, height, stodx,
      stody, src_top, dst_top;
  int dty, dlx, sty, slx;
  int cl_src_left, cl_src_bottom;
  int video_flg;
  LispPTR operation, cl_region;

  src = (LispPicture *)Addr68k_from_LADDR(args[1]);
  dst = (LispPicture *)Addr68k_from_LADDR(args[4]);
  N_GETNUMBER(args[2], src_left, bad_arg);
  N_GETNUMBER(args[3], src_bottom, bad_arg);
  N_GETNUMBER(args[5], dst_left, bad_arg);
  N_GETNUMBER(args[6], dst_bottom, bad_arg);

  if (args[9] == PAINT_atom)
    operation = PIX_SRC | PIX_DST;
  else if (args[9] == ERASE_atom)
    operation = PIX_NOT(PIX_SRC) & PIX_DST;
  else if (args[9] == INVERT_atom)
    operation = PIX_SRC ^ PIX_DST;
  else
    operation = PIX_SRC;

  cl_region = args[10];

  N_GETNUMBER(args[11], cl_src_left, bad_arg);
  N_GETNUMBER(args[12], cl_src_bottom, bad_arg);

  top = dst->height;
  left = bottom = 0;
  right = dst->width;

  if (cl_region != NIL_PTR) {
    LispPTR cl_value;
    int tmp, cl_left, cl_bottom;

    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, cl_left, bad_arg);
    left = max(left, cl_left);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, cl_bottom, bad_arg);
    bottom = max(bottom, cl_bottom);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, tmp, bad_arg);
    right = min(right, cl_left + tmp);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, tmp, bad_arg);
    top = min(top, cl_bottom + tmp);
  } /* end if( cl_region ) */

  left = max(left, dst_left);
  bottom = max(bottom, dst_bottom);

  if (args[6] != NIL_PTR) {
    N_GETNUMBER(args[7], width, bad_arg);
    right = min(right, dst_left + width);
  } /* end if( args[6] ) */

  if (args[7] != NIL_PTR) {
    N_GETNUMBER(args[8], height, bad_arg);
    top = min(top, dst_bottom + height);
  } /* end if( args[7] ) */

  stodx = dst_left - src_left;
  stody = dst_bottom - src_bottom;

  {
    int tmp;

    left = max(cl_src_left, max(0, left - stodx));
    bottom = max(cl_src_bottom, max(0, bottom - stody));
    tmp = src->width;
    right = min(cl_src_left + tmp, min(right - stodx, src_left + width));
    tmp = src->height;
    top = min(cl_src_bottom + tmp, min(top - stody, src_bottom + height));
  }

  if ((right <= left) || (top <= bottom)) return (NIL);

  height = top - bottom;
  width = right - left;

  dty = dst->height - (top + stody);
  dlx = left + stodx;
  sty = src->height - top;
  slx = left;

#ifdef VIDEO
  if ((video_flg = Video_OnOff_Flg)) Video_OnOff(FALSE);
#endif /* VIDEO */

  pr_rop((Pixrect *)dst->storage, dlx, dty, width, height, operation, (Pixrect *)src->storage, slx,
         sty);

#ifdef VIDEO
  if (video_flg) Video_OnOff(TRUE);
#endif /* VIDEO */

bad_arg:
  return (NIL);

} /* end Picture_Bitblt */

Picture_Bltshade(args) LispPTR *args;
{
  LispPicture *dst;
  unsigned int texture;
  LispPTR cl_region;
  int dst_left, dst_bottom, left, right, bottom, top, width, height, operation, video_flg;

  dst = (LispPicture *)Addr68k_from_LADDR(args[2]);
  N_GETNUMBER(args[1], texture, bad_arg);
  N_GETNUMBER(args[3], dst_left, bad_arg);
  N_GETNUMBER(args[4], dst_bottom, bad_arg);

  if (args[7] == PAINT_atom)
    operation = PIX_SRC | PIX_DST;
  else if (args[7] == ERASE_atom)
    operation = PIX_NOT(PIX_SRC) & PIX_DST;
  else if (args[7] == INVERT_atom)
    operation = PIX_SRC ^ PIX_DST;
  else
    operation = PIX_SRC;

  cl_region = args[8];

  switch (dst->bitsperpixel) {
    case 1: texture &= 1; break;
    case 8: texture &= 0xff; break;
    case 24:
      texture &= POINTERMASK;
      break;
    defaulsts:
      texture &= POINTERMASK;
      break;
  } /* end switch */

  left = bottom = 0;
  top = (int)(dst->height);
  right = (int)(dst->width);

  if (cl_region != NIL_PTR) {
    LispPTR cl_value;
    int tmp, cl_left, cl_bottom;

    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, cl_left, bad_arg);
    left = max(left, cl_left);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, cl_bottom, bad_arg);
    bottom = max(bottom, cl_bottom);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, tmp, bad_arg);
    right = min(right, cl_left + tmp);

    cl_region = cdr(cl_region);
    cl_value = car(cl_region);
    N_GETNUMBER(cl_value, tmp, bad_arg);
    top = min(top, cl_bottom + tmp);
  } /* end if( cl_region ) */

  left = max(left, dst_left);
  bottom = max(bottom, dst_bottom);

  if (args[5] != NIL_PTR) {
    N_GETNUMBER(args[5], width, bad_arg);
    right = min(right, dst_left + width);
  } /* end if( args[5] ) */

  if (args[6] != NIL_PTR) {
    N_GETNUMBER(args[6], height, bad_arg);
    top = min(top, dst_bottom + height);
  } /* end if( args[6] ) */

  if ((right <= left) || (top <= bottom)) return (NIL);

  height = top - bottom;
  width = right - left;

#ifdef VIDEO
  if ((video_flg = Video_OnOff_Flg)) Video_OnOff(FALSE);
#endif /* VIDEO */

  pr_rop((Pixrect *)dst->storage, left, (int)(dst->height) - top, width, height,
         operation | PIX_COLOR(texture), 0, 0, 0);

#ifdef VIDEO
  if (video_flg) Video_OnOff(TRUE);
#endif /* VIDEO */

bad_arg:
  return (NIL);

} /* end Picture_Bltshade */

#define OPEN_FOR_READ 0
#define OPEN_FOR_WRITE 1

VideoFile_Open(args) LispPTR *args;
{
  unsigned int *cell, videofile;
  int length, access;

  LStringToCString(args[1], file_name, MAX_NAME_LEN, length);
  access = (DLword)args[2];

  switch (access) {
    case OPEN_FOR_READ: videofile = open_rasterfile(file_name); break;
    case OPEN_FOR_WRITE: videofile = create_rasterfile(file_name); break;
    default: videofile = 0; break;
  } /* end switch( access ) */

  cell = (unsigned int *)createcell68k(TYPE_FIXP);
  *cell = videofile;
  return (LADDR_from_68k(cell));
} /* end VideoFile_Open */

int VideoFile_Close(args) LispPTR *args;
{
  unsigned int videofile;

  N_GETNUMBER(args[1], videofile, bad_arg);

  close_rasterfile(videofile);
  return (T);
bad_arg:
  return (NIL);
} /* end VideoFile_Close */

int VideoFile_Read(args) LispPTR *args;
{
  LispPicture *n_picture;
  unsigned int *cell, pix, videofile;

  N_GETNUMBER(args[1], (unsigned int)videofile, bad_arg);
  n_picture = (LispPicture *)Addr68k_from_LADDR(args[2]);

  if ((n_picture->storage = (unsigned int)read_rasterfile(videofile)) != NULL) {
    n_picture->width = (DLword)(((Pixrect *)n_picture->storage)->pr_width);
    n_picture->height = (DLword)(((Pixrect *)n_picture->storage)->pr_height);
    n_picture->bitsperpixel = (DLword)(((Pixrect *)n_picture->storage)->pr_depth);
    if (n_picture->bitsperpixel == 32) n_picture->bitsperpixel = 24;

    return (T); /* normal return */
  }             /* end if */

bad_arg:
  return (NIL);
} /* end VideoFile_Read */

int VideoFile_Write(args) LispPTR *args;
{
  unsigned int videofile;
  LispPicture *n_pict;
  int status;

  N_GETNUMBER(args[1], videofile, bad_arg);
  n_pict = (LispPicture *)Addr68k_from_LADDR(args[2]);

  if ((status = write_rasterfile(videofile, (Pixrect *)n_pict->storage))) {
    return (T);
  } /* end if( status ) */
bad_arg:
  return (NIL);
} /* end VideoFile_Write */

int VideoFile_Position(args) LispPTR *args;
{
  unsigned int videofile;
  int n, status;

  N_GETNUMBER(args[1], videofile, bad_arg);
  n = (DLword)args[2];

  if ((status = position_rasterfile(videofile, n))) { return (T); } /* end if( status ) */
bad_arg:
  return (NIL);

} /* end VideoFile_Position */
