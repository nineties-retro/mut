#ifndef MUT_LOG_OWN_H

#include "mut_log_type.h"

void mut_log_open(mut_log *, char const *);

void mut_log_file(mut_log *, char const *);

void mut_log_prog_name(mut_log *, char const *);

int mut_log_close(mut_log *);

#endif
