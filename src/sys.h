#ifndef SYS_H_
#define SYS_H_

#if defined(_WIN32) || defined(__CYGWIN__)
# ifndef WIN32
#  define WIN32 1
# endif
#else
# ifndef WIN32
#  define WIN32 0
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#define BIGENDIAN FALSE
#define DBG_WARNINGS FALSE

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int BOOL;
enum {
  FALSE = 0,
  TRUE = 1,
};

#define INLINE static inline
//#define INLINE static

#define RISING_EDGE(old,new,mask) (((~(old) & (mask)) & ((new) & (mask))) == (mask))
#define FALLING_EDGE(old,new,mask) RISING_EDGE(new,old,mask)

#define SET_LO8(var, data) do { var &= ~(0xFF); var |= data & 0xFF; } while(0)
#define SET_HI8(var, data) do { var &= ~(0xFF << 8); var |= (data & 0xFF) << 8; } while(0)
#define SET_HR8(var, data) do { var &= ~(0xFF << 16); var |= (data & 0xFF) << 16; } while(0)
#define GET_LO8(var) ((var >> 0) & 0xFF)
#define GET_HI8(var) ((var >> 8) & 0xFF)
#define GET_HR8(var) ((var >> 16) & 0xFF)

#define REG_SET16(reg, var, data) do { \
  if(reg & 1) SET_HI8(var, data); \
  else        SET_LO8(var, data); \
} while(0)

#define REG_GET16(reg, var) ((reg & 1) ? (var >> 8) : (var >> 0))

#include "core/util.h"

uint32 cpu_get_pc(void);

#endif
