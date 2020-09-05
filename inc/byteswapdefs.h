#ifndef BYTESWAPDEFS_H
#define BYTESWAPDEFS_H 1
unsigned int swapx(unsigned int word);
unsigned short byte_swap_word(unsigned short word);
void byte_swap_page(unsigned short *page, int wordcount);
void word_swap_page(unsigned short *page, int longwordcount);
void bit_reverse_region(unsigned short *top, int width, int height, int rasterwidth);
#ifdef RESWAPPEDDCODESTREAM
unsigned int byte_swap_code_block(unsigned int *base);
#endif
#endif
