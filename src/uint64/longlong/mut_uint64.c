#include <stdio.h>
#include "mut_uint64.h"
#include "mut_uint64_out.h"
#include "mut_errno.h"

int mut_uint64_out(FILE * sink, mut_uint64 ui)
{
	return fprintf(sink, "%Lu", ui) > 0;
}
