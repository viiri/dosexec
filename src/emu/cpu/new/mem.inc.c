#define AREA_IS_8BIT(addr) (((addr) & 0xF0000) == 0x10000)

uint8 cpu_memr8(uint32 addr)
{
  return mem_r8(addr, NULL);
}
uint16 cpu_memr16(uint32 addr)
{
  if((addr & 1) || AREA_IS_8BIT(addr)) {
    return cpu_memr8(addr+0) | (cpu_memr8(addr+1) << 8);
  }
  return mem_r16(addr, NULL);
}
void cpu_memw8(uint32 addr, uint8 data)
{
  mem_w8(addr, data, NULL);
}
void cpu_memw16(uint32 addr, uint16 data)
{
  if((addr & 1) || AREA_IS_8BIT(addr)) {
    cpu_memw8(addr+0, data >> 0);
    cpu_memw8(addr+1, data >> 8);
    return;
  }
  mem_w16(addr, data, NULL);
}
uint8 cpu_memr8io(uint32 addr)
{
  return mem_r8_io(addr);
}
uint16 cpu_memr16io(uint32 addr)
{
  return mem_r16_io(addr);
}
void cpu_memw8io(uint32 addr, uint8 data)
{
  mem_w8_io(addr, data);
}
void cpu_memw16io(uint32 addr, uint16 data)
{
  mem_w16_io(addr, data);
}
uint8 cpu_fetch8(void)
{
  _cpu.reg.ip++;
  return mem_r8(SegAddr(SREG_CS, _cpu.reg.ip-1), NULL);
}
uint16 cpu_fetch16(void)
{
  _cpu.reg.ip += 2;
  return mem_r16(SegAddr(SREG_CS, _cpu.reg.ip-2), NULL);
}
void cpu_push(uint16 val)
{
  _cpu.reg.sp -= 2;
  cpu_memw16(SegAddr(SREG_SS,_cpu.reg.sp), val);
}
uint16 cpu_pop(void)
{
  _cpu.reg.sp += 2;
  return cpu_memr16(SegAddr(SREG_SS,_cpu.reg.sp-2));
}
void cpu_push_psw(void)
{
  uint16 psw = cpu_get_psw();
  cpu_push(psw);
}
