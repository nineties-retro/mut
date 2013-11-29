/*
 *.intro: realloc leak detection.
 *
 */

#include <stdlib.h>		/* realloc, free */
#include <unistd.h>		/* write */

int main(void)
{
	char *x, *y;
	write(1, "A\n", 2);
	x = realloc(0, 8);
	write(1, "B\n", 2);
	y = realloc(0, 16);
	write(1, "C\n", 2);
	*y = 10;
	free(x);
	write(1, "D\n", 2);
	return 0;
}
