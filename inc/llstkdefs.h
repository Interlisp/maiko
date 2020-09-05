#ifndef LLSTKDEFS_H
#define LLSTKDEFS_H 1
#include "stack.h"
int do_stackoverflow(int incallp);
DLword *freestackblock(DLword n, StackWord *start68k, int align);
void decusecount68k(register FX *frame68k);
void flip_cursorbar(int n);
void blt(register DLword *dest68k, register DLword *source68k, int nw);
void stack_check(StackWord *start68k);
void walk_stack(StackWord *start68k);
void quick_stack_check(void);
void check_FX(FX *fx68k);
void check_BF(Bframe *bf68k);
int check_stack_rooms(FX *fx68k);
#endif
