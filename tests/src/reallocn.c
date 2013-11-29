/*
 *.intro: test realloc failure with -f
 *
 * Realloc and then free 1Mb chunks of memory in a loop until realloc
 * returns 0.  This will only happen if you don't use -f <URI:mut/doc/mut.1#f>
 * so that memory is not freed up by MUT.
 */

#include <stdlib.h>		/* realloc */
#include <unistd.h>		/* write */

int main(int argc, char ** argv)
{
	char *x= 0, *y;
	for (;;) {
		if ((y= realloc(x, 1<<19)) == 0) {
			write(1, "X\n", 2);
			free(x);
			return 0;
		}
		if ((x= realloc(y, 1<<19)) == 0) {
			write(1, "Y\n", 2);
			free(y);
			return 0;
		}
	}
	return 0;
}
