#ifndef DLPIDEFS_H
#define DLPIDEFS_H 1
#include <sys/types.h> /* for u_char, u_long */
int setup_dlpi_dev(char *device);
void flush_dlpi(int fd);
int dlpi_devtype(int fd);
char *savestr(char *s);
int dlunitdatareq(int fd, u_char *addrp, int addrlen, u_long minpri, u_long maxpri, u_char *datap, int datalen);
#endif
