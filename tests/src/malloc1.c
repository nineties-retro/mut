/*
 *.intro: A legal collection of malloc and free.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x;
	char *y;
	write(1, "A\n", 2);
	x= malloc(8);
	write(1, "B\n", 2);
	free(x);
	write(1, "C\n", 2);
	y= malloc(16);
	write(1, "D\n", 2);
	free(y);
	write(1, "E\n", 2);
	return 0;
}
