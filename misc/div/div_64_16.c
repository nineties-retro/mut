/*
 *.intro: division by 10 in the range [0..2^64] in nibble/byte chunks.
 *
 */

#include <stdio.h>
#include <limits.h>

typedef struct {
	unsigned int hi;
	unsigned int lo;
} uint64;

static void uin64_out(uint64 ui)
{
	printf("(%u, %u)\n", ui.hi, ui.lo);
}

static uint64 uint64_from_ulong_long(unsigned long long x)
{
	uint64 ans;
	ans.hi= x/(((unsigned long long)ULONG_MAX)+1);
	ans.lo= x%(((unsigned long long)ULONG_MAX)+1);
	return ans;
}


static unsigned long long uint64_to_ulong_long(uint64 x)
{
	return (unsigned long long)x.hi*(((unsigned long long)ULONG_MAX)+1) + (unsigned long long)x.lo;
}


typedef unsigned int uint;

#define nibble_bits 4
#define nibble_mask ((1<<nibble_bits)-1)
#define byte_bits   (nibble_bits*2)
#define byte_mask   ((1<<byte_bits)-1)



static uint64 udiv(uint64 n)
{
	uint64 ans;
	unsigned int hi= n.hi;
	unsigned int lo= n.lo;
	unsigned int ans_hi= 0;
	unsigned int ans_lo= 0;
	unsigned int s=sizeof(uint)*byte_bits - nibble_bits;
	unsigned int mask= nibble_mask;
	unsigned int m;
	unsigned int i;
	do {
		unsigned int x= (hi>>s)&mask;
		unsigned int q= x/10;
		m= x%10;
		ans_hi= (ans_hi<<nibble_bits)|q;
		mask= (x == 0) ? ((mask<<nibble_bits)|mask) : byte_mask;
		hi= (m<<s) | (hi&~(nibble_mask<<s));
		s -= nibble_bits;
	} while (s != 0);

	/* least significant nibble of hi */
	{
		unsigned int x= hi&mask;
		unsigned int q= x/10;
		m= x%10;
		ans_hi= (ans_hi<<nibble_bits)|q;
	}

	/* most significant nibble of lo */
	mask= byte_mask;
	s= sizeof(uint)*byte_bits - nibble_bits;
	{
		unsigned int x= ((m<<nibble_bits)|(lo>>s))&mask;
		unsigned int q= x/10;
		m= x%10;
		ans_lo= (ans_lo<<nibble_bits)|q;
		mask= (x == 0) ? ((mask<<nibble_bits)|mask) : byte_mask;
		lo= (m<<s) | (lo&~(nibble_mask<<s));
		s -= nibble_bits;
	}

	for (i= 1; i < (sizeof(uint)*byte_bits)/nibble_bits; i += 1) {
		unsigned int x= (lo>>s)&mask;
		unsigned int q= x/10;
		m= x%10;
		ans_lo= (ans_lo<<nibble_bits)|q;
		mask= (x == 0) ? ((mask<<nibble_bits)|mask) : byte_mask;
		lo= (m<<s) | (lo&~(nibble_mask<<s));
		s -= nibble_bits;
	}
	ans.hi= ans_hi;
	ans.lo= ans_lo;
	return ans;
}



int main(void)
{
	unsigned long long i;
	for (i= ULONG_LONG_MAX-20; i < ULONG_LONG_MAX; i++) {
		unsigned long long exact_answer= i/10;
		uint64 uii= uint64_from_ulong_long(i);
		uint64 xxx= udiv(uii);
		unsigned long long yyy= uint64_to_ulong_long(xxx);
		if ((i % (1<<20)) == 0)
			printf("I= %Lu\n", i);
		if (yyy != exact_answer) {
			printf("W %Lu %Lu %Lu\n", i, exact_answer, yyy);
		}
	}
	return 0;
}
