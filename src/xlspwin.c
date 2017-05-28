/* $Id: xlspwin.c,v 1.4 2001/12/26 22:17:07 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: xlspwin.c,v 1.4 2001/12/26 22:17:07 sybalsky Exp $ Copyright (C) Venue";



/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 		*/
/*                	1994, 1995, 1999, 2000, 2001 Venue.		*/
/*	    All Rights Reserved.					*/
/*									*/
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
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "lispemul.h"

#include "xdefs.h"
#include "xbitmaps.h"

#include "keyboard.h"
#include "devif.h"
#include "dbprint.h"

extern DLword *EmKbdAd068K
	    , *EmKbdAd168K
	    , *EmKbdAd268K
	    , *EmKbdAd368K
	    , *EmKbdAd468K
	    , *EmKbdAd568K
	    , *EmRealUtilin68K;
extern DLword *CTopKeyevent;
extern LispPTR *KEYBUFFERING68k;
extern int URaid_req
	 , KBDEventFlg;

extern DLword *DisplayRegion68k;

extern int Current_Hot_X, Current_Hot_Y;  /* X Cursor hotspots */

extern char Window_Title[255];
extern char Icon_Title[255];

extern int	save_argc;
extern char	**save_argv;

extern DspInterface currentdsp;

Pixmap      IconPixmap;
GC          CopyInvert_GC;
extern GC   cursor_source_gc;
XGCValues   gcv;
XEvent      report;

Cursor WaitCursor,
  DefaultCursor,
  VertScrollCursor,
  VertThumbCursor,
  ScrollUpCursor,
  ScrollDownCursor,
  HorizScrollCursor,
  HorizThumbCursor,
  ScrollLeftCursor,
  ScrollRightCursor;

