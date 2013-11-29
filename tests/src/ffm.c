/*
 *.intro: test free on already free memory.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x;
	write(1, "A\n", 2);
	x= malloc(8);
	write(1, "B\n", 2);
	free(x);
	write(1, "C\n", 2);
	free(x);
	return 0;
}
