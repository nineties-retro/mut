#include <stdio.h>		/* fprintf */
#include <limits.h>		/* ULONG_MAX */
#include "mut_assert.h"		/* mut_assert_check */
#include "mut_uint64.h"
#include "mut_uint64_out.h"
#include "mut_errno.h"

mut_uint64 mut_uint64_from_uint(unsigned int ui)
{
	mut_uint64 xui;
	xui.hi= 0;
	xui.lo= ui;
	return xui;
}


mut_uint64 mut_uint64_inc(mut_uint64 xui, unsigned int ui)
{
	mut_uint64 ans;

	if (ULONG_MAX - xui.lo < ui) { /* overflow */
		ans.lo = (xui.lo > ui)
			? ui - (ULONG_MAX - xui.lo) - 1
			: xui.lo - (ULONG_MAX - ui) - 1;
		ans.hi += 1;
	} else {
		ans.lo = xui.lo + ui;
		ans.hi = xui.hi;
	}
	return ans;
}



mut_uint64 mut_uint64_dec(mut_uint64 xui, unsigned int ui)
{
	mut_uint64 ans;

	if (xui.lo < ui) {
		mut_assert_check(xui.hi > 0); /* underflow */
		ans.hi -= 1;
		ans.lo = ULONG_MAX - (ui - xui.lo) + 1;
	} else {
		ans.hi = xui.hi;
		ans.lo = xui.lo - ui;
	}
	return ans;
}



int mut_uint64_lt(mut_uint64 a, mut_uint64 b)
{
	return (a.hi <= b.hi) && (a.lo < b.lo);
}


/*
 * 64-bit division by 10 using nibble/byte division.
 *
 * The performance of this routine is not critical given that it is
 * only used as part of outputing a 64-value in the statistics.
 * 
 * Feel free to replace this with a simpler and/or more efficent algorithm.
 */


#define mut_uint64_chunk_bits   4

#define mut_uint64_chunk_mask   ((1<<mut_uint64_chunk_bits)-1)

#define mut_uint64_2chunk_bits  (mut_uint64_chunk_bits*2)

#define mut_uint64_2chunk_mask  ((1<<mut_uint64_2chunk_bits)-1)

#define mut_uint64_init_shift \
  (sizeof(unsigned int)*mut_uint64_2chunk_bits - mut_uint64_chunk_bits)

#define mut_uint64_n_shifts \
  ((sizeof(unsigned int)*mut_uint64_2chunk_bits)/mut_uint64_chunk_bits)


/*
 * `mut_uint64_udiv10' divides the unsigned 64-bit value `n' by 10.
 * `mut_uint64_udiv10' returns the remainder and the quotient is
 * returned in `n' (i.e. the input value is overwritten with the quotient).
 */
static unsigned int mut_uint64_udiv10(mut_uint64 * n)
{
	unsigned int n_hi = n->hi;
	unsigned int n_lo = n->lo;
	unsigned int ans_hi = 0;
	unsigned int ans_lo = 0;
	unsigned int shift = mut_uint64_init_shift;
	unsigned int mask = mut_uint64_chunk_mask;
	unsigned int r;
	unsigned int i;
	do {
		unsigned int x = (n_hi>>shift)&mask;
		unsigned int q = x/10;
		r = x%10;
		ans_hi= (ans_hi<<mut_uint64_chunk_bits)|q;
		mask = (x == 0) 
			? ((mask<<mut_uint64_chunk_bits)|mask)
			: mut_uint64_2chunk_mask;
		n_hi = (r<<shift) | (n_hi&~(mut_uint64_chunk_mask<<shift));
		shift -= mut_uint64_chunk_bits;
	} while (shift != 0);

	/* least significant nibble of n_hi */
	{
		unsigned int x = n_hi&mask;
		unsigned int q = x/10;
		r = x%10;
		ans_hi= (ans_hi<<mut_uint64_chunk_bits)|q;
	}

	/* most significant nibble of n_lo */
	mask = mut_uint64_2chunk_mask;
	shift = mut_uint64_init_shift;
	{
		unsigned int x = ((r<<mut_uint64_chunk_bits)|(n_lo>>shift))&mask;
		unsigned int q = x/10;
		r = x%10;
		ans_lo = (ans_lo<<mut_uint64_chunk_bits)|q;
		mask= (x == 0)
			? ((mask<<mut_uint64_chunk_bits)|mask)
			: mut_uint64_2chunk_mask;
		n_lo = (r<<shift) | (n_lo&~(mut_uint64_chunk_mask<<shift));
		shift -= mut_uint64_chunk_bits;
	}

	for (i = 1; i < mut_uint64_n_shifts; i += 1) {
		unsigned int x = (n_lo>>shift)&mask;
		unsigned int q = x/10;
		r = x%10;
		ans_lo = (ans_lo<<mut_uint64_chunk_bits)|q;
		mask= (x == 0)
			? ((mask<<mut_uint64_chunk_bits)|mask)
			: mut_uint64_2chunk_mask;
		n_lo = (r<<shift) | (n_lo&~(mut_uint64_chunk_mask<<shift));
		shift -= mut_uint64_chunk_bits;
	}
	n->hi = ans_hi;
	n->lo = ans_lo;
	return r;
}


/*
 * The number of characters in: 2^64-1 (18446744073709551614)
 */
#define mut_uint64_max_decimal_chars 20



int mut_uint64_out(FILE * sink, mut_uint64 ui)
{
	size_t i = mut_uint64_max_decimal_chars;
	char buffer[mut_uint64_max_decimal_chars+1];

	buffer[i] = '\0';
	do {
		unsigned int r = mut_uint64_udiv10(&ui);
		mut_assert_check(r < 10);
		i -= 1;
		mut_assert_check(i >= 0);
		buffer[i] = r+'0';
	} while (ui.hi != 0 || ui.lo != 0);
	return fputs(&buffer[i], sink) != EOF;
}
