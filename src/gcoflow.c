/* $Id: gcoflow.c,v 1.3 1999/05/31 23:35:32 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */
static char *id = "$Id: gcoflow.c,v 1.3 1999/05/31 23:35:32 sybalsky Exp $ Copyright (C) Venue";



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
/*************************************************************************/
/*                                                                       */
/*                       File Name : gcpunt.c                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*                      Creation Date : July-8-1987                      */
/*                      Written by Tomoru Teruuchi                       */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*           Functions : gc_handleoverflow(arg);                         */
/*                       gcmaptable(arg);                                */
/*                                                                       */
/*************************************************************************/
/*           Descreption :                                               */
/*                                                                       */
/*************************************************************************/
/*                                                               \Tomtom */
/*************************************************************************/


#include "version.h"


#include "lispemul.h"
#include "lsptypes.h"
#include "address.h"
#include "adr68k.h"
#include "lspglob.h"
#include "gc.h"



#define MAXSMALLP		65535
#define HTBIGENTRYSIZE		4
#define WORDSPERPAGE		256
#define MAXTYPENUMBER		INIT_TYPENUM
#define Oddp(num) (((num % 2) != 0)?1:0)
#define Evenp(num,prim) (((num % prim) == 0)?1:0)
#define Increment_Allocation_Count(n)			\
    if (*Reclaim_cnt_word != NIL) 			\
		if (*Reclaim_cnt_word > n)		\
			(*Reclaim_cnt_word) -= n;	\
		else { *Reclaim_cnt_word = NIL; 	\
			  doreclaim();  		\
			};				\


DLword gc_handleoverflow(DLword arg)
{ struct htoverflow  *cell;
     struct dtd	        *ptr;
     LispPTR		cellcnt;
     LispPTR            addr;
	  cell = (struct htoverflow *)HToverflow;
			/* This proc. protected from interrupt */
	while((addr = cell->ptr) != NIL)
	   {
		REC_GCLOOKUP(addr, cell->pcase);
		cell->ptr = 0;
		cell->pcase = 0;
		++cell; /* (\ADDBASE CELL WORDSPERCELL) */
	   };
	ptr = (struct dtd *)GetDTD(TYPE_LISTP);
		/* same as "extern struct dtd *ListpDTD" */
	if ((cellcnt = ptr->dtd_cnt0) > 1024)
	   { Increment_Allocation_Count(cellcnt);
		ptr->dtd_oldcnt += cellcnt;
		ptr->dtd_cnt0 = 0;
	   };
	return(arg);
   }

DLword gcmaptable(DLword arg)
{ struct htoverflow	*cell;
	struct dtd	*ptr;
	LispPTR		cellcnt;
	int		typnum;
	LispPTR		addr;			
	cell = (struct htoverflow *)HToverflow;	
			/* This proc. protected from interrupt */
	while((addr = cell->ptr) != NIL)
	   {
		REC_GCLOOKUP(addr, cell->pcase);
		cell->ptr = 0;
		cell->pcase = 0;
		++cell; /* (\ADDBASE CELL WORDSPERCELL) */
	   };
	for(typnum = 1; typnum <= *MaxTypeNumber_word; ++typnum)
	   /* applied alltype */
	   { ptr = (struct dtd *)GetDTD(typnum);
		if ((cellcnt = ptr->dtd_cnt0) != 0)
		   { ptr->dtd_oldcnt += cellcnt;
		     ptr->dtd_cnt0 = 0;
		     Increment_Allocation_Count(cellcnt);
		   };
	   };
	return(arg);
   }




