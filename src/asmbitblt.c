/* $Id: asmbitblt.c,v 1.3 2001/12/24 01:08:58 sybalsky Exp $ (C) Copyright Venue, All Rights
 * Reserved  */
static char *id = "$Id: asmbitblt.c,v 1.3 2001/12/24 01:08:58 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*			File:	asmbitblt.c				*/
/*									*/
/*	Dummy C-function "bitblt", used to compile the bitblt code	*/
/*	for hand-optimization.						*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "lispemul.h"
#include "lispglobal.h"
#include "lispmap.h"
#include "lisptypes.h"
#include "emulglobal.h"
#include "address68k.h"
#include "address.h"
#include "arith.h"
#include "stack.h"
#include "cell.h"
#include "gcdata.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"

void bitblt(DLword *srcbase, DLword *dstbase, int sx, int dx, int w, int h, int srcbpl, int dstbpl,
            int backwardflg, int src_comp, int op, int gray, int num_gray, int curr_gray_line)

    DLword *srcbase,
    dstbase;
int sx, dx, w, h, srcbpl, dstbpl, backwardflg, src_comp, op, gray, num_gray, curr_gray_line;

{ new_bitblt_code; }
