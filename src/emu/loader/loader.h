#ifndef EMU_LOADER_LOADER_H_
#define EMU_LOADER_LOADER_H_

enum {
  DosProgram_COM,
  DosProgram_MZ,
};

typedef struct {
  uint16 pspseg;
  uint16 codeseg;
  uint16 envseg;

  size_t codelen;

  const char *prg;
  int prgtype;

  int argc;
  const char **argv;

  size_t envsize;
  const char **env;
} DosExecState;

BOOL loader(DosExecState *dos, const char *prg, int argc, const char *argv[]);

BOOL load_com(DosExecState *dos, FILE *fp);
BOOL start_com(DosExecState *dos);

BOOL load_mz(DosExecState *dos, FILE *fp);
BOOL start_mz(DosExecState *dos);

#endif
