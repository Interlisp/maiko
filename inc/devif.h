#ifndef DEVIF_H
#define DEVIF_H 1
/* $Id: devif.h,v 1.2 1999/01/03 02:05:57 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */

#ifdef XWINDOW
#include <X11/Xlib.h>
#endif /* XWINDOW */

typedef struct
  {
    short type;		/* Type of event */
    short next;		/* Index to next event */
  } IRQEvent;



/**************************************************************/
/*                     P o s i t i o n                        */
/*                                                            */
/* Support structure for the geometry calculations.           */
/**************************************************************/
typedef struct
  {
    int x;
    int y;
  } Mposition;



/**************************************************************/
/*                        R e g i o n                         */
/*                                                            */
/* Support structure for the geometry calculations.           */
/**************************************************************/
typedef struct
  {
    int	x;
    int	y;
    unsigned width;
    unsigned height;
  } MRegion;

/**************************************************************/
/*		           D e v R e c                        */
/*                                                            */
/* Definition common to all devices. Used for mouse, kbd and  */
/* display. The xxxInterfaceRec containing this device is     */
/* passed as the only argument to the device methods          */
/**************************************************************/
typedef struct
  {
    int	active;		/* ACTIVE, a flag.
			   TRUE if this device is activated. Use this
			   to prevent multiple consecutive initializations. */
    int	locked;		/* LOCK, a semaphore:  0 if dev is free.
			   Test and increment to use this device. */
    void (* enter)(void *);	/* ENTER, a function
			   args: interface rec (Kbd, Dsp, Mouse)
			   Called to set up the device. Has to be called before
			   anything else is done to the device. */
    void (* exit)(void *);	/* EXIT, a function
			   args: interface rec (Kbd, Dsp, Mouse)
			   Called to deactivate the device and restore the
			   device to its previous state */
    void (* before_raid)(void *); /* BEFORE_RAID, a function.
			   args: interface rec (Kbd, Dsp, Mouse)
			   Prepare this device for uraid. */
    void (* after_raid)(void *); /* BEFORE_RAID, a function.
			   args: interface rec (Kbd, Dsp, Mouse)
			   Cleanup and restart device after uraid. */
    void (* sync_device)(void *); /* SYNC_DEVICE, a function.
			   args: interface rec (Kbd, Dsp, Mouse)
			   Make reality and emulator coincide with each other */
  } DevRec;



/**************************************************************/
/*                M o u s e I n t e r f a c e                 */
/*                                                            */
/* Definition of the mouse. Note that the mouse is also       */
/* dependent on the IOPage                                    */
/**************************************************************/
typedef struct
  {
    unsigned TwoButtonP: 1;
   				 /* Interface towards Lisp */
    unsigned Left: 1;		/* Left button state. */
    unsigned Middle: 1;		/* Middle button state. */
    unsigned Right: 1;		/* Right button state. */
    				/* Mouse chording machinery */
    unsigned StateLeft:  1;
    unsigned StateMiddle: 1;	/* real middle state */
    unsigned StateRight: 1;
    unsigned FakeMiddle: 1;
    unsigned nil: 8;
    short tick;		/* Clock for timeout. */
    long StartTime;	/* The maximum timeout */
    long RunTimer;	/* Chording timer activate flag. */
    void (* NextHandler)(void);	/* Pointer to the next timer (used with 2button) */
  } Button;


typedef struct {
  MRegion   Last;	/* Last position the mouse was in. */
  Mposition New;	/* The place to move the mouse when we have time */
  Mposition Hotspot;	/* The current hotspot for the mousecursor */
  long	    Moved;	/* Did the mouse move? */
  DLword    Savebitmap[16];	/* The bitmap under the mouse. Taking down the
			   mouse involves blitting this bitmap to the screen.
			   When bringing the mouse back up to the screen you
			   first have to save the contents of the screen under
			   the mouse here.*/
  } MCursor;

typedef struct
  {
    DevRec device;
    void   (* Handler)(void);	/* Event handler for the mouse. */
    MCursor Cursor;
    Button Button;
    LispPTR *timestamp;
    unsigned int keyeventsize;	/* The sizeof() one kbd event */
    unsigned int maxkeyevent;	/* Offset to the end of the ringbuffer. */
    int eurokbd;		/* Keep tabs of the euro-ness of the kbd */
  } MouseInterfaceRec;
typedef MouseInterfaceRec *MouseInterface;



/**************************************************************/
/*                  K b d I n t e r f a c e                   */
/*                                                            */
/* Definition of the keyboard. Note that the keyboard is also */
/* dependent on the IOPage                                    */
/**************************************************************/
typedef struct
  {
    DevRec device;
    void (*device_event)(void);	/* Event handler for the keyboard. */
#ifdef DOS
    u_char KeyMap[0x80];	/* The key translation table. Use the keycode you
				   get from the keyboard as an index. The value
				   gives the lispkeycode.*/
    unsigned char lastbyte;	/* Last byte that we got from the keyboard. */
    unsigned int keyeventsize;	/* The sizeof() one kbd event */
    unsigned int maxkeyevent;	/* Offset to the end of the ringbuffer. */
    int eurokbd;		/* Keep tabs of the euro-ness of the kbd */
    void (* prev_handler)(void);/* The previous keyboard handler.
				   Keep this around
				   to restore when we exit Medley */
    int	URaid;			/* Put this in a better place later.. /jarl */
#endif /* DOS */
  } KbdInterfaceRec;
