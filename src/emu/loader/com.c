#include "sys.h"
/*--- Core includes ---*/
#include "core/util.h"
/*--- Emu includes ---*/
#include "emu/emu.h"
#include "emu/mem.h"
#include "emu/cpu/cpu.h"
#include "emu/bios/bios.h"
#include "emu/bios/mem.h"
#include "emu/loader/loader.h"
#include "core/lptr.h"

BOOL load_com(DosExecState *dos, FILE *fp)
{
  size_t i;
  fseek(fp, 0, SEEK_END);
  dos->codelen = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t segsize = SIZE2PARA(dos->codelen + 0x100);
  if(bios_mem_alloc(segsize, &dos->codeseg)) {
    logerr("Can't alloc code\n");
    return FALSE;
  }
  loginfo("CODE: %04X(%04X)\n", dos->codeseg, segsize);

  uint32 ptr = 0x100;
  for(i = 0; i < dos->codelen; i++) {
    uint8 byte;
    fread(&byte, 1, 1, fp);
    bios_w8(dos->codeseg,ptr, byte);
    ptr++;
  }
  dos->pspseg = dos->codeseg;
  return TRUE;
}

BOOL start_com(DosExecState *dos)
{
  cpu_set_reg(CPU_REG_CS, dos->codeseg);
  cpu_set_reg(CPU_REG_IP, 0x100);
  cpu_set_reg(CPU_REG_DS, dos->pspseg);
  cpu_set_reg(CPU_REG_ES, dos->pspseg);
  return TRUE;
}
