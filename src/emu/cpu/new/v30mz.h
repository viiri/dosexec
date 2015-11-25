#ifndef EMU_V30MZ_H_
#define EMU_V30MZ_H_

#if BIGENDIAN
#define PAIR8(l,h) h,l
#else
#define PAIR8(l,h) l,h
#endif

enum {
  INTVEC_DIVERR = 0,
  INTVEC_BRK = 1,
  INTVEC_NMI = 2,
  INTVEC_BRK3 = 3,
  INTVEC_BRKV = 4,
  INTVEC_BOUND = 5,
};

enum {
  SREG_ES = 0,
  SREG_CS = 1,
  SREG_SS = 2,
  SREG_DS = 3,
};

enum {
  MEMOPER_R8,
  MEMOPER_R16,
  MEMOPER_W8,
  MEMOPER_W16,
  MEMOPER_IOR8,
  MEMOPER_IOR16,
  MEMOPER_IOW8,
  MEMOPER_IOW16,
};

typedef struct {
  int32 cycles;
  int bus_owned;
  BOOL halted;

  int irq;
  BOOL irq_disable;

  uint8 insn;
  uint16 insn_ip;
  uint16 fault_ip;
  BOOL was_prefix;

  BOOL rep_zflag;

  BOOL pfx_reset;
  int segpfx;

  struct {
    BOOL cy;  /* bit0: Carry */
    BOOL p;   /* bit2: Parity */
    BOOL ac;  /* bit4: Aux Carry */
    BOOL z;   /* bit6: Zero */
    BOOL s;   /* bit7: Sign */
    BOOL brk; /* bit8: Break */
    BOOL ie;  /* bit9: Interrupt Enable */
    BOOL dir; /* bit10:Direction */
    BOOL v;   /* bit11:Overflow */
    BOOL md;  /* bit15:Mode (unused) */
  } psw;
  struct {
    union {
      struct { /* named 8bit */
        uint8 PAIR8(al, ah), PAIR8(cl, ch), PAIR8(dl, dh), PAIR8(bl, bh);
      };
      struct { /* named 16bit */
        uint16 aw, cw, dw, bw;
        uint16 sp, bp, ix, iy;
        uint16 es, cs, ss, ds;
      };
      struct { /* indexed 8bit */
        uint8 r8[16];
        uint8 sreg8[8];
      };
      struct {
        uint16 ar8[24];
      };
      struct { /* indexed 16bit */
        uint16 r16[8];
        uint16 sreg[4];
      };
      struct {
        uint16 ar16[12];
      };
    };
    uint16 ip;
  } reg;
  struct {
    int cycles;

    uint8 modrm;
    uint32 src0,src1,dst;

    uint16 ea;
    int ea_seg;
  } exec;
} wswan_v30mz_t;

#define SegBase(SEG) (_cpu.reg.sreg[SEG] << 4)
#define SegAddr(SEG,BASE) ((SegBase(SEG)) + (BASE))
#define SegDefault(SEG) ((_cpu.segpfx != -1) ? _cpu.segpfx : (SEG))

#include "emu/cpu/cpu.h"

#endif
