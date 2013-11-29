/*
 *.intro: test free on unallocated memory.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x= (char *)0x10;
	char *y;
	write(1, "A\n", 2);
	free(x);			/* CUR */
	write(1, "B\n", 2);
	y= malloc(8);
	write(1, "C\n", 2);
	free(y);
	return 0;
}
