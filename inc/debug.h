#ifndef DEBUG_H
#define DEBUG_H 1
/* $Id: debug.h,v 1.2 1999/01/03 02:05:56 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */

#ifndef BYTESWAP
	/************************************************/
	/*   Normal byte-order version of definitions	*/
	/************************************************/

typedef struct{
	DLword	W0;
	DLword	W1;
	DLword	W2;
	DLword	W3;
	DLword	WU;
	DLword	W4;
	DLword	W5;
	int		TIME;
	unsigned	MOUSESTATE : 3;
	unsigned	SHIFT1 : 1;
	unsigned	SHIFT2 : 1;
	unsigned	keLOCK : 1;
	unsigned	keCTRL : 1;
	unsigned	keMETA : 1;
	unsigned	keFONT : 1;
	unsigned	USERMODE1 : 1;
	unsigned	USERMODE2 : 1;
	unsigned	USERMODE3 : 1;
	unsigned	NIL1 : 4;
	DLword	MOUSEX;
	DLword	MOUSEY;
} KEYBOARDEVENT;

typedef struct{
	DLword	READ;
	DLword	WRITE;
} RING;

typedef struct{
	LispPTR	FLAGS;
	LispPTR	CODES;
	LispPTR	SHIFTCODES;
	LispPTR	ARMED;
	LispPTR	INTERRUPTLIST;
} KEYACTION;

typedef struct{
	unsigned int EVENTWAKEUPPENDING : 1;
	unsigned int nil1 : 7;
	unsigned int EVENTQUEUETAIL : 24;
	LispPTR EVENTNAME;
} EVENT;

typedef struct{
	unsigned int nil1 : 1;
	unsigned int MLOCKPERPROCESS : 1;
	unsigned int nil2 : 6;
	unsigned int MLOCKQUEUETAIL : 24;
	LispPTR	MLOCKOWNER;
	LispPTR	MLOCKNAME;
	LispPTR	MLOCKLINK;
} MONITORLOCK;

typedef struct{
	DLword  PROCFX0;
	DLword	PROCFX;
	unsigned int PROCSTATUS : 8;
	unsigned int PROCNAME : 24;
	unsigned int PROCPRIORITY : 8;
	unsigned int PROCQUEUE : 24;
	unsigned int nil1 : 8;
	unsigned int NEXTPROCHANDLE : 24;
	unsigned int PROCTIMERSET : 1;
	unsigned int PROCBEINGDELETED : 1;
	unsigned int PROCDELETED : 1;
	unsigned int PROCSYSTEMP : 1;
	unsigned int PROCNEVERSTARTED : 1;
	unsigned int nil2 : 3;
	unsigned int PROCWAKEUPTIMER : 24;
	LispPTR PROCTIMERLINK;
	LispPTR PROCTIMERBOX;
	LispPTR WAKEREASON;
	LispPTR PROCEVENTORLOCK;
	LispPTR PROCFORM;
	LispPTR RESTARTABLE;
	LispPTR PROCWINDOW;
	LispPTR PROCFINISHED;
	LispPTR PROCRESULT;
	LispPTR PROCFINISHEVENT;
	LispPTR PROCMAILBOX;
	LispPTR PROCDRIBBLEOUTPUT;
	LispPTR PROCINFOHOOK;
	LispPTR PROCTYPEAHEAD;
	LispPTR PROCREMOTEINFO;
	LispPTR PROCUSERDATA;
	LispPTR PROCEVENTLINK;
	LispPTR PROCAFTEREXIT;
	LispPTR PROCBEFOREEXIT;
	LispPTR PROCOWNEDLOCKS;
	LispPTR PROCEVAPPLYRESULT;
	LispPTR PROCTTYENTRYFN;
	LispPTR PROCEXITFN;
	LispPTR PROCHARDRESETINFO;
	LispPTR PROCRESTARTFORM;
	LispPTR PROCOLDTTYPROC;
	LispPTR nil3;
} PROCESS;

typedef struct{
	unsigned int PQPRIORITY : 8;
	unsigned int PQHIGHER : 24;
	LispPTR	PQLOWER;
	LispPTR	PQNEXT;
	LispPTR	PQLAST;
} PROCESSQUEUE;

