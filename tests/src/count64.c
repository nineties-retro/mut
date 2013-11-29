/*
 *.intro: test that 64-bit counters work.
 *
 * malloc and then free 1Mb chunks of memory until considerably greater
 * than 2^32 bytes have been allocated and freed.
 * 
 * Note this will only happen if you use -f <URI:mut/doc/mut.1#f>
 * so that memory is freed up by MUT.
 */

#include <stdlib.h>		/* malloc */
#include <unistd.h>		/* write */

int main(int argc, char ** argv)
{
	size_t i;
	char *x;
	for (i= 0; i < 5000; i += 1) {
		if ((x= malloc(1<<20)) == 0) {
			write(1, "X\n", 2);
			return 0;
		}
		free(x);
	}
	return 0;
}
