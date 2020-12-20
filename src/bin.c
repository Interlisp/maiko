/* $Id: bin.c,v 1.3 1999/05/31 23:35:24 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

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

#include "lispemul.h"
#include "lispmap.h"
#include "emlglob.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "cell.h"
#include "stream.h"

#include "bindefs.h"

LispPTR N_OP_bin(register int tos) {
  register Stream *stream68k; /* stream instance on TOS */
  register char *buff68k;     /* pointer to BUFF */

  if (GetTypeNumber(tos) == TYPE_STREAM) {
    stream68k = (Stream *)Addr68k_from_LADDR(tos);

    if (!stream68k->BINABLE) ERROR_EXIT(tos);

    if (stream68k->COFFSET >= stream68k->CBUFSIZE) ERROR_EXIT(tos);

    /* get BUFFER instance */
    buff68k = (char *)Addr68k_from_LADDR(stream68k->CBUFPTR);

    /* get BYTE data and set it to TOS */
    return (S_POSITIVE | (Get_BYTE(buff68k + (stream68k->COFFSET)++)));
  } else
    ERROR_EXIT(tos);
}
