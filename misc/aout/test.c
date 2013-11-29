/*
 *.intro: sample program for aout
 *
 */

#include <stdlib.h>
#include <stdio.h>

struct bob {
	int x;
	float y;
};

typedef struct bob bill;


static void foo(void)
{
	char *s;
	printf("malloc: start, calling malloc ...\n");
	s= malloc(10);
	printf("malloc: called malloc, calling free ...\n");
	free(s);
	printf("malloc: called free, returning ...\n");
}


int main(void)
{
	foo();
	return 0;
}
