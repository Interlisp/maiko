#ifndef SXHASHDEFS_H
#define SXHASHDEFS_H 1
typedef struct { LispPTR object; } SXHASHARG;
LispPTR SX_hash(register SXHASHARG *args);
LispPTR STRING_EQUAL_HASHBITS(SXHASHARG *args);
LispPTR STRING_HASHBITS(SXHASHARG *args);
#endif
