#ifndef KBDIF_H
#define KBDIF_H 1
/* $Id: kbdif.h,v 1.2 1999/01/03 02:06:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright  1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.					*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
#include <sys/types.h> /* for u_char */

/* The Keyboard structure. */

typedef struct {
  u_char KeyMap[0x80];
  void	(* sync_device)();	/* Make reality and emulator coincide with each other */
  void	(* enter_device)();
  void	(* exit_device)();
  void	(* device_event)();
  void	(* before_raid)();
  void	(* after_raid)();
  int	lispkeycode;
} KbdInterfaceRec, *KbdInterface;
#endif /* KBDIF_H */
