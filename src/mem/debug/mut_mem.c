/*
 *.intro: A debugging malloc/free/realloc implementation.
 */

#include "mut_assert.h"		/* mut_assert_check */
#include "mut_stddef.h"		/* size_t */
#include "mut_stdio.h"		/* FILE * */
#include "mut_stdlib.h"		/* malloc, free ... etc. */
#include "mut_string.h"		/* memset */
#include "mut_mem.h"
#include "mut_mem_ctrl.h"

struct mut_mem_source_pos {
	size_t       line;
	char const * file_name;
};


struct mut_mem_node {
	void *addr;
	struct mut_mem_node *next;
	size_t n_bytes;
	struct mut_mem_source_pos malloc;
	struct mut_mem_source_pos free;
};


struct mem {
	struct mut_mem_node *in_use;
	struct mut_mem_node *freed;
	size_t n_bytes_allocated;
	size_t n_bytes_in_use;
	size_t n_blocks_in_use;
	size_t max_bytes_in_use;
	size_t max_blocks_in_use;
	size_t n_malloc;
	size_t n_realloc;
	size_t n_realloc0;
	size_t n_free;
	size_t n_free0;
	FILE *log;
	FILE *report;
	void (*halt)(void);
};


static struct mem store;


void *mut_mem__malloc(size_t n_bytes, size_t line, char const *file_name)
{
	struct mut_mem_node * node;
	void                * addr;

	if ((addr = malloc(n_bytes)) == 0)
		goto could_not_allocate_n_bytes;

	if ((node = malloc(sizeof(struct mut_mem_node))) == 0)
		goto could_not_allocate_node;

	node->addr = addr;
	node->next = store.in_use;
	store.in_use = node;
	node->n_bytes = n_bytes;
	node->malloc.line = line;
	node->malloc.file_name = file_name;
	node->free.line = 0;
	node->free.file_name = (char const *)0;
	store.n_malloc += 1;
	store.n_bytes_allocated += n_bytes;
	store.n_bytes_in_use += n_bytes;
	if (store.n_bytes_in_use > store.max_bytes_in_use)
		store.max_bytes_in_use = store.n_bytes_in_use;
	store.n_blocks_in_use += 1;
	if (store.n_blocks_in_use > store.max_blocks_in_use)
		store.max_blocks_in_use = store.n_blocks_in_use;
	return addr;

could_not_allocate_node:
	free(addr);
could_not_allocate_n_bytes:
	return 0;
}


void mut_mem__free(void *addr, size_t line, char const *file_name)
{
	struct mut_mem_node **p, *n;

	store.n_free += 1;
	if (addr == 0) {
		store.n_free0 += 1;
		return;
	}
	n = store.in_use;
	p = &store.in_use;
	while (n) {
		if (n->addr == addr) {
			(void)memset(addr, 0xaa, n->n_bytes);
			n->free.line = line;
			n->free.file_name = file_name;
			*p = n->next;
			n->next = store.freed;
			store.freed = n;
			store.n_bytes_in_use -= n->n_bytes;
			store.n_blocks_in_use -= 1;
			return;
		} else {
			p= &n->next;
			n= n->next;
		}
	}
	for (n= store.freed; n; n= n->next) {
		if (n->addr == addr) {
			fprintf(store.log, "%s %lu freeing_freed_memory %s %lu\n",
				file_name,
				(unsigned long)line,
				n->free.file_name,
				(unsigned long)n->free.line);
			store.halt();
		}
	}
	fprintf(store.log, "%s %lu freeing_unallocated_memory %p\n",
		file_name,
		(unsigned long)line,
		addr);
	store.halt();
}


