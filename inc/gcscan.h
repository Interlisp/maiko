/* $Id: gcscan.h,v 1.2 1999/01/03 02:06:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/



#define HTSTKBIT 		0x200		/* = 512 */
#define HTENDS 			((struct hashentry *) htlptr)
#define GetStkCnt(entry1)	(entry1 >> 9)

/* extern DLword *HTmain;  -- this is real method */


const inline extern DLword gcscan1 (probe)
register int 	probe;				/* probe is offset */
  { register struct htlinkptr *htlptr;		/* overlay access method */ 
    register int contents;
    while (--probe >= 0)			/* End of HTmain Table ? */
	{ 
					/* Start addr. of scanning */	
   	  htlptr = (struct htlinkptr *)(HTmain+probe);
	  contents = ((struct htlinkptr *)WORDPTR(htlptr))->contents;
	  if (contents && (((struct hashentry *)WORDPTR(HTENDS))->collision || (GetStkCnt(contents) == 0)))
		 return(probe);
	}
    return(NIL);
   }

const inline extern DLword gcscan2 (probe)
register int 	probe;				/* probe is offset */
   {register struct htlinkptr *htlptr;		/* overlay access method */ 
	while (--probe >= 0)			/* End of HTmain Table ? */
	 { htlptr = (struct htlinkptr *)(HTmain+probe);
						/* Start addr. of scanning */
	   if (((HTSTKBIT | 1) & ((struct htlinkptr *)WORDPTR(htlptr))->contents) != 0)
			return(probe);		/* stackref or collision ON */
	 }
	return(NIL);
   }
