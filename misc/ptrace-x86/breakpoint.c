/*
 *.intro: test program for ptrace.c
 *
 */

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	printf("starting: ...\n");
	asm(".text ; .byte 0xcc;");	/* can't get INT 3 to be accepted */
	printf("breakpoint done ...\n");
	return 0;
}
