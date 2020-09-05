#ifndef KEYLIBDEFS_H
#define KEYLIBDEFS_H 1
unsigned long make_verification(unsigned long x, unsigned long y);
unsigned long date_integer16(const char *date);
unsigned long idate(const char *str);
unsigned long modify(unsigned long hostid);
int imod64bit(unsigned long x1, unsigned long x0, unsigned long y);
unsigned long make_verification(long unsigned int x, long unsigned int y);
unsigned long date_integer16(const char *date);
unsigned long idate(const char *str);
unsigned long modify(unsigned long hostid);
#endif
