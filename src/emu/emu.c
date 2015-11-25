#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/emu.h"
#include "emu/cpu/cpu.h"
#include "emu/mem.h"
#include "emu/bios/bios.h"

/* Emulator state */
BOOL emu_init(void)
{
  mem_init();
  cpu_init();
  bios_init();
  return TRUE;
}

void emu_finish(void)
{
  bios_finish();
  cpu_finish();
  mem_finish();
}

void emu_run(uint32 cyc)
{
  cpu_update(cyc);
}
