#ifndef IFPAGE_H
#define IFPAGE_H 1
/* $Id: ifpage.h,v 1.2 1999/01/03 02:06:01 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include "lispemul.h" /* for LispPTR, DLword */
#include "version.h" /* for BIGVM */

#define MACHINETYPE_MAIKO 3
#define IFPAGE_KEYVAL 0x15e3
#ifndef BYTESWAP
#ifdef BIGVM
	/* Normal definition, for big-endian BIGVM machines */
typedef struct ifpage {
	DLword    currentfxp;
	DLword    resetfxp;
	DLword    subovfxp;
	DLword    kbdfxp;
	DLword    hardreturnfxp;
	DLword    gcfxp;
	DLword    faultfxp;
	DLword    endofstack;
	DLword    lversion;
	DLword    minrversion;
	DLword    minbversion;
	DLword    rversion;
	DLword    bversion;
	DLword    machinetype;
	DLword    miscfxp;
	DLword    key;
	DLword    serialnumber;
	DLword    emulatorspace;
	DLword    screenwidth;
	DLword    nxtpmaddr;
	DLword    ex_nactivepages;
	DLword    ex_ndirtypages;
	DLword    filepnpmp0;
	DLword    filepnpmt0;
	DLword    teleraidfxp;
	DLword    filler1;
	DLword    filler2;
	DLword    filler3;
	DLword    usernameaddr;
	DLword    userpswdaddr;
	DLword    stackbase;
	DLword    faulthi;
	DLword    faultlo;
	DLword    devconfig;/*wasrealpagetable;*/
	DLword    rptsize;
	DLword    rpoffset;
	DLword    wasrptlast;
	DLword    embufvp;
	DLword    nshost0;
	DLword    nshost1;
	DLword    nshost2;
	DLword    mdszone;
	DLword    mdszonelength;
	DLword    emubuffers;
	DLword    emubuflength;
	DLword    ex_process_size; /* was lastnumchars */
	DLword    storagefullstate;  /* was sysdisk */
	DLword    isfmap;
	/* these are for \miscapply
	 * -- note that they are not ref counted, so don't pass the only pointer
	 * to something this way */
	LispPTR   miscstackfn;
	LispPTR   miscstackarg1;
	LispPTR   miscstackarg2;
	LispPTR   miscstackresult;
	DLword    nrealpages;
	DLword    lastlockedfilepage;
	DLword    lastdominofilepage;
	DLword    fptovpstart;
	DLword    fakemousebits;
	DLword    dl24bitaddressable;
	LispPTR   realpagetableptr;
	DLword    ex_dllastvmempage;
	DLword    fullspaceused;
	DLword    fakekbdad4;
	DLword    fakekbdad5;
	DLword	d1nil1;
	DLword	dlnil2;
	DLword	dlnil3;
	DLword	dlnil4;
	DLword	dlnil5;
	DLword	dlnil6;
	LispPTR	dlnilp1;
	LispPTR	dlnilp2;
	LispPTR	dlnilp3;
	unsigned dllastvmempage;
	int	    nactivepages;
	int    ndirtypages;
	unsigned  process_size; /* was lastnumchars */
} IFPAGE;
#else
/* Normal definition, for big-endian machines */
typedef struct ifpage {
	DLword    currentfxp;
	DLword    resetfxp;
	DLword    subovfxp;
	DLword    kbdfxp;
	DLword    hardreturnfxp;
	DLword    gcfxp;
	DLword    faultfxp;
	DLword    endofstack;
	DLword    lversion;
	DLword    minrversion;
	DLword    minbversion;
	DLword    rversion;
	DLword    bversion;
	DLword    machinetype;
	DLword    miscfxp;
	DLword    key;
	DLword    serialnumber;
	DLword    emulatorspace;
	DLword    screenwidth;
	DLword    nxtpmaddr;
	DLword    nactivepages;
	DLword    ndirtypages;
	DLword    filepnpmp0;
	DLword    filepnpmt0;
	DLword    teleraidfxp;
	DLword    filler1;
	DLword    filler2;
	DLword    filler3;
	DLword    usernameaddr;
	DLword    userpswdaddr;
	DLword    stackbase;
	DLword    faulthi;
	DLword    faultlo;
	DLword    devconfig;/*wasrealpagetable;*/
	DLword    rptsize;
	DLword    rpoffset;
	DLword    wasrptlast;
	DLword    embufvp;
	DLword    nshost0;
	DLword    nshost1;
	DLword    nshost2;
	DLword    mdszone;
	DLword    mdszonelength;
	DLword    emubuffers;
	DLword    emubuflength;
	DLword    process_size; /* was lastnumchars */
	DLword    storagefullstate;  /* was sysdisk */
	DLword    isfmap;
	/* these are for \miscapply
	 * -- note that they are not ref counted, so don't pass the only pointer
	 * to something this way */
	LispPTR   miscstackfn;
	LispPTR   miscstackarg1;
	LispPTR   miscstackarg2;
	LispPTR   miscstackresult;
	DLword    nrealpages;
	DLword    lastlockedfilepage;
	DLword    lastdominofilepage;
	DLword    fptovpstart;
	DLword    fakemousebits;
	DLword    dl24bitaddressable;
	LispPTR   realpagetableptr;
	DLword    dllastvmempage;
	DLword    fullspaceused;
	DLword    fakekbdad4;
	DLword    fakekbdad5;
} IFPAGE;
#endif /* BIGVM */
#else
#ifdef BIGVM
	/***********************************************************/
	/*       Byte-swapped/word-swapped BIGVM version	       */
	/***********************************************************/
