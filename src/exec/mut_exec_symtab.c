/*
 * $Id$
 *
 * Linked-list symbol table.
 */

#include "mut_errno.h"		/* errno */
#include "mut_log.h"		/* mut_log */
#include "mut_mem.h"		/* mut_malloc, mut_free */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_exec_symtab.h"

/*
 * Cheap and cheerful linked list implementation of the addr->line mapping.
 */
struct mut_exec_symtab_fun_line {
	struct mut_exec_symtab_fun_line *next;
	mut_exec_addr line_addr;
	unsigned short line_number;
};

/*
 * Cheap and cheerful linked list implementation of the addr->name mapping.
 */
struct mut_exec_symtab_fun {
	struct mut_exec_symtab_fun *next;
	char const *fun_name;
	mut_exec_addr fun_start_addr;
	char const *file_name;
	char const *dir_name;
	struct mut_exec_symtab_fun_line * fun_lines;
};


void mut_exec_symtab_open(struct mut_exec_symtab *s, mut_log *l)
{
	s->head = 0;
	s->cache = 0;
	s->end = 0;
	s->log = l;
}


static struct mut_exec_symtab_fun *mut_exec_symtab_fun_open(struct mut_exec_symtab *s,
							    char const *name,
							    mut_exec_addr start_addr, 
							    struct mut_exec_symtab_fun *next)
{
	struct mut_exec_symtab_fun *nf = mut_mem_malloc(sizeof(*nf));
	if (nf == 0) {
		mut_log_fatal(s->log, "mem.full", errno);
		(void)mut_log_end(s->log);
	} else {
		nf->next = next;
		nf->fun_name = name;
		nf->fun_start_addr = start_addr;
		nf->fun_lines = 0;
	}
	return nf;
}


int mut_exec_symtab_add_fun(struct mut_exec_symtab *s, char const *n, mut_exec_addr a)
{
	struct mut_exec_symtab_fun ** fp = &s->head;
	struct mut_exec_symtab_fun  * f;
	struct mut_exec_symtab_fun  * nf;

	while ((f = *fp) != 0) {
		if (a < f->fun_start_addr)
			break;
		fp = &f->next;
	}
	if ((nf = mut_exec_symtab_fun_open(s, n, a, f)) == 0)
		return 0;
	*fp = nf;
	return 1;
}


static struct mut_exec_symtab_fun_line *mut_exec_symtab_fun_line_open(struct mut_exec_symtab * s,
								      mut_exec_addr a,
								      unsigned int l,
								      struct mut_exec_symtab_fun_line *n)
{
	struct mut_exec_symtab_fun_line *nfl = mut_mem_malloc(sizeof(*nfl));
	if (nfl == 0) {
		mut_log_fatal(s->log, "mem.full", errno);
		(void)mut_log_end(s->log);
	} else {
		nfl->next = n;
		nfl->line_addr = a;
		nfl->line_number = l;
	}
	return nfl;
}


static int mut_exec_symtab_add_xxx(struct mut_exec_symtab *s, mut_exec_addr a, unsigned int l)
{
	struct mut_exec_symtab_fun_line  * fl = 0;
	struct mut_exec_symtab_fun_line ** flp = &s->cache->fun_lines;
	struct mut_exec_symtab_fun_line  * nfl;

	while ((fl = *flp) != 0) {
		if (a < fl->line_addr)
			break;
		flp = &fl->next;
	}
	if ((nfl = mut_exec_symtab_fun_line_open(s, a, l, fl)) == 0)
		return 0;
	*flp = nfl;
	return 1;
}


int mut_exec_symtab_add_range(struct mut_exec_symtab *s, mut_exec_addr a,
			      unsigned int l, char const *fn)
{
	struct mut_exec_symtab_fun * f;
#if 0
	if ((s->cache != 0) && (a >= s->cache->fun_start_addr) && (a < s->end)) {
		s->cache->file_name= fn;
		return mut_exec_symtab_add_xxx(s, a, l);
	}
#endif

	for (f = s->head; f != 0; f = f->next) {
		if (a >= f->fun_start_addr) {
			if (f->next == 0) {
				/* FIXME: need to set end here */
			} else if (a < f->next->fun_start_addr) {
				s->end = f->next->fun_start_addr;
			} else {
				continue;
			}
			s->cache = f;
			f->file_name = fn;
			return mut_exec_symtab_add_xxx(s, a, l);
		}
	}  
	mut_log_warning(s->log, "elf.xxx");
	mut_log_end(s->log);
	return 1;
}



int mut_exec_symtab_lookup_addr(const struct mut_exec_symtab *s, mut_exec_addr a,
				mut_exec_addr *sa, char const **n,
				char const **fn, unsigned int *l)
{
	struct mut_exec_symtab_fun * f;
	struct mut_exec_symtab_fun_line * fl;

	for (f = s->head; f != 0; f = f->next) {
		if ((a >= f->fun_start_addr)
		    && ((f->next == 0) || (a < f->next->fun_start_addr))) {
			*sa = f->fun_start_addr;
			*n = f->fun_name;
			*fn = f->file_name;
			for (fl = f->fun_lines; fl != 0; fl = fl->next) {
				if (a <= fl->line_addr) {
					*l = fl->line_number;
					return 1;
				}
			}
			/* just in case */
			*l = 0;
			return 1;
		}
	}  
	return 0;
}


mut_exec_addr mut_exec_symtab_lookup_name(const struct mut_exec_symtab *s, char const *n) 
{
	struct mut_exec_symtab_fun * f;

	for (f = s->head; f != 0; f = f->next) {
		if (strcmp(n, f->fun_name) == 0) {
			return f->fun_start_addr;
		}
	}  
	return 0;
}



void mut_exec_symtab_close(struct mut_exec_symtab *s)
{
	struct mut_exec_symtab_fun *f = s->head;

	while (f != 0) {
		struct mut_exec_symtab_fun * nf = f->next;
		struct mut_exec_symtab_fun_line * fl = f->fun_lines;
		while (fl != 0) {
			struct mut_exec_symtab_fun_line * nfl = fl->next;
			mut_mem_free(fl);
			fl = nfl;
		}
		mut_mem_free(f);
		f = nf;
	}
}
