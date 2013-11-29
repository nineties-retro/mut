/*
 *.intro: test handling a SEGV.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x= 0x00;
	write(1, "A\n", 2);
	*x= 0x20;
	write(1, "B\n", 2);
	return 0;
}
