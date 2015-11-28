#ifndef CORE_LPTR_H_
#define CORE_LPTR_H_

#define LPTR2FLAT(SEG,PTR) (((SEG) << 4) + ((PTR) & 0xFFFF))
#define SIZE2PARA(SIZE) (((SIZE) + 0xF) >> 4)

#endif
