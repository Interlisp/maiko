#ifndef LLSTKDEFS_H
#define LLSTKDEFS_H 1
#include "lispemul.h" /* for DLword */
#include "stack.h" /* for FX, StackWord, Bframe */
int do_stackoverflow(int incallp);
DLword *freestackblock(DLword n, StackWord *start68k, int align);
void decusecount68k(FX *frame68k);
void flip_cursorbar(int n);
void blt(DLword *dest68k, DLword *source68k, int nw);
void stack_check(StackWord *start68k);
void walk_stack(StackWord *start68k);
int quick_stack_check(void);
void check_FX(FX *fx68k);
void check_BF(Bframe *bf68k);
int check_stack_rooms(FX *fx68k);
#endif
