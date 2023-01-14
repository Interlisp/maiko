#ifndef LISPMAP_H
#define LISPMAP_H 1

/* $Id: lispmap.h,v 1.3 1999/01/03 02:06:08 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/



/*
		File Name :	lispmap.h(for TEST)

		**************NOTE*****************
		OLD DEFs are MOVED to lispmap.FULL
		**************NOTE*****************

		Global variables for LispSYSOUT

					Date :		December 18, 1986
					Edited by :	Takeshi Shimizu

*/




 /* Whole Lisp size */
#define LWORLD_SIZE		0x420000	/* byte */

/* 1 MDS entry size is 2(page) * 512 Byte */
#define MDSTT_SIZE		(LWORLD_SIZE >> 10 )

/* if you want to use the ATOMSPACE for Dummy then it must be 0x10000 take */
#define MAP_SHIFT	0x0

/* Following constants mean LISP word offset. */
/* these correspond with lisp mem map */
/* for IOCBPAGE */
#define IOCBPAGE_OFFSET	    256
#define IOCB_SIZE	      1


#ifdef BIGBIGVM

/**********************************************/
/*                                            */
/*    BIG-BIG-VM sysout layout (256Mb sysout) */
/*                                            */
/**********************************************/

/* for ATOMSPACE */
#define ATOM_HI	      0
#define ATOM_OFFSET	0x00000
#define ATOM_SIZE	0x10000

/* for IOPAGE */
#define IOPAGE_OFFSET	0x0FF00
#define IOPAGE_SIZE	      1

/* for STACKSPACE */
#define STK_HI		      1
#define STK_OFFSET	0x10000
#define STK_SIZE	0x10000

/* for PLISTSPACE */
#define PLIS_HI			2 /* place holder, really -- keep the old value, even though it's inconsistent with the OFFSET, because it's known by LISP, and is used as a dispatch constant. */
#define PLIS_OFFSET 0x30000
#define PLIS_SIZE 0x10



#define FPTOVP_HI		4	/* again, inconsistent with OFFSET. */
#define FPTOVP_OFFSET 0x20000
#define FPTOVP_SIZE  0x100000


/*for PAGEMAP */
#define PAGEMAP_HI	      5         /* Again, fake */
#define PAGEMAP_OFFSET	0x50000
#define PAGEMAP_SIZE	0x10000

/* for InterfacePage */
#define IFPAGE_HI	      20
#define IFPAGE_OFFSET	0x140000
#define IFPAGE_SIZE	  0x200

/* for PageMapTBL */
#define PAGEMAPTBL_OFFSET 0x140200
#define PAGEMAPTBL_SIZE	    0x800

/* for MISCSTATS */
#define MISCSTATS_OFFSET  0x140A00
#define MISCSTATS_SIZE	    0x200

/* for UFNTable */
#define UFNTBL_OFFSET	0x140C00
#define UFNTBL_SIZE	  0x200

/* for DTDspace */
#define DTD_HI		      20
#define DTD_OFFSET	0x141000
#define DTD_SIZE	 0x1000

/* for LOCKEDPAGETBL */
#define LOCKEDPAGETBL_OFFSET 0x147000
#define LOCKEDPAGETBL_SIZE 0x1000

/* for MDSTT */

#define MDS_HI		      24
#define MDS_OFFSET	0x180000
#define MDS_SIZE	 0x40000


/* for AtomHashTable */
#define ATMHT_HI	     21
#define ATMHT_OFFSET	0x150000
#define ATMHT_SIZE	0x10000

/* for PNPSPACE */
/* Now used to hold initial atoms */
#define PNP_HI		      8        /* Fake */
#define PNP_OFFSET	0x80000
#define PNP_SIZE	0x20000

#define ATOMS_HI		     44
#define ATOMS_OFFSET    0x2c0000
#define ATOMS_SIZE       0x20000

