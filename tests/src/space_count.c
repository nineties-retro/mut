/*
 *.intro: highlight the difference between max space and max count.
 *
 */

#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *big;
	char *a0= malloc(10);
	char *a1= malloc(10);
	char *a2= malloc(10);
	char *a3= malloc(10);
	char *a4= malloc(10);
	char *a5= malloc(10);
	char *a6= malloc(10);
	char *a7= malloc(10);
	char *a8= malloc(10);
	char *a9= malloc(10);
	free(a0);
	free(a1);
	free(a2);
	free(a3);
	free(a4);
	free(a5);
	free(a6);
	free(a7);
	free(a8);
	free(a9);
	big= malloc(1000);
	free(big);
	return 0;
}
