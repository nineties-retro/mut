/*
 *.intro: division by 10 in the range [0..2^32] using nibble/byte division.
 *
 */

#include <stdio.h>
#include <limits.h>

typedef unsigned int uint;

#define nibble_bits 4
#define nibble_mask ((1<<nibble_bits)-1)
#define byte_bits   (nibble_bits*2)
#define byte_mask   ((1<<byte_bits)-1)

static unsigned int udiv32(unsigned int n)
{
	unsigned int ans= 0;
	unsigned int s=sizeof(uint)*byte_bits - nibble_bits;
	unsigned int mask= nibble_mask;
	unsigned int i;
	for (i= 0; i < (sizeof(uint)*byte_bits)/nibble_bits; i += 1) {
		unsigned int x= (n>>s)&mask;
		unsigned int q= x/10;
		unsigned int m= x%10;
		ans= (ans<<nibble_bits)|q;
		mask= (x == 0) ? ((mask<<(nibble_bits))|mask) : byte_mask;
		n= (m<<s) | (n&~(nibble_mask<<s));
		s -= nibble_bits;
	}
	return ans;
}



int main(void)
{
	unsigned int i;
	for (i= 0x6000; i < 0x6010; i++) {
		unsigned int exact_answer= i/10;
		unsigned int xxx= udiv32(i);
		if (xxx != exact_answer) {
			printf("W %d %d %d\n", i, exact_answer, xxx);
		}
		printf("%d %d %d\n", i, exact_answer, xxx);
	}
	return 0;
}
