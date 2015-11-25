#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/bios/bios.h"
#include "emu/bios/bios_internal.h"
#include "emu/bios/mem.h"
#include "emu/cpu/cpu.h"
#include "emu/mem.h"

#define MAX_FILES 32

static FILE *s_open_files[MAX_FILES] = {NULL};

void print_stdout(const char *buf);

static BOOL prv_int21_02xx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  uint8 dl;
  dl = cpu_get_reg(CPU_REG_DW) & 0xFF;

  char buf[2];
  buf[0] = dl;
  buf[1] = '\0';
  print_stdout(buf);

  cpu_set_reg(CPU_REG_AW, dl);
  return TRUE;
}

static BOOL prv_int21_09xx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  uint16 ds, dw;
  ds = cpu_get_reg(CPU_REG_DS);
  dw = cpu_get_reg(CPU_REG_DW);

  char *buf = bios_read_dollar(ds, dw);
  print_stdout(buf);
  free(buf);

  cpu_set_reg(CPU_REG_AW, '$');
  return TRUE;
}

static BOOL prv_int21_2Axx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  time_t t;
  struct tm ti;

  t = time(NULL);
  localtime_r(&t, &ti);

  cpu_set_reg(CPU_REG_AW, ti.tm_wday);
  cpu_set_reg(CPU_REG_CW, ti.tm_year + 1900 - 1980);
  cpu_set_reg(CPU_REG_DW, ((ti.tm_mon+1) << 8) | ti.tm_mday);

  return TRUE;
}

static BOOL prv_int21_2Cxx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  time_t t;
  struct tm ti;

  t = time(NULL);
  localtime_r(&t, &ti);

  cpu_set_reg(CPU_REG_CW, (ti.tm_hour << 8) | (ti.tm_min));
  cpu_set_reg(CPU_REG_DW, (ti.tm_sec << 8));

  return TRUE;
}

static BOOL prv_int21_3Cxx_3Dxx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;

  size_t i;
  for(i = 0; i < MAX_FILES; i++) {
    if(s_open_files[i] == NULL)
      break;
  }
  if(i == MAX_FILES) {
    bios_ret(DOSERR_NO_FREE_HANDLES);
    return TRUE;
  }

  uint8 mode = aw & 0xFF;
  uint8 cl = cpu_get_reg(CPU_REG_CW) & 0xFF;
  uint16 ds, dw;
  ds = cpu_get_reg(CPU_REG_DS);
  dw = cpu_get_reg(CPU_REG_DW);
  char *buf = bios_read_asciz(ds, dw);

  FILE *fp;
  const char *fmode;
  if((aw >> 8) == 0x3C) {
    fmode = "wb";
    dbgcprint("creat(ptr=%04X:%04X[%s], attr=%02X)\n", ds,dw,buf, cl);
  }else{
    dbgcprint("open(ptr=%04X:%04X[%s], mode=%02X, attr=%02X)\n", ds,dw,buf, mode, cl);
    switch(mode & 7) {
      case 0: fmode = "rb"; break;
      case 1: fmode = "wb+"; break;
      case 2: fmode = "rb+"; break;
      default:
        dbgprint("Unhandled open mode %d\n", mode & 7);
        fmode = "rb";
        break;
    }
  }
  fp = fopen(buf, fmode);
  free(buf);
  if(fp == NULL) {
    bios_ret(DOSERR_FILE_NOT_FOUND);
    return TRUE;
  }

  s_open_files[i] = fp;
  bios_ret(DOSERR_NONE);
  cpu_set_reg(CPU_REG_AW, i);
  return TRUE;
}

static BOOL prv_int21_3Exx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  uint16 fh = cpu_get_reg(CPU_REG_BW);
  dbgcprint("close(fh=%04X)\n", fh);
  if(fh >= MAX_FILES) {
    bios_ret(DOSERR_INVALID_HANDLE);
    return TRUE;
  }
  if(s_open_files[fh] == NULL) {
    bios_ret(DOSERR_INVALID_HANDLE);
    return TRUE;
  }
  fclose(s_open_files[fh]);
  s_open_files[fh] = NULL;
  bios_ret(DOSERR_NONE);
  return TRUE;
}

