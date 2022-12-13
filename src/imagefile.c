/* $Id: imagefile.c,v 1.2 1999/01/03 02:07:07 sybalsky Exp $ (C) Copyright Venue, All Rights
 * Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <pixrect/pixrect_hs.h>

#define NOT_IMAGEFILE 0
#define TYPE_SUNRASTER 1
#define TYPE_PBM 2
#define TYPE_PPM 3

struct image_info {
  int depth;
  int width;
  int height;
};

struct rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

Pixrect *PPM_to_Pixrect(FILE *f), *PBM_to_Pixrect(FILE *f), *SunRaster_to_Pixrect(FILE *f);

#define MAX_BUFF_SIZE 1024
static char buff[MAX_BUFF_SIZE];

int Pixrect_to_File(Pixrect *pix, char *name)
{
  FILE *file;
  int err = 0;

  switch (pix->pr_depth) {
    case 1:
      file = fopen(name, "w");
      err = Pixrect_to_PBM(file, pix);
      fclose(file);
      break;
    case 32:
      file = fopen(name, "w");
      err = Pixrect_to_PPM(file, pix);
      fclose(file);
      break;
    case 8:
      break;
    defaults:
      break;
  } /* end switch( pix->depth ) */

  return (err);
} /* end Pixrect_to_File */

Pixrect *File_to_Pixrect(char *name)
{
  FILE *file;
  Pixrect *pix;
  int type;

  file = fopen(name, "r");

  type = image_file_type(file);

  switch (type) {
    case TYPE_SUNRASTER: pix = SunRaster_to_Pixrect(file); break;
    case TYPE_PBM: pix = PBM_to_Pixrect(file); break;
    case TYPE_PPM:
      pix = PPM_to_Pixrect(file);
      break;
    defaults:
      break;
  } /* end switch */

  fclose(file);

  return (pix);

} /* end File_to_Pixrect */

#define magic_number_PBM "P4"
#define magic_number_PPM "P6"

void Pixrect_to_PPM(FILE *file, Pixrect *pix)
{
  struct image_info info;

  info.width = pix->pr_width;
  info.height = pix->pr_height;
  info.depth = pix->pr_depth;
  write_PPM_header(file, &info);

  write_raw_PPM(file, mpr_d(pix)->md_image, pix->pr_width, pix->pr_height);

} /* end Pixrect_to_PPM */

Pixrect *PPM_to_Pixrect(FILE *file)
{
  Pixrect *pix;
  struct image_info info;

  read_PPM_header(file, &info);
  pix = mem_create(info.width, info.height, 32);
  read_raw_PPM(file, mpr_d(pix)->md_image, info.width, info.height);

  return (pix);
} /* PPM_to_Pixrect */

void Pixrect_to_PBM(FILE *file, Pixrect *pix)
{
  struct image_info info;

  info.width = pix->pr_width;
  info.height = pix->pr_height;
  info.depth = pix->pr_depth;
  write_PBM_header(file, &info);

  write_raw_PBM(file, mpr_d(pix)->md_image, pix->pr_width, pix->pr_height);

} /* end Pixrect_to_PBM */

Pixrect *PBM_to_Pixrect(FILE *file)
{
  Pixrect *pix;
  struct image_info info;

  read_PPM_header(file, &info);
  pix = mem_create(info.width, info.height, 32);
  read_raw_PPM(file, mpr_d(pix)->md_image, info.width, info.height);

  return (pix);

} /* end PBM_to_Pixrect */

Pixrect *SunRaster_to_Pixrect(FILE *file)
{
  Pixrect *pix;
  colormap_t cmap;

  cmap.type = RMT_NONE;

  pix = pr_load(file, &cmap);

  return (pix);
} /* end SunRaster_to_Pixrect */

void write_raw_PBM(FILE *file, unsigned char *data, int width, int height)
{
  int i, n, flg, len;

  n = ((width + 15) / 16) * 2; /* bytes per line */
  flg = (width % 16) / 8;      /* PBM is byts alignment */
  for (i = 0; i < height; i++, data += n) {
    len = fwrite(data, sizeof(data), (n - (flg ? 1 : 0)), file);
  } /* end for( i ) */
} /* end write_PBM_data */

