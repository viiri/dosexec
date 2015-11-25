#ifndef EMU_MEM_H_
#define EMU_MEM_H_

extern uint8 *_mem_ram;
extern size_t _mem_size;

void mem_init(void);
void mem_finish(void);

uint8 mem_r8(uint32 addr, int *waitstates);
uint16 mem_r16(uint32 addr, int *waitstates);
void mem_w8(uint32 addr, uint8 data, int *waitstates);
void mem_w16(uint32 addr, uint16 data, int *waitstates);

uint8 mem_r8_io(uint16 addr);
uint16 mem_r16_io(uint16 addr);
void mem_w8_io(uint16 addr, uint8 data);
void mem_w16_io(uint16 addr, uint16 data);

#endif
