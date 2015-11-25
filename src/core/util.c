#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"

int running;
FILE *logfp = NULL;
int log_level = 0;

void dbgxprintv(int flag, const char *str, va_list vl)
{
  if(logfp == NULL) {
    logfp = fopen("log.txt", "wb");
  }
  BOOL term_out = (~flag & DPFLG_LOGONLY || log_level > 1) && log_level >= 0;
  if((flag & DPFLG_LOGONLY) && log_level < 1)
    return;
  if(flag & DPFLG_PC) {
    fprintf(logfp, "[%08X] ", cpu_get_pc());
    if(term_out)
      fprintf(stderr, "[%08X] ", cpu_get_pc());
  }
  vfprintf(logfp, str, vl);
  if(term_out)
    vfprintf(stderr, str, vl);
}
void dbgxprint(int flag, const char *str, ...)
{
  va_list vl;
  va_start(vl, str);
  dbgxprintv(flag, str, vl);
  va_end(vl);
}