extern int LispWindowRequestedX,
  LispWindowRequestedY,
  LispWindowRequestedWidht,
  LispWindowRequestedHeight;

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void Create_LispWindow(dsp)
     DspInterface dsp;
{
  XSizeHints  szhint;
  XWMHints    Lisp_WMhints;
  XClassHint  xclasshint;
  XTextProperty	IconText, WindowNameText;
  XSetWindowAttributes Lisp_SetWinAttributes;

  Screen *screen;

  Window lspwinRoot;
  int lspwinX, lspwinY, Col2, Row2, Col3, Row3, GravSize;
  unsigned int lspwinHeight , lspwinWidth, lspwinBorder, lspwinDepth;
  char *WT, *IT;

  WT = Window_Title;
  IT = Icon_Title;

  GravSize = (int)(dsp->ScrollBarWidth/2) - (dsp->InternalBorderWidth);
  Col2 = dsp->Vissible.width;
  Row2 = dsp->Vissible.height;
  Col3 = dsp->Vissible.width + (int)(OUTER_SB_WIDTH(dsp)/2);
  Row3 = dsp->Vissible.height + (int)(OUTER_SB_WIDTH(dsp)/2);

  screen = ScreenOfDisplay( dsp->display_id, DefaultScreen( dsp->display_id ));
  dsp->LispWindow = XCreateSimpleWindow( dsp->display_id,
					RootWindowOfScreen( screen ),
					LispWindowRequestedX, /* Default upper left */
					LispWindowRequestedY, /* Default upper left */
					dsp->Vissible.width + OUTER_SB_WIDTH( dsp ), /* Default width */
					dsp->Vissible.height + OUTER_SB_WIDTH( dsp ), /* Default height */
					0, /* Default border */
					BlackPixelOfScreen( screen ),
					WhitePixelOfScreen( screen ) );

  Lisp_SetWinAttributes.bit_gravity = dsp->BitGravity;
  Lisp_SetWinAttributes.override_redirect = False;
  Lisp_SetWinAttributes.backing_store = DoesBackingStore( screen );

  XChangeWindowAttributes( dsp->display_id
			  , dsp->LispWindow
			  , CWBitGravity | CWOverrideRedirect
			  , &Lisp_SetWinAttributes );

  dsp->DisableEventMask = NoEventMask;
  dsp->EnableEventMask = ButtonPressMask |
    ButtonReleaseMask |
      EnterWindowMask |
	LeaveWindowMask |
	  StructureNotifyMask |
	      PointerMotionMask |
		ExposureMask |
		    KeyPressMask |
		      KeyReleaseMask;

  /* Have to make the GC before we make the icon. */
  gcv.function   = GXcopy;
  gcv.foreground = BlackPixelOfScreen( ScreenOfDisplay( dsp->display_id,
						       DefaultScreen( dsp->display_id )));
  gcv.background = WhitePixelOfScreen( ScreenOfDisplay( dsp->display_id,
						       DefaultScreen( dsp->display_id )));
  dsp->Copy_GC = XCreateGC( dsp->display_id, dsp->LispWindow
			   , GCForeground | GCBackground | GCFunction
			   , &gcv );

  szhint.max_width = dsp->Display.width + OUTER_SB_WIDTH( dsp );
  szhint.max_height = dsp->Display.height + OUTER_SB_WIDTH( dsp );
  szhint.min_width = OUTER_SB_WIDTH( dsp );
  szhint.min_height = OUTER_SB_WIDTH( dsp );
  szhint.win_gravity = dsp->BitGravity;
  szhint.flags = PMaxSize | PWinGravity | PSize | PPosition;

  Lisp_WMhints.icon_pixmap = make_Xicon(dsp);
  Lisp_WMhints.input = True;
  Lisp_WMhints.flags  = IconPixmapHint | InputHint;

  xclasshint.res_name = *save_argv;
  xclasshint.res_class = *save_argv;

  XStringListToTextProperty(&WT , 1, &WindowNameText);
  XStringListToTextProperty(&IT, 1, &IconText);

  XSetWMProperties( dsp->display_id,
		   dsp->LispWindow,
		   &WindowNameText,
		   &IconText,
		   save_argv,
		   save_argc,
		   &szhint,
		   &Lisp_WMhints,
		   &xclasshint);

  XSelectInput( dsp->display_id,  dsp->LispWindow, dsp->EnableEventMask );
  init_Xcursor(dsp->display_id, dsp->LispWindow);

  dsp->DisplayWindow = XCreateSimpleWindow( dsp->display_id,
					   dsp->LispWindow,
					   0, 0, dsp->Vissible.width, dsp->Vissible.height, 0,
					   BlackPixelOfScreen(screen),
					   WhitePixelOfScreen(screen));
  XChangeWindowAttributes( dsp->display_id
			  , dsp->DisplayWindow
			  , CWBitGravity | CWOverrideRedirect | CWBackingStore
			  , &Lisp_SetWinAttributes );
  XSelectInput( dsp->display_id,  dsp->DisplayWindow, dsp->EnableEventMask );
  XMapWindow( dsp->display_id, dsp->DisplayWindow );

  /*********************************************************************/
  /* Create all the vanilla pixmaps and cursors for the display window */
  /*********************************************************************/

  dsp->ScrollBarPixmap =
    XCreatePixmapFromBitmapData( dsp->display_id,
				dsp->LispWindow,
				check_bits,
				check_width,
				check_height,
				BlackPixelOfScreen( screen ),
				WhitePixelOfScreen( screen ),
				DefaultDepthOfScreen( screen ));
  dsp->GravityOnPixmap =
    XCreatePixmapFromBitmapData(dsp->display_id,
				dsp->LispWindow,
				check_bits,
				16, 16,
				BlackPixelOfScreen( screen ),
				WhitePixelOfScreen( screen ),
				DefaultDepthOfScreen( screen ));

  dsp->GravityOffPixmap =
    XCreatePixmapFromBitmapData(dsp->display_id,
				dsp->LispWindow,
				plain_bits,
				16, 16,
				BlackPixelOfScreen( screen ),
				WhitePixelOfScreen( screen ),
				DefaultDepthOfScreen( screen ));

  set_Xcursor( dsp
	      , (char*) default_cursor.cuimage
	      , (int) default_cursor.cuhotspotx
	      , (int) (15 - default_cursor.cuhotspoty)
	      , &DefaultCursor, 0 );
  set_Xcursor( dsp
	      , (char*) wait_cursor.cuimage
	      , (int) wait_cursor.cuhotspotx
	      , (int) (15 - wait_cursor.cuhotspoty)
	      , &WaitCursor, 0 );
  set_Xcursor( dsp
	      , (char*)scrolldown_cursor.cuimage
	      , (int) scrolldown_cursor.cuhotspotx
	      , (int) (15 - scrolldown_cursor.cuhotspoty)
	      , &ScrollDownCursor, 0 );
  set_Xcursor( dsp
	      , (char*)scrollleft_cursor.cuimage
	      , (int) scrollleft_cursor.cuhotspotx
	      , (int) (15 - scrollleft_cursor.cuhotspoty)
	      , &ScrollLeftCursor , 0);
  set_Xcursor( dsp
	      , (char*)vertscroll_cursor.cuimage
	      , (int) vertscroll_cursor.cuhotspotx
	      , (int) (15 - vertscroll_cursor.cuhotspoty)
	      , &VertScrollCursor, 0 );
  set_Xcursor( dsp
	      , (char*)vertthumb_cursor.cuimage
	      , (int) vertthumb_cursor.cuhotspotx
	      , (int) (15 - vertthumb_cursor.cuhotspoty)
	      , &VertThumbCursor, 0 );
  set_Xcursor( dsp
	      , (char*)horizscroll_cursor.cuimage
	      , (int) horizscroll_cursor.cuhotspotx
	      , (int) (15 - horizscroll_cursor.cuhotspoty)
	      , &HorizScrollCursor, 0 );
  set_Xcursor( dsp
	      , (char*)horizthumb_cursor.cuimage
	      , (int) horizthumb_cursor.cuhotspotx
	      , (int) (15 - horizthumb_cursor.cuhotspoty)
	      , &HorizThumbCursor, 0 );
  set_Xcursor( dsp
	      , (char*)scrollright_cursor.cuimage
	      , (int) scrollright_cursor.cuhotspotx
	      , (int) (15 - scrollright_cursor.cuhotspoty)
	      , &ScrollRightCursor, 0 );
  set_Xcursor( dsp
	      , (char*)scrollup_cursor.cuimage
	      , (int) scrollup_cursor.cuhotspotx
	      , (int) (15 - scrollup_cursor.cuhotspoty)
	      , &ScrollUpCursor, 0 );

  /********************************/
  /* Make all the toolkit windows */
  /********************************/
  dsp->VerScrollBar = XCreateSimpleWindow( dsp->display_id,
					  dsp->LispWindow,
					  Col2,
					  0-dsp->InternalBorderWidth, /* y */
					  dsp->ScrollBarWidth, /* width */
					  dsp->Vissible.height,
					  dsp->InternalBorderWidth,
					  BlackPixelOfScreen(screen),
					  WhitePixelOfScreen(screen));
  DefineCursor( dsp->display_id, dsp->VerScrollBar, &VertScrollCursor );
  XMapWindow( dsp->display_id, dsp->VerScrollBar );

  dsp->HorScrollBar = XCreateSimpleWindow( dsp->display_id,
					  dsp->LispWindow,
					  0-dsp->InternalBorderWidth,
					  Row2, /* y */
					  dsp->Vissible.width, /* width */
					  dsp->ScrollBarWidth,
					  dsp->InternalBorderWidth,
					  BlackPixelOfScreen(screen),
					  WhitePixelOfScreen(screen));
  DefineCursor( dsp->display_id, dsp->HorScrollBar, &HorizScrollCursor );
  XChangeWindowAttributes( dsp->display_id
			  , dsp->HorScrollBar
			  , CWOverrideRedirect
			  , &Lisp_SetWinAttributes );
  XMapWindow( dsp->display_id, dsp->HorScrollBar );

  dsp->VerScrollButton = XCreateSimpleWindow( dsp->display_id,
					     dsp->VerScrollBar,
					     0-dsp->InternalBorderWidth, /* x */
					     (int)((dsp->Vissible.y * dsp->Vissible.height) / dsp->Display.height), /* y */
					     dsp->ScrollBarWidth, /* width */
					     (int)((dsp->Vissible.height*dsp->Vissible.height) / dsp->Display.height) + 1,
					     dsp->InternalBorderWidth,
					     BlackPixelOfScreen(screen),
					     WhitePixelOfScreen(screen));
  XChangeWindowAttributes( dsp->display_id
			  , dsp->VerScrollButton
			  , CWOverrideRedirect
			  , &Lisp_SetWinAttributes );
  XSetWindowBackgroundPixmap( dsp->display_id,
			     dsp->VerScrollButton,
			     dsp->ScrollBarPixmap );
  XClearWindow( dsp->display_id, dsp->VerScrollButton );
  XMapWindow( dsp->display_id, dsp->VerScrollButton );

  dsp->HorScrollButton = XCreateSimpleWindow( dsp->display_id,
					     dsp->HorScrollBar,
					     (int)((dsp->Vissible.x * dsp->Vissible.width) / dsp->Display.width),
					     0-dsp->InternalBorderWidth, /* y */
					     (int)((dsp->Vissible.width*dsp->Vissible.width)/ dsp->Display.width) + 1,
					     dsp->ScrollBarWidth,
					     dsp->InternalBorderWidth,
					     BlackPixelOfScreen(screen),
					     WhitePixelOfScreen(screen));
  XChangeWindowAttributes( dsp->display_id
			  , dsp->HorScrollButton
			  , CWOverrideRedirect
			  , &Lisp_SetWinAttributes );
  XSetWindowBackgroundPixmap( dsp->display_id,
			     dsp->HorScrollButton,
			     dsp->ScrollBarPixmap );
  XClearWindow( dsp->display_id, dsp->HorScrollButton );
  XMapWindow( dsp->display_id, dsp->HorScrollButton );  

  dsp->NWGrav = XCreateSimpleWindow( dsp->display_id,
				    dsp->LispWindow,
				    Col2, Row2, GravSize, GravSize,
				    dsp->InternalBorderWidth,
				    BlackPixelOfScreen(screen),
				    WhitePixelOfScreen(screen));
  XSetWindowBackgroundPixmap( dsp->display_id
			     , dsp->NWGrav
			     , dsp->GravityOnPixmap );
  DefineCursor( dsp->display_id, dsp->NWGrav, &DefaultCursor );
  XChangeWindowAttributes( dsp->display_id
			  , dsp->NWGrav
			  , CWOverrideRedirect
			  , &Lisp_SetWinAttributes );
  XClearWindow( dsp->display_id, dsp->NWGrav );
  XMapWindow( dsp->display_id, dsp->NWGrav );

  dsp->SEGrav = XCreateSimpleWindow( dsp->display_id,
				    dsp->LispWindow,
				    Col3, Row3, GravSize, GravSize,
				    dsp->InternalBorderWidth,
				    BlackPixelOfScreen(screen),
				    WhitePixelOfScreen(screen));
  XSetWindowBackgroundPixmap( dsp->display_id
			     , dsp->SEGrav
			     , dsp->GravityOffPixmap );
  DefineCursor( dsp->display_id, dsp->SEGrav, &DefaultCursor );
  XChangeWindowAttributes( dsp->display_id
			  , dsp->SEGrav
			  , CWOverrideRedirect
			  , &Lisp_SetWinAttributes );
  XClearWindow( dsp->display_id, dsp->NWGrav );
  XMapWindow( dsp->display_id, dsp->SEGrav );

  dsp->SWGrav = XCreateSimpleWindow( dsp->display_id,
				    dsp->LispWindow,
				    Col2, Row3, GravSize, GravSize,
				    dsp->InternalBorderWidth,
				    BlackPixelOfScreen(screen),
				    WhitePixelOfScreen(screen));
  XSetWindowBackgroundPixmap( dsp->display_id
			     , dsp->SWGrav
			     , dsp->GravityOffPixmap );
  DefineCursor( dsp->display_id, dsp->SWGrav, &DefaultCursor );
  XClearWindow( dsp->display_id, dsp->NWGrav );
  XMapWindow( dsp->display_id, dsp->SWGrav );

  dsp->NEGrav = XCreateSimpleWindow( dsp->display_id,
				    dsp->LispWindow,
				    Col3, Row2, GravSize, GravSize,
				    dsp->InternalBorderWidth,
				    BlackPixelOfScreen(screen),
				    WhitePixelOfScreen(screen));
  XSetWindowBackgroundPixmap( dsp->display_id
			     , dsp->NEGrav
			     , dsp->GravityOffPixmap );
  DefineCursor( dsp->display_id, dsp->NEGrav, &DefaultCursor );
  XClearWindow( dsp->display_id, dsp->NWGrav );
  XMapWindow( dsp->display_id, dsp->NEGrav );

  /* DefineCursor( dsp->display_id, dsp->DisplayWindow, &WaitCursor ); */

  XLOCK;
  XMapWindow( dsp->display_id, dsp->LispWindow );
  XFlush(dsp->display_id);
  XUNLOCK;
} 


