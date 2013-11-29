/*
 *.intro: Code from figure 11.2 of <URI:mut/bib/bach>
 */


#include <stdio.h>

int data[32];

int main(void)
{
	int i;
	for (i= 0; i<32; i++)
		printf("data[%d] = %d\n", i, data[i]);
	printf("ptrace data addr 0x%p\n", data);
	return 0;
}
