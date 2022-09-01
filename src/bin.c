/* $Id: bin.c,v 1.3 1999/05/31 23:35:24 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

/***********************************************************************/
/*
                File Name :	bin.c

                Desc	:

                                Date :		Jul. 22, 1987
                                Edited by :	Takeshi Shimizu
                                Changed :

                Including :	OP_bin


*/
/**********************************************************************/
#include "version.h"
#include "adr68k.h"    // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#include "bindefs.h"   // for N_OP_bin
#include "emlglob.h"
#include "lispmap.h"   // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"  // for state, ERROR_EXIT, GetTypeNumber, Get_BYTE
#include "stream.h"    // for Stream

LispPTR N_OP_bin(LispPTR tos) {
  Stream *stream68k; /* stream instance on TOS */
  char *buff68k;     /* pointer to BUFF */

  if (GetTypeNumber(tos) == TYPE_STREAM) {
    stream68k = (Stream *)NativeAligned4FromLAddr(tos);

    if (!stream68k->BINABLE) ERROR_EXIT(tos);

    if (stream68k->COFFSET >= stream68k->CBUFSIZE) ERROR_EXIT(tos);

    /* get BUFFER instance */
    buff68k = (char *)NativeAligned2FromLAddr(stream68k->CBUFPTR);

    /* get BYTE data and set it to TOS */
    return (S_POSITIVE | (Get_BYTE(buff68k + (stream68k->COFFSET)++)));
  } else
    ERROR_EXIT(tos);
}
