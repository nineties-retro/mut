#ifndef MUT_EXEC_H
#define MUT_EXEC_H

#include "mut_exec_symtab.h"

struct mut_exec_strings {
	char * data;
	size_t size;
};

struct mut_exec {
	struct mut_exec_symtab symtab;
	struct mut_exec_strings strings;
	struct mut_exec_strings stabstr;
	int       has_full_symbols;
	mut_log * log;
};


typedef struct mut_exec mut_exec;

struct mut_exec_function {
	char const     * name;
	mut_exec_addr    addr;
	int              flags;
};

typedef struct mut_exec_function mut_exec_function;

int mut_exec_open(mut_exec *, char const *, mut_log *, char const *);

void mut_exec_functions_addr(mut_exec *, size_t, mut_exec_function *);

int mut_exec_addr_name(mut_exec *, mut_exec_addr, mut_exec_addr *, char const **, char const **, unsigned int *);

int mut_exec_has_backtrace_symbols(mut_exec * exec);

int mut_exec_close(mut_exec *);

#endif
