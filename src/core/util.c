#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"

FILE *logfp = NULL;
int log_levels[2] = {
  LogLevel_WARN, // terminal
  LogLevel_OFF, // log file
};

#define DPFLG_PC 1

void logxv(int lvl, int flag, const char *str, va_list vl)
{
  BOOL term_out = lvl >= log_levels[0];
  BOOL log_out = lvl >= log_levels[1];

  if(!term_out && !log_out)
    return;

  if(logfp == NULL && log_out) {
    logfp = fopen("log.txt", "wb");
  }
  if(flag & DPFLG_PC) {
    logx(lvl, 0, "[%08X] ", cpu_get_pc());
  }
  if(log_out) {
    vfprintf(logfp, str, vl);
    fflush(logfp);
  }
  if(term_out)
    vfprintf(stderr, str, vl);
}
void logx(int lvl, int flag, const char *str, ...)
{
  va_list vl;
  va_start(vl, str);
  logxv(lvl, flag, str, vl);
  va_end(vl);
}
