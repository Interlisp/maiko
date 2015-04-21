/* $Id: ubf1.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: ubf1.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ Copyright (C) Venue";


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#include "version.h"




#include <stdio.h>
#include "lispemul.h"
#include "adr68k.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "arith.h"
#include "medleyfp.h"
#include "my.h"
/************************************************************
	OP_ubfloat1  -- op 355  ==  UBFLOAT1
355/0	BOX
355/1	UNBOX
355/2	ABS
355/3	NEGATE
355/4	UFIX
***********************************************************/

N_OP_ubfloat1(int arg, int alpha)
{
  switch (alpha) {
    case 0: /* box */
      {
      register DLword *wordp;
      DLword *createcell68k(unsigned int type);
      wordp = createcell68k(TYPE_FLOATP);
      *((int *)wordp) = arg;
      return(LADDR_from_68k(wordp));
      }
    case 1: /* unbox */
      {
      float dest;
      int ret;
      N_MakeFloat(arg, dest, arg);
      ret = *(int *)&dest;
      return(ret);
      }
    case 2: /* abs */
      return(0x7FFFFFFF & arg);
    case 3: /* neg */
      return(0x80000000 ^ arg);
    case 4: /* ufix */
      {
      register float temp;
      int val;
      temp = *(float *)&arg;
      if ( (temp > ((float) 0x7fffffff)) ||
           (temp < ((float) 0x80000000)) )
               ERROR_EXIT(arg);
#ifdef I386
      I386Reset;
#endif /* I386 */

      val = (int)temp;
#ifdef I386
      I386Round;
#endif
      N_ARITH_SWITCH(val);
      }
    default: ERROR_EXIT(arg);
  }/* end switch */
}/* end N_OP_ubfloat1() */

	/* end module */