void *mut_mem__realloc(void *addr, size_t n_bytes, size_t line,
		       char const *file_name)
{
	struct mut_mem_node **p, *n;

	store.n_realloc += 1;
	if (addr == (void *)0) {
		store.n_realloc0 += 1;
		store.n_malloc -= 1;
		return mut_mem__malloc(n_bytes, line, file_name);
	}
	n = store.in_use;
	p = &store.in_use;
	while (n) {
		if (n->addr == addr) {
			void *new_addr;

			n->free.line = line;
			n->free.file_name = file_name;
			*p = n->next;
			n->next = store.freed;
			store.freed = n;
			store.n_bytes_in_use -= n->n_bytes;
			store.n_blocks_in_use -= 1;
			new_addr= mut_mem__malloc(n_bytes, line, file_name);
			if (new_addr != 0) {
				(void)memcpy(new_addr, addr, (n->n_bytes < n_bytes) ? n->n_bytes : n_bytes);
				(void)memset(addr, 0x1, n->n_bytes);
				store.n_malloc -= 1;
			}
			return new_addr;
		} else {
			p = &n->next;
			n = n->next;
		}
	}

	for (n= store.freed; n; n= n->next) {
		if (n->addr == addr) {
			fprintf(store.log, "%s %lu reallocing_freed_memory %s %lu\n",
				file_name,
				(unsigned long)line,
				n->free.file_name,
				(unsigned long)n->free.line);
			store.halt();
		}
	}
	fprintf(store.log, "%s %lu reallocing_unallocated_memory %p\n",
		file_name,
		(unsigned long)line,
		addr);
	store.halt();
	return 0;			/* just to keep compilers happy */
}


void mut_mem_init(void)
{
	store.in_use = (struct mut_mem_node *)0;
	store.freed = (struct mut_mem_node *)0;
	store.n_bytes_allocated = 0;
	store.n_bytes_in_use = 0;
	store.n_blocks_in_use = 0;
	store.max_bytes_in_use = 0;
	store.max_blocks_in_use = 0;
	store.n_malloc = 0;
	store.n_realloc = 0;
	store.n_realloc0 = 0;
	store.n_free = 0;
	store.n_free0 = 0;
	store.log = stderr;
	store.report = (FILE *)0;
	store.halt = abort;
}


void mut_mem_open(void)
{
}


FILE *mut_mem_log(FILE *log)
{
	FILE *previous_log= store.log;
	store.log= log;
	return previous_log;
}


FILE *mut_mem_report(FILE *log)
{
	FILE *previous_report= store.report;
	store.report= log;
	return previous_report;
}


/*
 * Return values :-
 *   0 - ok.
 *   1 - could not write to log file.
 *   2 - could not write to report file.
 */
int mut_mem_close(void)
{
	struct mut_mem_node *p, *n;
	int status = 0;

	if (store.report != (FILE *)0) {
		if (fprintf(store.report, "(mut_mem_report\n") < 0)
			status = 2;
		if (fprintf(store.report, "  (nBytesAllocated %lu)\n", (unsigned long)store.n_bytes_allocated) < 0)
			status = 2;
		if (fprintf(store.report, "  (nBytesInUse %lu)\n", (unsigned long)store.n_bytes_in_use) < 0)
			status = 2;
		if (fprintf(store.report, "  (maxBytesInUse %lu)\n", (unsigned long)store.max_bytes_in_use) < 0)
			status = 2;
		if (fprintf(store.report, "  (nBlocksInUse %lu)\n", (unsigned long)store.n_blocks_in_use) < 0)
			status = 2;
		if (fprintf(store.report, "  (maxBlocksInUse %lu)\n", (unsigned long)store.max_blocks_in_use) < 0)
			status = 2;
		if (fprintf(store.report, "  (nMallocCalls %lu)\n", (unsigned long)store.n_malloc) < 0)
			status = 2;
		if (fprintf(store.report, "  (nReallocCalls %lu)\n", (unsigned long)store.n_realloc) < 0)
			status = 2;
		if (fprintf(store.report, "  (nRealloc0Calls %lu)\n", (unsigned long)store.n_realloc0) < 0)
			status = 2;
		if (fprintf(store.report, "  (nFreeCalls %lu)\n", (unsigned long)store.n_free) < 0)
			status = 2;
		if (fprintf(store.report, "  (nFree0Calls %lu))\n", (unsigned long)store.n_free0) < 0)
			status = 2;
	}
	if (store.in_use) {
		for (p = store.in_use; p; p = n) {
			if (fprintf(store.log, "%s %lu memory_leak %p %lu\n", 
				    p->malloc.file_name,
				    (unsigned long)p->malloc.line,
				    p->addr,
				    (unsigned long)p->n_bytes) < 0)
				status= 1;
			store.n_bytes_in_use -= p->n_bytes;
			store.n_blocks_in_use -= 1;
			free(p->addr);
			n= p->next;
			free(p);
		}
	}
	mut_assert_check(store.n_bytes_in_use == 0);
	mut_assert_check(store.n_blocks_in_use == 0);
	for (p = store.freed; p; p = n) {
		free(p->addr);
		n= p->next;
		free(p);
	}
	return status;
}
