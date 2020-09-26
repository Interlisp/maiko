int fork_Unix(void);
#ifdef FULLSLAVENAME
int ForkUnixShell(int slot, char *PtySlave, char *termtype, char *shellarg);
#else
int ForkUnixShell(int slot, char ltr, char numb, char *termtype, char *shellarg);
#endif

