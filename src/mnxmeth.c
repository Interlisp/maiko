/* $Id: mnxmeth.c,v 1.2 1999/01/03 02:07:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: mnxmeth.c,v 1.2 1999/01/03 02:07:25 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <alloca.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xaw/BoxP.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xmu/Converters.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "lispemul.h"
#include "lspglob.h"
#include "cell.h"
#include "lsptypes.h"
#include "stream.h"
#include "array.h"
#include "dspdata.h"
#include "adr68k.h"
#include "mnxdefs.h"

#include "ScrollBoxP.h"
#include "ScrollBox.h"

#define BARTHICKNESS 10
#define BARFRAME 1

int MedleyWindowType, MedleyScreenType, MedleyBitmapType;
int gcfunction[4] = {GXcopy, GXor, GXxor, GXandInverted};
int bltshade_function[4] = {GXcopy, GXcopy, GXxor, GXcopy};
int xtinitialized = False;

extern DspInterfaceRec *FD_to_dspif[32]; /* map from FD # to display */
extern int LispReadFds;
extern int MNWReadFds;
extern char *sysout_name;
extern int save_argc;
extern char **save_argv;

extern DLword INVERT_atom;
extern DLword ERASE_atom;
extern DLword PAINT_atom;
extern DLword REPLACE_atom;

/**************************************************************/
/* Xerrhandler.                                               */
/* Utility function to make URaid the error handler for X.    */
/**************************************************************/
int Xerrhandler(Display *display, XErrorEvent *event)
{
  char msg[80];

  XGetErrorText(display, event->error_code, msg, 80);
  error(msg);
}

void intersectregions(MRegion *reg1, MRegion *reg2, MRegion *result)
{
  result->x = max(reg1->x, reg2->x);
  result->y = max(reg1->y, reg2->y);
  result->width = result->x - min(reg1->x + reg1->width, reg2->x + reg2->width);
  result->height = result->y - min(reg1->y + reg1->height, reg2->y + reg2->height);
}

/**************************************************************/
/* getwhiteborder                                             */
/* Utility function to calculate the white border of a window.*/
/* Takes a medley window as its argument.                     */
/**************************************************************/
int getwhiteborder(LispPTR medleywindow)
{
  int border;

  if (((MedleyWindow)Cptr(medleywindow))->WBORDER)
    border = LispIntToCInt(((MedleyWindow)Cptr(medleywindow))->WBORDER);
  else
    border = 0;

  if (border <= 1)
    return (0);
  else if (border <= 3)
    return (1);
  else
    return (2);
}

/**************************************************************/
/* getblackborder                                             */
/* Utility function to calculate the black border of a window.*/
/* Takes a medley window as its argument.                     */
/**************************************************************/
int getblackborder(LispPTR medleywindow)
{
  int border;

  if (((MedleyWindow)Cptr(medleywindow))->WBORDER)
    border = LispIntToCInt(((MedleyWindow)Cptr(medleywindow))->WBORDER);
  else
    border = 0;

  if (border <= 0)
    return (0);
  else if (border <= 2)
    return (1);
  else if (border <= 3)
    return (2);
  else
    return (border - 2);
}

/**************************************************************/
/* gettitlestring                                             */
/* Utility function to retrieve the titlestring from a window.*/
/* Takes a medley window as its argument. Returns a pointer to*/
/* a malloc'd c string or NULL if no string is present.       */
/**************************************************************/
char *gettitlestring(LispPTR medleywin)
{
  char *titlestring;
  int strlen;

  if (((MedleyWindow)Cptr(medleywin))->WTITLE == NIL)
    titlestring = NULL;
  else {
    strlen = LispStringLength(((MedleyWindow)Cptr(medleywin))->WTITLE);
    titlestring = (char *)malloc(strlen + 1);
    LispStringToCStr(((MedleyWindow)Cptr(medleywin))->WTITLE, titlestring);
  }
  return (titlestring);
}

/**************************************************************/
/* gettitleheight                                             */
/* Return the height of the title bar for this window.        */
/* Pick the value from the TITLEDS of the screen.             */
/**************************************************************/
int gettitleheight(LispPTR medleywin)
{
  if (((MedleyWindow)Cptr(medleywin))->WTITLE == NIL)
    return (0);
  else {
    DISPLAYDATA *dd;
    dd = TitleDDFromMw(medleywin);
    return (abs(LispIntToCInt(dd->ddlinefeed)));
  }
}

/**************************************************************/
/* setlineattributes                                          */
/* This function attempts to syncronize the line data in lisp */
/* and X.                                                     */
/**************************************************************/
void setlineattributes(Display *display, DspInterface dspif, WindowInterface wif, DISPLAYDATA *dd,
		       LispPTR mwidth, LispPTR mdash)
{
  int l, lw, ls;
  unsigned char *dash_list;

  lw = (mwidth == NIL) ? 0 : LispIntToCInt(mwidth);
  ls = (mdash == NIL) ? LineSolid : LineOnOffDash;

  if (dd->ddoperation == INVERT_atom) {
    XSetLineAttributes(display, wif->InvertGC, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, dspif->PixIGC, lw, ls, CapRound, JoinBevel);
  } else if (dd->ddoperation == ERASE_atom) {
    XSetLineAttributes(display, wif->EraseGC1, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, wif->EraseGC2, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, dspif->PixEGC, lw, ls, CapRound, JoinBevel);
  } else if (dd->ddoperation == PAINT_atom) {
    XSetLineAttributes(display, wif->PaintGC1, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, wif->PaintGC2, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, dspif->PixPGC, lw, ls, CapRound, JoinBevel);
  } else if (dd->ddoperation == REPLACE_atom) {
    XSetLineAttributes(display, wif->ReplaceGC, lw, ls, CapRound, JoinBevel);
    XSetLineAttributes(display, dspif->PixRGC, lw, ls, CapRound, JoinBevel);
  }

  if (mdash != NIL) {
    l = LispStringLength(mdash);
    dash_list = (unsigned char *)malloc(l + 1);
    LispStringToCStr(mdash, dash_list);

    if (dd->ddoperation == INVERT_atom) {
      XSetDashes(display, wif->InvertGC, 0, dash_list, l);
      XSetDashes(display, dspif->PixIGC, 0, dash_list, l);
    } else if (dd->ddoperation == ERASE_atom) {
      XSetDashes(display, wif->EraseGC1, 0, dash_list, l);
      XSetDashes(display, wif->EraseGC2, 0, dash_list, l);
      XSetDashes(display, dspif->PixEGC, 0, dash_list, l);
    } else if (dd->ddoperation == PAINT_atom) {
      XSetDashes(display, wif->PaintGC1, 0, dash_list, l);
      XSetDashes(display, wif->PaintGC2, 0, dash_list, l);
      XSetDashes(display, dspif->PixPGC, 0, dash_list, l);
    } else if (dd->ddoperation == REPLACE_atom) {
      XSetDashes(display, wif->ReplaceGC, 0, dash_list, l);
      XSetDashes(display, dspif->PixRGC, 0, dash_list, l);
    }
  }
}

void settileorigin(LispPTR medleywin, LispPTR x, LispPTR y)
{
  WindowInterface wif;
  Display *display;

  wif = WIfFromMw(medleywin);
  display = XtDisplay(wif->windowwidget);

  XSetTSOrigin(display, wif->InvertGC, x, y);
  XSetTSOrigin(display, wif->EraseGC1, x, y);
  XSetTSOrigin(display, wif->EraseGC2, x, y);
  XSetTSOrigin(display, wif->PaintGC1, x, y);
  XSetTSOrigin(display, wif->PaintGC2, x, y);
  XSetTSOrigin(display, wif->ReplaceGC, x, y);
  XSetTSOrigin(display, wif->dspif->PixIGC, x, y);
  XSetTSOrigin(display, wif->dspif->PixEGC, x, y);
  XSetTSOrigin(display, wif->dspif->PixPGC, x, y);
  XSetTSOrigin(display, wif->dspif->PixRGC, x, y);
}

/**************************************************************/
/**************************************************************/
void calculateshape(WindowInterface wif, LispPTR x, LispPTR y, LispPTR width, LispPTR height, int topwidoffset, int tophgtoffset)
{
  wif->windowreg.width = LispIntToCInt(width);
  wif->windowreg.height = LispIntToCInt(height);
  wif->windowreg.x = LispIntToCInt(x);
  wif->windowreg.y = LispIntToCInt(y);
  calcwif(wif, topwidoffset, tophgtoffset);
}

/**************************************************************/
/* calcwif                                                    */
/* layout the regions. All calculations based on the outer    */
/* region. The rest of the widget moves around this region.   */
/**************************************************************/
void calcwif(WindowInterface wif, int xoff, int yoff)
{
  int whiteborder, blackborder, blackborder2, titleheight, border;

  blackborder = getblackborder(wif->MedleyWindow);
  blackborder2 = blackborder + blackborder;
  whiteborder = getwhiteborder(wif->MedleyWindow);

  /* Calculate the top & outer region */
  wif->outerregion.y = 0;
  wif->outerregion.x = 0;

  wif->outerregion.width = wif->windowreg.width;
  wif->topregion.width = wif->outerregion.width + /* wif->outerregion.x; */
                         xoff;

  wif->outerregion.height = wif->windowreg.height;
  wif->topregion.height = wif->outerregion.height + yoff;

  wif->topregion.x = wif->windowreg.x - wif->outerregion.x;
  wif->topregion.y = (HeightOfScreen(wif->screen) - wif->windowreg.y - wif->windowreg.height);

  titleheight = gettitleheight(wif->MedleyWindow);

  /* Calculate the inner region. */
  border = 2 * (blackborder + whiteborder);
  wif->innerregion.x = blackborder;
  wif->innerregion.y = blackborder + titleheight;
  wif->innerregion.width = wif->windowreg.width - border;
  wif->innerregion.height = wif->windowreg.height - border - titleheight;

  wif->whiteborder = whiteborder;
  wif->blackborder2 = blackborder2;

  sprintf(wif->gstring, "%dx%d+%d+%d", wif->topregion.width, wif->topregion.height,
          wif->topregion.x, wif->topregion.y);
}

/**************************************************************/
/**************************************************************/
void refreshwindow(LispPTR medleywin)
{
  WindowInterface wif;
  DISPLAYDATA *dd;
  float top, shown;

  wif = WIfFromMw(medleywin);
  dd = ImDataFromMw(medleywin);
  calcwif(wif, wif->topregion.width - wif->outerregion.width,
          wif->topregion.height - wif->outerregion.height);
  XtVaSetValues(wif->formwidget, XtNclippX, dd->ddclippingleft, XtNclippY, dd->ddclippingbottom,
                XtNclippWidth, dd->ddclippingright - dd->ddclippingleft, XtNclippHeight,
                dd->ddclippingtop - dd->ddclippingbottom, NULL);
  if (XtIsRealized(wif->topwidget)) {
    XtConfigureWidget(wif->topwidget, wif->topregion.x, wif->topregion.y, wif->topregion.width,
                      wif->topregion.height, 0);

    XtConfigureWidget(wif->framewidget, wif->outerregion.x, wif->outerregion.y,
                      wif->outerregion.width, wif->outerregion.height, 0);

    XtConfigureWidget(wif->windowwidget, wif->innerregion.x, wif->innerregion.y,
                      wif->innerregion.width, wif->innerregion.height, wif->whiteborder);
  }
}

/************************************************************************/
/*									*/
/*			Translate X & Y window coords, allowing for offsets.		*/
/*									*/
/*			XVAL & YVAL are already real integers.			*/
/*									*/
/************************************************************************/

int translate_x(WindowInterface wif, int xval)
{ return (xval + (wif->xoffset - wif->windowreg.x - (wif->blackborder2 >> 1) - wif->whiteborder)); }

int translate_y(WindowInterface wif, int yval)
{ return (yval + wif->yoffset - (wif->windowreg.y + (wif->blackborder2 >> 1) + wif->whiteborder)); }

extern void SignalVJmpScroll();
extern void SignalVScroll();
extern void SignalHJmpScroll();
extern void SignalHScroll();
extern void HandleBackgroundButton();
extern void HandleBackgroundCrossing();
extern void HandleButton();
extern void HandleMotion();
extern void HandleKey();
extern void HandleCrossing();
extern void HandleFocus();
extern void HandleStructure();
extern void HandleTitle();
extern void HandleExpose();

/**************************************************************/
/* InitScratchDepth                                           */
/* Init the scratch image to fit the description of a bitmap  */
/**************************************************************/
InitScratchDepth(image, MBM, scdepth) XImage *image;
BITMAP *MBM;
int scdepth;
{ /* Set up the scratch image for this operation */
  image->width = MBM->bmwidth;
  image->height = MBM->bmheight;

  /* depth 1 implies that X should colorize */
  if ((MBM->bmbitperpixel & 0xFFFF) == 1) {
    image->depth = MBM->bmbitperpixel & 0xFFFF;
    image->bits_per_pixel = 1;
    image->bytes_per_line =
        (((image->width + (BITSPER_DLWORD - 1)) / BITSPER_DLWORD) * (BITSPER_DLWORD / 8));
    if (image->format != XYBitmap) {
      image->format = XYBitmap;
      _XInitImageFuncPtrs(image);
    }
  } else /* if(image->format != ZPixmap) */
  {
    image->depth = MBM->bmbitperpixel & 0xFFFF;
    image->format = ZPixmap;
    image->red_mask = (scdepth / 3);
    image->green_mask = ((scdepth - image->red_mask) / 2);
    image->blue_mask = (scdepth - image->red_mask - image->green_mask);
    image->bits_per_pixel = image->depth;
    _XInitImageFuncPtrs(image);
    image->bytes_per_line =
        ((((image->width * image->depth) + (BITSPER_DLWORD - 1)) / BITSPER_DLWORD) *
         (BITSPER_DLWORD / 8));
  }
}

