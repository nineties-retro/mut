#ifndef MUT_COUNTER_H
#define MUT_COUNTER_H

#include "mut_uint64.h"

typedef mut_uint64 mut_counter;

#define mut_counter_from_uint(ui) mut_uint64_from_uint(ui)
#define mut_counter_inc(c, d) mut_uint64_inc(c, d)
#define mut_counter_dec(c, d) mut_uint64_dec(c, d)
#define mut_counter_lt(a, b) mut_uint64_lt(a, b)

#endif
