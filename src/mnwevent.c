/* $Id: mnwevent.c,v 1.2 1999/01/03 02:07:25 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

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
#include "lsptypes.h"
#include "MyWindow.h"
#include "dbprint.h"
#include "mnxdefs.h"
#include "keyboard.h"
#include "lspglob.h"
#include "adr68k.h"
#include "cell.h"

extern XEvent report;

extern DLword *CTopMNWEvent;
int MNWEventFlg = 0; /* Count of MNW events needing service */

DspInterfaceRec *FD_to_dspif[32]; /* Map from FD # to a display */
int MNWReadFds = 0;

DLword *CTopMNWEvent;
LispPTR DOMNWEVENT_index;
LispPTR *MNWEVENTQUEUE68k;

extern LispPTR *MakeAtom68k();

extern DLword *EmCursorX68K, *EmCursorY68K;
extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K;
extern LispPTR *CLastUserActionCell68k;

extern int KBDEventFlg;
LispPTR *MNWBUFFERING68k = 0;

/************************************************************************/
/* Find_window                                                          */
/* Find the wif corresponding to the Xwindow. This function also places */
/* the found wif at the head of the chain. The rationale for this is    */
/* that a window that has gotten signals is likely to be the target of  */
/* the next signal also. The next time we look for this wif we'll find  */
/* at the head of the chain. (This is bubblesort)                       */
/*                                                                      */
/************************************************************************/
WindowInterface Find_window(DspInterface dspif, Window Xwindow)
{
  WindowInterface curr, prev;

  if (dspif->CreatedWifs == (WindowInterface)NULL) return (NIL);
  if (dspif->CreatedWifs->blackframe == Xwindow) return (dspif->CreatedWifs);

  /* Find the wif we are interested in. */
  for (prev = dspif->CreatedWifs, curr = prev->next;
       ((curr != (WindowInterface)NULL) && (curr->blackframe != Xwindow));
       curr = curr->next, prev = prev->next) {}
  if (curr == (WindowInterface)NULL) return (NIL); /* wif not found */

  /* Bubble curr to the head of the list */
  prev->next = curr->next;
  curr->next = dspif->CreatedWifs;
  dspif->CreatedWifs = curr;
  return (curr);
}

/************************************************************************/
/* Callback handling mechanism.                                         */
/* These procedures handle the callbacks from widgets.                  */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* NewEvent                                                             */
/* Get a pointer to the next event in line in the Lisp eventqueue.      */
/* Return NULL if fail. This is the head of DoRing.                     */
/************************************************************************/
MNWEvent *NewEvent() {
  if (!CTopMNWEvent) {
    LispPTR index;
    index = get_package_atom("\\MNWEVENTQUEUE", 14, "INTERLISP", 9, NIL);
    if (index != 0xFFFFFFFF) {
      MNWEVENTQUEUE68k = GetVALCELL68k(index);
      DOMNWEVENT_index = get_package_atom("\\DOMNWEVENT", 11, "INTERLISP", 9, NIL);
      CTopMNWEvent = (DLword *)NativeAligned2FromLAddr(*MNWEVENTQUEUE68k);
    }
  }
  if (CTopMNWEvent) {
    DLword w, r;
    r = RING_READ(CTopMNWEvent);
    w = RING_WRITE(CTopMNWEvent);

    if (r != w) return ((MNWEvent *)(CTopMNWEvent + w));
  }
  return (NULL);
}
/************************************************************************/
/* SignalMNWEvent                                                       */
/* Tell Lisp that an event happened. This is the tail part of DoRing    */
/*                                                                      */
/************************************************************************/
SignalMNWEvent() {
  DLword w, r;
  if (CTopMNWEvent) {
    r = RING_READ(CTopMNWEvent);
    w = RING_WRITE(CTopMNWEvent);

    if (r == 0) /* Queue was empty */
      ((RING *)CTopMNWEvent)->read = w;
    if (w >= MAXMNWEVENT)
      ((RING *)CTopMNWEvent)->write = MINMNWEVENT;
    else
      ((RING *)CTopMNWEvent)->write = w + MNWEVENTSIZE;

    if (*MNWBUFFERING68k == NIL) *MNWBUFFERING68k = ATOM_T;
    if (MNWEventFlg++ > 0) Irq_Stk_End = Irq_Stk_Check = 0; /* Ask for interrupt */
  }
}