/**************************************************************/
/* MakeScratchImageFromBM                                     */
/* Convert a Lisp BITMAP to fit in the scratch image in the   */
/* dspif. Use the scratch image in subsequent computations.   */
/**************************************************************/
MakeScratchImageFromBM(dspif, bitmap) DspInterface dspif;
BITMAP *bitmap;
{
  InitScratchDepth(&dspif->image, bitmap, DefaultDepthOfScreen(dspif->xscreen));

  dspif->image.data = (char *)Cptr((LispPTR)bitmap->bmbase);
}

/**************************************************************/
/* MakeScratchImageFromInt                                    */
/* Convert a Lisp integer to be a bitmap in the scratch image */
/* in the dspif. Use the scratch image in subsequent          */
/* computations.                                              */
/* DANGER!! This function presumes that you have a valid data */
/* slot. ie. the caller have to make sure that the data area  */
/* is allocated.                                              */
/**************************************************************/
int revbits[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};
MakeScratchImageFromInt(dspif, Texture) DspInterface dspif;
int Texture;
{
  int width, height, pixpos, x, y;
  unsigned char *bits = (unsigned char *)dspif->image.data;
  unsigned int tbits;
  width = height = 4;
  pixpos = 0;
  dspif->image.width = width;
  dspif->image.height = height;
  dspif->image.depth = 1;
  dspif->image.format = XYBitmap;
  dspif->image.bytes_per_line =
      ((width + (BITSPER_DLWORD - 1)) / BITSPER_DLWORD) * (BITSPER_DLWORD / 8);

  tbits = revbits[Texture >> 12];
  bits[0] = (tbits << 4) | tbits;
  tbits = revbits[15 & (Texture >> 8)];
  bits[1] = (tbits << 4) | tbits;
  tbits = revbits[15 & (Texture >> 4)];
  bits[2] = (tbits << 4) | tbits;
  tbits = revbits[15 & (Texture)];
  bits[3] = (tbits << 4) | tbits;
}

/**************************************************************/
/* settexture                                                 */
/* Argument is a Medleywindow. This function scans the texture*/
/* slot of the DISPLAYDATA structure. If that slot has a value*/
/* the Xwindow's background is updated to match that value.   */
/* NOTE!!: Medley tiles from the lower left corner. X tiles   */
/* the background from the upper left corner. This is not     */
/* accounted for.                                             */
/**************************************************************/
settexture(medleywin) LispPTR medleywin;
{
  DspInterface dspif;
  WindowInterface wif;
  DISPLAYDATA *dd;
  Pixmap bgpixmap;
  Display *display;

  dspif = DspIfFromMw(medleywin);
  wif = WIfFromMw(medleywin);
  dd = ImDataFromMw(medleywin);
  display = dspif->handle;

  if (dd->ddtexture != NIL) {
    if (GetTypeNumber(dd->ddtexture) == TYPE_SMALLP)
      if (LispIntToCInt(MScrFromMw(medleywin)->SCDEPTH) == 1) {
        char id[64];
        dspif->image.data = id; /* Danger zone! read comment for function in next call */
        MakeScratchImageFromInt(dspif, LispIntToCInt(dd->ddtexture));
      } else {
        unsigned long pixval;

        pixval = ~(unsigned long)LispIntToCInt(dd->ddtexture);
        /* Set the background for pixel ops in this window */
        XSetBackground(display, wif->InvertGC, pixval);
        XSetBackground(display, wif->EraseGC1, pixval);
        XSetBackground(display, wif->EraseGC2, pixval);
        XSetBackground(display, wif->PaintGC1, pixval);
        XSetBackground(display, wif->PaintGC2, pixval);
        XSetBackground(display, wif->ReplaceGC, pixval);
        XSetBackground(display, dspif->PixIGC, pixval);
        XSetBackground(display, dspif->PixEGC, pixval);
        XSetBackground(display, dspif->PixPGC, pixval);
        XSetBackground(display, dspif->PixRGC, pixval);
        XtVaSetValues(wif->windowwidget, XtNbackground, pixval, NULL);
        return (NIL);
      }

    else { /* Its a bitmap */
      MakeScratchImageFromBM(dspif, (BITMAP *)Cptr(dd->ddtexture));
    }
  } else { /* ddtexture is NIL */
    char id[64];
    dspif->image.data = id; /* Danger zone! read comment for function in next call */
    MakeScratchImageFromInt(dspif, 0);
  }

  if (wif->bgpixmap) {
    /* If we have an fgpixmap, junk it */
    XFreePixmap(display, wif->bgpixmap);
    wif->bgpixmap = 0;
  }

  bgpixmap = XCreatePixmapFromBitmapData(
      display, dspif->root, dspif->image.data, dspif->image.width, dspif->image.height,
      WhitePixelOfScreen(wif->screen), BlackPixelOfScreen(wif->screen),
      DefaultDepthOfScreen(wif->screen));
  XSetTile(display, wif->InvertGC, bgpixmap);
  XSetTile(display, wif->EraseGC1, bgpixmap);
  XSetTile(display, wif->EraseGC2, bgpixmap);
  XSetTile(display, wif->PaintGC1, bgpixmap);
  XSetTile(display, wif->PaintGC2, bgpixmap);
  XSetTile(display, wif->ReplaceGC, bgpixmap);
  /*  XSetTile(display, dspif->PixIGC, bgpixmap);
    XSetTile(display, dspif->PixEGC, bgpixmap);
    XSetTile(display, dspif->PixPGC, bgpixmap);
    XSetTile(display, dspif->PixRGC, bgpixmap); */
  XtVaSetValues(wif->windowwidget, XtNbackgroundPixmap, bgpixmap, NULL);

  wif->bgpixmap = bgpixmap;

  return (NIL);
}

MBMToDrawable(MBM, dspif, wif, srcx, srcy, dstx, dsty, width, height, operation) BITMAP *MBM;
WindowInterface wif;
DspInterface dspif;
int srcx, srcy, dstx, dsty, width, height, operation;
{
  Window window;
  window = XtWindow(wif->windowwidget);

  if ((MBM->bmbitperpixel == 1) || (MBM->bmbitperpixel == DefaultDepthOfScreen(wif->screen))) {
    InitScratchDepth(&dspif->image, MBM, DefaultDepthOfScreen(wif->screen));
    dspif->image.data = (char *)Cptr((LispPTR)MBM->bmbase);
    switch (operation) {
      case REPLACE:
        if (window)
          XPutImage(dspif->handle, window, wif->ReplaceGC, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        XPutImage(dspif->handle, wif->backing, dspif->PixRGC, &dspif->image, srcx, srcy, dstx, dsty,
                  width, height);
        break;

      case INVERT:
        if (window)
          XPutImage(dspif->handle, window, wif->InvertGC, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        XPutImage(dspif->handle, wif->backing, dspif->PixIGC, &dspif->image, srcx, srcy, dstx, dsty,
                  width, height);
        break;

      case PAINT:
        if (window)
          XPutImage(dspif->handle, window, wif->PaintGC1, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        if (window)
          XPutImage(dspif->handle, window, wif->PaintGC2, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        XPutImage(dspif->handle, wif->backing, dspif->PixPGC, &dspif->image, srcx, srcy, dstx, dsty,
                  width, height);
        break;

      case ERASE:
        if (window)
          XPutImage(dspif->handle, window, wif->EraseGC1, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        if (window)
          XPutImage(dspif->handle, window, wif->EraseGC2, &dspif->image, srcx, srcy, dstx, dsty,
                    width, height);
        XPutImage(dspif->handle, wif->backing, dspif->PixEGC, &dspif->image, srcx, srcy, dstx, dsty,
                  width, height);
        break;
    }
  } else {
    /* JDS 2/22/94:  I'm not sure this part is right; haven't thought about it. */
    int x, y;
    InitScratchDepth(&dspif->image, MBM, DefaultDepthOfScreen(wif->screen));
    dspif->image.depth = DefaultDepthOfScreen(wif->screen);
    dspif->image.data =
        (char *)malloc(dspif->image.bytes_per_line * dspif->image.width * dspif->image.depth);

    InitScratchDepth(&dspif->tmpimage, MBM, DefaultDepthOfScreen(wif->screen));
    dspif->tmpimage.data = (char *)Cptr((LispPTR)MBM->bmbase);
    for (x = 0; x < MBM->bmwidth; x++)
      for (y = 0; y < MBM->bmheight; y++)
        XPutPixel(&dspif->image, x, y, XGetPixel(&dspif->tmpimage, x, y));
    if (operation == INVERT_atom) {
      XPutImage(dspif->handle, window, wif->InvertGC, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
    } else if (operation == ERASE_atom) {
      XPutImage(dspif->handle, window, wif->EraseGC1, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
      XPutImage(dspif->handle, window, wif->EraseGC2, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
    } else if (operation == PAINT_atom) {
      XPutImage(dspif->handle, window, wif->PaintGC1, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
      XPutImage(dspif->handle, window, wif->PaintGC2, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
    } else if (operation == REPLACE_atom) {
      XPutImage(dspif->handle, window, wif->ReplaceGC, &dspif->image, srcx, srcy, dstx, dsty, width,
                height);
    }
    free(dspif->image.data);
  }
}

/************************************************************************/
/* DrawableToMBM                                                        */
/* Copy a drawable to a Medley bitmap. All coordinates are in X style.  */
/* (i.e. y-coordinate assumed to be transformed)                        */
/* Since XGetSubImage will generate an error if the drawable is outside */
/* the rootwindow we have to get the geometry and clipp it with the     */
/* geometry of the root window. (Urk!)                                  */
/* This offcause means that anything outside the screen will have to be */
/* ignored. (This has to be fixed. This sucks.).                        */
/************************************************************************/
DrawableToMBM(MBM, dspif, wif, xdrawable, xscreen, srcx, srcy, dstx, dsty, width, height,
              op) BITMAP *MBM;
WindowInterface wif;
Screen *xscreen;
Drawable xdrawable;
DspInterface dspif;
int srcx, srcy, dstx, dsty, width, height, op;
{
#ifdef NEVER
  if (MBM->bmbitperpixel == DefaultDepthOfScreen(xscreen)) {
    InitScratchDepth(&dspif->image, MBM, DefaultDepthOfScreen(xscreen));
    dspif->image.data = (char *)Cptr((LispPTR)MBM->bmbase);
    XGetSubImage(dspif->handle, xdrawable, srcx, srcy, width, height, AllPlanes, ZPixmap,
                 &dspif->image, dstx, dsty);
  } else {
    int x, y, scdepth;
    BITMAP tmp;

    scdepth = DefaultDepthOfScreen(xscreen);
    tmp.bmwidth = MBM->bmwidth;
    tmp.bmheight = MBM->bmheight;
    tmp.bmbitperpixel = scdepth;
    InitScratchDepth(&dspif->tmpimage, &tmp, scdepth);
    dspif->tmpimage.depth = scdepth;
    dspif->tmpimage.data = (char *)malloc(dspif->tmpimage.bytes_per_line * dspif->tmpimage.width *
                                          dspif->tmpimage.depth);
    XGetSubImage(dspif->handle, xdrawable, srcx, srcy, width, height, AllPlanes, ZPixmap,
                 &dspif->tmpimage, dstx, dsty);

    InitScratchDepth(&dspif->image, MBM, MBM->bmbitperpixel);

    dspif->image.data = (char *)Cptr((LispPTR)MBM->bmbase);
    for (x = 0; x < MBM->bmwidth; x++)
      for (y = 0; y < MBM->bmheight; y++)
        XPutPixel(&dspif->image, x, y, !XGetPixel(&dspif->tmpimage, x, y));
    free(dspif->tmpimage.data);
  }
#else  /* NEVER */
  InitScratchDepth(&dspif->image, MBM, 1);
  dspif->image.data = (char *)Cptr((LispPTR)MBM->bmbase);
  XGetSubImage(dspif->handle, xdrawable, srcx, srcy, width, height, AllPlanes, ZPixmap,
               &dspif->image, dstx, dsty);
#endif /* Never */
}

MNXBBTToXWindow(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                     /* args[1] = LispPTR to src BITMAP */
                                     /* args[2] = scr_x */
                                     /* args[3] = scr_y */
                                     /* args[4] = dst WINDOW */
                                     /* args[5] = dst_x */
                                     /* args[6] = dst_y */
                                     /* args[7] = width */
                                     /* args[8] = height */
                                     /* args[9] = operation */
{
  WindowInterface wif;
  DspInterface dspif;
  Display *display;
  BITMAP *bitmap;
  int width, height, depth, srcy, dsty;

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);
  bitmap = (BITMAP *)Cptr(args[1]);

  if (!wif->open) return (NIL); /* If window not open, don't do anything. */
  /* Transform the coordinates */
  width = LispIntToCInt(args[7]);
  height = LispIntToCInt(args[8]);
  dsty = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[6])) - height;
  srcy = (bitmap->bmheight & 0xFFFF) - LispIntToCInt(args[3]) - height;

  MBMToDrawable(bitmap, dspif, wif, LispIntToCInt(args[2]),     /* src x */
                srcy, translate_x(wif, LispIntToCInt(args[5])), /* dst x */
                dsty, width, height, args[9] & 0xF);
  XFlush(display);
}

MNXBBTFromXWindow(LispArgs args) /* args[0] = LispPTR to src WINDOW */
                                       /* args[1] = scr_x */
                                       /* args[2] = scr_y */
                                       /* args[3] = LispPTR to dst BITMAP */
                                       /* args[4] = dst_x */
                                       /* args[5] = dst_y */
                                       /* args[6] = width */
                                       /* args[7] = height */
                                       /* args[8] = operation */
/* Things to remember: This function does not handle integer-textures */
/* The handling of the OPERATION argument is done in lisp */
{
  WindowInterface wif;
  DspInterface dspif;
  Display *display;
  BITMAP *bitmap;
  int width, height, depth, srcx, srcy, dstx, dsty;

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);
  bitmap = (BITMAP *)Cptr(args[3]);

  if (!wif->open) return (NIL); /* If window not open, don't do anything. */

  /* Transform the coordinates */
  srcx = max(0, translate_x(wif, LispIntToCInt(args[1])));
  dstx = LispIntToCInt(args[4]);

  width = min(LispIntToCInt(args[6]), wif->innerregion.width - max(0, srcx));
  ;
  height = min(LispIntToCInt(args[7]), wif->innerregion.height - LispIntToCInt(args[2]));
  ;
  srcy = max(0, wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2])) - height);
  dsty = (bitmap->bmheight & 0xFFFF) - LispIntToCInt(args[5]) - height;

  DrawableToMBM(bitmap, dspif, wif, wif->backing, wif->screen, srcx, srcy, dstx, dsty, width,
                height, args[8] & 0xF);

  XFlush(display);
  return (ATOM_T);
}

MNXBBTWinWin(LispArgs args) /* args[0] = LispPTR to src WINDOW */
                                  /* args[1] = scr_x */
                                  /* args[2] = scr_y */
                                  /* args[3] = LispPTR to dst WINDOW */
                                  /* args[4] = dst_x */
                                  /* args[5] = dst_y */
                                  /* args[6] = width */
                                  /* args[7] = height */
                                  /* args[8] = operation */
/* Things to remember: This function does not handle integer-textures */
{
  WindowInterface srcwif, dstwif;
  DspInterface dspif;
  Display *display;
  Window swin, dwin;
  BITMAP *bitmap;
  DISPLAYDATA *dd;
  int width, height, depth, srcy, dsty, srcx, dstx;

  srcwif = WIfFromMw(args[0]);
  dstwif = WIfFromMw(args[3]);
  dspif = DspIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);
  display = XtDisplay(srcwif->windowwidget);
  swin = XtWindow(srcwif->windowwidget);
  dwin = XtWindow(dstwif->windowwidget);

  if (!srcwif->open) return (NIL); /* If window not open, don't do anything. */
  if (!dstwif->open) return (NIL); /* If window not open, don't do anything. */

  /* Transform the coordinates */
  width = LispIntToCInt(args[6]);
  height = LispIntToCInt(args[7]);
  dsty = dstwif->innerregion.height - translate_y(dstwif, LispIntToCInt(args[5])) - height;
  srcy = srcwif->innerregion.height - translate_y(srcwif, LispIntToCInt(args[2])) - height;
  srcx = translate_x(srcwif, LispIntToCInt(args[1]));
  dstx = translate_x(dstwif, LispIntToCInt(args[4]));

  switch (args[8] & 0xF) {
    case INVERT:
      XCopyArea(display, swin, dwin, srcwif->InvertGC, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, srcwif->backing, dstwif->backing, dspif->PixIGC, srcx, srcy, width, height,
                dstx, dsty);
      break;

    case ERASE:
      XCopyArea(display, swin, dwin, srcwif->EraseGC1, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, swin, dwin, srcwif->EraseGC2, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, srcwif->backing, dstwif->backing, dspif->PixEGC, srcx, srcy, width, height,
                dstx, dsty);
      break;

    case PAINT:
      XCopyArea(display, swin, dwin, srcwif->PaintGC1, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, swin, dwin, srcwif->PaintGC2, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, srcwif->backing, dstwif->backing, dspif->PixPGC, srcx, srcy, width, height,
                dstx, dsty);
      break;

    case REPLACE:
      XCopyArea(display, swin, dwin, srcwif->ReplaceGC, srcx, srcy, width, height, dstx, dsty);
      XCopyArea(display, srcwif->backing, dstwif->backing, dspif->PixRGC, srcx, srcy, width, height,
                dstx, dsty);
      break;
  }

  XFlush(display);
  return (ATOM_T);
}

/************************************************************************/
/* MNXcloseW                                                            */
/* Close a window and save the inner region of the window.              */
/* NOTE! we don't save the whiteframe, blackframe or the title. This is */
/* contrary to Medleys own window system. If you do tricks with the     */
/* SAVE field of the window, reconsider your evil ways.                 */
/************************************************************************/
MNXcloseW(LispArgs args)
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  BITMAP *bitmap;

  display = XDisplayFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);

  if (!wif->open) return (NIL); /* If window not open, don't close it. */
  wif->open = 0;                /* If it is, tell others it's closed.  */

  bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(args[0]))->SAVE);
  DrawableToMBM(bitmap, dspif, wif, wif->backing, wif->screen, 0, 0,
                /* wif->innerregion.x, wif->innerregion.y, */ 0, 0, wif->innerregion.width,
                wif->innerregion.height, REPLACE);
  XtUnrealizeWidget(wif->topwidget);

  XtRemoveEventHandler(wif->framewidget, ButtonPressMask | ButtonReleaseMask, False, HandleButton,
                       wif);
  XtRemoveEventHandler(wif->framewidget, PointerMotionMask, False, HandleMotion, wif);
  XtRemoveEventHandler(wif->framewidget, KeyPressMask | KeyReleaseMask, False, HandleKey, wif);
  XtRemoveEventHandler(wif->framewidget, EnterWindowMask | LeaveWindowMask, False, HandleCrossing,
                       wif);
  XtRemoveEventHandler(wif->framewidget, FocusChangeMask, False, HandleFocus, wif);
  XtRemoveEventHandler(wif->framewidget, ExposureMask, False, HandleTitle, wif);
  XtRemoveEventHandler(wif->windowwidget, ExposureMask, False, HandleExpose, wif);
  XtRemoveEventHandler(wif->topwidget, StructureNotifyMask, False, HandleStructure, wif);
  wif->not_exposed = 1; /* next time we open this, display it */
  XFlush(display);
  return (ATOM_T);
}

