/*
**.intro: division by 10 in the range [0..2^32] using 32-bit shift pattern.
**
*/

#include <stdio.h>
#include <limits.h>

int main(void)
{
	unsigned int i;
	unsigned int max_over= 0;
	unsigned int max_under= 0;
	for (i= 0; i <= USHRT_MAX*10000; i++) {
		unsigned int exact_answer= i/10;
#if 0
		unsigned int shift_quotient= (i>>4) + (i>>5) + (i>>7) + (i>>8) + (i>>11) + (i>>12) + (i>>14);
#endif
		unsigned int shift_quotient= (i>>3) - (i>>5) + (i>>8) + (i>>9) + (i>>11) - (i>>13) + (i>>16);
		unsigned int approx= (shift_quotient<<3) + (shift_quotient<<1);
		unsigned int xxx;
		if (approx > i) {
			unsigned int diff= approx-i;
			xxx= shift_quotient - (diff-1)/10 -1;
			if (diff > max_over) {
				max_over= diff;
			}
		} else if (approx < i) {
			unsigned int diff= i-approx;
			xxx= shift_quotient + diff/10;
			if (diff > max_under) {
				max_under= diff;
			}
		} else {
			xxx= shift_quotient;
		}
		if (xxx != exact_answer) {
			printf("W %d %d %d %d %d\n", i, exact_answer, shift_quotient, approx, xxx);
		}
	}
	printf("under= %u, over= %u\n", max_under, max_over);
	return 0;
}