void lisp_Xvideocolor( flag )
int flag;
{
  XLOCK;
  XCopyArea( currentdsp->display_id  
	    , currentdsp->DisplayWindow
	    , currentdsp->DisplayWindow
	    , currentdsp->Copy_GC
	    , 0, 0
	    , currentdsp->Vissible.width
	    , currentdsp->Vissible.height
	    , 0, 0 );

  /* Cursor */
  gcv.function = GXcopy;
  XChangeGC( currentdsp->display_id, cursor_source_gc , GCFunction , &gcv );
  /* JDS 011213: Remember set_xcursor does 15-y val, so do it here! */
  Set_XCursor( Current_Hot_X, 15-Current_Hot_Y );

  XFlush(currentdsp->display_id);
  XUNLOCK;
		
} /* end lisp_Xvideocolor */




extern int Current_Hot_X, Current_Hot_Y;  /* Cursor hotspot */

void set_Xmouseposition( x, y )
     int x
       , y;
{
  int dest_x
    , dest_y;
  
  TPRINT(( "set_Xmouseposition(%d,%d)\n", x, y ));
  
  dest_x = (x&0xFFFF) + Current_Hot_X - currentdsp->Vissible.x;
  dest_y = (y&0xFFFF) + Current_Hot_Y - currentdsp->Vissible.y;
  
  if( (dest_x>=0) && (dest_x<= currentdsp->Vissible.width)
     && (dest_y>=0) && (dest_y<=currentdsp->Vissible.height) ) {
    XLOCK;
    XWarpPointer( currentdsp->display_id, (Window)NULL, currentdsp->DisplayWindow
		 , 0, 0
		 , 0, 0
		 , dest_x, dest_y );
    XFlush( currentdsp->display_id );
    XUNLOCK;
  }
}				/* end set_Xmouseposition */