WindowInterface removewif(chain, wif) WindowInterface chain, wif;
{
  /* Recursive unlink off the wif from chain */
  if (chain == (WindowInterface)NULL)
    return (NULL);
  else if (chain == wif)
    return (chain->next);
  else {
    chain->next = removewif(chain->next, wif);
    return (chain);
  }
}

/************************************************************************/
/* bubblewif                                                            */
/* Make shure that the wif is the first wif in the chain of wifs on the */
/* dspif. If not, make it so.                                           */
/* Return wif if we find wif. Return NIL if we don't find wif.          */
/*                                                                      */
/************************************************************************/
WindowInterface bubblewif(dspif, wif) DspInterface dspif;
WindowInterface wif;
{
  WindowInterface curr, prev;

  if (dspif->CreatedWifs == (WindowInterface)NULL) return (NIL);
  if (dspif->CreatedWifs == wif) return (wif);

  /* Find the wif we are interested in. */
  for (prev = dspif->CreatedWifs, curr = prev->next;
       ((curr != (WindowInterface)NULL) && (curr != wif)); curr = curr->next, prev = prev->next)
    ;
  if (curr == (WindowInterface)NULL) return (NIL); /* wif not found */

  /* Bubble curr to the head of the list */
  prev->next = curr->next;
  curr->next = dspif->CreatedWifs;
  dspif->CreatedWifs = curr;
  return (wif);
}

/************************************************************************/
/* destroyw/Xdestroyw                                                   */
/* Destroy a window by freeing all structures associated with it.       */
/*                                                                      */
/************************************************************************/
destroyw(dspif, wif, medleyw) WindowInterface wif;
DspInterface dspif;
MedleyWindow medleyw;

{ /* Tell X to destroy its part. */

  if (wif->topwidget) XtDestroyWidget(wif->topwidget);
  if (wif->InvertGC) XFreeGC(dspif->handle, wif->InvertGC);
  if (wif->EraseGC1) XFreeGC(dspif->handle, wif->EraseGC1);
  if (wif->EraseGC2) XFreeGC(dspif->handle, wif->EraseGC2);
  if (wif->PaintGC1) XFreeGC(dspif->handle, wif->PaintGC1);
  if (wif->PaintGC2) XFreeGC(dspif->handle, wif->PaintGC2);
  if (wif->ReplaceGC) XFreeGC(dspif->handle, wif->ReplaceGC);

  XFreePixmap(dspif->handle, wif->backing);

  dspif->CreatedWifs = removewif(dspif->CreatedWifs, wif);

  /* Throw away the window and free the space. */
  (void)free(wif);
  medleyw->NativeIf = 0;
}

MNXdestroyW(LispArgs args)
{
  /* Medley interface to destroyw */
  destroyw(DspIfFromMw(args[0]), WIfFromMw(args[0]), Cptr(args[0]));
  return (ATOM_T);
}

/************************************************************************/
/* MNXdestroyDisplay                                                      */
/* Closeing down the screen means that we have to invalidate all window */
/* handles. When a window is opened (in Medley) we test the windows     */
/* handle. If it is NIL we have to reexternalize it before we open it.  */
/*                                                                      */
/************************************************************************/

MNXdestroyDisplay(LispArgs args) /* args[0] = LispPTR to MedleyScreen */
{
  DspInterface dspif;
  WindowInterface i;
  MedleyScreen MScreen;
  int Xfd;

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = MScreen->NativeIf;

  Xfd = ConnectionNumber(dspif->handle);
  FD_to_dspif[Xfd] = 0;
  LispReadFds &= ~(1 << Xfd);
  MNWReadFds &= ~(1 << Xfd);

  if (dspif->cursor) XFreeCursor(dspif->handle, dspif->cursor);
  dspif->cursor = 0;

  /* Smash the handles of all the created wifs. */
  if (dspif != NULL) {
    for (i = dspif->CreatedWifs; i != NULL; i = dspif->CreatedWifs)
      destroyw(dspif, i, (MedleyWindow)Cptr(i->MedleyWindow));

    if (dspif->legatewidget) XtDestroyWidget(dspif->legatewidget);
    XtCloseDisplay(dspif->handle);
    (void)free(dspif);
  }
}

MNXmoveW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                              /* args[1] = left */
                              /* args[2] = bottom */
                              /* args[3] = non-NIL => skip actually moving it */
{
  Display *dsp;
  WindowInterface wif;

  dsp = XDisplayFromMw(args[0]);
  wif = WIfFromMw(args[0]);

  if (!wif) return (NIL); /* no real window, no action. */
  wif->moving = 1;        /* tell event handler to expect an event */

  wif->windowreg.x = LispIntToCInt(args[1]);
  wif->windowreg.y = LispIntToCInt(args[2]);
  calcwif(wif, wif->topregion.width - wif->outerregion.width,
          wif->topregion.height - wif->outerregion.height);
  /* stolen from calculateshape */

  /* printf("MOVEW to %d, %d.\n", wif->windowreg.x, wif->windowreg.y); */
  if (!args[3]) {
    if (wif->open)
      XtMoveWidget(wif->topwidget, wif->topregion.x, wif->topregion.y, 0);
    else {
      wif->move_pend = 1; /* Save the move for when we open it. */
    }
    XFlush(dsp);
  }
  return (ATOM_T);
}

MNXshapeW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                               /* args[1] = left */
                               /* args[2] = bottom */
                               /* args[3] = width */
                               /* args[4] = height */
{
  Display *dsp;
  WindowInterface wif;
  XtWidgetGeometry geom;

  wif = WIfFromMw(args[0]);
  if (!wif) return (NIL); /* no real window, no action. */
  wif->reshaping = 1;     /* tell event handler to expect an event */

  dsp = XDisplayFromMw(args[0]);
  calculateshape(wif, args[1], args[2], args[3], args[4],
                 wif->topregion.width - wif->outerregion.width,
                 wif->topregion.height - wif->outerregion.height);
  /* printf("SHAPEW to %d, %d, %d, %d.\n", args[1]&0xFFFF, args[2]&0xFFFF,
                                 args[3]&0xFFFF, args[4]&0xFFFF); */
  if (wif->open) {
    refreshwindow(args[0]);
    XtVaSetValues(wif->formwidget, XtNextentX, 0, XtNextentY, 0, XtNextentWidth, args[3] & 0xFFFF,
                  XtNextentHeight, args[4] & 0xFFFF, NULL);
    XtMoveWidget(wif->topwidget, args[1] & 0xFFFF,
                 HeightOfScreen(wif->screen) - (args[2] & 0xFFFF) - wif->topregion.height);
  } else {
    wif->shape_pend = 1;
  }

  XFreePixmap(wif->dspif->handle, wif->backing);
  wif->backing = XCreatePixmap(dsp, wif->dspif->root, args[3] & 0xFFFF, args[4] & 0xFFFF, 1);

  XFlush(dsp);
  return (ATOM_T);
}

MNXtotopW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
{
  Display *dsp;
  WindowInterface wif;

  dsp = XDisplayFromMw(args[0]);
  wif = WIfFromMw(args[0]);

  if (!wif->open) return (NIL); /* If window not open, don't do anything. */
  XRaiseWindow(dsp, XtWindow(wif->topwidget));
  XFlush(dsp);
  return (ATOM_T);
}

MNXburyW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
{
  Display *dsp;
  WindowInterface wif;

  dsp = XDisplayFromMw(args[0]);
  wif = WIfFromMw(args[0]);

  if (!wif->open) return (NIL); /* If window not open, don't do anything. */
  XLowerWindow(dsp, XtWindow(wif->topwidget));
  XFlush(dsp);
  return (ATOM_T);
}

MNXshrinkW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                /* args[1] = LispPTR to icon MedleyWindow. */
                                /* args[2] = Iconwindow x */
                                /* args[3] = Iconwindow y */
                                /* args[4] = Iconwindow width */
                                /* args[5] = Iconwindow height */
{
  Screen *screen;
  Display *display;
  XImage Xbm;
  XWMHints Lisp_WMhints;
  BITMAP *bitmap;
  WindowInterface wif, iconwif;
  DspInterface dspif;
  int depth;

  dspif = DspIfFromMw(args[0]);
  display = dspif->handle;

  bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(args[0]))->SAVE);
  wif = WIfFromMw(args[0]);

  if (!wif->open) return (NIL); /* If window not open, don't do anything. */
  iconwif = WIfFromMw(args[1]);

  XtVaSetValues(wif->topwidget, XtNiconWindow, iconwif->blackframe, NULL);

  DrawableToMBM(bitmap, dspif, wif, wif->backing, XtScreen(wif->topwidget), 0, 0, 0, 0,
                wif->innerregion.width, wif->innerregion.height, REPLACE);
  XIconifyWindow(XtDisplay(wif->topwidget), XtWindow(wif->topwidget), DefaultScreen(display));
  XFlush(display);
  return (ATOM_T);
}

