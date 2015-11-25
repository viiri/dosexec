#ifndef EMU_CPU_H_
#define EMU_CPU_H_

void cpu_init(void);
void cpu_reset(void);
void cpu_finish(void);
void cpu_update(int32 cycles);

void cpu_bus_req(BOOL state); /* FALSE=Unrequest, TRUE=Request */

void cpu_irq(int state);

uint32 cpu_get_pc(void);
void cpu_regdump_mask(uint32 mask);
void cpu_regdump(void);

enum {
  CPU_REG_AW,
  CPU_REG_CW,
  CPU_REG_DW,
  CPU_REG_BW,
  CPU_REG_SP,
  CPU_REG_BP,
  CPU_REG_IX,
  CPU_REG_IY,
  CPU_REG_ES,
  CPU_REG_CS,
  CPU_REG_SS,
  CPU_REG_DS,

  CPU_REG_IP,
  CPU_REG_PSW,
};

uint16 cpu_get_reg(int reg);
void cpu_set_reg(int reg, uint16 val);

BOOL cpu_get_carry(void);
void cpu_set_carry(BOOL set);

typedef BOOL (*CpuIrqHandler)(uint8 irq_num);
void cpu_hook_int(int irq_num, CpuIrqHandler irqh);

#endif
