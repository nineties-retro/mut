/*
 *.intro: test realloc failure with -f
 *
 * malloc and then free 1Mb chunks of memory in a loop until malloc
 * returns 0.  This will only happen if you don't use -f <URI:mut/doc/mut.1#f>
 * so that memory is not freed up by MUT.
 */

#include <stdlib.h>		/* malloc */
#include <unistd.h>		/* write */

int main(int argc, char ** argv)
{
	char *x;
	for (;;) {
		if ((x= malloc(1<<20)) == 0) {
			write(1, "X\n", 2);
			return 0;
		}
		free(x);
	}
	return 0;
}