/************************************************************************/
/*									*/
/*				D o R i n g				*/
/*									*/
/*	Take keyboard events and turn them into Lisp event info		*/
/*	(when running under X)						*/
/*									*/
/************************************************************************/

void DoRing()
  {
    DLword w
         , r;
    KBEVENT *kbevent;

    TPRINT(( "TRACE: DoRing()\n" ));


do_ring:
    if(((*EmKbdAd268K) & 2113) == 0)
      { /*Ctrl-shift-NEXT*/
	error("******  EMERGENCY Interrupt ******");
	*EmKbdAd268K=KB_ALLUP; /*reset*/
	((RING*)CTopKeyevent)->read=0;  /* reset queue */
	((RING*)CTopKeyevent)->write=MINKEYEVENT; 
	/*return(0);*/
      } 
    else if(((*EmKbdAd268K) & 2114) == 0)
      { /* Ctrl-Shift-DEL */
	*EmKbdAd268K=KB_ALLUP; /*reset*/
	URaid_req=T;
	((RING*)CTopKeyevent)->read=0;  /* reset queue */
	((RING*)CTopKeyevent)->write=MINKEYEVENT; 
	/*return(0);*/
      }
	     
#ifdef OS4_TYPE4BUG
    else if(((*EmKbdAd268K) & 2120) == 0)
      { /* Ctrl-Shift-Return */
	*EmKbdAd268K=KB_ALLUP; /*reset*/
	URaid_req=T;
	((RING*)CTopKeyevent)->read=0;  /* reset queue */
	((RING*)CTopKeyevent)->write=MINKEYEVENT; 
      }
#endif

    r=RING_READ(CTopKeyevent);
    w=RING_WRITE(CTopKeyevent);

    if(r==w) /* event queqe FULL */
      goto KBnext;

    kbevent=(KBEVENT*)(CTopKeyevent+ w);
/*    RCLK(kbevent->time); */
    kbevent->W0= *EmKbdAd068K;
    kbevent->W1= *EmKbdAd168K;
    kbevent->W2= *EmKbdAd268K;
    kbevent->W3= *EmKbdAd368K;
    kbevent->W4= *EmKbdAd468K;
    kbevent->W5= *EmKbdAd568K;
    kbevent->WU= *EmRealUtilin68K;

    if(r==0) /* Queue was empty */
	((RING*)CTopKeyevent)->read=w;
    if(w >= MAXKEYEVENT) ((RING*)CTopKeyevent)->write = MINKEYEVENT;
    else ((RING*)CTopKeyevent)->write = w + KEYEVENTSIZE;

  KBnext:
    if(*KEYBUFFERING68k ==NIL) *KEYBUFFERING68k=ATOM_T;

  } 
