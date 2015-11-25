#ifndef EMU_BIOS_H_
#define EMU_BIOS_H_

typedef struct {
  BOOL running;
  uint8 ret_code;
} dosexec_bios_t;

extern dosexec_bios_t _bios;

void bios_init(void);
void bios_finish(void);

uint8 bios_r8(uint16 seg, uint16 ptr);
uint16 bios_r16(uint16 seg, uint16 ptr);
void bios_w8(uint16 seg, uint16 ptr, uint8 val);
void bios_w16(uint16 seg, uint16 ptr, uint16 val);
void bios_wlp(uint16 seg, uint16 ptr, uint16 vseg, uint16 vptr);

#endif
