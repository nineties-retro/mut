/*
 *.intro: mut_counter implementation.
 */

#include "mut_counter.h"
#include "mut_log.h"
#include "mut_log_counter.h"

void mut_log_counter(mut_log *log, mut_counter c)
{
	mut_log_ulong(log, c);
}
