#ifndef MUT_UINT64_H
#define MUT_UINT64_H

typedef unsigned long long mut_uint64;

#define mut_uint64_from_uint(ui) ((mut_uint64)(ui))
#define mut_uint64_inc(ui, i) ((ui)+(i))
#define mut_uint64_dec(ui, i) ((ui)-(i))
#define mut_uint64_lt(a, b) ((a) < (b))

#endif
