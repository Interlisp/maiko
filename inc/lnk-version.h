/* $Id: lnk-version.h,v 1.2 1999/01/03 02:06:12 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*			V E R S I O N . H			*/
/*									*/
/*  Version control:  Set the two values that keep sysouts and emul-    */
/*  ators in sync:  The LVERSION and MINBVERSION fields in the IFPAGE   */
/*									*/
/*  MINBVERSION is the current emulator version, incremented with each  */
/*  modification.  This must be >= a sysout's ifpage.minbversion.       */
/*									*/
/*  LVERSION is the minimum lisp version that will run with this emu-   */
/*  lator.  This must be <= a sysouts's ifpage.lversion.                */
/*									*/
/*  The loadup process sets both of these values in the sysout.         */
/*									*/
/*									*/
/*									*/
/*		C O N F I G U R A T I O N / O P T I O N   C O N T R O L			*/
/*									*/
/*		Given a release specification, set flags for the features		*/
/*		that release has.  This lets us set one flag in the make-			*/
/*		file, rather than remembering all the options that must change.		*/
/*									*/
/*		-DRELEASE=115   Medley 1.15, small atoms			*/
/*		-DRELEASE=200   Medley 2.0 as released							*/
/*		-DRELEASE=201   Medley with DOS & European kbd support							*/
/*		-DRELEASE=210   Medley with big VM							*/
/*		-DRELEASE=300   Medley bigvm as released.							*/
/*									*/
/*									*/
/************************************************************************/


  /* The current values */

#define LVERSION 21000
#define MINBVERSION 21001


  /* But remember old values, if we can figure them out from ifdef's */

#if (RELEASE == 115)

#undef LVERSION
#undef MINBVERSION
#define LVERSION 15000
#define MINBVERSION 15000
#undef BIGATOMS
#define NOEUROKBD
#define NOFORN
#define NOVERSION

#elif (RELEASE == 200)

  /* Medley 2.0 as released */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 20000
#define MINBVERSION 20000

#define BIGATOMS
#define NOEUROKBD
#define NOVERSION

#elif (RELEASE == 201 )


  /* Medley 2.0 with EUROKBD modification */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 20100
#define MINBVERSION 20100

#define BIGATOMS
#undef NOEUROKBD
#define NOVERSION

#elif (RELEASE == 210)

  /* Medley 2.1, big-vm Medley while in beta-test */
#undef LVERSION
#undef MINBVERSION
#define LVERSION 21000
#define MINBVERSION 21000

#define BIGATOMS
#define BIGVM
#define NEWCDRCODING


#	elif (RELEASE == 300 )


#endif


