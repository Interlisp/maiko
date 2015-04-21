/* @(#) my.h Version 2.15 (2/8/93). copyright venue   */

/************************************************************************/
/*									*/
/*				M Y . H					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

#define S_CHARACTER 0x70000
#ifdef BIGVM
#define IsNumber(address)     ((GETWORD(MDStypetbl +(((address) & 0x0fffff00)>>9))) & 0x1000)
#else
#define IsNumber(address)     ((GETWORD(MDStypetbl +(((address) & 0x0ffff00)>>9))) & 0x1000)
#endif


/************************************************************************/
/*									*/
/*			N _ M a k e F l o a t				*/
/*									*/
/*	Get a numeric argument as a flowting-point number, converting	*/
/*	from SMALLP or FIXP, if necessary.				*/
/*									*/
/************************************************************************/
#ifndef I386
#define	N_MakeFloat(arg, dest, tos){					\
	switch (SEGMASK & (LispPTR)arg) {				\
	case S_POSITIVE:						\
		dest = (float)(0xFFFF & (LispPTR)arg);			\
		break;							\
	case S_NEGATIVE:						\
		dest = (float)((int)(0xFFFF0000 | (LispPTR)arg));		\
		break;							\
	default:							\
		switch (GetTypeNumber(arg)) {				\
		  case TYPE_FLOATP: 					\
		    dest = *((float *)Addr68k_from_LADDR(arg));		\
		    break; 						\
		  case TYPE_FIXP: 					\
		    dest = (float)(*((int *)Addr68k_from_LADDR(arg)));	\
		    break;						\
		  default: ERROR_EXIT(tos);				\
		}							\
	}								\
}
#else
#define	N_MakeFloat(arg, dest, tos){		I386Reset;		\
	switch (SEGMASK & (int)arg) {				\
	case S_POSITIVE:						\
		dest = (float)(0xFFFF & (int)arg);			\
		break;							\
	case S_NEGATIVE:						\
		dest = (float)((int)(0xFFFF0000 | (int)arg));		\
		break;							\
	default:							\
		switch (GetTypeNumber(arg)) {				\
		  case TYPE_FLOATP: 	I386Reset;				\
		    dest = *((float *)Addr68k_from_LADDR(arg));		\
		    break; 						\
		  case TYPE_FIXP: 					\
		    dest = (float)(*((int *)Addr68k_from_LADDR(arg)));	\
		    break;						\
		  default: ERROR_EXIT(tos);				\
		}							\
	}								\
}
#endif /* I386 */



#define	N_GetPos(arg, dest, tos){					\
	if ((arg & SEGMASK) == S_POSITIVE)				\
		dest = arg & 0xFFFF;					\
	else	{							\
	if (GetTypeNumber(arg) != TYPE_FIXP) ERROR_EXIT(tos);		\
	if ((dest = *((int *)Addr68k_from_LADDR(arg))) & 0x80000000)	\
		ERROR_EXIT(tos);					\
		}							\
	}

#ifdef OS4
#define aref_switch(type, tos, baseL, index)				\
{									\
  LispPTR result;							\
  DLword *wordp;							\
									\
    switch (type) {							\
      case 38: /* pointer : 32 bits */					\
         return(*(((int *)Addr68k_from_LADDR(baseL)) + index));		\
      case 20: /* signed : 16 bits */					\
         result = (GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF;										\
         if (result & 0x8000) return(result | S_NEGATIVE);		\
         else return(result | S_POSITIVE);				\
      case 67: /* Character :  8 bits */				\
         return(S_CHARACTER | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF));								\
      case 22: /* signed : 32 bits */					\
         result = *(((int *)Addr68k_from_LADDR(baseL)) + index);	\
         N_ARITH_SWITCH(result);					\
      case 0: /* unsigned : 1 bit per element */			\
         return(S_POSITIVE | (((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + (index >> 3))) >> (7 - (index & 7))) & 1));					\
      case 3: /* unsigned : 8 bits per element */			\
         return(S_POSITIVE | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF));								\
      case 4: /* unsigned : 16 bits per element */			\
         return(S_POSITIVE | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF));								\
      case 54: /* Float : 32 bits */					\
         wordp = createcell68k(TYPE_FLOATP);				\
         *((int *)wordp) = *(((int *)Addr68k_from_LADDR(baseL)) + index);										\
         return(LADDR_from_68k(wordp));					\
      case 68: /* Character :  16 bits */				\
         return(S_CHARACTER | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF));								\
      case 86: /* XPointer : 32 bits */					\
         return(*(((int *)Addr68k_from_LADDR(baseL)) + index));		\
      default: /* Illegal or Unimplemented */				\
         ERROR_EXIT(tos);						\
    }/* end switch typenumber */					\
}
#else
static LispPTR __inline__
aref_switch(int type, LispPTR tos, LispPTR baseL, int index)
{								  
  LispPTR result;
  DLword *wordp;

  switch (type)
    {
    case 38: /* pointer : 32 bits */
      return(*(((LispPTR *)Addr68k_from_LADDR(baseL)) + index));		
    case 20: /* signed : 16 bits */					
      result = (GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF;
      if (result & 0x8000) return(result | S_NEGATIVE);
      else return(result | S_POSITIVE);				
    case 67: /* Character :  8 bits */				
      return(S_CHARACTER | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF));								
    case 22: /* signed : 32 bits */					
      result = *(((LispPTR *)Addr68k_from_LADDR(baseL)) + index);	
      N_ARITH_SWITCH(result);					
    case 0: /* unsigned : 1 bit per element */			
      return(S_POSITIVE | (((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + (index >> 3))) >> (7 - (index & 7))) & 1));					
    case 3: /* unsigned : 8 bits per element */			
      return(S_POSITIVE | ((GETBYTE(((char *)Addr68k_from_LADDR(baseL)) + index)) & 0xFF));								
    case 4: /* unsigned : 16 bits per element */			
      return(S_POSITIVE | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF));								
    case 54: /* Float : 32 bits */					
      wordp = createcell68k(TYPE_FLOATP);				
      *((LispPTR *)wordp) = *(((LispPTR *)Addr68k_from_LADDR(baseL)) + index);										
      return(LADDR_from_68k(wordp));					
    case 68: /* Character :  16 bits */				
      return(S_CHARACTER | ((GETWORD(((DLword *)Addr68k_from_LADDR(baseL)) + index)) & 0xFFFF));								
    case 86: /* XPointer : 32 bits */					
      return(*(((LispPTR *)Addr68k_from_LADDR(baseL)) + index));		
    default: /* Illegal or Unimplemented */				
      ERROR_EXIT(tos);						
    }/* end switch typenumber */					
}
#endif /* NEVER */

