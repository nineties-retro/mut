/*
 *.intro: An executable reader for statically linked a.out executables.
 *
 * You may be wondering why nlist isn't used?  The primary reason is that
 * this code was originally written on a minmal Linux installation 
 * which didn't contain it and so I didn't know about it.  Now that I've
 * seen the interface and its error reporting capabilities I'd stick with
 * the code here, but for development purposes, nlist would probably have
 * been the best choice.
 */

#include "mut_stddef.h"		/* size_t */
#include "mut_stdlib.h"		/* qsort */
#include "mut_string.h"		/* strcmp */
#include "mut_stdio.h"		/* fopen, fclose, fseek, SEEK_SET */
#include "mut_errno.h"		/* errno */
#include "mut_log.h"		/* mut_log */
#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_mem.h"		/* mut_malloc, mut_free */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_aout_mach.h"	/* mut_aout_mach */
#include "mut_exec.h"		/* mut_exec */

struct mut_exec_reader {
	struct exec  header;
	FILE       * source;
	mut_log    * log;
	char const * file_name;
	struct {
		struct nlist * elems;
		size_t         nelems;
	} symbols;
};

typedef struct mut_exec_reader mut_exec_reader;


static int mut_exec_check_magic(mut_exec_reader *reader)
{
	if (N_BADMAG(reader->header)) {
		unsigned int magic = N_GETMAGIC(reader->header);
		mut_log_fatal(reader->log, "aout.bad-magic", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_uint_hex(reader->log, magic);
		(void)mut_log_end(reader->log);
		return 0;
	}
	return 1;
}



static int mut_exec_check_type(mut_exec_reader *reader)
{
	unsigned int mach_type = N_GETMID(reader->header);
	if (mach_type != mut_aout_mach) {
		mut_log_fatal(reader->log, "aout.bad-magic", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_uint(reader->log, mach_type);
		mut_log_uint(reader->log, mut_aout_mach);
		(void)mut_log_end(reader->log);
		return 0;
	}
	return 1;
}


/*
 * mut_exec_strtab_open initialises exec->strings.
 * 
 * The string table starts at N_STROFF(reader->header) in the file.
 * The first sizeof(size_t) bytes consist of the length of the
 * string table in bytes (including the size itself).
 * 
 * Returns 0 on failure, non-zero value on success.
 */
static int mut_exec_strtab_open(mut_exec *exec, mut_exec_reader *reader)
{
	long offset = N_STROFF(reader->header);
	if (fseek(reader->source, offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_long(reader->log, offset);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_strings;
	}
	if (fread(&exec->strings.size, sizeof(size_t), 1, reader->source) != 1) {
		mut_log_fatal(reader->log, "io.read", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_ulong(reader->log, sizeof(size_t));
		(void)mut_log_end(reader->log);
		goto could_not_read_size;
	}
	if ((exec->strings.data = mut_mem_malloc(exec->strings.size)) == 0) {
		mut_log_mem_full(reader->log, errno);
		goto could_not_allocate_strings;
	}
	if (fread(&exec->strings.data[sizeof(size_t)], 1, exec->strings.size-sizeof(size_t), reader->source) != exec->strings.size-sizeof(size_t)) {
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		mut_log_ulong(reader->log, exec->strings.size-sizeof(size_t));
		(void)mut_log_end(reader->log);
		goto could_not_read_strings;
	}
	return 1;

could_not_read_strings:
	mut_mem_free(exec->strings.data);
could_not_allocate_strings:
could_not_read_size:
could_not_seek_to_strings:
	return 0;
}



static char const *mut_exec_strtab_lookup(mut_exec *exec, size_t str)
{
	mut_assert_pre(str < exec->strings.size);
	return &exec->strings.data[str];
}


static void mut_exec_strtab_close(mut_exec *exec)
{
	mut_mem_free(exec->strings.data);
}


static int mut_exec_reader_symtab_open(mut_exec_reader *reader)
{
	long offset;

	if (reader->header.a_syms%sizeof(struct nlist) != 0) {
		mut_log_fatal(reader->log, "aout.symbols.size", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_uint(reader->log, reader->header.a_syms);
		mut_log_uint(reader->log, sizeof(struct nlist));
		(void)mut_log_end(reader->log);
		goto incorrect_symtab_size;
	}
	reader->symbols.nelems = reader->header.a_syms/sizeof(struct nlist);
	if ((reader->symbols.elems = mut_mem_malloc(reader->header.a_syms)) == 0) {
		mut_log_mem_full(reader->log, errno);
		goto could_not_allocate_symbols;
	}
	offset = N_SYMOFF(reader->header);
	if (fseek(reader->source, offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_long(reader->log, offset);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_symbols;
	}
	if (fread(reader->symbols.elems, sizeof(struct nlist), reader->symbols.nelems, reader->source) != reader->symbols.nelems){
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		mut_log_ulong(reader->log, sizeof(struct nlist)*reader->symbols.nelems);
		(void)mut_log_end(reader->log);
		goto could_not_read_symbols;
	}
	return 1;

could_not_read_symbols:
could_not_seek_to_symbols:
	mut_mem_free(reader->symbols.elems);
could_not_allocate_symbols:
incorrect_symtab_size:
	return 0;
}



static void mut_exec_reader_symtab_close(mut_exec_reader *reader)
{
	mut_mem_free(reader->symbols.elems);
}



static int mut_exec_functions_is_function(struct nlist *symbol)
{
     unsigned char type = symbol->n_type;
     return ((type&(N_TEXT)) == N_TEXT) && (type < N_TYPE);
}


static size_t mut_exec_functions_count(mut_exec_reader *reader)
{
	size_t i;
	size_t n = 0;

	for (i = 0; i < reader->symbols.nelems; i += 1) {
		struct nlist * symbol = &reader->symbols.elems[i];
		if (mut_exec_functions_is_function(symbol))
			n += 1;
	}
	return n;
}


/*
 * Ideally the following should extract all the function symbols from
 * the executable and store them.  However, the a.out format doesn't
 * seem to have enough flags to uniquely identify function symbols.
 * Also various symbols have the same type and value.  Generally it
 * seems that the last symbol with the given value is the most appropriate
 * so this is the one that is kept.  Also any symbol ending with
 * is ".[oc]" is not included since it obviously isn't a function name.
 *
 * A scan is made over the symbols counting all those with
 * the N_TEXT flag to get an upper bound on the number of symbols.
 * 
 * A second scan is made this time filling in the array and making more
 * of an effort to accurately exclude those symbols that aren't functions.
 *
 * Once done, the function symmbols are sorted into address order to make
 * it easier to locate the function name for a given address when doing
 * a backtrace.
 *
 * Since there aren't that many symbols (even in a large file), a simple
 * array and qsort seems sufficient i.e. there is no need for a O(log n)
 * data structure.
 *
 * Note that symbol names in an a.out file are prefixed with a `_' but
 * the symbol name argument does not have any prefix so the comparison
 * needs to be fiddled slightly.
 */
static int mut_exec_extract_functions(mut_exec *exec, mut_exec_reader *reader)
{
	size_t i;
	size_t n_functions_ub = mut_exec_functions_count(reader);
	mut_exec_addr last_addr;
	last_addr = mut_exec_addr_from_ulong(999);
	for (i = 0; i < reader->symbols.nelems; i += 1) {
		struct nlist * symbol = &reader->symbols.elems[i];
		char const * name = mut_exec_strtab_lookup(exec, symbol->n_un.n_strx);
		if (mut_exec_functions_is_function(symbol)) {
			const char * name = mut_exec_strtab_lookup(exec, symbol->n_un.n_strx);
			size_t name_len = strlen(name);
#if SKIP_FILES
			if (name[name_len-2] == '.'
			    && (name[name_len-1] == 'o' || name[name_len-1] == 'c'))
				continue;
#endif
			mut_exec_addr addr = mut_exec_addr_from_ulong(symbol->n_value);
			const char * n = *name == '_' ? name+1 : name;
			if (!mut_exec_symtab_add_fun(&exec->symbols, n, addr))
				goto could_not_add_function;
		}
	}
	return 1;

could_not_add_function:
	return 0;
}


int mut_exec_open(mut_exec *exec, char const *file_name, mut_log *log,
		  char const *undesirable_suffix)
{
	int status = 1;
	mut_exec_reader reader;
	reader.log = log;
	reader.file_name = file_name;

	if ((reader.source = fopen(file_name, "r")) == (FILE *)0) {
		mut_log_fatal(log, "io.open", errno);
		mut_log_string(log, file_name);
		(void)mut_log_end(log);
		return 0;
	}

	if (fread(&reader.header, sizeof(struct exec), 1, reader.source) != 1) {
		mut_log_fatal(log, "io.read", errno);
		mut_log_string(log, file_name);
		mut_log_ulong(log, sizeof(struct exec));
		(void)mut_log_end(log);
		goto could_not_read_header;
	}
	if (!mut_exec_check_magic(&reader))
		goto incorrect_magic_value;
	if (!mut_exec_check_type(&reader))
		goto incorrect_machine_type;
	if (!mut_exec_strtab_open(exec, &reader))
		goto could_not_open_strtab;
	if (!mut_exec_reader_symtab_open(&reader))
		goto could_not_open_reader_symtab;
	mut_exec_symtab_open(&exec->symbols, log);
	if (!mut_exec_extract_functions(exec, &reader))
		goto could_not_extract_functions;
	mut_exec_reader_symtab_close(&reader);
	if (fclose(reader.source) != 0) {
		mut_log_warning(log, "io.close");
		mut_log_string(log, file_name);
		(void)mut_log_end(log);
	}
	return 1;

could_not_extract_functions:
	mut_exec_reader_symtab_close(&reader);
could_not_open_reader_symtab:
could_not_open_strtab:
incorrect_machine_type:
incorrect_magic_value:
could_not_read_header:
	(void)fclose(reader.source);
	return 0;
}


void mut_exec_functions_addr(mut_exec *exec, size_t n, struct mut_exec_function *fs)
{
	size_t i;
	for (i = 0;  i < n;  i += 1) {
		mut_exec_addr sa = 
			mut_exec_symtab_lookup_name(&exec->symbols, fs[i].name);
		fs[i].addr = sa;
		fs[i].flags = (sa != 0);
	}
}


int mut_exec_addr_name(mut_exec *e, mut_exec_addr a,
		       mut_exec_addr *fa, char const **n, 
		       char const **fn, unsigned int *l)
{
	return mut_exec_symtab_lookup_addr(&e->symbols, a, fa, n, fn, l);
}


int mut_exec_has_backtrace_symbols(mut_exec *exec)
{
	return 1;
}


int mut_exec_close(mut_exec *exec)
{
	mut_exec_strtab_close(exec);
	mut_exec_symtab_close(&exec->symbols);
	return 1;
}
