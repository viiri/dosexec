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

static BOOL prv_build_psp(DosExecState *dos)
{
  int i;
  bios_w16(dos->pspseg,0x00, 0x20CD); // INT 20h
  bios_w16(dos->pspseg,0x02, dos->codeseg + SIZE2PARA(dos->codelen) + 1); // Program ending address
  bios_w8 (dos->pspseg,0x05, 0xC3); // DOS function dispatcher (just returns)
  bios_wlp(dos->pspseg,0x0A, dos->pspseg, 0x0005); // Program termination code ptr
  bios_wlp(dos->pspseg,0x0E, dos->pspseg, 0x0005); // Break handler code ptr
  bios_wlp(dos->pspseg,0x12, dos->pspseg, 0x0005); // Critical error code ptr
  bios_w16(dos->pspseg,0x2C, dos->envseg);
  bios_w16(dos->pspseg,0x50, 0x21CD); // INT 21h
  bios_w8 (dos->pspseg,0x52, 0xCB); // RETF

  size_t arglen = 0;
  for(i = 0; i < dos->argc; i++) {
    bios_w8(dos->pspseg,0x81+arglen, ' ');
    arglen++;

    const char *ptr = dos->argv[i];
    while(*ptr) {
      bios_w8(dos->pspseg,0x81+arglen, *ptr);
      ptr++;
      arglen++;
    }
  }
  bios_w8(dos->pspseg,0x81+arglen, 0x0D);
  bios_w8(dos->pspseg,0x80, arglen);

  return TRUE;
}

static BOOL prv_build_env(DosExecState *dos)
{
  size_t i;

  dos->envsize = 0;
  for(i = 0; dos->env[i][0] != '\0'; i++) {
    dos->envsize += strlen(dos->env[i]) + 1;
  }
  if(dos->envsize == 0)
    dos->envsize = 1;
  if(bios_mem_alloc(SIZE2PARA(dos->envsize), &dos->envseg)) {
    logerr("Can't allocate ENV memory\n");
    return FALSE;
  }
  loginfo("ENV: %04X\n", dos->envseg);

  uint16 ofs = 0;
  for(i = 0; dos->env[i][0] != '\0'; i++) {
    size_t l;
    for(l = 0; dos->env[i][l] != '\0'; l++) {
      bios_w8(dos->envseg, ofs++, dos->env[i][l]);
    }
    bios_w8(dos->envseg,ofs++, 0);
  }

  return TRUE;
}

static BOOL prv_load_program(DosExecState *dos)
{
  FILE *fp;
  fp = fopen(dos->prg, "rb");
  if(fp == NULL) {
    logerr("Can't open program\n");
    return FALSE;
  }
  uint16 magic;
  fread(&magic, 2, 1, fp);
  fseek(fp, 0, SEEK_SET);

  if(magic == 0x5A4D) { // MZ
    dos->prgtype = DosProgram_MZ;
    logdbg("Loading MZ\n");
    return load_mz(dos, fp);
  }else{
    dos->prgtype = DosProgram_COM;
    logdbg("Loading COM\n");
    return load_com(dos, fp);
  }
}

static BOOL prv_start_program(DosExecState *dos)
{
  switch(dos->prgtype) {
    case DosProgram_MZ:
      return start_mz(dos);
    case DosProgram_COM:
      return start_com(dos);
  }
  return FALSE;
}

const char *s_empty_env[] = {
  "",
};

BOOL loader(DosExecState *dos, const char *prg, int argc, const char *argv[])
{
  dos->env = s_empty_env;
  dos->prg = prg;
  dos->argc = argc;
  dos->argv = argv;

  if(!prv_build_env(dos)) {
    logerr("Can't build ENV\n");
    return FALSE;
  }

  dos->pspseg = 0xFFFF;
  if(!prv_load_program(dos)) {
    logerr("Can't load program\n");
    return FALSE;
  }
  if(dos->pspseg == 0xFFFF) { // Not set by the loader
    if(bios_mem_alloc(0x10, &dos->pspseg)) {
      logerr("Can't allocate PSP memory\n");
      return FALSE;
    }
  }
  loginfo("PSP: %04X\n", dos->pspseg);
  if(!prv_build_psp(dos)) {
    logerr("Can't build PSP\n");
    return FALSE;
  }
  if(!prv_start_program(dos)) {
    logerr("Can't start program\n");
    return FALSE;
  }

  return TRUE;
}
