/* $Id: mnxdefs.h,v 1.2 1999/01/03 02:06:17 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */





/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


#ifdef XTK
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xaw/Scrollbar.h>
#endif /* XTK */

typedef void (*PFV)();		/* Pointer to Function returning Void */
typedef int (*PFI)();		/* Pointer to Function returning Int */
typedef char (*PFC)();		/* Pointer to Function returning Char */
typedef float (*PFF)();		/* Pointer to Function returning Float */
typedef int (*PFP)();		/* Pointer to Function returning a Pointer */
typedef unsigned long (*PFUL)(); /* Pointer to Function returning an unsigned long */


typedef struct {
  int	x;
  int	y;
  int	width;
  int	height;
} MRegion;


typedef struct {
  int	x;
  int	y;
} MPoint,
  MPosition;


typedef struct {
  unsigned	left:1;
  unsigned	middle:1;
  unsigned	right:1;
  unsigned	nil:28;
} MButton;

typedef union
{
  struct {
    PFI		InitW;		/* Initialize a window */
    PFI		Openw;
    PFI		Closew;
    PFI		MoveW;
    PFI		ShapeW;
    PFI		TotopW;
    PFI		BuryW;
    PFI		ShrinkW;
    PFI		ExpandW;
    PFI		DestroyW;	/* Finalize a window */
    PFI		GCIndicator;
    PFI		Query;
    PFI		DestroyMe;	/* Finalize self */
    PFI		MakePromptW;
    PFI		BBTtoWin;	/* Bitblt cases lisp -> native window, */
    PFI		BBTfromWin;	/* native window -> lisp bitmap */
    PFI		BBTWinWin;	/* native window -> native window */
    PFI		GetWindowProp;
    PFI		PutWindowProp;
    PFI		GrabPointer;
    PFI		UngrabPointer;
    PFI		DrawBox;
    PFI		MovePointer;
    PFI		MouseConfirm;
    PFI		SetCursor;
  } Method;
  PFI vector[ 25 ];
} DisplayDispatchTableRec, *DisplayDispatchTable;




typedef union
{
  struct {
    PFI		CloseFn;
    PFI		XPosition;
    PFI		YPosItion;
    PFI		Font;
    PFI		FontCreate;
    PFI		StringWidth;
    PFI		CharWidth;
    PFI		CharWidthY;
    PFI		LeftMargin;
    PFI		RightMargin;
    PFI		TopMargin;
    PFI		BottomMargin;
    PFI		ClippingRegion;
    PFI		PushState;
    PFI		PopState;
    PFI		DefaultState;
    PFI		Scale;
    PFI		Scale2;
    PFI		Translate;
    PFI		Rotate;
    PFI		Color;
    PFI		BackColor;
    PFI		Operation;
    PFI		MoveTo;
    PFI		Reset;
    PFI		NewPage;
    PFI		LineFeed;
    PFI		TerPri;
    PFI		ScaleFactor;
    PFI		OutcharFn;
    PFI		CharSet;
    PFI		DrawPoint;
    PFI		DrawLine;
    PFI		DrawCurve;
    PFI		DrawCircle;
    PFI		DrawElips;
    PFI		DrawPolygon;
    PFI		FillPolygon;
    PFI		FillCircle;
    PFI		DrawArc;
    PFI		BltShade;
    PFI		BitBlt;
    PFI		ScaledBitBlt;
    PFI		WritePixel;
    PFI		BitmapSize;
	PFI     Offsets;
    } Method;
  PFI vector[ 44 ];
} ImageOpDispatchTableRec, *ImageOpDispatchTable;


