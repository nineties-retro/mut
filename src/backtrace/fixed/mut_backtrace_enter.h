#ifndef MUT_BACKTRACE_ENTER_H
#define MUT_BACKTRACE_ENTER_H

#include "mut_stddef.h"
#include "mut_exec.h"
#include "mut_backtrace_type.h"

int mut_backtrace_enter(mut_backtrace *, mut_exec_addr);

#endif
