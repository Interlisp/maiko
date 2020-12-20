/* $Id: asmbbt.c,v 1.3 1999/05/31 23:35:23 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*			File:	asmbbt.c				*/
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
#include "lspglob.h"
#include "lispmap.h"
#include "lsptypes.h"
#include "emlglob.h"
#include "adr68k.h"
#include "address.h"
#include "arith.h"
#include "stack.h"
#include "cell.h"
#include "gcdata.h"

#include "bb.h"
#include "bitblt.h"
#include "pilotbbt.h"

void bitblt(DLword *srcbase, DLword *dstbase, int sx, int dx, int w, int h, int srcbpl, int dstbpl,
            int backwardflg, int src_comp, int op, int gray, int num_gray, int curr_gray_line) {
  new_bitblt_code;
}
