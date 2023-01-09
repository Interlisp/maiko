/* $Id: ubf1.c,v 1.3 1999/05/31 23:35:44 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include "adr68k.h"      // for LAddrFromNative
#include "arith.h"       // for N_ARITH_SWITCH
#include "lispemul.h"    // for state, ERROR_EXIT, DLword, LispPTR
#include "lspglob.h"
#include "lsptypes.h"    // for TYPE_FLOATP
#include "mkcelldefs.h"  // for createcell68k
#include "my.h"          // for N_MakeFloat
#include "ubf1defs.h"    // for N_OP_ubfloat1

/************************************************************
        OP_ubfloat1  -- op 355  ==  UBFLOAT1
355/0	BOX
355/1	UNBOX
355/2	ABS
355/3	NEGATE
355/4	UFIX
***********************************************************/

LispPTR N_OP_ubfloat1(LispPTR arg, int alpha) {
  switch (alpha) {
    case 0: /* box */
    {
      LispPTR *wordp;
      wordp = createcell68k(TYPE_FLOATP);
      *wordp = arg;
      return (LAddrFromNative(wordp));
    }
    case 1: /* unbox */
    {
      float dest;
      LispPTR ret;
      N_MakeFloat(arg, dest, arg);
      ret = *(LispPTR *)&dest;
      return (ret);
    }
    case 2: /* abs */ return (0x7FFFFFFF & arg);
    case 3: /* neg */ return (0x80000000 ^ arg);
    case 4: /* ufix */
    {
      float temp;
      int val;
      temp = *(float *)&arg;
      if ((temp > ((float)0x7fffffff)) || (temp < ((float)0x80000000))) ERROR_EXIT(arg);
      val = (int)temp;
      N_ARITH_SWITCH(val);
    }
    default: ERROR_EXIT(arg);
  } /* end switch */
} /* end N_OP_ubfloat1() */

/* end module */
