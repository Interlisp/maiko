/* $Id: rawrs232c.h,v 1.2 1999/01/03 02:06:22 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */



/************************************************************************/
/*									*/
/*	(C) Copyright 1989-96 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

typedef struct raw232cparam {
LispPTR BauRate;
LispPTR BitsPerChar;
LispPTR Parity;
LispPTR LocalLine;
LispPTR NoOfStopBits;
LispPTR FlowCnt;
LispPTR InEOL;
LispPTR OutEOL;
LispPTR InputMaxBell;
LispPTR Canon;
LispPTR Echo;
LispPTR ModemStatusLine;
LispPTR RTSCTSCnt;
} RawRSParam;


#define RAW_RS_NONE 	1
#define RAW_RS_ODD 	2
#define RAW_RS_EVEN 	3
#define RAW_RS_XON 	2
#define RAW_RS_CR 	1
#define RAW_RS_LF 	2
#define RAW_RS_CRLF 	3
 
