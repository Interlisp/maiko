/* $Id: XVersion.h,v 1.2 1999/01/03 02:05:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */






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



#ifdef XWINDOW
#ifdef XV11R4
#undef XV11R1
#undef XV11R2
#undef XV11R3
#endif /* XV11R4 */


#ifdef XV11R3
#undef XV11R1
#undef XV11R2
#undef XV11R4
#endif /* XV11R3 */


#ifdef XV11R2
#undef XV11R1
#undef XV11R3
#undef XV11R4
#endif /* XV11R2 */


#ifdef XV11R1
#undef XV11R2
#undef XV11R3
#undef XV11R4
#endif /* XV11R1 */


#if ( !(defined( XV11R1 ))  \
   && !(defined( XV11R2 ))  \
   && !(defined( XV11R3 ))  \
   && !(defined( XV11R4 )) )
#define XV11R4			/* newest version */
#endif
#endif /* XWINDOW */