MNXexpandW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
{
  DspInterface dspif;
  WindowInterface wif;
  BITMAP *bitmap;

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);

  if (!args[0]) return (NIL);
  if (!wif->open) return (NIL); /* If window not open, don't do anything. */
  XMapWindow(XtDisplay(wif->topwidget), XtWindow(wif->topwidget));
  showtitle(args[0]);
  bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(args[0]))->SAVE);
  MBMToDrawable(bitmap, dspif, wif, 0, 0, 0, 0, wif->innerregion.width, wif->innerregion.height,
                REPLACE);
  XFlush(XtDisplay(wif->topwidget));
  return (ATOM_T);
}

MNXcreateW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                /* args[1] = left */
                                /* args[2] = bottom */
                                /* args[3] = width */
                                /* args[4] = height */
{
  XGCValues gcv;
  XVisualInfo vinfo, *vinfo2;
  XSizeHints sizehints;

  Screen *screen;
  Display *display;
  Window *window;
  MedleyScreenRec *mscreen;

  WindowInterface wif;
  DspInterface dspif;

  int width, height, border, whiteborder, blackborder, titleheight;
  int strlen, scrno, vcount;
  char *tmpstring;
  BITMAP *bitmap;
  DISPLAYDATA *dd;

  /***********************************/
  /* Initialize the window structure */
  /***********************************/
  wif = (WindowInterface)malloc(sizeof(WindowInterfaceRec));
  WIfFromMw(args[0]) = wif;
  mscreen = ScrnFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);

  scrno = DefaultScreen(display);
  wif->screen = ScreenOfDisplay(display, scrno);
  wif->Dispatch = &(dspif->ImageOp);
  wif->MedleyWindow = args[0]; /* so we know what WINDOW this corresponds to */
  /* Cache the MScreen for the benefit of the signals. */
  wif->MedleyScreen = ((MedleyWindow)Cptr(args[0]))->SCREEN;
  wif->dspif = dspif;
  wif->fgpixmap = 0; /* Cache for the foreground pixmap. */
  wif->bgpixmap = 0; /* Cache for the foreground pixmap. */
  whiteborder = getwhiteborder(args[0]);
  blackborder = getblackborder(args[0]);
  tmpstring = gettitlestring(args[0]);

  dd = ImDataFromMw(args[0]);
  wif->xoffset = LispIntToCInt(dd->ddxoffset);
  wif->yoffset = LispIntToCInt(dd->ddyoffset);
  /* Special case for Icons and windows with no content and only a titlebar */
  if (wif->innerregion.height == 0) whiteborder = 0;

  calculateshape(wif, args[1], args[2], args[3], args[4], 0, 0);
  /********************************/
  /* Create the X window instance */
  /********************************/
  wif->topwidget = XtVaAppCreateShell(tmpstring, tmpstring, applicationShellWidgetClass, display,
                                      XtNgeometry, wif->gstring, XtNargc, save_argc, XtNargv,
                                      save_argv, XtNborderWidth, 0, NULL);

  wif->formwidget =
      XtCreateManagedWidget("scrollBox", scrollBoxWidgetClass, wif->topwidget, NULL, 0);

  wif->framewidget = XtVaCreateManagedWidget(
      "frame", boxWidgetClass, wif->formwidget, XtNheight, wif->outerregion.height, XtNwidth,
      wif->outerregion.width, XtNborderWidth, blackborder, XtNbackground,
      BlackPixelOfScreen(wif->screen), XtNvSpace, 0, XtNdefaultDistance, 0, XtNhorizDistance, 0,
      XtNvertDistance, 0, NULL);
  boxClassRec.core_class.compress_motion = False;

  wif->windowwidget = XtVaCreateManagedWidget(
      "window", boxWidgetClass, wif->framewidget, XtNheight, wif->innerregion.height, XtNwidth,
      wif->innerregion.width, XtNx, wif->innerregion.x, XtNy, wif->innerregion.y, XtNborderColor,
      WhitePixelOfScreen(wif->screen), XtNborderWidth, whiteborder, XtNvSpace, 0,
      XtNdefaultDistance, 0, XtNhorizDistance, 0, XtNvertDistance, 0, XtNbottom, XtChainBottom,
      XtNleft, XtChainLeft, NULL);

  gcv.function = GXcopy;
  gcv.foreground = dspif->black;
  gcv.background = dspif->white;
  wif->ReplaceGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXxor;
  gcv.foreground = (dspif->black) ^ (dspif->white);
  gcv.background = 0;
  wif->InvertGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXor;
  gcv.foreground = dspif->black & (~dspif->white);
  gcv.background = 0;
  wif->PaintGC1 = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXandInverted;
  gcv.foreground = dspif->white & (~dspif->black);
  gcv.background = 0;
  wif->PaintGC2 = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXor;
  gcv.foreground = dspif->white & (~dspif->black);
  gcv.background = 0;
  wif->EraseGC1 = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXandInverted;
  gcv.foreground = dspif->black & (~dspif->white);
  gcv.background = 0;
  wif->EraseGC2 = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXandInverted;
  gcv.foreground = dspif->black & (~dspif->white);
  gcv.background = 0;
  wif->gc = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  /* Push this wif onto the list of created wifs. */
  wif->next = dspif->CreatedWifs;
  dspif->CreatedWifs = wif;

  wif->backing = XCreatePixmap(display, dspif->root, args[3] & 0xFFFF, args[4] & 0xFFFF, 1);
  bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(args[0]))->SAVE);
  MBMToDrawable(bitmap, dspif, wif, 0, 0, wif->innerregion.x, wif->innerregion.y,
                wif->innerregion.width, wif->innerregion.height, REPLACE);

  free(tmpstring);
  return (ATOM_T);
}

/**************************************************************/
/* showtitle                                                  */
/* Update the title frame of the window. It is assumed that   */
/* the titlehight is the same as the abs(linefeed) height for */
/* the displaydata of the screen.                             */
/**************************************************************/

showtitle(win) LispPTR win;
{
  BITMAP *bitmap;
  DISPLAYDATA *dd;
  WindowInterface wif;
  DspInterface dspif;
  char *tmpstring;
  int xpos, dsty, i, width, stringlen, titleheight;

  wif = WIfFromMw(win);
  dspif = DspIfFromMw(win);

  dd = TitleDDFromMw(win);
  tmpstring = gettitlestring(win);
  titleheight = gettitleheight(win);
  if (tmpstring == NULL) return (ATOM_T);
  stringlen = strlen(tmpstring);

  bitmap = (BITMAP *)Cptr(dd->ddpilotbbt);
  MakeScratchImageFromBM(dspif, bitmap);

  /* Clear the window of prewious gunk. */
  XClearWindow(dspif->handle, wif->blackframe);

  /* Start text flush to the border */
  xpos = LispIntToCInt(((MedleyWindow)Cptr(win))->WBORDER);
  dsty = titleheight - bitmap->bmheight + 2; /* The +2 is ad hoc */

  /* Set the name property in the server */
  XtVaSetValues(wif->topwidget, XtNtitle, tmpstring, NULL);

  /* Add character set switching code... */

  if ((bitmap->bmbitperpixel == 1) || (bitmap->bmbitperpixel == DefaultDepthOfScreen(wif->screen)))
    for (i = 0; i < stringlen; i++) {
      width = GETWORD((DLword *)Cptr(dd->ddwidthscache + tmpstring[i]));
      XPutImage(dspif->handle, wif->blackframe, dspif->TitleGC, &dspif->image,
                GETWORD(Cptr(dd->ddoffsetscache + tmpstring[i])), /* src x */
                0,                                                /* src y */
                xpos,                                             /* dst x */
                dsty,                                             /* ??? dst y */
                width, bitmap->bmheight                           /* height */
                );
      xpos += width;
    }
  else {
    /* Take the long way out. Make an image of the cached bitmap and */
    /* convert it to the depth of the screen. Then dump it to the server */
    int x, y;

    dspif->image.width = bitmap->bmwidth;
    dspif->image.height = bitmap->bmheight;
    dspif->image.bytes_per_line =
        ((bitmap->bmwidth + (BITSPER_DLWORD - 1)) / BITSPER_DLWORD) * (BITSPER_DLWORD / 8);
    dspif->image.depth = DefaultDepthOfScreen(wif->screen);
    dspif->image.data =
        (char *)malloc(dspif->image.bytes_per_line * dspif->image.width * dspif->image.depth);

    dspif->tmpimage.width = bitmap->bmwidth;
    dspif->tmpimage.height = bitmap->bmheight;
    dspif->tmpimage.depth = bitmap->bmbitperpixel & 0xFFFF;
    dspif->tmpimage.data = (char *)Cptr((LispPTR)bitmap->bmbase);
    dspif->tmpimage.bytes_per_line =
        ((bitmap->bmwidth + (BITSPER_DLWORD - 1)) / BITSPER_DLWORD) * (BITSPER_DLWORD / 8);
    for (x = 0; x < bitmap->bmwidth; x++)
      for (y = 0; y < bitmap->bmheight; y++)
        XPutPixel(&dspif->image, x, y, XGetPixel(&dspif->tmpimage, x, y));
    for (i = 0; i < stringlen; i++) {
      width = GETWORD((DLword *)Cptr(dd->ddwidthscache + tmpstring[i]));
      XPutImage(dspif->handle, wif->blackframe, dspif->TitleGC, &dspif->image,
                GETWORD(Cptr(dd->ddoffsetscache + tmpstring[i])), /* src x */
                0,                                                /* src y */
                xpos,                                             /* dst x */
                dsty,                                             /* ??? dst y */
                width, bitmap->bmheight                           /* height */
                );
      xpos += width;
    }
    free(dspif->image.data);
  }
  free(tmpstring);
  return (ATOM_T);
}

/************************************************************************/
/* MNXopenW                                                               */
/* Open a window and get the inner region of the window from the SAVE   */
/* slot on the MedWindow                                                */
/* NOTE! we don't get whiteframe, blackframe or the title. See MNXcloseW. */
/************************************************************************/
MNXopenW(LispArgs args)
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  char *tmpstring;
  BITMAP *bitmap;
  XGCValues gcv;
  Window window, root, parent, *children;
  int nch;

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = dspif->handle;

  XtAddEventHandler(wif->framewidget, ButtonPressMask | ButtonReleaseMask, False, HandleButton,
                    wif);
  XtAddEventHandler(wif->framewidget, PointerMotionMask, False, HandleMotion, wif);
  XtAddEventHandler(wif->framewidget, KeyPressMask | KeyReleaseMask, False, HandleKey, wif);
  XtAddEventHandler(wif->framewidget, EnterWindowMask | LeaveWindowMask, False, HandleCrossing,
                    wif);
  XtAddEventHandler(wif->framewidget, FocusChangeMask, False, HandleFocus, wif);
  XtAddEventHandler(wif->framewidget, ExposureMask, False, HandleTitle, wif);
  XtAddEventHandler(wif->windowwidget, ExposureMask, False, HandleExpose, wif);
  XtAddEventHandler(wif->topwidget, StructureNotifyMask, False, HandleStructure, wif);

  XtRealizeWidget(wif->topwidget);
#ifdef NEVER
  if (wif->shape_pend) /* We reshaped while closed; do it for real here */
  {
    refreshwindow(args[0]);
    XtVaSetValues(wif->formwidget, XtNextentX, 0, XtNextentY, 0, XtNextentWidth,
                  wif->windowreg.width, XtNextentHeight, wif->windowreg.height, NULL);
    XtMoveWidget(wif->topwidget, wif->topregion.x, wif->topregion.y);
    wif->move_pend = wif->shape_pend = 0;
  } else if (wif->move_pend) /* We moved while closed; do the move here. */
  {
    wif->move_pend = 0;
    XtMoveWidget(wif->topwidget, wif->topregion.x, wif->topregion.y, 0);
  }
#endif
  wif->blackframe = XtWindow(wif->framewidget);
  wif->handle = window = XtWindow(wif->windowwidget);

  /* wif->parent = XtWindow(wif->topwidget); */
  XQueryTree(display, XtWindow(wif->topwidget), &root, &parent, &children, &nch);
  wif->parent = parent;
  XFree(children);

  refreshwindow(args[0]);
  settileorigin(args[0], 0, wif->innerregion.height);
  settexture(args[0]);
  showtitle(args[0]);
  XtSetSensitive(wif->framewidget, True);
  XtSetSensitive(wif->windowwidget, True);

  if (!wif->backing) {
    wif->backing =
        XCreatePixmap(display, window, wif->outerregion.width, wif->outerregion.height, 1);
  }

  bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(args[0]))->SAVE);
  MBMToDrawable(bitmap, dspif, wif, 0, 0,
                /* wif->innerregion.x, wif->innerregion.y, */ 0, 0, wif->innerregion.width,
                wif->innerregion.height, REPLACE);

  XRaiseWindow(display, XtWindow(wif->topwidget));

  XFlush(display);
  wif->open = 1; /* mark the window open. */
  return (ATOM_T);
}

MNXresetW(LispArgs args)
{
  Display *display;
  WindowInterface wif;
  DISPLAYDATA *dd;

  display = XDisplayFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) return (NIL); /* if the window is closed, do nothing */
  if (XtIsRealized(wif->topwidget)) {
#ifdef NEVER
    showtitle(args[0]);
    settexture(args[0]);
    XClearWindow(display, XtWindow(wif->windowwidget));
#else
    bltshade_internal(display, 0, dd->ddtexture, 0, 0, wif->innerregion.width,
                      wif->innerregion.height, wif, wif->dspif);
#endif /* NEVER */
  }
  XFlush(display);
  return (ATOM_T);
}

MNXSetOffsets(LispArgs args) /* args[0] = window XOFFSET/YOFFSET changed in */
                                   /* args[1] = new XOFFSET */
                                   /* args[2] = new YOFFSET */
{
  Display *display;
  WindowInterface wif;

  wif = WIfFromMw(args[0]);
  wif->xoffset = LispIntToCInt(args[1]);
  wif->yoffset = LispIntToCInt(args[2]);
}

/*********************/
/* Imageop Methods.  */
/*********************/

