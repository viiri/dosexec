#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/emu.h"
#include "emu/bios/bios.h"
#include "emu/loader/loader.h"

#define ICONV_USE 1
#define ICONV_FROM "SJIS"
#define ICONV_TO "UTF-8"

#if ICONV_USE
#include <iconv.h>
#endif

void print_stdout(const char *buf)
{
#if ICONV_USE
  iconv_t cd;
  cd = iconv_open(ICONV_TO, ICONV_FROM);
  size_t ilen = strlen(buf);
  char *iptr = (char*)buf;
  size_t olen = ilen * 6 + 10;
  char *obuf = calloc(1, olen);
  char *optr = obuf;
  iconv(cd, &iptr, &ilen, &optr, &olen);
  *optr = '\0';
  printf("%s", obuf);
  free(obuf);
#else
  printf("%s", buf);
#endif
  fflush(stdout);
}

int main(int argc, const char *argv[])
{
  DosExecState dos;
  emu_init();
  if(!loader(&dos, argv[1], argc-2, argv+2)) {
    logerr("Couldn't begin emulation.\n");
    return EXIT_FAILURE;
  }
  while(_bios.running) {
    emu_run(100000);
  }
  emu_finish();

  return _bios.ret_code;
}