void HandleMotion(Widget widget, WindowInterface wif, XMotionEvent *xevent, Boolean *continue_to_dispatch)
{
  MNWPointerMotionEvent *mevent;

  {
    *CLastUserActionCell68k = MiscStats->secondstmp;
    (*((DLword *)EmMouseX68K)) = *EmCursorX68K = xevent->x_root;
    (*((DLword *)EmMouseY68K)) = *EmCursorY68K = xevent->y_root;
    /* printf("mouse x %d, y %d\n", xevent->x_root, xevent->y_root); */
    DoRing();
    if ((KBDEventFlg) > 0) {
      /* immediately request for IRQ check */
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
  }
}

void HandleButton(Widget widget, WindowInterface wif, XButtonEvent *xevent, Boolean *continue_to_dispatch)
{
  MNWButtonEvent *mevent;

  lisp_Xbutton(xevent, 0);
}

void HandleKey(Widget widget, WindowInterface wif, XKeyEvent *xevent, Boolean *continue_to_dispatch)
{
  lisp_Xkeyboard(xevent, 0);

#ifdef NEVER
  if ((mevent = NewEvent()) != NULL) {
    mevent->screen = wif->MedleyScreen;
    mevent->window = wif->MedleyWindow;
    mevent->event = MNWButton;
    mevent->pos.x = xevent->x_root;
    mevent->pos.y = xevent->y_root;
    mevent->button.left = xevent->state | Button1Mask;
    mevent->button.middle = xevent->state | Button2Mask;
    mevent->button.right = xevent->state | Button3Mask;
    SignalMNWEvent();
  }
#endif /* NEVER */
}

void HandleCrossing(Widget widget, WindowInterface wif, XCrossingEvent *xevent, Boolean *continue_to_dispatch)
{
  switch (xevent->type) {
    case EnterNotify:
      DoMNWRing(MNWMouseIn, wif->MedleyScreen, wif->MedleyWindow, 0, 0, 0, 0);
      break;

    case LeaveNotify:
      if (NotifyInferior != xevent->detail)
        DoMNWRing(MNWMouseOut, wif->MedleyScreen, wif->MedleyWindow, 0, 0, 0, 0);
      break;
  }
}

void HandleBackgroundCrossing(Widget widget, DspInterface dspif, XCrossingEvent *xevent, Boolean *continue_to_dispatch)
{
  switch (xevent->type) {
    case EnterNotify: DoMNWRing(MNWMouseIn, dspif->screen, 0, 0, 0, 0, 0); break;

    case LeaveNotify:
      if (NotifyInferior != xevent->detail) DoMNWRing(MNWMouseOut, dspif->screen, 0, 0, 0, 0, 0);
      break;
  }
}

void HandleFocus(Widget widget, WindowInterface wif, XFocusChangeEvent *xevent, Boolean *continue_to_dispatch)
{
  MNWFocusEvent *mevent;
  /*
    if((mevent = (MNWFocusEvent *)NewEvent()) != NULL){
      mevent->screen = wif->MedleyScreen;
      mevent->window = wif->MedleyWindow;
      if(xevent->type == FocusIn) mevent->event = MNWFocusIn;
      else mevent->event = MNWFocusOut;
      SignalMNWEvent();
    }
  */
}

void HandleStructure(Widget widget, WindowInterface wif, XAnyEvent *xevent, Boolean *continue_to_dispatch)
{
  LispPTR screen, MWindow;
  Window window;
  Display *display;
  DspInterfaceRec *dspif;

  screen = wif->MedleyScreen;
  MWindow = wif->MedleyWindow;
  window = xevent->window;
  dspif = wif->dspif;
  display = dspif->handle;

  switch (xevent->type) {
    case UnmapNotify: DoMNWRing(MNWClose, screen, MWindow, 0, 0, 0, 0); break;

    case MapNotify: break;

    case ConfigureNotify:
      if (!wif) break; /* don't change inner windows */
      /* if (wif->moving || wif->reshaping)
         {
         wif->moving = wif->reshaping = 0;
         break;
         }
         else */
      {
        int l, b, b2, w, h;
        XConfigureEvent *ev = (XConfigureEvent *)xevent;
        /* printf("M/S event send %d, x %d, y %d, w %d, h %d\n",
                                xevent->send_event, ev->x, ev->y,
                                ev->width, ev->height); */

        l = ev->x;
        b = ev->y;

        if ((!xevent->send_event))
        {
          Window parent, root, *children;
          int nch;

          XQueryTree(display, window, &root, &parent, &children, &nch);
          XFree(children);
          if (parent != root)
            XTranslateCoordinates(display, parent, dspif->root, ev->x, ev->y, &l, &b, &window);
          /* printf("[translated x %d, y %d]\n", l, b); */
        }

        b2 = HeightOfScreen(wif->screen) - b - ev->height;

        if ((wif->topregion.width == ev->width) && (wif->topregion.height == ev->height)) {
          if ((wif->topregion.x != l) || (wif->topregion.y != b)) {
            /* printf("movew to %d, %d.\n", l+wif->outerregion.x, b2+wif->outerregion.y); */
            DoMNWRing(MNWMove, screen, MWindow, l + wif->outerregion.x, b2 + wif->outerregion.y,
                      ev->width - (wif->topregion.width - wif->outerregion.width),
                      ev->height - (wif->topregion.height - wif->outerregion.height));
          }
        } else /* if ((ev->x != wif->outerregion.x) || (ev->y != wif->outerregion.y)) */
        {
          /* printf("shapew to %d, %d, %d, %d.\n", l+wif->outerregion.x, b2+wif->outerregion.y,
                                     ev->width-(wif->topregion.width - wif->outerregion.width),
             ev->height-(wif->topregion.height - wif->outerregion.height)); */
          DoMNWRing(MNWShapeReq, screen, MWindow, l + wif->outerregion.x, b2 + wif->outerregion.y,
                    ev->width - (wif->topregion.width - wif->outerregion.width),
                    ev->height - (wif->topregion.height - wif->outerregion.height));
        }
      }
      break;

    case ReparentNotify:
      if (wif) wif->parent = ((XReparentEvent *)xevent)->parent;
      break;
  }
}

void HandleBackgroundButton(Widget widget, LispPTR wif, XButtonEvent *xevent, Boolean *continue_to_dispatch)
{
  MNWButtonEvent *mevent;

  { /* Copied from HandleMotion -- tell Lisp of mouse move */
    *CLastUserActionCell68k = MiscStats->secondstmp;
    (*((DLword *)EmMouseX68K)) = *EmCursorX68K = xevent->x_root;
    (*((DLword *)EmMouseY68K)) = *EmCursorY68K = xevent->y_root;
    DoRing();
    if ((KBDEventFlg) > 0) {
      /* immediately request for IRQ check */
      Irq_Stk_End = Irq_Stk_Check = 0;
    }
  }

  lisp_Xbutton(xevent, 0);
#ifdef NEVER
  if ((xevent->type == ButtonPress) && ((mevent = (MNWButtonEvent *)NewEvent()) != NULL)) {
    mevent->screen = wif /*->MedleyScreen*/;
    mevent->window = 0;
    mevent->event = MNWButton;
    mevent->pos.x = xevent->x_root;
    mevent->pos.y = xevent->y_root;
    mevent->button.left = xevent->state | Button1Mask;
    mevent->button.middle = xevent->state | Button2Mask;
    mevent->button.right = xevent->state | Button3Mask;
    SignalMNWEvent();
  }
#endif
}

/* Handle expose events on the frame widget -- print the title */
void HandleTitle(Widget widget, WindowInterface wif, XExposeEvent *xevent, Boolean *continue_to_dispatch)
{
  if (xevent->y <= 12)
    showtitle(wif->MedleyWindow, ((MedleyWindow)Cptr(wif->MedleyWindow))->WTITLE);
}

/* Handle expose events on the frame widget -- print the title */
void HandleExpose(Widget widget, WindowInterface wif, XExposeEvent *xevent, Boolean *continue_to_dispatch)
{
  XCopyPlane(wif->dspif->handle, wif->backing, XtWindow(wif->windowwidget), wif->ReplaceGC,
             xevent->x, xevent->y, xevent->width, xevent->height, xevent->x, xevent->y, 1);
}

void SignalVJmpScroll(Widget widget, WindowInterface wif, XtPointer percent_ptr /* This is a *float */)
{
  MNWJumpScrollReqEvent *event;

  if ((event = (MNWJumpScrollReqEvent *)NewEvent()) != NULL) {
    event->screen = wif->MedleyScreen;
    event->window = wif->MedleyWindow;
    event->event = MNWScrollJmpReq;
    event->xpercent = 0.0;
    event->ypercent = *(float *)percent_ptr;
    SignalMNWEvent();
  }
}

void SignalHJmpScroll(Widget widget, WindowInterface wif, XtPointer percent_ptr)
{
  MNWJumpScrollReqEvent *event;

  if ((event = (MNWJumpScrollReqEvent *)NewEvent()) != NULL) {
    event->screen = wif->MedleyScreen;
    event->window = wif->MedleyWindow;
    event->event = MNWScrollJmpReq;
    event->ypercent = 0.0;
    event->xpercent = *(float *)percent_ptr;
    SignalMNWEvent();
  }
}

void SignalVScroll(Widget widget, WindowInterface wif, int position)
{
  MNWScrollReqEvent *event;

  if ((event = (MNWScrollReqEvent *)NewEvent()) != NULL) {
    event->screen = wif->MedleyScreen;
    event->window = wif->MedleyWindow;
    event->event = MNWScrollReq;
    event->dx = 0;
    event->dy = position;
    SignalMNWEvent();
  }
}

void SignalHScroll(Widget widget, WindowInterface wif, int position)
{
  MNWScrollReqEvent *event;

  if ((event = (MNWScrollReqEvent *)NewEvent()) != NULL) {
    event->screen = wif->MedleyScreen;
    event->window = wif->MedleyWindow;
    event->event = MNWScrollReq;
    event->dy = 0;
    event->dx = -position;
    SignalMNWEvent();
  }
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void getMNWsignaldata(int fd)
{
  Display *display;
  DspInterfaceRec *dspif;
  XEvent report;
  Window window;
  WindowInterfaceRec *wif;
  LispPTR MWindow;
  LispPTR screen;

  display = (dspif = FD_to_dspif[fd])->handle;
  screen = dspif->screen;
  TPRINT(("TRACE: getMNWsignaldata()\n"));

  while (XtAppPending(dspif->xtcontext)) {
    XtAppNextEvent(dspif->xtcontext, &report);
    XtDispatchEvent(&report);
  }

#ifdef NEVER
  while (XPending(display)) {
    /* XtAppNextEvent(dspif->xtcontext, &report); */
    XNextEvent(display, &report);
    window = report.xany.window;

    wif = Find_window(dspif, window);

    if (wif) {
      switch (report.type) {
        case KeyPress:
        case KeyRelease: lisp_Xkeyboard(&report, 0); break;

        case ButtonPress:
        case ButtonRelease: lisp_Xbutton(&report, 0); break;

        case MotionNotify: {
          *CLastUserActionCell68k = MiscStats->secondstmp;
          (*((DLword *)EmMouseX68K)) = *EmCursorX68K = report.xmotion.x_root;
          (*((DLword *)EmMouseY68K)) = *EmCursorY68K = report.xmotion.y_root;
          DoRing();
          if ((KBDEventFlg) > 0) {
            /* immediately request for IRQ check */
            Irq_Stk_End = Irq_Stk_Check = 0;
          }
        } break;

        case EnterNotify:
          if (MWindow) DoMNWRing(MNWMouseIn, screen, MWindow, 0, 0, 0, 0);
          break;

        case LeaveNotify:
          if (NotifyInferior != report.xcrossing.detail)
            DoMNWRing(MNWMouseOut, screen, MWindow, 0, 0, 0, 0);
          break;

        case FocusIn: DoMNWRing(MNWFocusIn, screen, MWindow, 0, 0, 0, 0); break;

        case FocusOut: DoMNWRing(MNWFocusOut, screen, MWindow, 0, 0, 0, 0); break;

        case Expose:
          if (wif->not_exposed) {
            char *tmpstring;
            BITMAP *bitmap;

            showtitle(MWindow, ((MedleyWindow)Cptr(MWindow))->WTITLE);
            bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(MWindow))->SAVE);

            MBMToDrawable(bitmap, dspif, wif, 0, 0, wif->innerregion.x, wif->innerregion.y,
                          wif->innerregion.width, wif->innerregion.height);
            XFlush(display);
            wif->not_exposed = 0; /* don't expose it again */
          }

          break;

        case NoExpose:

        case CreateNotify: break;

        case DestroyNotify:

        case CirculateNotify:
          if (PlaceOnBottom == report.xcirculate.place) {
            DspInterface dspif;
            BITMAP *bitmap;

            dspif = DspIfFromMscr(screen);
            bitmap = (BITMAP *)Cptr(((MedleyWindow)Cptr(MWindow))->SAVE);

            /* Copy the whole black frame to the bitmap. */
            DrawableToMBM(bitmap, dspif, wif->blackframe, wif->screen, 0, 0, 0, 0,
                          wif->outerregion.width, wif->outerregion.height);
            DrawableToMBM(bitmap, dspif, wif->handle, wif->screen, 0, 0, wif->innerregion.x,
                          wif->innerregion.y, wif->innerregion.width, wif->innerregion.height);
          }
          break;

        default: break;
      }
    }
    /*       else {*/ /* No wif found. Assume it is a widget window. */
                      /* XtDispatchEvent(&report); */
                      /*      } */
  }
#endif /* NEVER */
} /* end getMNWsignaldata() */