#define aset_switch(type, tos)						\
{									\
   switch (type) {							\
      case 38: /* pointer : 32 bits */					\
	GCLOOKUP(*(((int *)Addr68k_from_LADDR(base)) + index), DELREF); \
	GCLOOKUP(data, ADDREF);						\
        *(((int *)Addr68k_from_LADDR(base)) + index) = data;		\
        return(data);							\
      case 20: /* signed : 16 bits */					\
        new = data & 0xFFFF;						\
        if ((((data & SEGMASK) == S_POSITIVE) && ((data & 0x8000) == 0)) ||\
	    (((data & SEGMASK) == S_NEGATIVE) && (data & 0x8000)))	\
        GETWORD(((DLword *)Addr68k_from_LADDR(base)) + index) = new;		\
        else ERROR_EXIT(tos);						\
        return(data);							\
      case 67: /* Character :  8 bits */				\
        if ((data & SEGMASK) != S_CHARACTER) ERROR_EXIT(tos);	\
        new = data & 0xFFFF;						\
        if (new > 0xFF) ERROR_EXIT(tos);				\
        GETBYTE(((char *)Addr68k_from_LADDR(base)) + index) = new;		\
        return(data);							\
      case 22: /* signed : 32 bits */					\
        N_GETNUMBER(data, new, doufn);					\
        *(((int *)Addr68k_from_LADDR(base)) + index) = new;		\
        return(data);							\
      case 0: /* unsigned : 1 bit per element */			\
        N_GetPos(data, new, tos);					\
        if (new > 1) ERROR_EXIT(tos);					\
        if (new) {							\
          new = (1 << (7 - (index & 7)));				\
          GETBYTE(((char *)Addr68k_from_LADDR(base)) + (index >> 3)) |= new;	\
        }								\
        else {								\
          new = 0xFF - (1 << (7 - (index & 7)));			\
          GETBYTE(((char *)Addr68k_from_LADDR(base)) + (index >> 3)) &= new;	\
        }								\
        return(data);							\
      case 3: /* unsigned : 8 bits per element */			\
        N_GetPos(data, new, tos);					\
        if (new > 0xFF) ERROR_EXIT(tos);				\
        GETBYTE(((char *)Addr68k_from_LADDR(base)) + index) = new;		\
        return(data);							\
      case 4: /* unsigned : 16 bits per element */			\
        N_GetPos(data, new, tos); 					\
        if (new > 0xFFFF) ERROR_EXIT(tos);				\
        GETWORD(((DLword *)Addr68k_from_LADDR(base)) + index) = new;		\
        return(data);							\
      case 54: /* Float : 32 bits */					\
        if (GetTypeNumber(data) != TYPE_FLOATP) ERROR_EXIT(tos);	\
        *(((int *)Addr68k_from_LADDR(base)) + index) = *((int *)Addr68k_from_LADDR(data));								\
        return(data);							\
      case 68: /* Character :  16 bits */				\
        if ((data & SEGMASK) != S_CHARACTER) ERROR_EXIT(tos); 	\
        new = data & 0xFFFF;						\
        GETWORD(((DLword *)Addr68k_from_LADDR(base)) + index) = new;		\
        return(data);							\
      case 86: /* XPointer : 32 bits */					\
        *(((int *)Addr68k_from_LADDR(base)) + index) = data;		\
        return(data);							\
      default: /* Illegal or Unimplemented */				\
        ERROR_EXIT(tos);						\
    }/* end switch typenumber */					\
}