typedef struct wifrec
{
  int		error;		   /* Place to save error number for diagnosis */
  MRegion	windowreg;	   /* The position Lisp thinks the window is in, LISP coordinates! */
  MRegion	topregion;	   /* The toplevel widget's size. */
  MRegion	outerregion;   /* The window's outer size. */
  MRegion	innerregion;   /* The window's inner size. */
  MRegion	extent;		   /* The extent of the whole window. */
  int		whiteborder;   /* The white border of the window. */
  int		blackborder2;
  Window	blackframe;	   /* The black part of the windowframe */
  Window	handle;		   /* The actual X window for displaying things */
  Pixmap    backing;	   /* Pixmap to store window image / temp results */
  GC		ReplaceGC;	   /* GC for operations in REPLACE mode */
  GC		InvertGC;	   /* GC for operations in INVERT mode */
  GC		PaintGC1;	   /* Gc #1 for operations in PAINT mode */
  GC		PaintGC2;	   /* GC #2 for operations in PAINT mode */
  GC		EraseGC1;	   /* GC #1 for operations in ERASE mode */
  GC		EraseGC2;	   /* GC #2 for operations in ERASE mode */
  GC		gc;			   /* GC for random use */
				/* Window Methods */
  ImageOpDispatchTable	Dispatch;
  LispPTR	MedleyWindow;	/* The Lisp WINDOW this corresponds to */
  LispPTR	MedleyScreen;	/* The Lisp SCREEN this corresponds to */
  Screen	*screen;	   /* The X screen this uses */
  int		depth;		   /* Depth of the window */

  int		op;	           /* current OP of window (0 = replace, etc) */
  int		xoffset;       /* X offset, from the stream */
  int		yoffset;	   /* Y offset, from the stream */
  LispPTR	FGcolor;	   /* Foreground color (fixp or bitmap for fill ops) */
  LispPTR	BGcolor;	   /* Background color (fixp or bitmap for fill ops) */

  Pixmap	fgpixmap;	   /* These two slots are caches to make sure that  */
  Pixmap	bgpixmap;	   /* the pixmaps in them a) stay around till they  */
				           /* are needed, and (b) get garbage collected */
  unsigned	not_exposed:1; /* T if next Expose should decache SAVE bitmap */
  unsigned	moving:1;	   /* T if lisp MOVEW called, so ignore X event */
  unsigned	reshaping:1;   /* T if lisp SHAPEW called, so ignore X event */
  unsigned	scrollfn:1;	   /* T if we have a scrollfn */
  unsigned	noscrollbars:1;	/* T if windowprop NOSCROLLBARS is set. */
  unsigned  open:1;			/* T if this window is open; NIL if not */
  unsigned  move_pend:1;	/* T if we moved this window while closed */
  unsigned	shape_pend:1;	/* T if we reshaped this window while closed */
  unsigned	nil:24;		    /* space for future flags */
  struct wifrec *next;		/* Thread all windows on this screen */
  Window	parent;		    /* The parent window of this window */
  struct dspifrec *dspif;	/* the dspif for this window */
#ifdef XTK
  Widget	topwidget;	    /* The hold on the widget of this window */
  Widget	formwidget;
  Widget	framewidget;	/* The widget to represent the frame */
  Widget	windowwidget;	/* The white region of the window */
  char		gstring[32];	/* A string to hold geometry strings in. */
#endif /*XTK */
} WindowInterfaceRec, *WindowInterface;



	/******************************************************/
	/*                                                    */
	/*         D S P I N T E R F A C E R E C              */
	/*                                                    */
	/*  Display-interface record:  The interface from     */
	/*  Medley to the X display for a given Medley-       */
	/*  Native-Windows screen.                            */
	/*                                                    */
	/******************************************************/

typedef struct dspifrec
{
  int		error;		/* A generic errornumber */
  Display	*handle;	/* The X Display this dspif describes */
  Mask		DisableEventMask;
  Mask		EnableEventMask;
  XImage	image;		/* The scratch image structure (used in blt'ing) */
  XImage	tmpimage;	/* The tmp image (used for depth conversion) */
  GC		TitleGC;	/* The gc for title blitting. */
  DisplayDispatchTableRec Dispatch; /* The methods for the display */
  ImageOpDispatchTableRec ImageOp;
  LispPTR	screen;		/* The lisp SCREEN this display  corresponds to */
  Screen	*xscreen;	/* The X screen we're on on this display */
  Window	root;		/* The root window for this screen */
  Cursor	cursor;		/* The cursor in effect on this screen */
  int		black;		/* black pixel for this screen */
  int		white;		/* white pixel for this screen */
  int		width;		/* Width of the screen, in pixels */
  int		height;		/* Height of the screen, in pixels */
  int		depth;		/* SCDEPTH for this screen -- Medley's view */
  WindowInterface CreatedWifs;	/* A list of all windows on this screen */
  WindowInterface promptw;		/* The PROMPTWINDOW for this screen */
#ifdef XTK
  XtAppContext	xtcontext;	/* The application context for this display */
#endif				/*XTK */

  int		bw_plane;	/* plane-mask for the plane that distinguishes B & W */
  int		bw_inverted;	/* T if B & W are backwards from Lisp */

  GC		PixRGC;		/* GC for blt'ing FROM window in REPLACE mode */
  GC		PixPGC;		/* GC for blt'ing FROM window in PAINT mode */
  GC		PixIGC;		/* GC for blt'ing FROM window in INVERT mode */
  GC		PixEGC;		/* GC for blt'ing FROM window in ERASE mode */
  GC		BoxingGC;	/* GC for drawing boxes on full screen */
  Widget	gcindicator;
  Widget	legatewidget;
} DspInterfaceRec, *DspInterface;

#ifdef NEVER

