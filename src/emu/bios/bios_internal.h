#ifndef BIOS_INTERNAL_H_
#define BIOS_INTERNAL_H_

enum {
  DOSERR_NONE = 0,
  DOSERR_BAD_FUNC_NUM,
  DOSERR_FILE_NOT_FOUND,
  DOSERR_PATH_NOT_FOUND,
  DOSERR_NO_FREE_HANDLES,
  DOSERR_ACCESS_DENIED,
  DOSERR_INVALID_HANDLE,
  DOSERR_MCB_DESTROYED,
  DOSERR_OUT_OF_MEMORY,
  DOSERR_MBA_INVALID,
};

typedef BOOL (*BiosIrqHandler)(uint8 irq_num, uint16 aw);
BOOL bios_ret(uint8_t code);
BOOL int_handler_fallback(uint8 irq_num, uint16 aw);

char *bios_read_dollar(uint16 seg, uint16 ptr);
char *bios_read_asciz(uint16 seg, uint16 ptr);

BOOL bios_int21h(uint8 irq_num);

#endif
