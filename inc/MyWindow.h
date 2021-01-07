#ifndef MYWINDOW_H
#define MYWINDOW_H 1
/* $Id: MyWindow.h,v 1.2 1999/01/03 02:05:47 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*			    M y W i n d o w . h				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "ldeXdefs.h"

#define VERTICAL   0
#define HORIZONTAL 1

#define PERCENT_OF_SCREEN 95
#define SCROLL_PITCH      30

typedef struct _MyEvent
  {
    int       type; 		/* Event type */
    int     (*func)();     	/* Pointer to Event Handler */
    struct _MyEvent *next;	/* Pointer to next event */
  } MyEvent;

typedef struct _MyWindow
  {
    char      *name;	/* name of this window */
    Window     win;         /* window id */
    int        x,y;         /* x and y coordinates */
    int        width,height;/* window size */
    int        border;      /* border width */
    GC        *gc; 		/* Current GC */
    unsigned   long event_mask;
    Cursor    *cursor;      /* Current Cursor */
    int       (*before_create)();        /* Pointer to Initializer */
    int       (*after_create)();
    int	      (*before_resize)();        /* Pointer to Configurator */
    int	      (*after_resize)(); 
    int       (*event_func)();           /* Pointer to Event Handler */
    MyEvent   *event_head;
    struct _MyWindow *parent;      /* pointer to parent window */
    struct _MyWindow *next;	/* pointer of next window */
  } MyWindow;

typedef struct
  {
    int left_x;  /* x coordinate of upper left corner */
    int top_y;   /* y coordinate of upper left corner */
    int right_x; /* x coordinate of lower right corner */
    int bottom_y;/* y coordinate of lower right corner */
  } DisplayArea;

#define CreateWindow(display, parent_win,child_win) { \
		if( parent_win && child_win ) { \
			if( (child_win)->before_create ) \
			   ((child_win)->before_create)(parent_win,child_win); \
 			(child_win)->win = XCreateSimpleWindow( display \
						, (parent_win)->win \
						, (child_win)->x \
						, (child_win)->y \
						, (child_win)->width \
						, (child_win)->height \
						, (child_win)->border \
						, Black_Pixel \
						, White_Pixel ); \
			XLOCK;					\
			XFlush( display ); 	\
			XUNLOCK;		\
			(child_win)->parent = parent_win; \
			if( (child_win)->after_create ) \
			    ((child_win)->after_create)(parent_win,child_win);\
		} \
}

#define ResizeWindow(display,window) { 				\
		if( window ) {									\
			if( (window)->before_resize ) 				\
				((window)->before_resize)( window ); 	\
			XLOCK;\
			XMoveResizeWindow( (display) 					\
					, (window)->win 					\
					, (window)->x 						\
					, (window)->y 						\
					, (window)->width 					\
					, (window)->height ); 				\
			XFlush( display ); 							\
			XUNLOCK;									\
			if( (window)->after_resize ) 				\
				((window)->after_resize)( window ); 	\
		} \
}

#define DefineCursor(display, window,mycursor) { \
		XLOCK;													\
		XDefineCursor( display, (window)->win, *(mycursor) ); \
		XFlush( display ); 	 \
		XUNLOCK;										\
		(window)->cursor = mycursor; \
}

#endif /* MYWINDOW_H */
