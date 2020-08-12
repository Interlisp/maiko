/* $Id: call-c.c,v 1.2 1999/01/03 02:06:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: call-c.c,v 1.2 1999/01/03 02:06:48 sybalsky Exp $ Copyright (C) Venue";

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*    F O R E I G N - F U N C T I O N   C A L L   I N T E R F A C E	*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#include "lispemul.h"

lispPTR call_c_fn(args);
{
  void() *fn = args[0];
  (*fn)();
  return (NIL);
}
