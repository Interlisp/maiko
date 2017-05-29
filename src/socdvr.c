/* $Id: socdvr.c,v 1.2 1999/01/03 02:07:33 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */
static char *id = "$Id: socdvr.c,v 1.2 1999/01/03 02:07:33 sybalsky Exp $ Copyright (C) Venue";

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
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include <sys/time.h>
#include <sys/file.h>
#include <errno.h>
#include <X11/Xproto.h>

#include "lispemul.h"
#include "arith.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "lispmap.h"

#define min(x, y) (((x) > (y)) ? (y) : (x))

/***********************************************************/
/*	       L S t r i n g T o C S t r i n g		   */
/*							   */
/*  Convert a lisp string to a C string up to MaxLen long. */
/***********************************************************/

#define LStringToCString(Lisp, C, MaxLen, Len)                                                     \
  {                                                                                                \
    OneDArray *arrayp;                                                                             \
    char *base;                                                                                    \
    short *sbase;                                                                                  \
    int i;                                                                                         \
                                                                                                   \
    arrayp = (OneDArray *)(Addr68k_from_LADDR((unsigned int)Lisp));                                \
    Len = min(MaxLen, arrayp->fillpointer);                                                        \
                                                                                                   \
    switch (arrayp->typenumber) {                                                                  \
      case THIN_CHAR_TYPENUMBER:                                                                   \
        base =                                                                                     \
            ((char *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset));  \
        for (i = 0; i < Len; i++) C[i] = base[i];                                                  \
        C[Len] = '\0';                                                                             \
        break;                                                                                     \
                                                                                                   \
      case FAT_CHAR_TYPENUMBER:                                                                    \
        sbase =                                                                                    \
            ((short *)(Addr68k_from_LADDR((unsigned int)arrayp->base))) + ((int)(arrayp->offset)); \
        base = (char *)sbase;                                                                      \
        for (i = 0; i < Len * 2; i++) C[i] = base[i];                                              \
        C[Len * 2] = '\0';                                                                         \
        break;                                                                                     \
                                                                                                   \
      default: error("LStringToCString can not handle\n");                                         \
    }                                                                                              \
  }

#define FGetNum(ptr, place)                     \
  {                                             \
    if (((ptr)&SEGMASK) == S_POSITIVE) {        \
      (place) = ((ptr)&0xffff);                 \
    } else if (((ptr)&SEGMASK) == S_NEGATIVE) { \
      (place) = (int)((ptr) | 0xffff0000);      \
    } else {                                    \
      return (NIL);                             \
    }                                           \
  }

#define MAX_NAME_LEN 256
static char XServer_Name[MAX_NAME_LEN]; /* Name of host with X server */
static int XPort_Number = 0;            /* Display # to ask for on it */

int XServer_Fd = -1; /* The socket for the X server */

extern DLword *Lisp_world;

/************************************************************************/
/*									*/
/*			O p e n _ S o c k e t				*/
/*									*/
/*	Open a connection to an X server, by calling the CLX routine	*/
/*	"connect_to_server".  Returns T if successful, and NIL if not.	*/
/*	The socket's fd is left in XServer_Fd, a global, so only one	*/
/*	server connection can be open at one time (FIX THIS).		*/
/*									*/
/*	args[0] - The lisp string name of the host the server is on	*/
/*	args[1] - The SMALLP server-number we're connecting to.		*/
/*									*/
/************************************************************************/

Open_Socket(args) LispPTR *args;
{
#ifdef TRACE
  printf("TRACE: Open_Socket()\n");
#endif

  int length;

  LStringToCString(args[0], XServer_Name, MAX_NAME_LEN, length);
  FGetNum(args[1], XPort_Number);
  XPort_Number -= X_TCP_PORT;

  if (XServer_Fd == -1) {
    XServer_Fd = connect_to_server(XServer_Name, XPort_Number);

    if (XServer_Fd < 0) /* error in connect. */
    {
      perror("connecting to X server");
      return (NIL);
    }

    { /* Make it non-blocking I/O */
      int res;
      res = fcntl(XServer_Fd, F_GETFL);
      res |= FNDELAY;
      res = fcntl(XServer_Fd, F_SETFL, res);
    }
  } /* end if(XServer_Fd) */

  return (ATOM_T);
}

