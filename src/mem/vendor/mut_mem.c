/*
 * A wrapper for the vendor supplied malloc/realloc/free that supports
 * the same interface as the debugging malloc
 */

#include "mut_stddef.h"
#include "mut_stdio.h"
#include "mut_mem.h"
#include "mut_mem_ctrl.h"

void mut_mem_init(void)
{
}


void mut_mem_open(void)
{
}


FILE *mut_mem_log(FILE * ignore)
{
	return (FILE *)0;
}



FILE *mut_mem_report(FILE * ignore)
{
	return (FILE *)0;
}



int mut_mem_close(void)
{
	return 0;
}
