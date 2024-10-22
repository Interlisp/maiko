#ifndef RAWCOLORDEFS_H
#define RAWCOLORDEFS_H 1
#include "lispemul.h" /* for LispPTR */
void C_slowbltchar(LispPTR *args);
LispPTR Colorize_Bitmap(LispPTR args[]);
void Draw_8BppColorLine(LispPTR *args);
void Uncolorize_Bitmap(LispPTR args[]);
#endif
