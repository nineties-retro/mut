#ifndef MUT_LOG_H
#define MUT_LOG_H

#include "mut_log_type.h"

void mut_log_fatal(mut_log *, char const *, int);

void mut_log_mem_full(mut_log *, int);

void mut_log_error(mut_log *, char const *);

void mut_log_warning(mut_log *, char const *);

void mut_log_string(mut_log *, char const *);

void mut_log_int(mut_log *, int);

void mut_log_uint(mut_log *, unsigned int);

void mut_log_uint_hex(mut_log *, unsigned int);

void mut_log_long(mut_log *, long);

void mut_log_ulong(mut_log *, unsigned long);

void mut_log_ulonglong(mut_log *, unsigned long);

void mut_log_info(mut_log *, char const *);

int mut_log_end(mut_log *);

#endif