/* for DEFSPACE */
#define DEFS_HI		     10        /* Fake */
#define DEFS_OFFSET	0xA0000
#define DEFS_SIZE	0x20000

/* for VALSPACE */
#define VALS_HI		     12        /* Fake */
#define VALS_OFFSET	0xC0000
#define VALS_SIZE	0x20000

/* for Small Positive */
#define SPOS_HI		     14        /* Fake */
#define S_POSITIVE	0xE0000
#define SPOS_SIZE	0x10000

/* for Small Negative */
#define SNEG_HI		     15        /* Fake */
#define S_NEGATIVE	0xF0000
#define SNEG_SIZE	0x10000

/* for characters */
#define S_CHARACTER	0x70000

/* for HTMAIN */
#define HTMAIN_HI	     22
#define HTMAIN_OFFSET  0x160000
#define HTMAIN_SIZE	 0x10000

/* for HTOVERFLOW */
#define HTOVERFLOW_OFFSET 0x170000
#define HTOVERFLOW_SIZE	     0x100

/* for HTBIGCOUNT */
#define HTBIG_HI	     23
#define HTBIG_OFFSET   0x170100
#define HTBIG_SIZE       0x8000

/* for HTCOLL */
#define HTCOLL_HI	     28
#define HTCOLL_OFFSET  0x1C0000
#define HTCOLL_SIZE	0x100000


/* DISPLAYREGION */

#define DISPLAY_HI	     18
#define DISPLAY_OFFSET 0x120000



#define ARRAY_OFFSET		0x2e0000


/* #define MDS_BOTTOM_OFFSET	0x200000  NOT USED ANYWHERE 1/29/98 JDS */

#else

/* NOT BIG-BIG VM */


/* for ATOMSPACE */
#define ATOM_HI	      0
#define ATOM_OFFSET	0x00000
#define ATOM_SIZE	0x10000

/* for IOPAGE */
#define IOPAGE_OFFSET	0x0FF00
#define IOPAGE_SIZE	      1

/* for STACKSPACE */
#define STK_HI		      1
#define STK_OFFSET	0x10000
#define STK_SIZE	0x10000

/* for PLISTSPACE */
#ifndef BIGVM
#define PLIS_HI		      2
#define PLIS_OFFSET	0x20000
#define PLIS_SIZE	0x20000
#else
#define PLIS_HI			2 /* place holder, really -- keep the old value, even though it's inconsistent with the OFFSET, because it's known by LISP, and is used as a dispatch constant. */
#define PLIS_OFFSET 0x30000
#define PLIS_SIZE 0x10
#endif

#ifdef BIGVM
#define FPTOVP_HI		4	/* again, inconsistent with OFFSET. */
#define FPTOVP_OFFSET 0x20000
#define FPTOVP_SIZE 0x40000
#else
/* for FPTOVP */
#define FPTOVP_HI	      4
#define FPTOVP_OFFSET	0x40000
#define FPTOVP_SIZE	0x10000
#endif /* BIGVM */

/*for PAGEMAP */
#define PAGEMAP_HI	      5
#define PAGEMAP_OFFSET	0x50000
#define PAGEMAP_SIZE	0x10000

/* for InterfacePage */
#define IFPAGE_HI	      6
#define IFPAGE_OFFSET	0x60000
#define IFPAGE_SIZE	  0x200

/* for PageMapTBL */
#define PAGEMAPTBL_OFFSET 0x60200
#define PAGEMAPTBL_SIZE	    0x800

/* for MISCSTATS */
#define MISCSTATS_OFFSET  0x60A00
#define MISCSTATS_SIZE	    0x200

/* for UFNTable */
#define UFNTBL_OFFSET	0x60C00
#define UFNTBL_SIZE	  0x200

/* for DTDspace */
#define DTD_HI		      6
#define DTD_OFFSET	0x61000
#define DTD_SIZE	 0x1000

/* for LOCKEDPAGETBL */
#define LOCKEDPAGETBL_OFFSET 0x67000
#define LOCKEDPAGETBL_SIZE 0x1000

