/*
 *.intro: 3-level deep calling hierarchy
 *
 */

#include <unistd.h>

void c(int x)
{
	write(1, "C\n", 2);
}

void b(int x)
{
	write(1, "B\n", 2);
	c(x);
}

void a(int x)
{
	write(1, "A\n", 2);
	b(x);
}

int main(void)
{
	a(10);
	return 0;
}
