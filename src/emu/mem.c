#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/regs.h"
#include "emu/emu.h"
#include "emu/mem.h"
#include "emu/cpu/cpu.h"

#define IO_DBG_LO 0x0
#define IO_DBG_HI 0x100

#ifdef IO_DBG
#define IORD_DBG IO_DBG
#define IOWR_DBG IO_DBG
#endif
#ifdef IO_DBG_LO
#define IORD_DBG_LO IO_DBG_LO
#define IOWR_DBG_LO IO_DBG_LO
#endif
#ifdef IO_DBG_HI
#define IORD_DBG_HI IO_DBG_HI
#define IOWR_DBG_HI IO_DBG_HI
#endif
#ifdef IORD_DBG
#define IORD_DBG_LO IORD_DBG
#define IORD_DBG_HI IORD_DBG
#endif
#ifdef IOWR_DBG
#define IOWR_DBG_LO IOWR_DBG
#define IOWR_DBG_HI IOWR_DBG
#endif
#ifndef IORD_DBG_LO
#define IORD_DBG_LO 0x100
#endif
#ifndef IORD_DBG_HI
#define IORD_DBG_HI 0x100
#endif
#ifndef IOWR_DBG_LO
#define IOWR_DBG_LO 0x100
#endif
#ifndef IOWR_DBG_HI
#define IOWR_DBG_HI 0x100
#endif

uint8 *_mem_ram;
size_t _mem_size;

void mem_init(void)
{
  size_t i;
  _mem_size = 0x10000 << 4;

  _mem_ram = malloc(_mem_size);
  for(i = 0; i < _mem_size; i++) {
    mem_w8(i, 0, NULL);
  }
}

void mem_finish(void)
{
  if(_mem_ram != NULL) {
    free(_mem_ram);
    _mem_ram = NULL;
  }
}

uint8 mem_r8(uint32 addr, int *waitstates)
{
  (void)waitstates;
  return _mem_ram[addr];
}
uint16 mem_r16(uint32 addr, int *waitstates)
{
  (void)waitstates;
#if BIGENDIAN || 1
  uint16 res;
  res  = mem_r8(addr, NULL);
  res |= mem_r8(addr+1, NULL) << 8;
  return res;
#else
  return *(uint16*)(_mem_ram + addr);
#endif
}

void mem_w8(uint32 addr, uint8 data, int *waitstates)
{
  (void)waitstates;
  _mem_ram[addr] = data;
}
void mem_w16(uint32 addr, uint16 data, int *waitstates)
{
  (void)waitstates;
#if BIGENDIAN || 1
  mem_w8(addr+0, data >> 0, NULL);
  mem_w8(addr+1, data >> 8, NULL);
#else
  *(uint16*)(_mem_ram + addr) = data;
#endif
}

uint8 mem_r8_io(uint16 addr)
{
  uint8 res = 0;
  // TODO: hooks
  if((int16)addr >= IORD_DBG_LO && addr <= IORD_DBG_HI)
    dbgcprint("IORD:%02X=>%02X\n", addr, res);
  return res;
}
uint16 mem_r16_io(uint16 addr)
{
  uint16 ret;
  ret  = mem_r8_io(addr+0);
  ret |= mem_r8_io(addr+1) << 8;
  return ret;
}
void mem_w8_io(uint16 addr, uint8 data)
{
  if((int16)addr >= IOWR_DBG_LO && addr <= IOWR_DBG_HI)
    dbgcprint("IOWR:%02X<=%02X\n", addr, data);
  // TODO: hooks
}
void mem_w16_io(uint16 addr, uint16 data)
{
  mem_w8_io(addr+0, data >> 0);
  mem_w8_io(addr+1, data >> 8);
}
