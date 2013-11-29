/*
 *.intro: A legal collection of calloc and cfree.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x;
	char *y;
	write(1, "A\n", 2);
	x= calloc(8, 8);
	write(1, "B\n", 2);
	y= calloc(16, 8);
	write(1, "C\n", 2);
	cfree(y);
	write(1, "D\n", 2);
	cfree(x);
	write(1, "E\n", 2);
	return 0;
}
