/*
 *.intro: test for the -z option (<URI:mut/src/ui/tty/mut_ui.c#z>)
 *
 * With -z the output is H, with it G.
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	int * x;
	int y;
	int z;
	write(1, "A\n", 2);
	x= malloc(sizeof(int));
	write(1, "B\n", 2);
	*x= 0xdeadbeef;
	write(1, "C\n", 2);
	y= *x;
	write(1, "D\n", 2);
	free(x);
	write(1, "E\n", 2);
	z= *x;
	write(1, "F\n", 2);
	if (y == z)
		write(1, "G\n", 2);
	else
		write(1, "H\n", 2);
	return 0;
}
