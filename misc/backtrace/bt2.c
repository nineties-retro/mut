/*
 *.intro: 3-level deep call hierarchy + force use of EBP.
 *
 * This is much the same as <URI:mut/misc/backtrace/bt.c>, the difference
 * is that at various levels, a complex expression is included to try
 * and force the compiler to use EBP for some purpose other than a 
 * frame-pointer.
 */

#include <unistd.h>

void c(int x)
{
	write(1, "C\n", 2);
}

void b(int x)
{
	size_t pi, qi, ri, si, ti;
	int p[10], q[10], r[10], s[10], t[10];
	for (pi= 0; pi < 10; pi += 1)
		for (qi= 0; qi < 10; qi += 1)
			for (ri= 0; ri < 10; ri += 1)
				for (si= 0; si < 10; si += 1)
					for (ti= 0; ti < 10; ti += 1) {
						write(1, "B\n", 2);
						c(p[pi]+q[qi]+r[ri]+s[si]+t[ti]);
					}
}

void a(int x)
{
	write(1, "A\n", 2);
	b(x);
}

int main(void)
{
	size_t pi, qi, ri;
	int p[10], q[10], r[10];
	for (pi= 0; pi < 10; pi += 1)
		for (qi= 0; qi < 10; qi += 1)
			for (ri= 0; ri < 10; ri += 1)
				a(p[pi]+q[qi]+r[ri]);
	return 0;
}
