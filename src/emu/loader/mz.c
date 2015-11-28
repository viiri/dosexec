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

struct EXE {
  unsigned short signature; /* == 0x5a4D */
  unsigned short bytes_in_last_block;
  unsigned short blocks_in_file;
  unsigned short num_relocs;
  unsigned short header_paragraphs;
  unsigned short min_extra_paragraphs;
  unsigned short max_extra_paragraphs;
  unsigned short ss;
  unsigned short sp;
  unsigned short checksum;
  unsigned short ip;
  unsigned short cs;
  unsigned short reloc_table_offset;
  unsigned short overlay_number;
};

struct EXE_RELOC {
  unsigned short offset;
  unsigned short segment;
};

BOOL load_mz(DosExecState *dos, FILE *fp)
{
  size_t i;
  struct EXE exe;
  fread(&exe, sizeof(struct EXE), 1, fp);

  size_t exesize = exe.blocks_in_file * 0x200;
  if(exe.bytes_in_last_block)
    exesize = exesize - 0x200 + exe.bytes_in_last_block;
  exesize -= exe.header_paragraphs * 0x10;

  dos->codelen = exesize + (exe.min_extra_paragraphs << 4) + 0x100;
  dos->codelen += 0x1000; // HACK
  if(bios_mem_alloc(SIZE2PARA(dos->codelen), &dos->codeseg)) {
    logerr("Can't alloc code\n");
    return FALSE;
  }
  dos->pspseg = dos->codeseg;
  dos->codelen -= 0x100;
  dos->codeseg += 0x10;
  loginfo("CODE: %04X(%04X)\n", dos->codeseg, SIZE2PARA(dos->codelen));

  fseek(fp, exe.header_paragraphs * 0x10, SEEK_SET);
  uint32_t ptr = LPTR2FLAT(dos->codeseg, 0);
  for(i = 0; i < exesize; i++) {
    uint8 byte;
    fread(&byte, 1, 1, fp);
    mem_w8(ptr, byte, NULL);
    ptr++;
  }

  // Apply relocations
  fseek(fp, exe.reloc_table_offset, SEEK_SET);
  for(i = 0; i < exe.num_relocs; i++) {
    struct EXE_RELOC reloc;
    fread(&reloc, sizeof(struct EXE_RELOC), 1, fp);
    reloc.segment += dos->codeseg;
    bios_w16(reloc.segment,reloc.offset, dos->codeseg + bios_r16(reloc.segment,reloc.offset));
  }

  cpu_set_reg(CPU_REG_SS, dos->codeseg + exe.ss);
  cpu_set_reg(CPU_REG_SP, exe.sp);
  cpu_set_reg(CPU_REG_IP, exe.ip);
  cpu_set_reg(CPU_REG_CS, dos->codeseg + exe.cs);

  return TRUE;
}

BOOL start_mz(DosExecState *dos)
{
  cpu_set_reg(CPU_REG_DS, dos->pspseg);
  cpu_set_reg(CPU_REG_ES, dos->pspseg);
  return TRUE;
}
