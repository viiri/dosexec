#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/cpu/new/v30mz.h"
#include "emu/mem.h"
#include "emu/emu.h"

static wswan_v30mz_t _cpu;

static BOOL _cpu_parity[256];
static int _cpu_modrm_regb[256];
static int _cpu_modrm_regw[256];
static int _cpu_modrm_rmb[256];
static int _cpu_modrm_rmw[256];
/* To properly dereference r8 */
static const int _cpu_regb_list[8] = {
#if BIGENDIAN
  1,3,5,7,0,2,4,6
#else
  0,2,4,6,1,3,5,7
#endif
};

static CpuIrqHandler _cpu_irq_handlers[256];

uint16 cpu_get_psw(void)
{
  return  (_cpu.psw.cy << 0) |
          (_cpu.psw.p  << 2) |
          (_cpu.psw.ac << 4) |
          (_cpu.psw.z  << 6) |
          (_cpu.psw.s  << 7) |
          (_cpu.psw.brk<< 8) |
          (_cpu.psw.ie << 9) |
          (_cpu.psw.dir<<10) |
          (_cpu.psw.v  <<11) |
#if UNIT_TESTS
          0x0002;
#else
          (_cpu.psw.md <<15) |
          0x7002;
#endif
}
void cpu_set_psw(uint16 val)
{
  _cpu.psw.cy = (val >> 0) & 1;
  _cpu.psw.p  = (val >> 2) & 1;
  _cpu.psw.ac = (val >> 4) & 1;
  _cpu.psw.z  = (val >> 6) & 1;
  _cpu.psw.s  = (val >> 7) & 1;
  _cpu.psw.brk= (val >> 8) & 1;
  _cpu.psw.ie = (val >> 9) & 1;
  _cpu.psw.dir= (val >>10) & 1;
  _cpu.psw.v  = (val >>11) & 1;
  _cpu.psw.md = (val >>15) & 1;
}

void cpu_init(void)
{
  int i, l, tmp;
  /* Table generation */
  /* Parity table */
  for(i = 0; i < 256; i++) {
    _cpu_irq_handlers[i] = NULL;
  }
  for(i = 0; i < 256; i++) {
    tmp = 0;
    for(l = i; l > 0; l >>= 1)
      tmp += (l & 1);
    _cpu_parity[i] = tmp & 1 ? FALSE : TRUE;
  }
  /* ModR/M register table */
  for(i = 0; i < 256; i++) {
    _cpu_modrm_regb[i] = _cpu_regb_list[(i >> 3) & 7];
    _cpu_modrm_regw[i] =                (i >> 3) & 7;
    _cpu_modrm_rmb [i] = _cpu_regb_list[(i >> 0) & 7];
    _cpu_modrm_rmw [i] =                (i >> 0) & 7;
  }

  cpu_reset();
}
void cpu_reset(void)
{
  _cpu.cycles = 0;
  _cpu.halted = FALSE;
  _cpu.bus_owned = 0;

  _cpu.pfx_reset = TRUE;
  _cpu.segpfx = -1;
  _cpu.was_prefix = FALSE;

  _cpu.irq = 0;
  _cpu.irq_disable = FALSE;

  _cpu.reg.ip = 0x0000;
  _cpu.reg.cs = 0xFFFF;
  _cpu.reg.ds = 0x0000;
  _cpu.reg.ss = 0x0000;
  _cpu.reg.es = 0x0000;

  /* TODO: Randomize register contents? */

  cpu_set_psw(0);
  _cpu.psw.md = TRUE;
}
void cpu_finish(void)
{
}

#include "emu/cpu/new/mem.inc.c"
#include "emu/cpu/new/irq.inc.c"
#include "emu/cpu/new/exec.inc.c"

int cpu_cycle(void)
{
  int cyc = 0;

  if(_cpu.pfx_reset)
    _cpu.segpfx = -1;

  if(!_cpu.was_prefix)
    cpu_check_irq();

  _cpu.irq_disable = FALSE;
  _cpu.insn_ip = _cpu.reg.ip;
  if(!_cpu.was_prefix)
    _cpu.fault_ip = _cpu.reg.ip;

  _cpu.was_prefix = FALSE;
  _cpu.pfx_reset = TRUE;
  _cpu.exec.ea_seg = -1;

  _cpu.insn = cpu_fetch8();
  cyc += cpu_exec();

  return cyc;
}

void cpu_update(int32 cycles)
{
  _cpu.cycles += cycles;
  while(_cpu.cycles >= 1) {
    if(_cpu.halted && cpu_has_irq() == 0) {
      _cpu.cycles = 0;
      break;
    }
    if(_cpu.bus_owned) {
      _cpu.cycles = 0;
      break;
    }
    //cpu_regdump();
    _cpu.cycles -= cpu_cycle();
  }
}

void cpu_bus_req(BOOL state)
{
  if(state)
    _cpu.bus_owned++;
  else
    _cpu.bus_owned--;
}

void cpu_irq(int state)
{
  _cpu.irq = state;
}

uint32 cpu_get_pc(void)
{
  return (SegBase(SREG_CS)<<12) | _cpu.insn_ip;
}
void cpu_regdump_mask(uint32 regs)
{
  int i;
  const char *regname[8] = {
    "AW","CW","DW","BW","SP","BP","IX","IY"
  };
  const char *sregname[4] = {
    "ES","CS","SS","DS"
  };
  for(i = 0; i < 8; i++) {
    if(~regs & (1 << (i+0)))
      continue;
    loginfo("%s: %04X  ", regname[i], _cpu.reg.r16[i]);
  }
  loginfo("\n");
  for(i = 0; i < 4; i++) {
    if(~regs & (1 << (i+8)))
      continue;
    loginfo("%s: %04X  ", sregname[i], _cpu.reg.sreg[i]);
  }
  loginfo("\n");
  if(regs & (1 << 12))
    loginfo("IP: %04X\n", _cpu.reg.ip);
  if(regs & (1 << 13))
    loginfo("PSW:%04X\n", cpu_get_psw());
}
void cpu_regdump(void)
{
  cpu_regdump_mask(0xFFFF);
}

uint16 cpu_get_reg(int reg)
{
  switch(reg) {
    case CPU_REG_IP:
      return _cpu.reg.ip;
    case CPU_REG_PSW:
      return cpu_get_psw();
    default:
      return _cpu.reg.ar16[reg];
  }
}
void cpu_set_reg(int reg, uint16 val)
{
  switch(reg) {
    case CPU_REG_IP:
      _cpu.reg.ip = val;
      break;
    case CPU_REG_PSW:
      cpu_set_psw(val);
      break;
    default:
      _cpu.reg.ar16[reg] = val;
      break;
  }
}

BOOL cpu_get_carry(void)
{
  return _cpu.psw.cy;
}
void cpu_set_carry(BOOL set)
{
  _cpu.psw.cy = set;
}

void cpu_hook_int(int irq_num, CpuIrqHandler irqh)
{
  if(irq_num < 0) {
    for(irq_num = 0; irq_num < 0x100; irq_num++) {
      cpu_hook_int(irq_num, irqh);
    }
    return;
  }
  _cpu_irq_handlers[irq_num] = irqh;
}
