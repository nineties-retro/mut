/*
 *.intro: division by 10 in the range [0..2^32] using 8-bit shift pattern.
 *
 */

#include <stdio.h>
#include <limits.h>

int main(void)
{
	unsigned int i;
	unsigned int max_over= 0;
	unsigned int max_under= 0;
	for (i= 0; i <= USHRT_MAX*1000; i++) {
		unsigned int exact_answer= i/10;
		unsigned int shift_answer= (i>>4) + (i>>5) + (i>>7);
		unsigned int approx= (shift_answer<<3) + (shift_answer<<1);
		unsigned int xxx;
		unsigned int diff;
		if (approx > i) {
			diff= approx-i;
			xxx= shift_answer - (diff-1)/10 -1;
			if (diff > max_over) max_over= diff;
		} else if (approx < i) {
			diff= i-approx;
			xxx= shift_answer + diff/10;
			if (diff > max_under) max_under= diff;
		} else {
			diff= 0;
			xxx= shift_answer;
		}
		if (xxx != exact_answer)
			printf("%d %d %d %d %d %d\n", i, exact_answer, shift_answer, approx, diff, xxx);
	}
	printf("under= %u, over= %u\n", max_under, max_over);
	return 0;
}
