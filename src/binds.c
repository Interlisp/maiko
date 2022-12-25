/* $Id: binds.c,v 1.3 1999/05/31 23:35:24 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>
#include "lispemul.h"
#include "lspglob.h"
#include "emlglob.h"
#include "testtooldefs.h"
#include "bindsdefs.h"

/**************************************************
N_OP_bind(stack_pointer, tos, n1, n2)

        Entry:	BIND		opcode[021]

        1. bind PVAR slot to NIL. (n1 times)
        2. bind PVAR slot to value of slot in Evaluation stack. (n2 times)
           or push TopOfStack to Evaluation stack.
        3. Push		[upper word]	1's complement of bind slots
                        [lower word]	2word offset from PVar

***************************************************/

LispPTR *N_OP_bind(LispPTR *stack_pointer, LispPTR tos, unsigned byte1, unsigned byte2) {
  unsigned n1;         /* # slots to bind to NIL (0, 0) */
  unsigned n2;         /* # slots to bind to value in stack */
  LispPTR *ppvar; /* pointer to argued slot in Pvar area */
  unsigned i;          /* temporary for control */

#ifdef TRACE
  printPC();
  printf("TRACE: N_OP_bind()\n");
#endif

  n1 = byte1 >> 4;
  n2 = byte1 & 0xf;
  ppvar = (LispPTR *)PVar + 1 + byte2;

  for (i = 0; i < n1; i++) { *--ppvar = NIL_PTR; }

  if (n2 == 0) {
    *stack_pointer++ = tos; /* push TopOfStack to Evaluation stack */
  } else {
    *--ppvar = tos; /* bind to TopOfStack */
    for (i = 1; i < n2; i++) { *--ppvar = *(--stack_pointer); }
  }

  i = ~(n1 + n2); /* x: 1's complement of number of bind slots */
  *stack_pointer = (i << 16) | (byte2 << 1);
  return (stack_pointer);
}

/**************************************************
LispPTR N_OP_unbind(stackpointer)

        Entry:	UNBIND		opcode[022]

        1. pop stackpointer until the slot (num, lastpvar) is found
           (Note: TOPOFSTACK is ignored)
        2. unbind lastpvar slot (set to 0xFFFF). (num times)

***************************************************/

LispPTR *N_OP_unbind(LispPTR *stack_pointer) {
  DLword num;     /* number of unbind sot */
  LispPTR *ppvar; /* pointer to last PVAR slot. */
  DLword i;       /* temporary for control */
  LispPTR value;

#ifdef TRACE
  printPC();
  printf("TRACE: N_OP_unbind()\n");
#endif

  /* now, stack_pointer points the latter part in slot */
  for (; !(*--stack_pointer & 0x80000000);)
    ; /* scan (until MSB == 1) */

  value = *stack_pointer;
  num = (DLword) ~(value >> 16);
  ppvar = (LispPTR *)(PVar + 2 + GetLoWord(value));
  value = 0xffffffff;
  for (i = 0; i < num; i++) { *--ppvar = value; }
  return (stack_pointer);
}

/**************************************************
N_OP_dunbind

        Entry:	DUNBIND		opcode[023]

        1. if TopOfStack is unbound
                unbind num slots from PVar.
           if TopOfStack is bound
                pop CurrentStack until the slot (num, lastpvar) is found.
                unbind num slots from lastpvar.
        2. pop the top of CurrentStackPTR to TopOfStack.

***************************************************/

LispPTR *N_OP_dunbind(LispPTR *stack_pointer, LispPTR tos) {
  DLword num;     /* number of unbind sot */
  LispPTR *ppvar; /* pointer to last PVAR slot. */
  DLword i;       /* temporary for control */
  LispPTR value;

#ifdef TRACE
  printPC();
  printf("TRACE: N_OP_dunbind()\n");
#endif

  if (tos & 0x80000000) {
    /* check MSB bit of High word in tos, 1: unbound, 0: bound */

    /* tos is unbound */
    num = ~(GetHiWord(tos));
    value = 0xffffffff;
    if (num != 0) {
      ppvar = (LispPTR *)(PVar + 2 + GetLoWord(tos));
      for (i = 0; i < num; ++i) { *--ppvar = value; }
    }
  } else {
    /* tos is bound */
    /* now, stack_pointer points the latter part in slot */
    for (; !((*--stack_pointer) & 0x80000000);)
      ;
    /* scan (until MSB == 1) */

    value = *stack_pointer;
    num = ~(GetHiWord(value));
    ppvar = (LispPTR *)(PVar + 2 + GetLoWord(value));
    value = 0xffffffff;
    for (i = 0; i < num; i++) { *--ppvar = value; }
  }

  return (stack_pointer);
}
