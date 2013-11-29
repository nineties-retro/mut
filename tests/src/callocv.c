/*
 *.intro: tests what happens when calloc returns 0.
 *
 */

#include <stdlib.h>		/* calloc */
#include <string.h>		/* memset */
#include <unistd.h>		/* write */

int main(int argc, char ** argv)
{
	char *x;
	for (;;) {
		if ((x= calloc(2, 1<<20)) == 0) {
			write(1, "X\n", 2);
			return 1;
		}
		memset(x, 0xff, 2*(1<<20));
	}
	return 0;
}
