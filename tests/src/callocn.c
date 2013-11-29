/*
 *.intro: primarily tests if 64-bit counters work.
 *
 * Calloc and then free 1MB chunks of memory in a loop until greater than
 * ULONG_MAX bytes have been allocated.
 *
 * You'll need to use -f <URI:mut/doc/mut.1#f> otherwise you'll 
 * probably run out of memory.  This makes the test useful
 * to see what happens in this case.
 */

#include <stdlib.h>		/* calloc */
#include <unistd.h>		/* write */

int main(int argc, char ** argv)
{
	unsigned long i;
	char *x;
	for (i= 0; i < 5000; i += 1) {
		if ((x= calloc(16, 1<<16)) == 0) {
			write(1, "X\n", 2);
			return 0;
		}
		free(x);
	}
	return 0;
}