static BOOL prv_int21_3Fxx_40xx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  uint16 fh = cpu_get_reg(CPU_REG_BW);
  if(fh >= MAX_FILES) {
    bios_ret(DOSERR_INVALID_HANDLE);
    return TRUE;
  }
  if(s_open_files[fh] == NULL) {
    bios_ret(DOSERR_INVALID_HANDLE);
    return TRUE;
  }

  uint16 count = cpu_get_reg(CPU_REG_CW);
  uint16 ds, dw;
  ds = cpu_get_reg(CPU_REG_DS);
  dw = cpu_get_reg(CPU_REG_DW);

  uint8 *buf = malloc(count);
  if((aw >> 8) == 0x3F) {
    dbgcprint("read(fh=%04X, buf=%04X:%04X, count=%04X)\n", fh, ds,dw, count);
    count = fread(buf, 1, count, s_open_files[fh]);
    for(int i = 0; i < count; i++) {
      bios_w8(ds, dw + i, buf[i]);
    }
  }else{
    dbgcprint("write(fh=%04X, buf=%04X:%04X, count=%04X)\n", fh, ds,dw, count);
    for(int i = 0; i < count; i++) {
      buf[i] = bios_r8(ds, dw + i);
    }
    count = fwrite(buf, 1, count, s_open_files[fh]);
  }

  bios_ret(DOSERR_NONE);
  cpu_set_reg(CPU_REG_AW, count);
  return TRUE;
}

static BOOL prv_int21_48xx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  uint16 para;
  uint16 size = cpu_get_reg(CPU_REG_BW);

  dbgcprint("malloc(%04X)\n", size);
  if(bios_ret(bios_mem_alloc(size, &para))) {
    cpu_set_reg(CPU_REG_AW, para);
  }
  return TRUE;
}

static BOOL prv_int21_49xx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;

  dbgcprint("free(%04X)\n", cpu_get_reg(CPU_REG_ES));
  bios_ret(bios_mem_free(cpu_get_reg(CPU_REG_ES)));

  return TRUE;
}

static BOOL prv_int21_4Axx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  (void)aw;
  uint16 para = cpu_get_reg(CPU_REG_ES);
  uint16 size = cpu_get_reg(CPU_REG_BW);
  dbgcprint("memresize(ptr=%04X, size=%04X)\n", para, size);
  bios_ret(bios_mem_resize(size, para));
  return TRUE;
}

static BOOL prv_int21_4Cxx(uint8 irq_num, uint16 aw)
{
  (void)irq_num;
  _bios.running = FALSE;
  _bios.ret_code = aw;
  cpu_bus_req(TRUE); // Kill the CPU
  return TRUE;
}

static const BiosIrqHandler _irq21_handlers[256][257] = {
  [0x02] = { [0x100] = prv_int21_02xx, }, // DOS 1+ - Write character to stdout
  [0x09] = { [0x100] = prv_int21_09xx, }, // DOS 1+ - Write string to stdout
  [0x2A] = { [0x100] = prv_int21_2Axx, }, // DOS 1+ - Get system date
  [0x2C] = { [0x100] = prv_int21_2Cxx, }, // DOS 1+ - Get system time
  [0x3C] = { [0x100] = prv_int21_3Cxx_3Dxx, }, // DOS 2+ - Create or truncate file
  [0x3D] = { [0x100] = prv_int21_3Cxx_3Dxx, }, // DOS 2+ - Open existing file
  [0x3E] = { [0x100] = prv_int21_3Exx, }, // DOS 2+ - Close file
  [0x3F] = { [0x100] = prv_int21_3Fxx_40xx, }, // DOS 2+ - Read from file
  [0x40] = { [0x100] = prv_int21_3Fxx_40xx, }, // DOS 2+ - Write to file
  [0x48] = { [0x100] = prv_int21_48xx, }, // DOS 2+ - Allocate memory
  [0x49] = { [0x100] = prv_int21_49xx, }, // DOS 2+ - Free memory
  [0x4A] = { [0x100] = prv_int21_4Axx, }, // DOS 2+ - Resize memory block
  [0x4C] = { [0x100] = prv_int21_4Cxx, }, // DOS 2+ - exit
};

BOOL bios_int21h(uint8 irq_num)
{
  uint16 aw;
  uint8 ah,al;
  BiosIrqHandler handler;
  (void)irq_num;

  handler = int_handler_fallback;
  aw = cpu_get_reg(CPU_REG_AW);
  ah = aw >> 8;
  al = aw & 0xFF;
  if(_irq21_handlers[ah][al] != NULL) {
    handler = _irq21_handlers[ah][al];
  }else if(_irq21_handlers[ah][0x100] != NULL) {
    handler = _irq21_handlers[ah][0x100];
  }
  return handler(irq_num, aw);
}