typedef struct  fdev{
	unsigned	RESETABLE : 1;
	unsigned	RANDOMACCESSP : 1;
	unsigned	NODIRECTORIES : 1;
	unsigned	PAGEMAPPED : 1;
	unsigned	FDBINABLE : 1;
	unsigned	FDBOUTABLE : 1;
	unsigned	FDEXTENDABLE : 1;
	unsigned	BUFFERED : 1;
	unsigned	DEVICENAME : 24;
	unsigned	REMOTEP : 1;
	unsigned	SUBDIRECTORIES : 1;
	unsigned	INPUT_INDIRECTED : 1;
	unsigned	OUTPUT_INDIRECTED : 1;
	unsigned	NIL1 : 4;
	unsigned	DEVICEINFO : 24;
	LispPTR	OPENFILELST ;
	LispPTR	HOSTNAMEP ;
	LispPTR	EVENTFN ;
	LispPTR	DIRECTORYNAMEP ;
	LispPTR	OPENFILE ;
	LispPTR	CLOSEFILE ;
	LispPTR	REOPENFILE ;
	LispPTR	GETFILENAME ;
	LispPTR	DELETEFILE ;
	LispPTR	GENERATEFILES ;
	LispPTR	RENAMEFILE ;
	LispPTR	OPENP ;
	LispPTR	REGISTERFILE ;
	LispPTR	UNREGISTERFILE ;
	LispPTR	FREEPAGECOUNT ;
	LispPTR	MAKEDIRECTORY ;
	LispPTR	CHECKFILENAME ;
	LispPTR	HOSTALIVEP ;
	LispPTR	BREAKCONNECTION ;
	LispPTR	BIN ;
	LispPTR	BOUT ;
	LispPTR	PEEKBIN ;
	LispPTR	READCHAR ;
	LispPTR	WRITECHAR ;
	LispPTR	PEEKCHAR ;
	LispPTR	UNREADCHAR ;
	LispPTR	READP ;
	LispPTR	EOFP ;
	LispPTR	BLOCKIN ;
	LispPTR	BLOCKOUT ;
	LispPTR	FORCEOUTPUT ;
	LispPTR	GETFILEINFO ;
	LispPTR	SETFILEINFO ;
	LispPTR	CHARSETFN ;
	LispPTR	INPUTSTREAM ;
	LispPTR	OUTPUTSTREAM ;
	LispPTR	GETFILEPTR ;
	LispPTR	GETEOFPTR ;
	LispPTR	SETFILEPTR ;
	LispPTR	BACKFILEPTR ;
	LispPTR	SETEOFPTR ;
	LispPTR	LASTC ;
	LispPTR	GETNEXTBUFFER ;
	LispPTR	RELEASEBUFFER ;
	LispPTR	READPAGES ;
	LispPTR	WRITEPAGES ;
	LispPTR	TRUNCATEFILE ;
	LispPTR	WINDOWOPS ;
	LispPTR	WINDOWDATA ;
}FDEV;
typedef struct  package{
	LispPTR	INDEX ;
	LispPTR	TABLES ;
	LispPTR	NAME ;
	LispPTR	NAMESYMBOL ;
	LispPTR	NICKNAMES ;
	LispPTR	USE_LIST ;
	LispPTR	USED_BY_LIST ;
	LispPTR	EXTERNAL_ONLY ;
	LispPTR	INTERNAL_SYMBOLS ;
	LispPTR	EXTERNAL_SYMBOLS ;
	LispPTR	SHADOWING_SYMBOLS ;
} PACKAGE;

#ifdef NEVER
typedef struct  {
	unsigned nil1 : 8 ;
	unsigned BASE : 24 ;
	unsigned READ_ONLY_P : 1 ;
	unsigned nil2 : 1 ;
	unsigned BIT_P : 1 ;
	unsigned STRING_P : 1 ;
	unsigned nil3 : 1 ;
	unsigned DISPLACED_P : 1 ;
	unsigned FILL_POINTER_P : 1 ;
	unsigned EXTENDABLE_P : 1 ;
	unsigned TYPE_NUMBER : 8 ;
	DLword OFFSET;
	DLword FILL_POINTER;
	DLword TOTAL_SIZE;
} ONED_ARRAY;
#endif /* NEVER */
#else

	/****************************************************************/
	/*   Byte-swapped, word-swapped definitions, for e.g. 80386's	*/
	/****************************************************************/

