/* $Id: kbdif.h,v 1.2 1999/01/03 02:06:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */





/************************************************************************/
/*									*/
/*	(C) Copyright  1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.					*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/


/* The Keyboard structure. */

typedef struct {
  u_char KeyMap[0x80];
#ifdef DOS
  u_char lastbyte;
  void	(*prev_handler)();
#endif /* DOS */
  void	(* sync_device)();	/* Make reality and emualtor coinside with eachother */
  void	(* enter_device)();
  void	(* exit_device)();
  void	(* device_event)();
  void	(* before_raid)();
  void	(* after_raid)();
  int	lispkeycode;
#ifdef DOS
  int	device_active;
  int	device_locked;
#endif /* DOS */
} KbdInterfaceRec, *KbdInterface;


#ifndef TRUE
#define FALSE 0
#define TRUE  !FALSE
#endif /* TRUE */