typedef struct ifpage {
	DLword    resetfxp;
	DLword    currentfxp;  /* hi word */
	DLword    kbdfxp;
	DLword    subovfxp;  /* hi word */
	DLword    gcfxp;
	DLword    hardreturnfxp;  /* hi word */
	DLword    endofstack;
	DLword    faultfxp;  /* hi word */
	DLword    minrversion;
	DLword    lversion;  /* hi word */
	DLword    rversion;
	DLword    minbversion;  /* hi word */
	DLword    machinetype;
	DLword    bversion;  /* hi word */
	DLword    key;
	DLword    miscfxp;  /* hi word */
	DLword    emulatorspace;
	DLword    serialnumber;  /* hi word */
	DLword    nxtpmaddr;
	DLword    screenwidth;  /* hi word */
	DLword    ex_ndirtypages;
	DLword    ex_nactivepages;  /* hi word */
	DLword    filepnpmt0;
	DLword    filepnpmp0;  /* hi word */
	DLword    filler1;
	DLword    teleraidfxp;  /* hi word */
	DLword    filler3;
	DLword    filler2;  /* hi word */
	DLword    userpswdaddr;
	DLword    usernameaddr;  /* hi word */
	DLword    faulthi;
	DLword    stackbase;  /* hi word */
	DLword    devconfig;/*wasrealpagetable;*/
	DLword    faultlo;  /* hi word */
	DLword    rpoffset;
	DLword    rptsize;  /* hi word */
	DLword    embufvp;
	DLword    wasrptlast;  /* hi word */
	DLword    nshost1;
	DLword    nshost0;  /* hi word */
	DLword    mdszone;
	DLword    nshost2;  /* hi word */
	DLword    emubuffers;
	DLword    mdszonelength;  /* hi word */
	DLword    ex_process_size;
	DLword    emubuflength;  /* hi word */
	DLword    isfmap;
	DLword    storagefullstate;  /* hi word */
	/* these are for \miscapply
	 * -- note that they are not ref counted, so don't pass the only pointer
	 * to something this way */
	LispPTR   miscstackfn;
	LispPTR   miscstackarg1;
	LispPTR   miscstackarg2;
	LispPTR   miscstackresult;
	DLword    lastlockedfilepage;
	DLword    nrealpages; /* hi word */
	DLword    fptovpstart;
	DLword    lastdominofilepage; /* hi word */
	DLword    dl24bitaddressable;
	DLword    fakemousebits; /* hi word */
	LispPTR   realpagetableptr;
	DLword    fullspaceused;
	DLword    ex_dllastvmempage; /* hi word */
	DLword    fakekbdad5;
	DLword    fakekbdad4; /* hi word */
	DLword	d1nil1;
	DLword	dlnil2;
	DLword	dlnil3;
	DLword	dlnil4;
	DLword	dlnil5;
	DLword	dlnil6;
	LispPTR	dlnilp1;
	LispPTR	dlnilp2;
	LispPTR	dlnilp3;
	unsigned dllastvmempage;
	int	    nactivepages;
	int    ndirtypages;
	unsigned process_size; /* was lastnumchars */
} IFPAGE;
#else
	/***********************************************************/
	/*       Byte-swapped/word-swapped version, for 386i       */
	/***********************************************************/
