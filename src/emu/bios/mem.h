#ifndef BIOS_MEM_H_
#define BIOS_MEM_H_

void bios_mem_init(void);
void bios_mem_finish(void);

uint8 bios_mem_alloc(uint16 size, uint16 *ptr);
uint8 bios_mem_free(uint16 para);
uint8 bios_mem_resize(uint16 size, uint16 para);

#endif