/************************************************************************/
/*									*/
/*			C l o s e _ S o c k e t				*/
/*									*/
/*	Close the socket connection to the X server.			*/
/*									*/
/************************************************************************/

Close_Socket() {
  int stat;

#ifdef TRACE
  printf("TRACE: Close_Socket()\n");
#endif

  if ((stat = close(XServer_Fd)) ==
      -1) { /* close failed; return NIL, but squash the old fd anyhow */
    XServer_Fd = -1;
    perror("Close_socket");
    return (NIL);
  } else { /* close succeeded; return T. */
    XServer_Fd = -1;
    return (ATOM_T);
  }
} /* end Close_Socket */

/************************************************************************/
/*									*/
/*			   R e a d _ S o c k e t			*/
/*									*/
/*	Read up to 1 packet's worth from the X-server socket.		*/
/*									*/
/************************************************************************/

typedef struct {  /* Format for an X-server packet */
  DLword nil[22]; /* Packet header */
  DLword length;  /* Request byte length */
  char data[592]; /* Data body */
} PACKET;

#define PACKET_DEFOFFSET 46
#define PACKET_MAXSIZE 638

Read_Socket(args) LispPTR *args;
{
  PACKET *packet;
  char *buffer;
  int length, actlen;

#ifdef TRACE
  printf("TRACE: Read_Socket()\n");
#endif

  if (XServer_Fd >= 0) {
    packet = (PACKET *)Addr68k_from_LADDR(args[0]);

    if ((length = (int)(packet->length) - PACKET_DEFOFFSET) > 0) {
      buffer = &(packet->data[0]);

      if ((actlen = read(XServer_Fd, buffer, length)) > 0) {
        packet->length = (DLword)(actlen + PACKET_DEFOFFSET);
        return (ATOM_T);
      }               /* end if(actlen) */
      if (actlen < 0) /* error !*/
      {
        if ((errno != EWOULDBLOCK) & (errno != EINTR)) perror("reading X connection");
        return (NIL);
      }

    } /* end if(length) */

  } /* end if( fd ) */

  return (NIL);

} /* end Read_Socket */

/************************************************************************/
/*									*/
/*			  W r i t e _ S o c k e t			*/
/*									*/
/*	Write a packet of information to the X server's socket.		*/
/*									*/
/************************************************************************/

Write_Socket(args) LispPTR *args;
{
  PACKET *packet;
  char *buffer;
  int length, actlen;

#ifdef TRACE
  printf("TRACE: Write_Socket()\n");
#endif

  if (XServer_Fd >= 0) {
    packet = (PACKET *)Addr68k_from_LADDR(args[0]);

    if ((length = (int)(packet->length) - PACKET_DEFOFFSET) > 0) {
      buffer = &(packet->data[0]);

      if ((actlen = write(XServer_Fd, buffer, length)) > 0) {
        packet->length = (DLword)(actlen + PACKET_DEFOFFSET);
        return (ATOM_T);

      }               /* end if( actlen ) */
      if (actlen < 0) /* error !*/
      {
        if (errno != EINTR) perror("writing X connection");
        return (NIL);
      }

    } /* end if(length) */

  } /* end if( fd ) */

  packet->length = 0;
  return (NIL);

} /* end Write_Socket */

/************************************************************************/
/*									*/
/*		      K b d _ T r a n s i t i o n			*/
/*									*/
/*	Stuff a key transition into the C-level buffer from Lisp.	*/
/*									*/
/*	args[0] - the key number (in lisps terms? Not sure)		*/
/*	args[1] - upflg -- is it an up or down-transition? 		*/
/*									*/
/************************************************************************/
extern int KBDEventFlg;

Kbd_Transition(args) LispPTR *args;
/* args[0] is key-number */
/* args[1] is up-flg     */
{
  DLword key_number;

  key_number = (DLword)(args[0] & 0xffff);
  if (args[1])
    kb_trans(key_number, 1);
  else
    kb_trans(key_number, 0);

  DoRing();
  /* If there's something for lisp to do, ask for an interrupt: */
  if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;

} /* end Kbd_Transition */