void read_raw_PBM(FILE *file, char *data, int width, int height)
{
  int n, i, flg, len;

  n = ((width + 15) / 16) * 2; /* bytes per line */
  flg = (width % 16) / 8;      /* PBM is byts alignment */
  for (i = 0; i < height; i++, data += n) {
    len = fread(data, sizeof(data), (n - (flg ? 1 : 0)), file);
  } /* end for( i ) */
} /* end read_raw_PBM */

void write_raw_PPM(FILE file, unsigned char *data, int width, int height)
{
  struct rgb color24;
  int i, j, len;

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++, ((union fbunit *)data)++) {
      color24.r = ((union fbunit *)data)->channel.R;
      color24.g = ((union fbunit *)data)->channel.G;
      color24.b = ((union fbunit *)data)->channel.B;
      len = fwrite(&color24, sizeof(color24), 1, file);
    } /* end for( j ) */
  }   /* end for( i ) */
} /* end write_raw_PPM */

void read_raw_PPM(FILE *file, unsigned char *data, int width, int height)
{
  struct rgb color24;
  int i, j, len;

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++, ((union fbunit *)data)++) {
      len = fread((char *)(&color24), sizeof(color24), 1, file);
      ((union fbunit *)data)->channel.R = color24.r;
      ((union fbunit *)data)->channel.G = color24.g;
      ((union fbunit *)data)->channel.B = color24.b;
    } /* end for( j ) */
  }   /* end for( i ) */
} /* endi read_raw_PPM */

int image_file_type(FILE *file)
{
  int c, file_type;

  if ((c = getc(file)) == -1) return (NOT_IMAGEFILE);

  switch (c) {
    case 'Y': /* may be Sun Raster format */ file_type = TYPE_SUNRASTER; break;
    case 'P': {
      if ((c = getc(file)) == -1) return (NOT_IMAGEFILE);
      if (c == '4')
        file_type = TYPE_PBM;
      else if (c == '6')
        file_type = TYPE_PPM;
      else
        file_type = NOT_IMAGEFILE;
    } /* end case P */
    break;
    defaults:
      file_type = NOT_IMAGEFILE;
      break;
  } /* end switch( c ) */

  rewind(file);

  return (file_type);
} /* end image_file_type */

int read_PBM_header(FILE *file, struct image_info *info)
{
  int err;

  err = skip_line(file);
  if (err == 0) return (err);

  err = read_available_line(file, buff);
  if (err == 0) return (err);
  err = sscanf(buff, "%d %d", &(info->width), &(info->height));
  if (err == 0) return (err);

  return (err);

} /* end read_PBM_header */

void write_PBM_header(FILE *file, struct image_info *info)
{
  fprintf(file, "P4\n");
  fprintf(file, "%d %d\n", info->width, info->height);
} /* write_PBM_header */

int read_PPM_header(FILE *file, struct image_info *info)
{
  int err;

  err = skip_line(file);
  if (err == 0) return (err);

  err = read_available_line(file, buff);
  if (err == 0) return (err);
  err = sscanf(buff, "%d %d", &(info->width), &(info->height));
  if (err == 0) return (err);

  err = skip_line(file);

  return (err);

} /* read_PPM_header */

void write_PPM_header(FILE *file, struct image_info *info)
{
  fprintf(file, "P6\n");
  fprintf(file, "%d %d\n", info->width, info->height);
  fprintf(file, "255\n");
} /* end write_PPM_header */

int skip_line(FILE *file)
{
  char *err;
  char buff[MAX_BUFF_SIZE];

  err = fgets(buff, (int)MAX_BUFF_SIZE, file);
  return ((int)err);
} /* end skip_line */

int read_available_line(FILE *file, char *buff)
{
  char *err;
  err = fgets(buff, MAX_BUFF_SIZE, file);
  while ((err != 0) && (buff[0] == '#')) { err = fgets(buff, MAX_BUFF_SIZE, file); } /* end while */
  return ((int)err);
} /* end read_available_line */
