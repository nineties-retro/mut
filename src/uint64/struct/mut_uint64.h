#ifndef MUT_UINT64_H
#define MUT_UINT64_H

struct mut_uint64 {
	unsigned int hi;
	unsigned int lo;
};

typedef struct mut_uint64 mut_uint64;

mut_uint64 mut_uint64_from_uint(unsigned int);

mut_uint64 mut_uint64_inc(mut_uint64, unsigned int);

mut_uint64 mut_uint64_dec(mut_uint64, unsigned int);

int mut_uint64_lt(mut_uint64, mut_uint64);

#endif