MNXdrawpoint(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                  /* args[1] = x */
                                  /* args[2] = y */
                                  /* args[3] = brush */
{
  Display *display;
  Window window;
  DspInterface dspif;
  WindowInterface wif;
  int width, x, y;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  dd = ImDataFromMw(args[0]);
  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  if (args[3] == NIL)
    width = 0;
  else
    width = LispIntToCInt(args[3]);

  x = translate_x(wif, LispIntToCInt(args[1]));
  y = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2]));

  /** Handle the four operation cases **/
  if (dd->ddoperation == INVERT_atom) {
    XDrawPoint(display, window, wif->InvertGC, x, y);
    XDrawPoint(display, wif->backing, dspif->PixIGC, x, y);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawPoint(display, window, wif->EraseGC1, x, y);
    XDrawPoint(display, window, wif->EraseGC2, x, y);
    XDrawPoint(display, wif->backing, dspif->PixEGC, x, y);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawPoint(display, window, wif->PaintGC1, x, y);
    XDrawPoint(display, window, wif->PaintGC2, x, y);
    XDrawPoint(display, wif->backing, dspif->PixPGC, x, y);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawPoint(display, window, wif->ReplaceGC, x, y);
    XDrawPoint(display, wif->backing, dspif->PixRGC, x, y);
  }

  XFlush(display);
  return (NIL);
}

