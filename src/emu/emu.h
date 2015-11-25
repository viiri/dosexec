#ifndef EMU_EMU_H_
#define EMU_EMU_H_

/* Emulator state */
BOOL emu_init(void);
void emu_finish(void);

void emu_run(uint32 cyc);

#endif