typedef struct ifpage {
	DLword    resetfxp;
	DLword    currentfxp;  /* hi word */
	DLword    kbdfxp;
	DLword    subovfxp;  /* hi word */
	DLword    gcfxp;
	DLword    hardreturnfxp;  /* hi word */
	DLword    endofstack;
	DLword    faultfxp;  /* hi word */
	DLword    minrversion;
	DLword    lversion;  /* hi word */
	DLword    rversion;
	DLword    minbversion;  /* hi word */
	DLword    machinetype;
	DLword    bversion;  /* hi word */
	DLword    key;
	DLword    miscfxp;  /* hi word */
	DLword    emulatorspace;
	DLword    serialnumber;  /* hi word */
	DLword    nxtpmaddr;
	DLword    screenwidth;  /* hi word */
	DLword    ndirtypages;
	DLword    nactivepages;  /* hi word */
	DLword    filepnpmt0;
	DLword    filepnpmp0;  /* hi word */
	DLword    filler1;
	DLword    teleraidfxp;  /* hi word */
	DLword    filler3;
	DLword    filler2;  /* hi word */
	DLword    userpswdaddr;
	DLword    usernameaddr;  /* hi word */
	DLword    faulthi;
	DLword    stackbase;  /* hi word */
	DLword    devconfig;/*wasrealpagetable;*/
	DLword    faultlo;  /* hi word */
	DLword    rpoffset;
	DLword    rptsize;  /* hi word */
	DLword    embufvp;
	DLword    wasrptlast;  /* hi word */
	DLword    nshost1;
	DLword    nshost0;  /* hi word */
	DLword    mdszone;
	DLword    nshost2;  /* hi word */
	DLword    emubuffers;
	DLword    mdszonelength;  /* hi word */
	DLword    process_size;
	DLword    emubuflength;  /* hi word */
	DLword    isfmap;
	DLword    storagefullstate;  /* hi word */
	/* these are for \miscapply
	 * -- note that they are not ref counted, so don't pass the only pointer
	 * to something this way */
	LispPTR   miscstackfn;
	LispPTR   miscstackarg1;
	LispPTR   miscstackarg2;
	LispPTR   miscstackresult;
	DLword    lastlockedfilepage;
	DLword    nrealpages; /* hi word */
	DLword    fptovpstart;
	DLword    lastdominofilepage; /* hi word */
	DLword    dl24bitaddressable;
	DLword    fakemousebits; /* hi word */
	LispPTR   realpagetableptr;
	DLword    fullspaceused;
	DLword    dllastvmempage; /* hi word */
	DLword    fakekbdad5;
	DLword    fakekbdad4; /* hi word */
} IFPAGE;
#endif /* BIGVM */
#endif /* BYTESWAP */
#endif
