/* $Id: xmkicon.c,v 1.3 2001/12/24 01:09:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: xmkicon.c,v 1.3 2001/12/24 01:09:09 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "lispemul.h"
#include "dbprint.h"

#include "xdefs.h"
#include "devif.h"

#include "xmkicondefs.h"

XImage IconImage;

extern int Bitmap_Pad, Default_Depth, Lisp_icon_width, Lisp_icon_height;
extern char Lisp_icon[];

extern Pixmap IconPixmap;
extern char iconpixmapfile[1024];

/************************************************************************/
/*									*/
/*			m a k e _ X i c o n				*/
/*									*/
/*	Make the icon for the shrunken Medley window.			*/
/*									*/
/************************************************************************/

Pixmap make_Xicon(DspInterface dsp)
{
  unsigned int width, height;
  int value, x_hot, y_hot;
  char *getenv();

#ifdef TRACE
  printf("In make_Xicon().\n");
#endif
  value = XReadBitmapFile(dsp->display_id, RootWindow(dsp->display_id, 0), iconpixmapfile, &width,
                          &height, &IconPixmap, &x_hot, &y_hot);
  if (value == BitmapOpenFailed) {
    IconImage.width = Lisp_icon_width;
    IconImage.height = Lisp_icon_height;
    IconImage.xoffset = 0;
    IconImage.format = XYBitmap;
    IconImage.data = (char *)Lisp_icon;
#if defined(BYTESWAP)
    IconImage.byte_order = LSBFirst;
#else  /* BYTESWAP */
    IconImage.byte_order = MSBFirst;
#endif /* BYTESWAP */
    IconImage.bitmap_unit = 8;
    IconImage.bitmap_pad = Bitmap_Pad;
    IconImage.depth = 1;
    IconImage.bytes_per_line = Lisp_icon_width / 8;
#if defined(X_ICON_IN_X_BITMAP_FORMAT)
    IconImage.bitmap_bit_order = LSBFirst;
#else
    IconImage.bitmap_bit_order = MSBFirst;
#endif

    IconPixmap = XCreatePixmap(
        dsp->display_id, dsp->LispWindow, Lisp_icon_width, Lisp_icon_height,
        DefaultDepthOfScreen(ScreenOfDisplay(dsp->display_id, DefaultScreen(dsp->display_id))));

    XPutImage(dsp->display_id, IconPixmap, dsp->Copy_GC, &IconImage, 0, 0, 0, 0, Lisp_icon_width,
              Lisp_icon_height);

  } else if (value == BitmapFileInvalid)
    fprintf(stderr, "Iconpixmapfile %s contains invalid bitmap data\n", iconpixmapfile);
  else if (value == BitmapNoMemory)
    fprintf(stderr, "Not enough memory to allocate icon pixmap\n");
  return (IconPixmap);
} /* end make_Xicon */
