#ifndef MUT_MEM_H
#define MUT_MEM_H

#include "mut_stdlib.h"

#define mut_mem_malloc(n_bytes)        malloc(n_bytes)
#define mut_mem_realloc(addr, n_bytes) realloc(addr, n_bytes)
#define mut_mem_free(addr)             free(addr)

#endif
