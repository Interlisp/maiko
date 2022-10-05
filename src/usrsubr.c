/* $Id: usrsubr.c,v 1.3 1999/05/31 23:35:46 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>

#include "usrsubrdefs.h"

/** User defined subrs here.  Do NOT attempt to use this unless you FULLY
    understand the dependencies of the LDE architecture.                 **/

int UserSubr(int user_subr_index, int num_args, unsigned *args) {
  int result = 0;

  /* *** remove the printf when finished debugging your user subr *** */

  printf("debug: case: 0x%x, args: 0x%x\n", user_subr_index, num_args);
  {
    int i;
    for (i = 0; i < num_args; i++) printf("debug: arg[%d]: 0x%x\n", i, args[i]);
  }

  switch (user_subr_index) {
    case 0:
      printf("sample UFN\n");
      result = args[0];
      break;
  default:
      return (-1); /* DO UFN */
  }

  return (result);
}
