#ifndef CORE_UTIL_H_
#define CORE_UTIL_H_

extern int running;

#define DPFLG_PC 1
#define DPFLG_LOGONLY 2

void dbgxprint(int flag, const char *str, ...);
void dbgprint(const char *str, ...);
void dbgcprint(const char *str, ...);

#define dbgprint(x...) dbgxprint(0, x)
#define dbgcprint(x...) dbgxprint(DPFLG_PC, x)

#endif