/************************************************************************/
/*									*/
/*				D o R i n g				*/
/*									*/
/*	Take keyboard events and turn them into Lisp event info		*/
/*	(when running under X)						*/
/*									*/
/************************************************************************/

void DoMNWRing(int type, LispPTR screen, LispPTR window, int l, int b, int wid, int h)
{
  DLword w, r;
  MNWEvent *event;
  int foo = MNWEVENTSIZE; /* so we can examine the value */

  if (!CTopMNWEvent) {
    LispPTR index;
    index = get_package_atom("\\MNWEVENTQUEUE", 14, "INTERLISP", 9, NIL);
    if (index != 0xFFFFFFFF) {
      MNWEVENTQUEUE68k = GetVALCELL68k(index);
      DOMNWEVENT_index = get_package_atom("\\DOMNWEVENT", 11, "INTERLISP", 9, NIL);
      CTopMNWEvent = (DLword *)NativeAligned2FromLAddr(*MNWEVENTQUEUE68k);
    }
  }
do_ring:

  if (CTopMNWEvent) {
    r = RING_READ(CTopMNWEvent);
    w = RING_WRITE(CTopMNWEvent);

    if (r == w) return;

    event = (MNWEvent *)(CTopMNWEvent + w);

    event->Any.event = type;
    event->Any.screen = screen;
    event->Any.window = window;
    event->Any.pad[0] = l;
    event->Any.pad[1] = b;
    event->Any.pad[2] = wid;
    event->Any.pad[3] = h;
    if (r == 0) /* Queue was empty */
      ((RING *)CTopMNWEvent)->read = w;
    if (w >= MAXMNWEVENT)
      ((RING *)CTopMNWEvent)->write = MINMNWEVENT;
    else
      ((RING *)CTopMNWEvent)->write = w + MNWEVENTSIZE;

    if (*MNWBUFFERING68k == NIL) *MNWBUFFERING68k = ATOM_T;
    if (MNWEventFlg++ > 0) Irq_Stk_End = Irq_Stk_Check = 0; /* Ask for interrupt */
  }
}