/******************************************/
/* The Legate window structure is a       */
/* WindowInterfaceRec with some stuff on  */
/* the tail end of the record. This is    */
/* to be viewed as the subclassing of the */
/* ordinary WindowInterfaceRec.           */
/******************************************/
typedef struct
{
  WindowInterfaceRec	promptw; /* The prompt window. */
  Widget	barwidget;	/* A place to hold the bar widgets. */
  Widget	gcindicator;	/* The garbage collector status window */
  Widget	menuwindow;	/* The background popup menu window */
} LegateWindowInterfaceRec, *LegateWindowInterface;
#endif /* NEVER */



typedef struct{
  LispPTR	SCONOFF;
  LispPTR	SCDESTINATION;
  LispPTR	SCWIDTH;
  LispPTR	SCHEIGHT;
  LispPTR	SCTOPW;
  LispPTR	SCTOPWDS;
  LispPTR	SCTITLEDS;
  LispPTR	SCFDEV;
  LispPTR	SCDS;
  LispPTR	SCDATA;
  DspInterface	NativeIf;
#ifdef THIRTYTWOBITS
  int		junk;
#endif /* BITS */
  LispPTR	NATIVE_INFO;
  LispPTR	NATIVETYPE;
  LispPTR	WINIMAGEOPS;
  LispPTR	WINFDEV;
  LispPTR	CREATEWFN;
  LispPTR	OPENWFN;
  LispPTR	CLOSEWFN;
  LispPTR	MOVEWFN;
  LispPTR	RELMOVEWFN;
  LispPTR	SHRINKWFN;
  LispPTR	EXPANDWFN;
  LispPTR	SHAPEWFN;
  LispPTR	REDISPLAYFN;
  LispPTR	GETWINDOWPROPFN;
  LispPTR	SETWINDOWPROPFN;
  LispPTR	BURYWFN;
  LispPTR	TOTOPWFN;
  LispPTR	IMPORTWFN;
  LispPTR	EXPORTWFN;
  LispPTR	DESTROYFN;
  LispPTR	SETCURSORFN;
  LispPTR	PROMPTW;
  LispPTR	SHOWGCFN;
  LispPTR	DSPCREATEFN;
  LispPTR	BBTTOWIN;
  LispPTR	BBTFROMWIN;
  LispPTR	BBTWINWIN;
  LispPTR	SCCURSOR;
  LispPTR	SCKEYBOARD;
  LispPTR	SCDEPTH;
} MedleyScreenRec, *MedleyScreen;




typedef struct{
  LispPTR	DSP;
  LispPTR	NEXTW;
  LispPTR	SAVE;
  LispPTR	REG;
  LispPTR	BUTTONEVENTFN;
  LispPTR	RIGHTBUTTONFN;
  LispPTR	CURSORINFN;
  LispPTR	CURSOROUTFN;
  LispPTR	CURSORMOVEFN;
  LispPTR	REPAINTFN;
  LispPTR	RESHAPEFN;
  LispPTR	EXTENT;
  LispPTR	USERDATA;
  LispPTR	VERTSCROLLREG;
  LispPTR	HORIZSCROLLREG;
  LispPTR	SCROLLFN;
  LispPTR	VERTSCROLLWINDOW;
  LispPTR	HORIZSCROLLWINDOW;
  LispPTR	CLOSEFN;
  LispPTR	MOVEFN;
  LispPTR	WTITLE;
  LispPTR	NEWREGION;
  LispPTR	WBORDER;	/* Assumed to allways be a SMALLP */
  LispPTR	PROCESS;
  LispPTR	WINDOWENTRYFN;
  LispPTR	SCREEN;
  WindowInterface NativeIf;
#ifdef THIRTYTWOBITS
  int		junk1;
#endif /* THIRTYTWOBITS */
  LispPTR	MISCNATIVE;
#ifdef THIRTYTWOBITS
  int		junk2;
#endif /* THIRTYTWOBITS */
  LispPTR	NATIVE_P1;
} MedleyWindowRec, *MedleyWindow;

typedef LispPTR *LispArgs;


typedef struct WinList
  {
	struct WinList *prior;
	struct WinList *next;
	WindowInterfaceRec *thiswin;
  } winlist;



  /* Structure that defines an entry in the ring buffer of MNW events */
  /* This uses the same ring-buffer scheme as keyboard codes do       */

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  int		pad[4];		/* Ad hoc fields to be able to access slots by */
				/* position in the struct rather than name. */
} MNWAnyEvent,
  MNWCloseEvent,
  MNWFocusEvent,
  MNWFocusInEvent,
  MNWFocusOutEvent,
  MNWToTopEvent;

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  MPosition	pos;
} MNWMoveEvent,
  MNWPointerMotionEvent;
  
typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  MRegion	reg;
} MNWReshapeEvent,
  MNWShapeReqEvent;

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  MPosition	pos;
  MButton	button;
} MNWButtonEvent,
  MNWButtonDownEvent,
  MNWButtonUpEvent;

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  MPosition	pos;
  MButton	button;
} MNWMouseEvent,
  MNWMouseInEvent,
  MNWMouseOutEvent;

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  int		dx;
  int		dy;
} MNWScrollReqEvent;

typedef struct {
  LispPTR	screen;		/* lisp SCREEN this event happened for */
  LispPTR	window;		/* lisp WINDOW this event happened for */
  int		event;		/* What kind of event this is (see defs below for code) */
  float		xpercent;
  float		ypercent;
} MNWJumpScrollReqEvent;

typedef union mnwevenT
  {
    MNWAnyEvent			Any;
    MNWCloseEvent		Close;
    MNWFocusEvent		Focus;
    MNWFocusInEvent		FocusBegin;
    MNWFocusOutEvent		FocusEnd;
    MNWToTopEvent		Top;
    MNWMoveEvent		Move;
    MNWReshapeEvent		Reshape;
    MNWShapeReqEvent		ShapeReq;
    MNWButtonEvent		Button;
    MNWButtonDownEvent		ButtonDown;
    MNWButtonUpEvent		ButtonUp;
    MNWMouseEvent		Mouse;
    MNWMouseInEvent		MouseIn;
    MNWMouseOutEvent		MouseOut;
    MNWScrollReqEvent		ScrollReq;
    MNWJumpScrollReqEvent	JumpScrollReq;
  } MNWEvent;

#define MNWClose    1
#define MNWMove     2
#define MNWReshape  3		/* This window was reshaped */
#define MNWFocusIn  4		/* Focus moved into this window */
#define MNWFocusOut 5		/* Focus moved out of this window */
#define MNWButton   6
#define MNWButtonUp 7
#define MNWMouseIn  8
#define MNWMouseOut 9		/* Mouse moved out of this window */
#define MNWToTop    10		/* This window brought to top */
#define MNWShapeReq 11		/* Window mgr asked to reshape this window */
#define MNWScrollReq 12		/* Window widget asked for incremental scroll. */
#define MNWScrollJmpReq 13	/* Window widget asked for jmp scroll. */
#define MNWPointerMotion 14

#define MINMNWEVENT 2		/* allow 2 words for the ring buffer */
#define MNWEVENTSIZE  ((sizeof(MNWEvent)+1)>>1)
#define MAXMNWEVENT (MINMNWEVENT + (100* MNWEVENTSIZE))

#define MNWTitle 0
#define MNWScrollFn 1
#define MNWNoScrollbars 2
#define MNWScrollExtent 3
#define MNWScrollExtentUse 4
#define MNWBorder 5

#define REPLACE 0		/* Operations for BITBLT, etc, as they come from lisp */
#define PAINT   1
#define INVERT  2
#define ERASE   3


/***************************************************/
/* Macros for dealing with pointer complexity.     */
/***************************************************/


/***************************************************/
/* The argument for all these macros is a LispPTR  */
/* to a Medley window structure.                   */
/* The result of the calculation is a C pointer.   */
/***************************************************/
#define Cptr(LADDR) Addr68k_from_LADDR(LADDR)
#define WIfFromMw(win) (((MedleyWindow)Cptr(win))->NativeIf)
#define ScrnFromMw(win) ((MedleyScreenRec *)Cptr(((MedleyWindow)Cptr(win))->SCREEN))
#define DspstreamFromMw(win) ((Stream *)Cptr(((MedleyWindow)Cptr(win))->DSP))
#define ImDataFromMw(win) ((DISPLAYDATA *)Cptr(DspstreamFromMw(win)->IMAGEDATA))
#define MScrFromMw(win) ((MedleyScreen)Cptr(((MedleyWindow)Cptr(win))->SCREEN))
#define TitleDSFromMw(win) ((Stream *)Cptr(MScrFromMw(win)->SCTITLEDS))
#define TitleDDFromMw(win) ((DISPLAYDATA *)Cptr(TitleDSFromMw(win)->IMAGEDATA))
#define DspIfFromMw(win) (MScrFromMw(win)->NativeIf)
#define XDisplayFromMw(win) (DspIfFromMw(win)->handle)
#define XWindowFromMw(win) (WIfFromMw(win)->handle)

#define DspIfFromMscr(scr) (((MedleyScreen)Cptr(scr))->NativeIf)

#ifndef max
#define max( a, b ) (((a)>(b))?(a):(b))
#endif /* max */

#ifndef min
#define min( a, b ) (((a)<(b))?(a):(b))
#endif /* min */