typedef struct{
	DLword	W1;
	DLword	W0;
	DLword	W3;
	DLword	W2;
	DLword	W4;
	DLword	WU;
	/* only swapped down to here, and MOUSEX & Y -- there */
	/* looks like a missing word in the block at this point. */
	DLword	W5;
	int		TIME;
	unsigned	MOUSESTATE : 3;
	unsigned	SHIFT1 : 1;
	unsigned	SHIFT2 : 1;
	unsigned	LOCK : 1;
	unsigned	CTRL : 1;
	unsigned	META : 1;
	unsigned	FONT : 1;
	unsigned	USERMODE1 : 1;
	unsigned	USERMODE2 : 1;
	unsigned	USERMODE3 : 1;
	unsigned	NIL1 : 4;
	DLword	MOUSEY;
	DLword	MOUSEX;
} KEYBOARDEVENT;

typedef struct{
	DLword	WRITE;
	DLword	READ;
} RING;

typedef struct{
	LispPTR	FLAGS;
	LispPTR	CODES;
	LispPTR	SHIFTCODES;
	LispPTR	ARMED;
	LispPTR	INTERRUPTLIST;
} KEYACTION;

typedef struct{
	unsigned int EVENTQUEUETAIL : 24;
	unsigned int nil1 : 7;
	unsigned int EVENTWAKEUPPENDING : 1;
	LispPTR EVENTNAME;
} EVENT;

typedef struct{
	unsigned int MLOCKQUEUETAIL : 24;
	unsigned int nil2 : 6;
	unsigned int MLOCKPERPROCESS : 1;
	unsigned int nil1 : 1;
	LispPTR	MLOCKOWNER;
	LispPTR	MLOCKNAME;
	LispPTR	MLOCKLINK;
} MONITORLOCK;

typedef struct{
	DLword	PROCFX;
	DLword  PROCFX0;
	unsigned int PROCNAME : 24;
	unsigned int PROCSTATUS : 8;
	unsigned int PROCQUEUE : 24;
	unsigned int PROCPRIORITY : 8;
	unsigned int NEXTPROCHANDLE : 24;
	unsigned int nil1 : 8;
	unsigned int PROCWAKEUPTIMER : 24;
	unsigned int nil2 : 3;
	unsigned int PROCNEVERSTARTED : 1;
	unsigned int PROCSYSTEMP : 1;
	unsigned int PROCDELETED : 1;
	unsigned int PROCBEINGDELETED : 1;
	unsigned int PROCTIMERSET : 1;
	LispPTR PROCTIMERLINK;
	LispPTR PROCTIMERBOX;
	LispPTR WAKEREASON;
	LispPTR PROCEVENTORLOCK;
	LispPTR PROCFORM;
	LispPTR RESTARTABLE;
	LispPTR PROCWINDOW;
	LispPTR PROCFINISHED;
	LispPTR PROCRESULT;
	LispPTR PROCFINISHEVENT;
	LispPTR PROCMAILBOX;
	LispPTR PROCDRIBBLEOUTPUT;
	LispPTR PROCINFOHOOK;
	LispPTR PROCTYPEAHEAD;
	LispPTR PROCREMOTEINFO;
	LispPTR PROCUSERDATA;
	LispPTR PROCEVENTLINK;
	LispPTR PROCAFTEREXIT;
	LispPTR PROCBEFOREEXIT;
	LispPTR PROCOWNEDLOCKS;
	LispPTR PROCEVAPPLYRESULT;
	LispPTR PROCTTYENTRYFN;
	LispPTR PROCEXITFN;
	LispPTR PROCHARDRESETINFO;
	LispPTR PROCRESTARTFORM;
	LispPTR PROCOLDTTYPROC;
	LispPTR nil3;
} PROCESS;

typedef struct{
	unsigned int PQHIGHER : 24;
	unsigned int PQPRIORITY : 8;
	LispPTR	PQLOWER;
	LispPTR	PQNEXT;
	LispPTR	PQLAST;
} PROCESSQUEUE;

