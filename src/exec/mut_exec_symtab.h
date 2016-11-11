#ifndef MUT_EXEC_SYMTAB_H
#define MUT_EXEC_SYMTAB_H

struct mut_exec_symtab {
	struct mut_exec_symtab_fun * head;
	struct mut_exec_symtab_fun * cache;
	mut_exec_addr end;
	mut_log *log;
};

void mut_exec_symtab_open(struct mut_exec_symtab *, mut_log *);

int mut_exec_symtab_add_fun(struct mut_exec_symtab *, char const *,
			    mut_exec_addr);

int mut_exec_symtab_add_range(struct mut_exec_symtab *, mut_exec_addr,
			      unsigned int, char const *);

int mut_exec_symtab_lookup_addr(const struct mut_exec_symtab *, mut_exec_addr,
				mut_exec_addr *, char const **, char const **,
				unsigned int *);

mut_exec_addr mut_exec_symtab_lookup_name(const struct mut_exec_symtab *,
					  char const *);

void mut_exec_symtab_close(struct mut_exec_symtab *);

#endif
