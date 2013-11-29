/*
 *.intro: sample program for aout
 *
 */

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	char *s;
	printf("malloc: start, calling malloc ...\n");
	s= malloc(10);
	printf("malloc: called malloc, calling free ...\n");
	free(s);
	printf("malloc: called free, returning ...\n");
	return 0;
}