typedef struct  fdev{
	unsigned	DEVICENAME : 24;
	unsigned	BUFFERED : 1;
	unsigned	FDEXTENDABLE : 1;
	unsigned	FDBOUTABLE : 1;
	unsigned	FDBINABLE : 1;
	unsigned	PAGEMAPPED : 1;
	unsigned	NODIRECTORIES : 1;
	unsigned	RANDOMACCESSP : 1;
	unsigned	RESETABLE : 1;
	unsigned	DEVICEINFO : 24;
	unsigned	NIL1 : 4;
	unsigned	OUTPUT_INDIRECTED : 1;
	unsigned	INPUT_INDIRECTED : 1;
	unsigned	SUBDIRECTORIES : 1;
	unsigned	REMOTEP : 1;
	LispPTR	OPENFILELST ;
	LispPTR	HOSTNAMEP ;
	LispPTR	EVENTFN ;
	LispPTR	DIRECTORYNAMEP ;
	LispPTR	OPENFILE ;
	LispPTR	CLOSEFILE ;
	LispPTR	REOPENFILE ;
	LispPTR	GETFILENAME ;
	LispPTR	DELETEFILE ;
	LispPTR	GENERATEFILES ;
	LispPTR	RENAMEFILE ;
	LispPTR	OPENP ;
	LispPTR	REGISTERFILE ;
	LispPTR	UNREGISTERFILE ;
	LispPTR	FREEPAGECOUNT ;
	LispPTR	MAKEDIRECTORY ;
	LispPTR	CHECKFILENAME ;
	LispPTR	HOSTALIVEP ;
	LispPTR	BREAKCONNECTION ;
	LispPTR	BIN ;
	LispPTR	BOUT ;
	LispPTR	PEEKBIN ;
	LispPTR	READCHAR ;
	LispPTR	WRITECHAR ;
	LispPTR	PEEKCHAR ;
	LispPTR	UNREADCHAR ;
	LispPTR	READP ;
	LispPTR	EOFP ;
	LispPTR	BLOCKIN ;
	LispPTR	BLOCKOUT ;
	LispPTR	FORCEOUTPUT ;
	LispPTR	GETFILEINFO ;
	LispPTR	SETFILEINFO ;
	LispPTR	CHARSETFN ;
	LispPTR	INPUTSTREAM ;
	LispPTR	OUTPUTSTREAM ;
	LispPTR	GETFILEPTR ;
	LispPTR	GETEOFPTR ;
	LispPTR	SETFILEPTR ;
	LispPTR	BACKFILEPTR ;
	LispPTR	SETEOFPTR ;
	LispPTR	LASTC ;
	LispPTR	GETNEXTBUFFER ;
	LispPTR	RELEASEBUFFER ;
	LispPTR	READPAGES ;
	LispPTR	WRITEPAGES ;
	LispPTR	TRUNCATEFILE ;
	LispPTR	WINDOWOPS ;
	LispPTR	WINDOWDATA ;
}FDEV;
typedef struct  package{
	LispPTR	INDEX ;
	LispPTR	TABLES ;
	LispPTR	NAME ;
	LispPTR	NAMESYMBOL ;
	LispPTR	NICKNAMES ;
	LispPTR	USE_LIST ;
	LispPTR	USED_BY_LIST ;
	LispPTR	EXTERNAL_ONLY ;
	LispPTR	INTERNAL_SYMBOLS ;
	LispPTR	EXTERNAL_SYMBOLS ;
	LispPTR	SHADOWING_SYMBOLS ;
} PACKAGE;
#ifdef NEVER
typedef struct  {
	unsigned BASE : 24 ;
	unsigned nil1 : 8 ;
	DLword OFFSET;
	unsigned TYPE_NUMBER : 8 ;
	unsigned EXTENDABLE_P : 1 ;
	unsigned FILL_POINTER_P : 1 ;
	unsigned DISPLACED_P : 1 ;
	unsigned nil3 : 1 ;
	unsigned STRING_P : 1 ;
	unsigned BIT_P : 1 ;
	unsigned nil2 : 1 ;
	unsigned READ_ONLY_P : 1 ;
	DLword TOTAL_SIZE;
	DLword FILL_POINTER;
} ONED_ARRAY;
#endif /* NEVER */

#endif /* BYTESWAP */

#endif /* DEBUG_H */
