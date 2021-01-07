#ifndef UNIXFORK_H
#define UNIXFORK_H 1
int fork_Unix(void);
int ForkUnixShell(int slot, char *PtySlave, char *termtype, char *shellarg);
#endif /* UNIXFORK_H */
