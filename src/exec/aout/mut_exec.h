#ifndef MUT_EXEC_H
#define MUT_EXEC_H

/*
 *.intro: interface to a.out executables.
 */

#include "mut_stddef.h"		/* size_t */
#include <a.out.h>		/* struct exec, struct nlist */
#include "mut_exec_symtab.h"		/* size_t */

struct mut_exec_function {
	char const    * name;
	mut_exec_addr   addr;
	int             flags;
};


struct mut_exec {
	struct {
		char   * data;
		size_t   size;
	} strings;
	struct mut_exec_symtab symbols;
};

typedef struct mut_exec mut_exec;

int mut_exec_open(mut_exec *, char const *, mut_log *, const char *);

void mut_exec_functions_addr(mut_exec *, size_t, struct mut_exec_function *);

int mut_exec_addr_name(mut_exec *, mut_exec_addr, mut_exec_addr *,
                   char const **, char const **, unsigned int *);

int mut_exec_has_backtrace_symbols(mut_exec * exec);

int mut_exec_close(mut_exec *);

#endif
