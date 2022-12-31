#ifndef BYTESWAPDEFS_H
#define BYTESWAPDEFS_H 1

/****************************************************************/
/*                                                              */
/*                 swap halves of a single 4-byte word          */
/*                                                              */
/****************************************************************/
static inline unsigned int swapx(unsigned int word) {
  return (((word >> 16) & 0xffff) | ((word & 0xffff) << 16));
}

void word_swap_page(void *page, unsigned longwordcount);
void bit_reverse_region(unsigned short *top, int width, int height, int rasterwidth);
#ifdef RESWAPPEDCODESTREAM
unsigned int byte_swap_code_block(unsigned int *base);
#endif
#endif