/**************************************************************/
/* MNXdrawline                                                  */
/* color not yet implemented                                  */
/* Medleys Drawline can't handle dashing.                     */
/* Thick lines in Medley are handled as parallellograms       */
/* instead of rectangles.                                     */
/* Remember that this method has an OPERATION arument in Lisp */
/* so we have to fix that there.                              */
/**************************************************************/
MNXdrawline(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                 /* args[1] = x1 */
                                 /* args[2] = y1 */
                                 /* args[3] = x2 */
                                 /* args[4] = y2 */
                                 /* args[5] = width */
                                 /* Args[6] = dashing */
{
  WindowInterface wif;
  Display *display;
  Window window;
  DspInterface dspif;
  int y1, y2, x1, x2;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */
  dd = ImDataFromMw(args[0]);
  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  /* Transform the Y coordinates */
  y1 = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2]));
  y2 = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[4]));
  x1 = translate_x(wif, LispIntToCInt(args[1]));
  x2 = translate_x(wif, LispIntToCInt(args[3]));

  setlineattributes(display, dspif, wif, dd, args[5], args[6]);

  if (dd->ddoperation == INVERT_atom) {
    XDrawLine(display, window, wif->InvertGC, x1, y1, x2, y2);
    XDrawLine(display, wif->backing, dspif->PixIGC, x1, y1, x2, y2);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawLine(display, window, wif->EraseGC1, x1, y1, x2, y2);
    XDrawLine(display, window, wif->EraseGC2, x1, y1, x2, y2);
    XDrawLine(display, wif->backing, dspif->PixEGC, x1, y1, x2, y2);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawLine(display, window, wif->PaintGC1, x1, y1, x2, y2);
    XDrawLine(display, window, wif->PaintGC2, x1, y1, x2, y2);
    XDrawLine(display, wif->backing, dspif->PixPGC, x1, y1, x2, y2);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawLine(display, window, wif->ReplaceGC, x1, y1, x2, y2);
    XDrawLine(display, wif->backing, dspif->PixRGC, x1, y1, x2, y2);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdrawcircle                                                */
/* Note: 23040 is 360 * 64 sixtyfourth degrees                */
/* It is assumed that the BRUSH argument is a fixp that       */
/* represents a ROUND brush.                                  */
/**************************************************************/
MNXdrawcircle(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                   /* args[1] = centerX */
                                   /* args[2] = centerY */
                                   /* args[3] = radius */
                                   /* args[4] = brush */
                                   /* args[5] = dashing */
{
  int radius, d, x, y;
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  radius = LispIntToCInt(args[3]);
  d = radius << 1;
  y = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2])) - radius;
  x = translate_x(wif, LispIntToCInt(args[1])) - radius;

  setlineattributes(display, dspif, wif, dd, args[4], args[5]);

  if (dd->ddoperation == INVERT_atom) {
    XDrawArc(display, window, wif->InvertGC, x, y, d, d, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixIGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawArc(display, window, wif->EraseGC1, x, y, d, d, 0, 23040);
    XDrawArc(display, window, wif->EraseGC2, x, y, d, d, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixEGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawArc(display, window, wif->PaintGC1, x, y, d, d, 0, 23040);
    XDrawArc(display, window, wif->PaintGC2, x, y, d, d, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixPGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawArc(display, window, wif->ReplaceGC, x, y, d, d, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixRGC, x, y, d, d, 0, 23040);
  }
  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdrawarc                                                   */
/* It is assumed that the BRUSH argument is a fixp that       */
/* represents a ROUND brush.                                  */
/**************************************************************/
MNXdrawarc(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                /* args[1] = centerX */
                                /* args[2] = centerY */
                                /* args[3] = radius */
                                /* args[4] = startangle in 64'ths degrees */
                                /* args[5] = ndegrees in 64'ths degrees */
                                /* args[6] = brush */
                                /* args[7] = dashing */
                                /* Brush and dashing args are ignored for the moment. */
{
  int radius, d, x, y, start, ndeg;
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  radius = LispIntToCInt(args[3]);
  d = radius << 1;
  x = translate_x(wif, LispIntToCInt(args[1])) - radius;
  y = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2])) - radius;
  start = LispIntToCInt(args[4]);
  ndeg = LispIntToCInt(args[5]);

  setlineattributes(display, dspif, wif, dd, args[6], args[7]);

  if (dd->ddoperation == INVERT_atom) {
    XDrawArc(display, window, wif->InvertGC, x, y, d, d, start, ndeg);
    XDrawArc(display, wif->backing, dspif->PixIGC, x, y, d, d, start, ndeg);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawArc(display, window, wif->EraseGC1, x, y, d, d, start, ndeg);
    XDrawArc(display, window, wif->EraseGC2, x, y, d, d, start, ndeg);
    XDrawArc(display, wif->backing, dspif->PixEGC, x, y, d, d, start, ndeg);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawArc(display, window, wif->PaintGC1, x, y, d, d, start, ndeg);
    XDrawArc(display, window, wif->PaintGC2, x, y, d, d, start, ndeg);
    XDrawArc(display, wif->backing, dspif->PixPGC, x, y, d, d, start, ndeg);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawArc(display, window, wif->ReplaceGC, x, y, d, d, start, ndeg);
    XDrawArc(display, wif->backing, dspif->PixRGC, x, y, d, d, start, ndeg);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdrawelipse                                                */
/* Note: 23040 is 360 * 64 sixtyfourth degrees                */
/* It is assumed that the BRUSH argument is a fixp that       */
/* represents a ROUND brush.                                  */
/**************************************************************/
MNXdrawelipse(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                   /* args[1] = centerX */
                                   /* args[2] = centerY */
                                   /* args[3] = semiminorradius */
                                   /* args[4] = semimajorradius */
                                   /* args[5] = orientation */
                                   /* args[6] = brush */
                                   /* args[7] = dashing */

{
  int radius, d1, d2, x, y;
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  x = translate_x(wif, LispIntToCInt(args[1])) - radius;
  y = wif->innerregion.height - translate_x(wif, LispIntToCInt(args[2])) - radius;
  d1 = LispIntToCInt(args[3]);
  d2 = LispIntToCInt(args[4]);

  setlineattributes(display, dspif, wif, dd, args[6], args[7]);

  if (dd->ddoperation == INVERT_atom) {
    XDrawArc(display, window, wif->InvertGC, x, y, d1, d2, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixIGC, x, y, d1, d2, 0, 23040);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawArc(display, window, wif->EraseGC1, x, y, d1, d2, 0, 23040);
    XDrawArc(display, window, wif->EraseGC2, x, y, d1, d2, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixEGC, x, y, d1, d2, 0, 23040);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawArc(display, window, wif->PaintGC1, x, y, d1, d2, 0, 23040);
    XDrawArc(display, window, wif->PaintGC2, x, y, d1, d2, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixPGC, x, y, d1, d2, 0, 23040);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawArc(display, window, wif->ReplaceGC, x, y, d1, d2, 0, 23040);
    XDrawArc(display, wif->backing, dspif->PixRGC, x, y, d1, d2, 0, 23040);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXfillcircle                                                */
/* Note: 23040 is 360 * 64 sixtyfourth degrees                */
/**************************************************************/
MNXfillcircle(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                   /* args[1] = centerX */
                                   /* args[2] = centerY */
                                   /* args[3] = radius */
                                   /* Brush and dashing args are ignored for the moment. */
{
  int radius, d, x, y;
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  radius = LispIntToCInt(args[3]);
  d = radius << 1;
  x = translate_x(wif, LispIntToCInt(args[1])) - radius;
  y = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2])) - radius;

  if (dd->ddoperation == INVERT_atom) {
    XFillArc(display, window, wif->InvertGC, x, y, d, d, 0, 23040);
    XFillArc(display, wif->backing, dspif->PixIGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == ERASE_atom) {
    XFillArc(display, window, wif->EraseGC1, x, y, d, d, 0, 23040);
    XFillArc(display, window, wif->EraseGC2, x, y, d, d, 0, 23040);
    XFillArc(display, wif->backing, dspif->PixEGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == PAINT_atom) {
    XFillArc(display, window, wif->PaintGC1, x, y, d, d, 0, 23040);
    XFillArc(display, window, wif->PaintGC2, x, y, d, d, 0, 23040);
    XFillArc(display, wif->backing, dspif->PixPGC, x, y, d, d, 0, 23040);
  } else if (dd->ddoperation == REPLACE_atom) {
    XFillArc(display, window, wif->ReplaceGC, x, y, d, d, 0, 23040);
    XFillArc(display, wif->backing, dspif->PixRGC, x, y, d, d, 0, 23040);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXwritepixel                                                */
/**************************************************************/
MNXwritepixel(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                   /* args[1] = x */
                                   /* args[2] = y */
{
  int x, y;
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  x = translate_x(wif, LispIntToCInt(args[1]));
  y = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2]));

  if (dd->ddoperation == INVERT_atom) {
    XDrawPoint(display, window, wif->InvertGC, x, y);
    XDrawPoint(display, wif->backing, dspif->PixIGC, x, y);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawPoint(display, window, wif->EraseGC1, x, y);
    XDrawPoint(display, window, wif->EraseGC2, x, y);
    XDrawPoint(display, wif->backing, dspif->PixEGC, x, y);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawPoint(display, window, wif->PaintGC1, x, y);
    XDrawPoint(display, window, wif->PaintGC2, x, y);
    XDrawPoint(display, wif->backing, dspif->PixPGC, x, y);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawPoint(display, window, wif->ReplaceGC, x, y);
    XDrawPoint(display, wif->backing, dspif->PixRGC, x, y);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdrawpolygon                                               */
/* Lisp sends down a pointer to a vector of points. The vector*/
/* is organized as [x1,y1,x2,y2,...] MNXdrawpolygon smashes this*/
/* vector with the integer values converted to C integers.    */
/**************************************************************/
MNXdrawpolygon(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                    /* args[1] = LispPTR to Pointvector */
                                    /* args[2] = Closedp */
{
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  int i;
  Arrayp *pv;
  short *xpt, y;
  long *Mpt;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  pv = (Arrayp *)Cptr(args[1]);
  Mpt = (long *)Cptr(pv->base);
  xpt = (short *)Cptr(pv->base);

  /* XPoint is defined as shorts, base is defined as longs. */
  /* Pack the vector. */
  for (i = 0; i < pv->length; i++) xpt[i] = (short)Mpt[i];

  /* Transmogrify the y coordinates */
  y = (short)wif->innerregion.height;
  for (i = 1; i < pv->length; i += 2) {
    xpt[i] = y - translate_y(wif, xpt[i]);
    xpt[i - 1] = translate_x(wif, xpt[i - 1]);
  }
  /* If CLOSED then set last point are eq to the first. */
  if (args[2] != NIL) {
    xpt[pv->length++] = xpt[0];
    xpt[pv->length++] = xpt[1];
  }

  if (dd->ddoperation == INVERT_atom) {
    XDrawLines(display, window, wif->InvertGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixIGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawLines(display, window, wif->EraseGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, window, wif->EraseGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixEGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawLines(display, window, wif->PaintGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, window, wif->PaintGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixPGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawLines(display, window, wif->ReplaceGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixRGC, xpt, (pv->length >> 1), CoordModeOrigin);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

MNXfillpolygon(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                    /* args[1] = LispPTR to Pointvector */
                                    /* args[2] = Closedp */
{
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  int i;
  Arrayp *pv;
  short *xpt, y;
  long *Mpt;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  pv = (Arrayp *)Cptr(args[1]);
  Mpt = (long *)Cptr(pv->base);
  xpt = (short *)Cptr(pv->base);

  /* XPoint is defined as shorts, base is defined as longs. */
  /* Pack the vector. */
  for (i = 0; i < pv->length; i++) xpt[i] = (short)Mpt[i];

  /* Transmogrify the y coordinates */
  y = (short)wif->innerregion.height;
  for (i = 1; i < pv->length; i += 2) {
    xpt[i] = y - translate_y(wif, xpt[i]);
    xpt[i - 1] = translate_x(wif, xpt[i - 1]);
  }

  /* If CLOSED then set last point are eq to the first. */
  if (args[2] != NIL) {
    xpt[pv->length++] = xpt[0];
    xpt[pv->length++] = xpt[1];
  }

  if (dd->ddoperation == INVERT_atom) {
    XFillPolygon(display, window, wif->InvertGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, wif->backing, dspif->PixIGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == ERASE_atom) {
    XFillPolygon(display, window, wif->EraseGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, window, wif->EraseGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, wif->backing, dspif->PixEGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == PAINT_atom) {
    XFillPolygon(display, window, wif->PaintGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, window, wif->PaintGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, wif->backing, dspif->PixPGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == REPLACE_atom) {
    XFillPolygon(display, window, wif->ReplaceGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XFillPolygon(display, wif->backing, dspif->PixRGC, xpt, (pv->length >> 1), CoordModeOrigin);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdrawcurve                                                 */
/* NOTE: Not yet implemented. Works like MNXdrawpolygon!!!!!    */
/* Lisp sends down a pointer to a vector of points. The vector*/
/* is organized as [x1,y1,x2,y2,...] MNXdrawpolygon smashes this*/
/* vector with the integer values converted to C integers.    */
/**************************************************************/
MNXdrawcurve(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                  /* args[1] = LispPTR to Pointvector */
                                  /* args[2] = Closedp */
{
  WindowInterface wif;
  Window window;
  Display *display;
  DspInterface dspif;
  int i;
  Arrayp *pv;
  short *xpt, y;
  long *Mpt;
  DISPLAYDATA *dd;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  pv = (Arrayp *)Cptr(args[1]);
  Mpt = (long *)Cptr(pv->base);
  xpt = (short *)Cptr(pv->base);

  /* XPoint is defined as shorts, base is defined as longs. */
  /* Pack the vector. */
  for (i = 0; i < pv->length; i++) xpt[i] = (short)Mpt[i];

  /* Transmogrify the y coordinates */
  y = (short)wif->innerregion.height;
  for (i = 1; i < pv->length; i += 2) {
    xpt[i] = y - translate_y(wif, xpt[i]);
    xpt[i - 1] = translate_x(wif, xpt[i - 1]);
  }

  /* If CLOSED then set last point are eq to the first. */
  if (args[2] != NIL) {
    xpt[pv->length++] = xpt[0];
    xpt[pv->length++] = xpt[1];
  }

  if (dd->ddoperation == INVERT_atom) {
    XDrawLines(display, window, wif->InvertGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixIGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == ERASE_atom) {
    XDrawLines(display, window, wif->EraseGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, window, wif->EraseGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixEGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == PAINT_atom) {
    XDrawLines(display, window, wif->PaintGC1, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, window, wif->PaintGC2, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixPGC, xpt, (pv->length >> 1), CoordModeOrigin);
  } else if (dd->ddoperation == REPLACE_atom) {
    XDrawLines(display, window, wif->ReplaceGC, xpt, (pv->length >> 1), CoordModeOrigin);
    XDrawLines(display, wif->backing, dspif->PixRGC, xpt, (pv->length >> 1), CoordModeOrigin);
  }

  XFlush(display);
  return (NIL); /* Report that all is ok. */
}

/************************************************************/
/* MNXclippingregion                                          */
/************************************************************/
MNXclippingregion(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                       /* args[1] = left */
                                       /* args[2] = bottom */
                                       /* args[3] = width */
                                       /* args[4] = height */

{
  WindowInterface wif;
  Window window;
  DspInterface dspif;
  Display *display;
  XRectangle rect;
  DISPLAYDATA *dd;

  /* This is a noop. We read the region straight from the displaydata slots. */

  return (ATOM_T);
}

/**************************************************************/
/* MNXoperation                                                 */
/* Sets the GC for the window according to the following:     */
/* 0=replace, 1=paint, 2=invert, 3=erase                      */
/* Remember that some functions have operation as argument and*/
/* Thus have to call this function explicitly twice. Once to  */
/* set the new state and once to put the old state back.      */
/**************************************************************/
MNXoperation(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                  /* args[1] = Smallp for function. */
{
  WindowInterface wif;
  Window window;
  DspInterface dspif;
  Display *display;
  DISPLAYDATA *dd;

  wif = WIfFromMw(args[0]);
  window = XWindowFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  /* This is a noop. We manipulate the GC from the displaydata slots. */

  if (dd->ddoperation == REPLACE_atom)
    wif->op = 0;
  else if (dd->ddoperation == PAINT_atom)
    wif->op = 1;
  else if (dd->ddoperation == INVERT_atom)
    wif->op = 2;
  else if (dd->ddoperation == ERASE_atom)
    wif->op = 3;

  return (NIL);
}

/**************************************************************/
/* MNXdspcolor                                                  */
/*                                                            */
/*                                                            */
/*                                                            */
/*                                                            */
/**************************************************************/
MNXdspcolor(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                 /* args[1] = LispPTR to FIXP or BITMAPP */
{
  WindowInterface wif;
  DspInterface dspif;
  Display *display;
  DISPLAYDATA *dd;
  int fgcol;
  char id[64];

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);
  dd = ImDataFromMw(args[0]);

  if (wif->fgpixmap) {
    /* If we have an fgpixmap, junk it */
    XFreePixmap(display, wif->fgpixmap);
    wif->fgpixmap = 0;
  }

  if (TYPE_SMALLP == GetTypeNumber(args[1]))
    if (LispIntToCInt(MScrFromMw(args[0])->SCDEPTH) == 1) {
      /* A fixp on a BW screen: make a texture of this */
      dspif->image.data = id;
      MakeScratchImageFromInt(dspif, LispIntToCInt(args[1]));
    } else { /* A fixp on a color screen: color the foreground */
      fgcol = ~LispIntToCInt(args[1]);
      XSetForeground(display, wif->InvertGC, fgcol);
      XSetForeground(display, wif->EraseGC1, fgcol);
      XSetForeground(display, wif->EraseGC2, fgcol);
      XSetForeground(display, wif->PaintGC1, fgcol);
      XSetForeground(display, wif->PaintGC2, fgcol);
      XSetForeground(display, wif->ReplaceGC, fgcol);
      return (NIL);
    }
  else {
    BITMAP *bitmap;

    bitmap = (BITMAP *)Cptr(args[1]);
    MakeScratchImageFromBM(dspif, bitmap);
  }

  if (dspif->image.depth == 1) {
    wif->fgpixmap = XCreatePixmapFromBitmapData(dspif->handle, XtWindow(wif->windowwidget),
                                                dspif->image.data, dspif->image.width,
                                                dspif->image.height, 0, 1, dspif->image.depth);
    XSetStipple(display, wif->InvertGC, wif->fgpixmap);
    XSetStipple(display, wif->EraseGC1, wif->fgpixmap);
    XSetStipple(display, wif->EraseGC2, wif->fgpixmap);
    XSetStipple(display, wif->PaintGC1, wif->fgpixmap);
    XSetStipple(display, wif->PaintGC2, wif->fgpixmap);
    XSetStipple(display, wif->ReplaceGC, wif->fgpixmap);
  } else {
    wif->fgpixmap = XCreatePixmapFromBitmapData(
        dspif->handle, XtWindow(wif->windowwidget), dspif->image.data, dspif->image.width,
        dspif->image.height, BlackPixelOfScreen(wif->screen), WhitePixelOfScreen(wif->screen),
        dspif->image.depth);
    XSetTile(display, wif->InvertGC, wif->fgpixmap);
    XSetTile(display, wif->EraseGC1, wif->fgpixmap);
    XSetTile(display, wif->EraseGC2, wif->fgpixmap);
    XSetTile(display, wif->PaintGC1, wif->fgpixmap);
    XSetTile(display, wif->PaintGC2, wif->fgpixmap);
    XSetTile(display, wif->ReplaceGC, wif->fgpixmap);
  }
  return (NIL); /* Report that all is ok. */
}

/**************************************************************/
/* MNXdspbackcolor                                              */
/*                                                            */
/*                                                            */
/*                                                            */
/*                                                            */
/**************************************************************/
MNXdspbackcolor(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
/* args[1] = background type 0=fixp for color 1=bitmap for stiple. */
/* args[2] = background, fixp or bitmap */
{ settexture(args[0]); }

/************************************************************/
/* MNXBitblt - can only copy from bitmap to window. we need   */
/* to make it do WINDOW -> BITMAP and WINDOW -> WINDOW      */
/************************************************************/
MNXBitBltBW() { error("Call to MNXBitBltBW. This function is not in use anymore."); }

MNXbltshadeBW(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                   /* args[1] = LispPTR to TEXTURE */
                                   /* args[2] = dst_x */
                                   /* args[3] = dst_y */
                                   /* args[4] = width */
                                   /* args[5] = height */
                                   /* args[6] = operation */
                                   /* args[7] = background, fixp or bitmap */
{
  WindowInterface wif;
  DspInterface dspif;
  Display *display;
  Screen *screen;
  int width, height, depth, regheight, dstx, dsty;
  char tmpstring[128], *data;
  int op = args[6] & 0xFFFF;

  wif = WIfFromMw(args[0]);
  dspif = DspIfFromMw(args[0]);
  display = XDisplayFromMw(args[0]);

  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  regheight = LispIntToCInt(args[5]);
  width = LispIntToCInt(args[4]);

  dstx = translate_x(wif, LispIntToCInt(args[2]));
  dsty = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[3])) - regheight;

  bltshade_internal(display, op, args[1], dstx, dsty, width, regheight, wif, dspif);
}

bltshade_internal(display, op, texture, dstx, dsty, width, regheight, wif, dspif) Display *display;
LispPTR texture;
int op, dstx, dsty, width, regheight;
WindowInterface wif;
DspInterface dspif;
{
  Pixmap bgpixmap;
  char id[64];

  if (TYPE_SMALLP == GetTypeNumber(texture)) {
#ifdef NEVER
    if (dspif->depth == 1) {
#endif /* NEVER */
      dspif->image.data = id;
      MakeScratchImageFromInt(dspif, texture & 0xFFFF);
#ifdef NEVER
    } else { /* Medley sez' color the foreground */
      XSetForeground(display, wif->gc, ~LispIntToCInt(texture));
      XFillRectangle(display, XtWindow(wif->windowwidget), wif->gc, dstx, dsty, width, regheight);
      XFlush(display);
      return (ATOM_T);
    }
#endif     /* NEVER */
  } else { /* Its a bitmap. Medley sez' make a texture of this */
    BITMAP *bitmap;

    bitmap = (BITMAP *)Cptr(texture);
    MakeScratchImageFromBM(dspif, bitmap);
  }
#ifdef NEVER
  if (dspif->image.depth == 1) {
#endif /* NEVER */
    bgpixmap = XCreatePixmapFromBitmapData(dspif->handle, XtWindow(wif->windowwidget),
                                           dspif->image.data, dspif->image.width,
                                           dspif->image.height, 0, 1, dspif->image.depth);
    XSetStipple(display, wif->gc, bgpixmap);
#ifdef NEVER
  } else {
    bgpixmap = XCreatePixmapFromBitmapData(dspif->handle, XtWindow(wif->windowwidget),
                                           dspif->image.data, dspif->image.width,
                                           dspif->image.height, BlackPixelOfScreen(wif->screen),
                                           WhitePixelOfScreen(wif->screen), dspif->image.depth);
    XSetTile(display, wif->gc, bgpixmap);
  }
#endif /* NEVER */

  XSetFunction(display, wif->gc, bltshade_function[op % 4]);
  switch (op) {
    case 0:
      /* XSetFillStyle(display, wif->gc, FillOpaqueStippled); */
      XSetFillStyle(display, dspif->PixRGC, FillOpaqueStippled);
      XSetStipple(display, dspif->PixRGC, bgpixmap);
      XSetTSOrigin(display, dspif->PixRGC, dstx, dsty);
      XFillRectangle(display, wif->backing, dspif->PixRGC, dstx, dsty, width, regheight);
      break;
    case 1:
      XSetFillStyle(display, wif->gc, FillStippled);
      XSetFillStyle(display, dspif->PixRGC, FillStippled);
      XSetStipple(display, dspif->PixRGC, bgpixmap);
      XSetTSOrigin(display, dspif->PixRGC, dstx, dsty);
      XFillRectangle(display, wif->backing, dspif->PixRGC, dstx, dsty, width, regheight);
      break;
    case 2:
      XSetForeground(display, wif->gc, dspif->white ^ dspif->black);
      XSetFillStyle(display, wif->gc, FillStippled);
      XSetFillStyle(display, dspif->PixIGC, FillStippled);
      XSetStipple(display, dspif->PixIGC, bgpixmap);
      XSetTSOrigin(display, dspif->PixIGC, dstx, dsty);
      XFillRectangle(display, wif->backing, dspif->PixIGC, dstx, dsty, width, regheight);
      break;
    case 3:
      XSetForeground(display, wif->gc, dspif->white);
      XSetFillStyle(display, wif->gc, FillStippled);
      XSetFillStyle(display, dspif->PixEGC, FillStippled);
      XSetStipple(display, dspif->PixEGC, bgpixmap);
      XSetTSOrigin(display, dspif->PixEGC, dstx, dsty);
      XFillRectangle(display, wif->backing, dspif->PixEGC, dstx, dsty, width, regheight);
      break;
  }

  /*   XSetTSOrigin(display, wif->gc, dstx, dsty);
    XFillRectangle(display, XtWindow(wif->windowwidget), wif->gc,
                   dstx, dsty, width, regheight); */

  XCopyPlane(display, wif->backing, XtWindow(wif->windowwidget), wif->ReplaceGC, dstx, dsty, width,
             regheight, dstx, dsty, 1);

  XFreePixmap(display, bgpixmap);
  switch (op) /* undo the changing of foreground color above */
  {
    case 0:
    case 1: XSetFillStyle(display, dspif->PixRGC, FillSolid); break;
    case 2:
      XSetForeground(display, wif->gc, dspif->black);
      XSetFillStyle(display, dspif->PixIGC, FillSolid);
      break;
    case 3:
      XSetForeground(display, wif->gc, dspif->black);
      XSetFillStyle(display, dspif->PixEGC, FillSolid);
      break;
  }
  /* XSetFunction(display, wif->gc, gcfunction[wif->op % 4]);	 */
  XFlush(display);
}

XClearToEOL(LispArgs args) /* args[0] = MedleyWindow */
{
  Display *display;
  Window window;
  WindowInterface wif;
  DISPLAYDATA *dd;
  int dsty, dstx, height;

  dd = ImDataFromMw(args[0]);
  display = XDisplayFromMw(args[0]);
  window = XWindowFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  height = abs(dd->ddlinefeed);
  dstx = translate_x(wif, LispIntToCInt(args[1]));
  dsty = wif->innerregion.height - translate_y(wif, LispIntToCInt(args[2])) - height;
  XClearArea(display, window, dstx, dsty, 0, /* Out to the end */
             height,                         /* The height */
             False                           /* Don't generate exposure events */
             );
  XFlush(display);
}

MNXNewPage(LispArgs args) /* args[0] = MedleyWindow */
{
  Display *display;
  Window window;

  display = XDisplayFromMw(args[0]);
  window = XWindowFromMw(args[0]);
  /* ... */
  XFlush(display);
}

/************************************************************/
/* MNXOutchar                                                 */
/* NOTE this function returns NIL if we succeed and T if we */
/* bump into the right margin.                              */
/************************************************************/
MNXOutchar(LispArgs args) /* args[0] = MedleyWindow */
                                /* args[1] = Charcode */
                                /* args[2] = Stream */
                                /* args[3] = DD */
                                /* DSPXOFFSET and DSPYOFFSET are ignored here. */
{
  /* Doesn't switch charsets. */
  Display *display;
  Window window;
  char tmpstring[2];
  DspInterface dspif;
  WindowInterface wif;
  Stream *s;
  DISPLAYDATA *dd;
  BITMAP *bitmap;
  int cx, cy, char8code, right, rmargin, lmargin, newpos;
  int w, height, sx, sy;

  dspif = DspIfFromMw(args[0]);
  wif = WIfFromMw(args[0]);
  if (!wif->open) MNXopenW(args); /* if window is closed, open it. */

  display = dspif->handle;
  window = XtWindow(wif->windowwidget);

  if (!window) return (NIL); /* No window, pretend we succeeded. */

  s = ((Stream *)Cptr(args[2]));
  dd = ((DISPLAYDATA *)Cptr(args[3]));
  char8code = LispIntToCInt(args[1]) & 255;

  /* Get the current position */
  cx = translate_x(wif, LispIntToCInt(dd->ddxposition));
  rmargin = LispIntToCInt(dd->ddrightmargin);
  lmargin = LispIntToCInt(dd->ddleftmargin);
  w = GETWORD((DLword *)Cptr(dd->ddcharimagewidths + char8code));

  right = cx + w;

  if ((right > rmargin) && (cx > lmargin)) /* a linebreak. Punt. */
    return (ATOM_T);

  newpos = cx + GETWORD((DLword *)Cptr(dd->ddwidthscache + char8code));
  dd->ddxposition = CIntToLispInt(newpos);

  /* Y transform. Ypos is the baseline, it is adjusted by ascent */
  cy = translate_y(wif, LispIntToCInt(dd->ddyposition));
  cy = wif->innerregion.height - cy - dd->ddcharsetascent;

  /* This bitmap should be cached on the server! */
  /* Do this in the CHANGECHARSET code. */
  bitmap = (BITMAP *)Cptr(dd->ddpilotbbt);
  MakeScratchImageFromBM(dspif, bitmap);

  sx = GETWORD(Cptr(dd->ddoffsetscache + char8code));
  sy = 0;

  if (dd->ddoperation == INVERT_atom) {
    XPutImage(display, window, wif->InvertGC, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, wif->backing, dspif->PixIGC, &dspif->image, sx, sy, cx, cy, w,
              bitmap->bmheight);
  } else if (dd->ddoperation == ERASE_atom) {
    XPutImage(display, window, wif->EraseGC1, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, window, wif->EraseGC2, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, wif->backing, dspif->PixEGC, &dspif->image, sx, sy, cx, cy, w,
              bitmap->bmheight);
  } else if (dd->ddoperation == PAINT_atom) {
    XPutImage(display, window, wif->PaintGC1, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, window, wif->PaintGC2, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, wif->backing, dspif->PixPGC, &dspif->image, sx, sy, cx, cy, w,
              bitmap->bmheight);
  } else if (dd->ddoperation == REPLACE_atom) {
    XPutImage(display, window, wif->ReplaceGC, &dspif->image, sx, sy, cx, cy, w, bitmap->bmheight);
    XPutImage(display, wif->backing, dspif->PixRGC, &dspif->image, sx, sy, cx, cy, w,
              bitmap->bmheight);
  }

  /* Increment the CHARPOSITION of the stream with 1 */
  s->CHARPOSITION++;
  XFlush(display);
  return (NIL);
}

/************************************************************/
/* The WINDOWPROP interface                                 */
/************************************************************/
MNXgetwindowprop(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
{}

MNXputwindowprop(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
                                      /* args[1] = Lispint to despatch on. */
                                      /* args[n] = args for the method. */
{
  int method;
  Window window;
  XWindowAttributes attr;
  WindowInterface wif;

  method = LispIntToCInt(args[1]);
  wif = WIfFromMw(args[0]);

  /* Discard the dispatching lispint and */
  /* make the argvector be on the form */
  /* <medleywindow> <methodarg1> <methodarg2> ... */
  args[1] = args[0];
  args++;

  switch (method) {
    case MNWTitle: showtitle(args[0]); break;
    case MNWScrollFn:
      XtVaSetValues(WIfFromMw(args[0])->formwidget, XtNbarOnOff, (args[1] != NIL), XtNmedleyWindow,
                    wif, XtNhScroll, &SignalHScroll, XtNhJmpScroll, &SignalHJmpScroll, XtNvScroll,
                    &SignalVScroll, XtNvJmpScroll, &SignalVJmpScroll, NULL);
      if (XtWindow(wif->topwidget)) {
        XGetWindowAttributes(wif->dspif->handle, XtWindow(wif->topwidget), &attr);
        wif->topregion.width = attr.width;
        wif->topregion.height = attr.height;
      }
      break;
    case MNWNoScrollbars:
      XtVaSetValues(WIfFromMw(args[0])->formwidget, XtNbarOnOff, (args[1] == NIL), NULL);
      if (XtWindow(wif->topwidget)) {
        XGetWindowAttributes(wif->dspif->handle, XtWindow(wif->topwidget), &attr);
        wif->topregion.width = attr.width;
        wif->topregion.height = attr.height;
      }
      break;
    case MNWScrollExtent:
      XtVaSetValues(WIfFromMw(args[0])->formwidget, XtNextentX, LispIntToCInt(args[1]), XtNextentY,
                    LispIntToCInt(args[2]), XtNextentWidth, LispIntToCInt(args[3]), XtNextentHeight,
                    LispIntToCInt(args[4]), NULL);
      break;
    case MNWScrollExtentUse: break;
    case MNWBorder: refreshwindow(args[0]); break;
    default: break;
  }
}

/************************************************************/
/* Mouse position and friends. Have to be vectored through  */
/* the screen.                                              */
/************************************************************/

static Cursor grab_cursor = 0; /* Cursor in effect while pointer is grabbed */

/************************************************************************/
/*									*/
/*			G r a b  P o i n t e r	*/
/*	args[0] = screen to do it on		*/
/*	args[1] = cursor to use (?)			*/
/*	args[2] = cursor hotspot x			*/
/*	args[3] = cursor hotspot y			*/
/*									*/
/*									*/
/************************************************************************/

MNXGrabPointer(LispArgs args)
/* args[0] = medley screen */
/* args[1] = cursor to use, or NIL */
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  char *tmpstring;
  BITMAP *bitmap;
  XGCValues gcv;
  MedleyScreen MScreen;
  Pixmap bits;
  Window promptw;
  int res, i;
  extern unsigned char reversedbits[];
  unsigned char *src;
  unsigned char srcbits[32]; /* holds the reversed bits for cursor */

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = MScreen->NativeIf;
  display = dspif->handle;
  wif = dspif->promptw; /* handle on the prompt window */
  promptw = XtWindow(wif->framewidget);

  if (args[1]) {
    src = (unsigned char *)Cptr(args[1]);
    for (i = 0; i < 32; i++) srcbits[i] = reversedbits[src[i]];

    bits = XCreatePixmapFromBitmapData(display, dspif->root, srcbits, 16, 16, 1, 0, 1);
    grab_cursor = XCreatePixmapCursor(display, bits, bits, &dspif->black, &dspif->white,
                                      args[2] & 0xFFFF, 16 - (args[3] & 0xFFFF));
  } else {
    grab_cursor = None;
  }

  res =
      XGrabPointer(display, promptw, False, PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                   GrabModeAsync, GrabModeAsync, None, grab_cursor, CurrentTime);

  if (grab_cursor != None) { XFreePixmap(display, bits); }
  if (res == GrabSuccess)
    return (ATOM_T);
  else
    return (NIL);
}

MNXUngrabPointer(LispArgs args)
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  char *tmpstring;
  BITMAP *bitmap;
  XGCValues gcv;
  MedleyScreen MScreen;
  int res;

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = MScreen->NativeIf;
  display = dspif->handle;

  /* wif = WIfFromMw(args[0]); */

  res = XUngrabPointer(display, CurrentTime);

  if (grab_cursor != None) XFreeCursor(display, grab_cursor);

  grab_cursor = None;

  return (ATOM_T);
}

static int box_drawn = 0; /* T if there's a box on already */

MNXDrawBox(LispArgs args) /* args[0] = Medley SCREEN to draw box on */
                                /* args[1] = Left of box */
                                /* args[2] = bottom of box */
                                /* args[3] = width of box */
                                /* args[4] = height of box */
                                /* args[5] = T to force turning box off iff it's on */
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  MedleyScreen MScreen;
  int res, l, b, w, h;
  GC gc;
  Window root;

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = MScreen->NativeIf;
  display = dspif->handle;
  gc = dspif->BoxingGC;
  root = dspif->root;

  l = LispIntToCInt(args[1]);
  b = LispIntToCInt(args[2]);
  w = LispIntToCInt(args[3]);
  h = LispIntToCInt(args[4]);

  if (w < 0) /* Negative width, change X coord and make it positive */
  {
    l += w;
    w = -w;
  }

  if (h < 0) /* Negative height, change Y coord and make it positive */
  {
    b += h;
    h = -h;
  }

  b = HeightOfScreen(dspif->xscreen) - b - h; /* convert to X's Y-coord */

  if (args[5]) {
    if (box_drawn) XDrawRectangle(display, root, gc, l, b, w, h);
    box_drawn = 0;
  } else {
    XDrawRectangle(display, root, gc, l, b, w, h);
    box_drawn = !box_drawn;
  }
}

MNXMovePointer(LispArgs args) /* args[0] = LispPTR to SCREEN */
                                    /* args[1] = new pointer X */
                                    /* args[2] = new pointer Y */
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  MedleyScreen MScreen;
  int res, l, b, w, h;
  GC gc;
  Window root;

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = MScreen->NativeIf;
  display = dspif->handle;
  gc = dspif->BoxingGC;
  root = dspif->root;

  XWarpPointer(display, None, root, 0, 0, 0, 0, args[1] & 0xFFFF, args[2] & 0xFFFF);
  return (ATOM_T);
}

MNXmouseconfirm(LispArgs args) /* args[0] = LispPTR to MedleyWindow */
{ /* Use a DIALOG WIDGET here. */ }

MNXgarb(LispArgs args) /* args[0] = LispPTR to MedleyScreen */
                             /* args[1] = NIL for GC off, non-NIL for GC on */
{
  DspInterface dspif;

  dspif = DspIfFromMscr(args[0]);
  if (dspif->gcindicator)
    if (XtIsRealized(dspif->gcindicator))
      if (args[1] == NIL)
        XClearWindow(XtDisplay(dspif->gcindicator), XtWindow(dspif->gcindicator));
      else
        XFillRectangle(XtDisplay(dspif->gcindicator), XtWindow(dspif->gcindicator),
                       DefaultGCOfScreen(XtScreen(dspif->gcindicator)), 0, 0, 8, 8);
}

MNXMakePromptWindow(LispArgs args) /* args[0] = Lisp window, the prompt window */
{
  Widget form, menu;
  DspInterface dspif;
  WindowInterface wif;

  wif = WIfFromMw(args[0]);
  dspif = wif->dspif;

  dspif->promptw = wif; /* Save prompt window, for grabbing pointer events */

  dspif->legatewidget = XtVaAppCreateShell(
      "Medley Status", "Medley Status", applicationShellWidgetClass, XDisplayFromMw(args[0]),
      XtNargc, save_argc, XtNargv, save_argv, XtNborderWidth, 0, NULL);

  form = XtVaCreateManagedWidget("bar", boxWidgetClass, dspif->legatewidget, NULL);

  dspif->gcindicator =
      XtVaCreateManagedWidget("gcindicator", boxWidgetClass, form, XtNwidth, 8, XtNheight, 8, NULL);

  menu = XtVaCreateManagedWidget("Background", commandWidgetClass, form, NULL);

  XtAddEventHandler(menu, ButtonPressMask | ButtonReleaseMask, False, HandleBackgroundButton,
                    dspif->screen);
  XtAddEventHandler(dspif->legatewidget, PointerMotionMask, False, HandleMotion, 0);
  XtAddEventHandler(dspif->legatewidget, EnterWindowMask | LeaveWindowMask, False,
                    HandleBackgroundCrossing, dspif);
  XtAddEventHandler(dspif->legatewidget, KeyPressMask | KeyReleaseMask, False, HandleKey, dspif);
  XtRealizeWidget(dspif->legatewidget);
  XtSetSensitive(dspif->legatewidget, True);
  XtSetSensitive(form, True);
  return (ATOM_T);
}

/************************************************************/
/*                                                          */
/*                 M N X S e t C u r s o r                  */
/*                                                          */
/*  Set the cursor for all windows on a given screen.       */
/*                                                          */
/************************************************************/

static Cursor MNX_cursor = NULL; /* Cursor in active use */

MNXSetCursor(LispArgs args) /* args[0] = Medley screen */
                                  /* args[1] = bits for new cursor */
                                  /* args[2] = hot-spot x */
                                  /* args[3] = hot-spot y */
{
  Display *display;
  WindowInterface wif;
  DspInterface dspif;
  char *tmpstring;
  BITMAP *bitmap;
  XGCValues gcv;
  MedleyScreen MScreen;
  Pixmap bits;
  Cursor new_cursor;
  Window win;
  unsigned char srcbits[32]; /* hold the reversed-bits for cursor */
  extern unsigned char reversedbits[];
  unsigned char *src;
  int i;

  MScreen = (MedleyScreen)Cptr(args[0]);
  dspif = DspIfFromMscr(args[0]);
  display = dspif->handle;

  src = (unsigned char *)Cptr(args[1]);
  for (i = 0; i < 32; i++) srcbits[i] = reversedbits[src[i]];

  bits = XCreatePixmapFromBitmapData(display, dspif->root, srcbits, 16, 16, 1, 0, 1);
  new_cursor = XCreatePixmapCursor(display, bits, bits, &dspif->black, &dspif->white,
                                   args[2] & 0xFFFF, args[3] & 0xFFFF);

  for (wif = dspif->CreatedWifs; wif; wif = wif->next)
    if (win = XtWindow(wif->windowwidget)) XDefineCursor(display, win, new_cursor);

  XFreePixmap(display, bits);

  if (dspif->cursor) { XFreeCursor(display, dspif->cursor); }
  dspif->cursor = new_cursor;

  return (NIL);
}

/************************************************************/
/*                       I n i t D s p                      */
/*                                                          */
/* The slots for identifier, object type and window system  */
/* has to be filled in.                                     */
/************************************************************/
InitDsp(LispArgs args) /* arg[0] = LispPTR to MedleyScreen */
{
  DspInterface dspif;
  MedleyScreen SCREEN;
  char *tmpstring, *defaulthost;
  int strlen;
  Display *display;
  Screen *screen;
  XGCValues gcv;
  int black, white;         /* Pixel values for this display */
  int mask, diff, inverted; /* For finding a plane where they differ */
  Pixmap tpx;               /* temp pixmap of depth 1 for creating GCs */
  int Xfd;

  SCREEN = (MedleyScreen)Cptr(args[0]);
  dspif = (DspInterface)malloc(sizeof(DspInterfaceRec));
  SCREEN->NativeIf = dspif;

  if (!xtinitialized) {
    XtToolkitInitialize();
    xtinitialized = True;
  }
  dspif->xtcontext = XtCreateApplicationContext();

  /* if NATIVE_INFO has a string use it for the hostname */
  /* else use the environment var DISPLAY or "unix:0.0" */
  if (LispStringP(SCREEN->NATIVE_INFO)) {
    strlen = LispStringLength(SCREEN->NATIVE_INFO);
    tmpstring = (char *)alloca(strlen + 1);
    LispStringToCStr(SCREEN->NATIVE_INFO, tmpstring);
    dspif->handle =
        XtOpenDisplay(dspif->xtcontext, tmpstring, NULL, "window", NULL, 0, &save_argc, save_argv);
  } else
    dspif->handle =
        XtOpenDisplay(dspif->xtcontext, NULL, NULL, "window", NULL, 0, &save_argc, save_argv);

  /* Set up the native structure here */
  if (dspif->handle == NULL) return (CIntToLispInt(2));

  XSetErrorHandler(Xerrhandler);

  display = dspif->handle;
  dspif->xscreen = screen = DefaultScreenOfDisplay(display);

  Xfd = ConnectionNumber(display);
  FD_to_dspif[Xfd] = dspif;
  LispReadFds |= (1 << Xfd);
  MNWReadFds |= (1 << Xfd);
#ifndef ISC
#ifndef HPUX
  fcntl(Xfd, F_SETOWN, getpid());
#endif /* HPUX */
#endif /* ISC */

  dspif->screen = args[0];                  /* So we know which SCREEN this display is */
  dspif->root = RootWindowOfScreen(screen); /* And the root window for the screen */
  dspif->gcindicator = 0;
  dspif->legatewidget = 0;
  dspif->cursor = 0;

  gcv.function = GXcopy;
  gcv.foreground = dspif->white = WhitePixelOfScreen(screen);
  gcv.background = dspif->black = BlackPixelOfScreen(screen);
  dspif->TitleGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  tpx = XCreatePixmap(display, dspif->root, 10, 10, 1);

  gcv.function = GXcopy;
  gcv.foreground = 1;
  gcv.background = 0;
  dspif->PixRGC = XCreateGC(display, tpx, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXor;
  gcv.foreground = 1;
  gcv.background = 0;
  dspif->PixPGC = XCreateGC(display, tpx, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXxor;
  gcv.foreground = 1;
  gcv.background = 0;
  dspif->PixIGC = XCreateGC(display, tpx, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXandInverted;
  gcv.foreground = 1;
  gcv.background = 0;
  dspif->PixEGC = XCreateGC(display, tpx, GCForeground | GCBackground | GCFunction, &gcv);

  XFreePixmap(display, tpx);

  dspif->image.format = XYBitmap;
  dspif->image.xoffset = 0;
#if (defined(BYTESWAP))
  dspif->image.byte_order = LSBFirst;
#else  /* BYTESWAP */
  dspif->image.byte_order = MSBFirst;
#endif /* BYTESWAP */
  dspif->image.bitmap_unit = 16 /*BitmapUnit( dspif->handle )*/;
  dspif->image.bitmap_bit_order = MSBFirst;
  dspif->image.bitmap_pad = 16 /* 32 */;
  _XInitImageFuncPtrs(&dspif->image);

  dspif->tmpimage.format = XYBitmap;
  dspif->tmpimage.xoffset = 0;
#if (defined(BYTESWAP))
  dspif->tmpimage.byte_order = LSBFirst;
#else  /* BYTESWAP */
  dspif->tmpimage.byte_order = MSBFirst;
#endif /* BYTESWAP */
  dspif->tmpimage.bitmap_unit = 16 /*BitmapUnit( dspif->handle )*/;
  dspif->tmpimage.bitmap_bit_order = MSBFirst;
  dspif->tmpimage.bitmap_pad = 16 /* 32 */;
  _XInitImageFuncPtrs(&dspif->tmpimage);

  dspif->CreatedWifs = NULL; /* List of created wif. */
  /* Fill in the screen dimensions too! */
  SCREEN->SCWIDTH = CIntToLispInt(WidthOfScreen(screen));
  SCREEN->SCHEIGHT = CIntToLispInt(HeightOfScreen(screen));
#ifndef NEVER
  SCREEN->SCDEPTH = CIntToLispInt(PlanesOfScreen(screen));
#endif

  /* Methods for the display */
  dspif->Dispatch.Method.InitW = MNXcreateW;
  dspif->Dispatch.Method.Openw = MNXopenW;
  dspif->Dispatch.Method.Closew = MNXcloseW;
  dspif->Dispatch.Method.MoveW = MNXmoveW;
  dspif->Dispatch.Method.ShapeW = MNXshapeW;
  dspif->Dispatch.Method.TotopW = MNXtotopW;
  dspif->Dispatch.Method.BuryW = MNXburyW;
  dspif->Dispatch.Method.ShrinkW = MNXshrinkW;
  dspif->Dispatch.Method.ExpandW = MNXexpandW;
  dspif->Dispatch.Method.DestroyW = MNXdestroyW;
  dspif->Dispatch.Method.MakePromptW = MNXMakePromptWindow;
  dspif->Dispatch.Method.DestroyMe = MNXdestroyDisplay;
  dspif->Dispatch.Method.BBTtoWin = MNXBBTToXWindow;
  dspif->Dispatch.Method.BBTfromWin = MNXBBTFromXWindow;
  dspif->Dispatch.Method.BBTWinWin = MNXBBTWinWin;
  dspif->Dispatch.Method.GetWindowProp = MNXgetwindowprop;
  dspif->Dispatch.Method.PutWindowProp = MNXputwindowprop;
  dspif->Dispatch.Method.GrabPointer = MNXGrabPointer;
  dspif->Dispatch.Method.UngrabPointer = MNXUngrabPointer;
  dspif->Dispatch.Method.DrawBox = MNXDrawBox;
  dspif->Dispatch.Method.MovePointer = MNXMovePointer;
  dspif->Dispatch.Method.MouseConfirm = MNXmouseconfirm;
  dspif->Dispatch.Method.GCIndicator = MNXgarb;
  dspif->Dispatch.Method.SetCursor = MNXSetCursor;

  /* ImageOp methods */
  dspif->ImageOp.Method.BitBlt = MNXBitBltBW;
  dspif->ImageOp.Method.BltShade = MNXbltshadeBW;
  dspif->ImageOp.Method.DrawPoint = MNXdrawpoint;
  dspif->ImageOp.Method.DrawLine = MNXdrawline;
  dspif->ImageOp.Method.DrawCurve = MNXdrawcurve;
  dspif->ImageOp.Method.DrawCircle = MNXdrawcircle;
  dspif->ImageOp.Method.FillCircle = MNXfillcircle;
  dspif->ImageOp.Method.DrawCurve = MNXdrawcurve;
  dspif->ImageOp.Method.DrawPolygon = MNXdrawpolygon;
  dspif->ImageOp.Method.FillPolygon = MNXfillpolygon;
  dspif->ImageOp.Method.DrawElips = MNXdrawelipse;
  dspif->ImageOp.Method.WritePixel = MNXwritepixel;
  dspif->ImageOp.Method.DrawArc = MNXdrawarc;
  dspif->ImageOp.Method.OutcharFn = MNXOutchar;
  dspif->ImageOp.Method.NewPage = MNXNewPage;
  dspif->ImageOp.Method.ClippingRegion = MNXclippingregion;
  dspif->ImageOp.Method.Operation = MNXoperation;
  dspif->ImageOp.Method.Color = MNXdspcolor;
  dspif->ImageOp.Method.BackColor = MNXdspbackcolor;
  dspif->ImageOp.Method.Reset = MNXresetW;
  dspif->ImageOp.Method.Offsets = MNXSetOffsets;

  XSynchronize(display, True);

  /**************************************************/
  /*  Create GCs for BITBLTs from window to bitmap  */
  /**************************************************/

  diff = black & (~white);
  inverted = 0;

  if (diff)
    for (mask = 1; mask; mask <<= 1) {
      if (mask & diff) break;
    }
  else {
    mask = 1;
    inverted = 1;
    for (diff = white & (~black); mask; mask <<= 1) {
      if (mask & diff) break;
    }
  }
#ifdef NEVER
  gcv.function = GXcopy;
  gcv.foreground = !inverted;
  gcv.background = inverted;
  dspif->GetRGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXxor;
  dspif->GetIGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXor;
  dspif->GetPGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);

  gcv.function = GXandInverted;
  dspif->GetEGC = XCreateGC(display, dspif->root, GCForeground | GCBackground | GCFunction, &gcv);
  dspif->bw_plane = mask;
  dspif->bw_inverted = inverted;
#endif /* NEVER */
  gcv.function = GXxor;
  gcv.line_style = LineSolid;
  gcv.line_width = 0;
  gcv.foreground = 255;
  gcv.background = 0;
  dspif->BoxingGC =
      XCreateGC(display, dspif->root,
                GCForeground | GCBackground | GCFunction | GCLineWidth | GCLineStyle, &gcv);
  XSetSubwindowMode(display, dspif->BoxingGC, IncludeInferiors);

  /* Check the requested depth against possibilities */

  if (SCREEN->SCDEPTH) {
    int i;
    int *dlist;
    int dcount;

    dlist = XListDepths(display, XScreenNumberOfScreen(screen), &dcount);
    for (i = 0; i < dcount; i++)
      if (dlist[i] == (SCREEN->SCDEPTH & 0xFFFF)) return (NIL);
    return (CIntToLispInt(1)); /* Can't find depth */
  } else {
    SCREEN->SCDEPTH = CIntToLispInt(PlanesOfScreen(screen));
  }
  return (NIL);
}

/************************************************************/
/*                                                          */
/*             i n i t _ m n w _ i n s t a n c e            */
/*                                                          */
/*                                                          */
/************************************************************/
init_mnw_instance(args) LispPTR *args;
{
  int type;

  /* HACK !! HACK !! make this better later!! */
  MedleyWindowType = GetTypeNumber(*GetVALCELL68k(MAKEATOM("PROMPTWINDOW")));
  MedleyScreenType = GetTypeNumber(*GetVALCELL68k(MAKEATOM("LASTSCREEN")));
  MedleyBitmapType = 0;

  type = GetTypeNumber(args[0]);

  if (type == MedleyScreenType) {
    return (InitDsp(args));
  } else
    error("Bogus type\n");
  return (ATOM_T);
}

/**************************************************************/
/*           d i s p a t c h _ m n w _ m e t h o d            */
/*                                                            */
/* The arguments to the dispatcher are as follows             */
/* args[0] = methodindex, the index into the dispatch vector  */
/* args[1] = self, the object to dispatch from                */
/* args[n] = the misc arguments.                              */
/*                                                            */
/* The dispatcher finds the method dispatch table for the     */
/* object and dispatches the method. The method is called     */
/* with the methodindex removed.                              */
/**************************************************************/
dispatch_mnw_method(args) LispPTR *args;
{
  int type;
  int index;

  index = LispIntToCInt(args[0]);
  type = GetTypeNumber(args[1]);

  if (args[1] == NULL) error("No object to dispatch from\n");

  if (type == MedleyWindowType) {
    WindowInterface wif;

    wif = WIfFromMw(args[1]);
    if (wif == NULL) {
      /* error("Attempt to dispatch on a non-native window.\n"); */
      return (NIL);
    }
    if ((wif->Dispatch->vector[index]) == NULL) {
      error("Display method not defined.\n");
      return (NIL);
    }
    /* Now, dispatch with the selector (first) arg shave off */
    return ((wif->Dispatch->vector[index])(&args[1]));
  } else if (type == MedleyScreenType) {
    DspInterface dspif;

    dspif = DspIfFromMscr(args[1]);
    if (dspif == NULL) {
      error("Attempt to dispatch on a non-native screen.\n");
      return (NIL);
    }
    if ((dspif->Dispatch.vector[index]) == NULL) {
      error("Display method not defined.\n");
      return (NIL);
    }
    /* Dispatch with selector and screen popped off */
    return ((dspif->Dispatch.vector[index])(&args[2]));
  } else {
    error("Bogus type\n");
    return (NIL);
  }
}