typedef KbdInterfaceRec *KbdInterface;

/**************************************************************/
/*                  D s p I n t e r f a c e                   */
/*                                                            */
/* Definition of the display. This structure collects all the */
/* special knowledge needed to manipulate the screen.         */
/**************************************************************/
/*
 * NOTE: At this time only the DspInterface methods
 *       bitblt_to_screen(), clearscreen(), mouse_visible(), and mouse_invisible()
 *       are called, and the mouse_* are only used for DOS.
 *       All the other methods are not implemented and not called.
 */
typedef struct DspInterfaceRec
  {
    DevRec device;
  
    unsigned long (* drawline)(void);	/* DRAWLINE
				 args: dont know yet. Not yet implemented.*/
    unsigned long (* cleardisplay)(struct DspInterfaceRec *);	/* CLEARDISPLAY, a function
				 args: self
				 clears the screen.*/

    unsigned long (* get_color_map_entry)(void);
    unsigned long (* set_color_map_entry)(void *);
    unsigned long (* available_colors)(void);	/* How many colors do I have on my palette */
    unsigned long (* possible_colors)(void); /* How many colors is it possible to select from */

#ifdef NOTYET
    unsigned long (* get_color_map)(void); /* get a pointer to a colormap */
    unsigned long (* set_color_map)(void); /* set the current colormap */
    unsigned long (* make_color_map)(void); /* return a brand new colormap */
#endif /* NOTYET */

    unsigned long (* medley_to_native_bm)(void); /* 1 bit/pix to native bit/pix */
    unsigned long (* native_to_medley_bm)(void); /* native bit/pix to 1 bit/pix */

    unsigned long (* bitblt_to_screen)(struct DspInterfaceRec *, DLword *, int, int, int, int);	/* BITBLT_TO_SCREEN, a function
					   args: self, buffer left top width height.
					   biblt's buffer to the screen. */
    unsigned long (* bitblt_from_screen)(void);
    unsigned long (* scroll_region)(void); /* ie. bbt from screen to screen */
    unsigned long (* mouse_invisible)(struct DspInterfaceRec *, void *);	/* MOUSE_INVISIBLE
				   args: self (a dsp), iop (an IOPAGE preferably the one and only)
				   This method makes the mouse invisible on the screen. Note that
				   the dsp supplies the method and the iop supplies the data. */
    unsigned long (* mouse_visible)(int x, int y);	/* MOUSE_VISIBLE
				   args: x, y position where the mouse/cursor should be displayed.
				   NOTE: this should probably include the DspInterface as the first arg?
							*/
    MRegion Display;		/* Dimensions of the physical display. */
    unsigned short unused0;	/* alignment padding for next field */
    unsigned short bitsperpixel;
    unsigned long colors;	/* cache for the available_colors */
    unsigned long oldstate; /* Keep the old state around */
    unsigned long graphicsmode; /* Magic cookie used to set the state. */
    unsigned long numberofbanks;
#ifdef DOS
    unsigned long BytesPerLine;
    unsigned long DisplayStartAddr;
    unsigned long DisplaySegSize;
    unsigned long DisplaySegMagnitude;
    unsigned long LinesPerBank;
    unsigned short LastLineLen[32];     /* length of last line fragment per bank */
    unsigned short LinesInBank[32];     /* True # of full lines in this bank */
					/* # of lines we can do with the full-line dumpline */
					/* for sure. */
    unsigned short LinesBeforeBank[32]; /* Scan lines before start of this bank. */

    void (* SwitchBank)(); /* Method to switch the bank (see vesa standard) */
#elif XWINDOW
    char *identifier;
    int BitGravity;
    Display *display_id;
    Window LispWindow;
    Window DisplayWindow;
    Window HorScrollBar;
    Window VerScrollBar;
    Window HorScrollButton;
    Window VerScrollButton;
    Window NEGrav;
    Window SEGrav;
    Window SWGrav;
    Window NWGrav;
    GC	 Copy_GC;
    MRegion Visible;
    unsigned int InternalBorderWidth;
    unsigned int ScrollBarWidth;
    Pixmap ScrollBarPixmap;
    Pixmap GravityOnPixmap;
    Pixmap GravityOffPixmap;
    XImage ScreenBitmap;
    long   DisableEventMask;
    long   EnableEventMask;
#endif /* XWINDOW */
  } DspInterfaceRec;
typedef DspInterfaceRec *DspInterface;



#ifdef XWINDOW
#define DefineCursor(dsp, window, mycursor)                     \
  do {								\
    XLOCK;                                                      \
    XDefineCursor((dsp)->display_id, window, *(mycursor) );     \
    XUNLOCK(dsp);                                               \
  } while (0)
#endif /* XWINDOW */

#define OUTER_SB_WIDTH(dsp) ((dsp)->ScrollBarWidth + 2*((dsp)->InternalBorderWidth))

#ifndef min
#define min( a, b ) (((a)<(b))?(a):(b))
#endif /* min */

#ifndef max
#define max( a, b ) (((a)>(b))?(a):(b))
#endif /* max */

#ifndef mid
#define mid(a, b, c) max( min( b, max( a, c ), min( a, max( b, c ))))
#endif /* mid */

#define MINKEYEVENT	2	/* leave 2 words for read,write offsets */
#define NUMBEROFKEYEVENTS 383

#endif