/* for MDSTT */
#ifdef BIGVM
    /* In BIGVM, MDS type table is at 19.,,0 for 1 segment */
#define MDS_HI		      20
#define MDS_OFFSET	0x140000
#define MDS_SIZE	 0x10000
#else
#define MDS_HI		      6
#define MDS_OFFSET	0x68000
#define MDS_SIZE	 0x8000
#endif /* BIGVM */

/* for AtomHashTable */
#define ATMHT_HI	      7
#define ATMHT_OFFSET	0x70000
#define ATMHT_SIZE	0x10000

/* for PNPSPACE */
#define PNP_HI		      8
#define PNP_OFFSET	0x80000
#define PNP_SIZE	0x20000

#define ATOMS_HI	      8
#define ATOMS_OFFSET    0x80000
#define ATOMS_SIZE      0x20000

/* for DEFSPACE */
#define DEFS_HI		     10
#define DEFS_OFFSET	0xA0000
#define DEFS_SIZE	0x20000

/* for VALSPACE */
#define VALS_HI		     12
#define VALS_OFFSET	0xC0000
#define VALS_SIZE	0x20000

/* for Small Positive */
#define SPOS_HI		     14
#define S_POSITIVE	0xE0000
#define SPOS_SIZE	0x10000

/* for Small Negative */
#define SNEG_HI		     15
#define S_NEGATIVE	0xF0000
#define SNEG_SIZE	0x10000

/* for characters */
#define S_CHARACTER	0x70000

#ifdef BIGVM
/* for HTMAIN */
#define HTMAIN_HI	     16
#define HTMAIN_OFFSET  0x100000
#define HTMAIN_SIZE	 0x10000

/* for HTOVERFLOW */
#define HTOVERFLOW_OFFSET 0x110000
#define HTOVERFLOW_SIZE	     0x100

/* for HTBIGCOUNT */
#define HTBIG_HI	     16
#define HTBIG_OFFSET   0x110100
#define HTBIG_SIZE       0x8000

/* for HTCOLL */
#define HTCOLL_HI	     10
#define HTCOLL_OFFSET  0xA0000
#define HTCOLL_SIZE	0x40000
#else
/* for HTMAIN */
#define HTMAIN_HI	     16
#define HTMAIN_OFFSET  0x100000
#define HTMAIN_SIZE	 0x8000

/* for HTOVERFLOW */
#define HTOVERFLOW_OFFSET 0x108000
#define HTOVERFLOW_SIZE	     0x100

/* for HTBIGCOUNT */
#define HTBIG_HI	     16
#define HTBIG_OFFSET   0x108100
#define HTBIG_SIZE       0x8000

/* for HTCOLL */
#define HTCOLL_HI	     17
#define HTCOLL_OFFSET  0x110000
#define HTCOLL_SIZE	0x10000
#endif /* BIGVM */


/* DISPLAYREGION */

#define DISPLAY_HI	     18
#define DISPLAY_OFFSET 0x120000


#ifdef MEDLEY
/* for ARRAYSPACE & MDS  for PROT-LISP */
#define ARRAY_OFFSET		0x150000
#elif defined(BIGVM)
#define ARRAY_OFFSET		0x150000
#else
#define ARRAY_OFFSET		0x130000
#endif

/* #define MDS_BOTTOM_OFFSET	0x200000   NOT used anywhere JDS 1/29/98 */

#endif /* BIGBIGVM */



/* for PnCharSpace(use only PROT-LISP ) */
#define PNCHAR_HI	   0x20
#define PNCHAR_OFFSET  0x200000
#define PNCHAR_SIZE	0x10000


/***** SEG definitions for AtomCellN *****/
/* following defs correspond with D machine memory layout */
/**** NOTE!!  if D's layout changes, modify following defs */
#define D_PLISHI	2
#define D_PNHI		010
#define D_DEFSHI	012
#define D_VALSHI	014

#endif
