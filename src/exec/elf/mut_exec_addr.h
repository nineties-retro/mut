#ifndef MUT_EXEC_ADDR_H
#define MUT_EXEC_ADDR_H

#include "mut_elf.h"		/* Elf32_Addr */

typedef Elf32_Addr mut_exec_addr;

#define mut_exec_addr_to_int(xx) (xx)
#define mut_exec_addr_from_ulong(xx) (xx)
#define mut_exec_addr_from_int(xx) (xx)
#define mut_exec_addr_to_voidp(xx) ((void*)(xx))

#define mut_exec_addr_inc(xx, d) ((xx)+(d))
#define mut_exec_addr_lt(xx, yyy) ((xx)<(yy))
#define mut_exec_addr_aligned(xx, a) (((xx)&((a)-1)) == 0)

#endif
