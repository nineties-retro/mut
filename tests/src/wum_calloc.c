/*
 *.intro: test writing unallocated memory around a calloc.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *x= calloc(10,1);
	write(1, "A\n", 2);
	x[00]= 'A';
	x[01]= 'B';
	x[10]= 'W';
	x[11]= 'X';
	x[12]= 'Y';
	x[13]= 'Z';
	write(1, "B\n", 2);
	free(x);
	write(1, "C\n", 2);
	return 0;
}
