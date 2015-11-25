#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/emu.h"
#include "emu/mem.h"
#include "emu/cpu/cpu.h"
#include "emu/bios/bios.h"
#include "emu/bios/mem.h"

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
  char *obuf = malloc(olen);
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

static BOOL prv_load_file(const char *fn, uint16 *size, uint16 *para)
{
  FILE *fp;
  size_t i;
  uint8 byte;
  uint16 len;
  fp = fopen(fn, "rb");
  if(fp == NULL)
    return FALSE;
  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *size = len + 0x100;
  if(bios_mem_alloc((*size + 0xF) >> 4, para)) {
    fprintf(stderr, "Can't alloc code\n");
    return FALSE;
  }

  uint32 ptr = (*para << 4) + 0x100;
  for(i = 0; i < len; i++) {
    fread(&byte, 1, 1, fp);
    mem_w8(ptr, byte, NULL);
    ptr++;
  }
  return TRUE;
}

void load_program(int argc, const char *argv[])
{
  (void)argc;

  const char *env = "";
  size_t env_len = 1;
  const char *prg = argv[0];
  argc--;
  argv++;
  uint16 prglen = 0;
  uint16 codeseg = 0;
  uint16 pspseg = 0;
  uint16 envseg = 0;
  int i;

  prv_load_file(prg, &prglen, &codeseg);
  pspseg = codeseg;
  if(bios_mem_alloc((env_len + 0xF) >> 4, &envseg)) {
    fprintf(stderr, "Can't alloc ENV\n");
  }

  dbgprint("CODE:%04X(%04X)\n", codeseg, prglen);
  dbgprint("PSP :%04X\n", pspseg);
  dbgprint("ENV :%04X\n", envseg);

  // Set the environment
  bios_w8(envseg,0x00, env[0]); // empty~

  // Set up the PSP
  bios_w16(pspseg,0x00, 0x20CD); // INT 20h
  bios_w16(pspseg,0x02, codeseg + ((prglen + 0xF) >> 4) + 1); // Program ending address
  bios_w8 (pspseg,0x05, 0xC3); // DOS function dispatcher (just returns)
  bios_wlp(pspseg,0x0A, pspseg, 0x0005); // Program termination code ptr
  bios_wlp(pspseg,0x0E, pspseg, 0x0005); // Break handler code ptr
  bios_wlp(pspseg,0x12, pspseg, 0x0005); // Critical error code ptr
  bios_w16(pspseg,0x2C, envseg);
  bios_w16(pspseg,0x50, 0x21CD); // INT 21h
  bios_w8 (pspseg,0x52, 0xCB); // RETF

  size_t arglen = 0;
  dbgprint("argstr: '");
  for(i = 0; i < argc; i++) {
    bios_w8(pspseg,0x81+arglen, ' ');
    dbgprint(" ");
    arglen++;

    const char *ptr = argv[i];
    while(*ptr) {
      bios_w8(pspseg,0x81+arglen, *ptr);
      dbgprint("%c", *ptr);
      ptr++;
      arglen++;
    }
  }
  dbgprint("', %zu chars\n", arglen);
  bios_w8(pspseg,0x81+arglen, 0x0D);
  bios_w8(pspseg,0x80, arglen);

  // Set the initial registers
  cpu_set_reg(CPU_REG_DS, pspseg);
  cpu_set_reg(CPU_REG_CS, codeseg);
  cpu_set_reg(CPU_REG_IP, 0x100);
}

int main(int argc, const char *argv[])
{
  emu_init();
  load_program(argc-1, argv+1);
  while(_bios.running) {
    emu_run(100000);
  }
  emu_finish();

  return _bios.ret_code;
}
