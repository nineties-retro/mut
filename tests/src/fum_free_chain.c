/*
 *.intro: test free on unallocated memory in a function to see backtrace.
 *
 */

#include <stdlib.h>
#include <unistd.h>

static void foo(void)
{
	char *x= (char *)0x10;
	free(x);			/* CUR */
}

int main(void)
{
	char *y;
	write(1, "A\n", 2);
	foo();
	write(1, "B\n", 2);
	y= malloc(8);
	write(1, "C\n", 2);
	free(y);
	return 0;
}
