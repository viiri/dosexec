#include "sys.h"
/*--- Core includes ---*/
#include "core/lptr.h"
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/bios/bios.h"
#include "emu/bios/bios_internal.h"
#include "emu/bios/mem.h"
#include "emu/cpu/cpu.h"
#include "emu/mem.h"

dosexec_bios_t _bios;

uint8 bios_r8(uint16 seg, uint16 ptr) {
  return mem_r8(LPTR2FLAT(seg, ptr), NULL);
}
uint16 bios_r16(uint16 seg, uint16 ptr) {
  return mem_r16(LPTR2FLAT(seg, ptr), NULL);
}
void bios_w8(uint16 seg, uint16 ptr, uint8 val) {
  mem_w8(LPTR2FLAT(seg, ptr), val, NULL);
}
void bios_w16(uint16 seg, uint16 ptr, uint16 val) {
  mem_w16(LPTR2FLAT(seg, ptr), val, NULL);
}
void bios_wlp(uint16 seg, uint16 ptr, uint16 vseg, uint16 vptr) {
  mem_w16(LPTR2FLAT(seg, ptr+0), vptr, NULL);
  mem_w16(LPTR2FLAT(seg, ptr+2), vseg, NULL);
}

BOOL bios_ret(uint8_t code) {
  if(code == DOSERR_NONE) {
    cpu_set_carry(FALSE);
    return TRUE;
  }
  cpu_set_carry(TRUE);
  cpu_set_reg(CPU_REG_AW, code);
  logdbg_pc("BIOS call failure: 0x%X\n", code);
  return FALSE;
}

BOOL int_handler_fallback(uint8 irq_num, uint16 aw)
{
  logerr_pc("Unknown INT %02Xh: %04X\n", irq_num, aw);
  bios_ret(DOSERR_BAD_FUNC_NUM);
  return TRUE;
}

static BOOL prv_int_fallback(uint8 irq_num)
{
  return int_handler_fallback(irq_num, cpu_get_reg(CPU_REG_AW));
}

static char *prv_bios_read_str(uint16 seg, uint16 ptr, int type) {
  size_t pbuf_size = 0x80;
  char *buf = malloc(pbuf_size);
  size_t idx = 0;
  char c;
  for(;;) {
    c = bios_r8(seg, ptr++);
    if(type == 0) {
      if(c == '$') break;
      if(c == '\0') continue;
    }else{
      if(c == '\0') break;
    }
    buf[idx++] = c;
    if(idx == pbuf_size) {
      pbuf_size *= 2;
      buf = realloc(buf, pbuf_size);
    }
  }
  buf[idx] = '\0';
  return buf;
}

char *bios_read_dollar(uint16 seg, uint16 ptr) {
  return prv_bios_read_str(seg, ptr, 0);
}
char *bios_read_asciz(uint16 seg, uint16 ptr) {
  return prv_bios_read_str(seg, ptr, 1);
}

void bios_init(void)
{
  bios_mem_init();

  if(bios_mem_alloc(0x1000, &_bios.internal_seg)) {
    logerr("Can't alloc BIOS internal segment.\n");
    return;
  }

  _bios.running = TRUE;
  _bios.ret_code = 0;

  cpu_hook_int(-1, prv_int_fallback);
  cpu_hook_int(0x21, bios_int21h);
}

void bios_finish(void)
{
  bios_mem_finish();
}
