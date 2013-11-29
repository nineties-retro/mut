#include <stdio.h>
#include "foo.h"

static int def(int x)
{
	return abc(x*2);
}

int main(void)
{
	printf("%d\n", def(4));
	return 0;
}
