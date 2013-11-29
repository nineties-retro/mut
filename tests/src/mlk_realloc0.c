/*
 *.intro: realloc leak detection.
 *
 */

#include <stdlib.h>		/* malloc */
#include <unistd.h>		/* write */

int main(void)
{
	char *x;
	write(1, "A\n", 2);
	x= realloc(0, 8);
	write(1, "B\n", 2);
	*x = 10;
	return 0;
}
