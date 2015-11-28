#ifndef CORE_UTIL_H_
#define CORE_UTIL_H_

extern int running;

enum {
  LogLevel_DEBUG,
  LogLevel_INFO,
  LogLevel_WARN,
  LogLevel_ERROR,
  LogLevel_OFF,
};

#define DPFLG_PC 1

void logx(int lvl, int flag, const char *str, ...);

#define log(lvl, x...) logx(lvl, 0, x)
#define log_pc(lvl, x...) logx(lvl, DPFLG_PC, x)

#define logdbg(x...) log(LogLevel_DEBUG, x)
#define logdbg_pc(x...) log_pc(LogLevel_DEBUG, x)
#define loginfo(x...) log(LogLevel_INFO, x)
#define loginfo_pc(x...) log_pc(LogLevel_INFO, x)
#define logwarn(x...) log(LogLevel_WARN, x)
#define logwarn_pc(x...) log_pc(LogLevel_WARN, x)
#define logerr(x...) log(LogLevel_ERROR, x)
#define logerr_pc(x...) log_pc(LogLevel_ERROR, x)

#endif
