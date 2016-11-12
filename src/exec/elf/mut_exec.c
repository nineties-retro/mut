/*
 *.intro: An executable reader for dynamically linked ELF executables.
 *
 */

#include <limits.h>		/* CHAR_BIT */
#include "mut_stddef.h"		/* size_t */
#include "mut_stdlib.h"		/* qsort */
#include "mut_string.h"		/* strcmp */
#include "mut_stdio.h"		/* fopen, fclose, fseek, SEEK_SET */
#include "mut_errno.h"		/* errno */
#include "mut_log.h"		/* mut_log */
#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_mem.h"		/* mut_malloc, mut_free */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_exec.h"		/* mut_exec */

struct mut_exec_stab {
	mut_elf_word other_and_string_index;
	mut_elf_word line_and_type;
	mut_elf_word value;
};

/* <URI:mut/misc/stabs/README#fun.code> */
#define mut_exec_stab_type_FUN 0x24

/* <URI:mut/misc/stabs/README#sline.code> */
#define mut_exec_stab_type_LINE 0x44

/* <URI:mut/misc/stabs/README#so.code> */
#define mut_exec_stab_type_SO 0x64

/* /usr/include/stab.def */
#define mut_exec_stab_type_PSYM 0xa0


static int mut_exec_stab_type(struct mut_exec_stab * s)
{
	unsigned int mask= (1<<(CHAR_BIT*sizeof(mut_elf_half)))-1;
	unsigned int v= s->line_and_type&mask;
	return (int)v;
}

static size_t mut_exec_stab_str(struct mut_exec_stab * s)
{
	unsigned int mask= (1<<(CHAR_BIT*sizeof(mut_elf_half)))-1;
	size_t v= s->other_and_string_index&mask;
	return v;
}


#define mut_exec_stab_line(s) \
	((s)->line_and_type >> (CHAR_BIT*sizeof(mut_elf_half)))

#define mut_exec_stab_addr(s) ((s)->value)
#define mut_exec_stab_offset(s) ((s)->value)

enum {
	/*
	 * The maximum number of sections that were are interested
	 * in caching the details of.  If you add another section,
	 * then obviously increase this value.
	 */
	mut_exec_reader_sections_max = 4
};

struct mut_exec_reader {
	mut_elf_ehdr   header;
	struct {
		mut_elf_shdr header[mut_exec_reader_sections_max];
		size_t     free;
	} section;
	struct {
		char   * data;
		size_t   size;
	} strings;
	mut_elf_shdr * dsymtab;
	mut_elf_shdr * symtab;
	mut_elf_shdr * stabs;
	mut_elf_shdr * plt;
	mut_elf_shdr   strtab;
	FILE       * source;
	mut_log    * log;
	char const * file_name;
};

typedef struct mut_exec_reader mut_exec_reader;


/*
 * Takes a mut_exec_reader that has been opened on a file and checks
 * that the e_ident fields hold the correct magic values.  Returns
 * a non-zero value if they do, otherwise a fatal error is logged
 * and 0 is returned.
 */
static int mut_exec_check_id(mut_exec_reader *reader)
{
	if (reader->header.e_ident[EI_MAG0] != ELFMAG0
	    ||  reader->header.e_ident[EI_MAG1] != ELFMAG1
	    ||  reader->header.e_ident[EI_MAG2] != ELFMAG2
	    ||  reader->header.e_ident[EI_MAG3] != ELFMAG3) {
		mut_log_fatal(reader->log, "elf.not", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	} 
	return 1;
}


static int mut_exec_check_class(mut_exec_reader *reader)
{
	if (reader->header.e_ident[EI_CLASS] != mut_elf_class) {
		char class[2];
		mut_log_warning(reader->log, "elf.bad-class");
		mut_log_string(reader->log, reader->file_name);
		class[0]= reader->header.e_ident[EI_CLASS];
		class[1]= '\0';
		mut_log_string(reader->log, &class[0]);
		(void)mut_log_end(reader->log);
		return 0;
	}
	return 1;
}



static int mut_exec_check_type(mut_exec_reader *reader)
{
	if (reader->header.e_type != ET_EXEC) {
		mut_log_fatal(reader->log, "elf.not-exec", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	}
	return 1;
}



/*
 * Read in the contents of the .strtab section of the executable
 * into exec->strings.  The offset of the start of the string table
 * is in reader->symtab->sh_link (see <URI:mut/bib/nohr#sym2str>).
 * The string table section header is read into reader->symtab.
 *
 * If there are any problems, an error is logged and 0 is returned.
 * Otherwise a non-zero value is returned.
 */
static int
mut_exec_strtab_open(mut_exec *exec,
		     mut_exec_reader *r,
		     mut_elf_shdr *symtab,
		     struct mut_exec_strings *strs)
{
	const size_t es = r->header.e_shentsize;
	const size_t shi = symtab->sh_link;
	const long o = r->header.e_shoff + shi * es;
	mut_elf_shdr strtab;

	if (r->header.e_shentsize != sizeof(strtab)) {
		mut_log_fatal(r->log, "elf.section.size", 0);
		mut_log_string(r->log, r->file_name);
		mut_log_uint(r->log, r->header.e_shentsize);
		mut_log_uint(r->log, sizeof(strtab));
		(void)mut_log_end(r->log);
		goto bad_section_header_size;
	}
	if (shi > r->header.e_shnum) {
		mut_log_fatal(r->log, "elf.section.index.range", 0);
		mut_log_string(r->log, r->file_name);
		mut_log_string(r->log, ".strtab");
		mut_log_uint(r->log, shi);
		mut_log_uint(r->log, r->header.e_shnum);
		(void)mut_log_end(r->log);
		goto hash_link_range_error;
	}
	if (fseek(r->source, o, SEEK_SET) < 0) {
		int e = errno;
		mut_log_fatal(r->log, "io.seek", 0);
		mut_log_string(r->log, r->file_name);
		mut_log_int(r->log, e);
		(void)mut_log_end(r->log);
		goto could_not_seek_to_header;
	}
	if (fread(&strtab, sizeof(strtab), 1, r->source) != 1) {
		int e = errno;
		mut_log_fatal(r->log, "io.read", errno);
		mut_log_string(r->log, r->file_name);
		mut_log_int(r->log, e);
		(void)mut_log_end(r->log);
		goto could_not_read_header;
	}
	if (strtab.sh_type != SHT_STRTAB) {
		mut_log_fatal(r->log, "elf.symtab.strtab", 0);
		mut_log_string(r->log, r->file_name);
		mut_log_int(r->log, strtab.sh_type);
		mut_log_int(r->log, SHT_STRTAB);
		(void)mut_log_end(r->log);
		goto could_not_read_header;
	}
	/*
	 * @@@ should check that strtab.sh_size is less than
	 * the size of the file.
	 */
	strs->size = strtab.sh_size;
	strs->data = mut_mem_malloc(strtab.sh_size);
	if (strs->data == 0) {
		mut_log_fatal(r->log, "mem.full", errno);
		mut_log_uint(r->log, strtab.sh_size);
		(void)mut_log_end(r->log);
		goto could_not_allocate_strings;
	}
	if (fseek(r->source, strtab.sh_offset, SEEK_SET) < 0) {
		int e = errno;
		mut_log_fatal(r->log, "io.seek", errno);
		mut_log_string(r->log, r->file_name);
		mut_log_int(r->log, e);
		(void)mut_log_end(r->log);
		goto could_not_seek_to_strings;
	}
	if (fread(strs->data, strtab.sh_size, 1, r->source) != 1) {
		int e = errno;
		mut_log_fatal(r->log, "io.read", errno);
		mut_log_string(r->log, r->file_name);
		mut_log_int(r->log, e);
		(void)mut_log_end(r->log);
		goto could_not_read_strings;
	}
	return 1;

could_not_read_strings:
could_not_seek_to_strings:
	mut_mem_free(strs->data);
could_not_allocate_strings:
could_not_read_header:
could_not_seek_to_header:
hash_link_range_error:
bad_section_header_size:
	return 0;
}


static char const *
mut_exec_strtab_lookup(struct mut_exec_strings *strs, mut_elf_word str)
{
	mut_assert_pre(str < strs->size);
	return &strs->data[str];
}


static void mut_exec_strtab_close(struct mut_exec_strings *strs)
{
	mut_mem_free(strs->data);
}



/*
 * Initialises exec->symtab with the symbol table based on information
 * in the symbol table header contained in reader->symtab and 
 * the strings contained in exec->strings.
 *
 * If there are any problems with doing this, then an error is logged
 * and 0 is returned.  Otherwise a non-zero value is returned.
 */
static int
mut_exec_extract_symbols(mut_exec *exec,
			 mut_exec_reader *reader,
			 mut_elf_shdr *symtab,
			 struct mut_exec_strings *strtab,
			 char const *undesirable_suffix,
			 mut_elf_shdr *plt)
{
	mut_elf_sym * symbols;
	size_t      n_symbols;
	size_t      i;
	size_t      f = 1;

	mut_assert_pre(strtab->data != 0);

	if (symtab->sh_size % sizeof(mut_elf_sym) != 0) {
		mut_log_fatal(reader->log, "elf.sym.size", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto incorrect_symtab_size;
	}
	n_symbols= symtab->sh_size/sizeof(mut_elf_sym);
	symbols= mut_mem_malloc(symtab->sh_size);
	if (symbols ==  0) {
		mut_log_mem_full(reader->log, errno);
		goto could_not_allocate_symbols;
	}
	if (fseek(reader->source, symtab->sh_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_long(reader->log, symtab->sh_offset);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_symbols;
	}
	if (fread(symbols, symtab->sh_size, 1, reader->source) != 1){
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_symbols;
	}
	for (i = 0; i < n_symbols; i += 1) {
		mut_elf_sym * s= &symbols[i];
		if (mut_elf_st_type(s->st_info) == STT_FUNC) {
			mut_exec_addr sa= mut_exec_addr_from_ulong(s->st_value);
			char const *n= mut_exec_strtab_lookup(strtab, s->st_name);
			char * start = undesirable_suffix ? strstr(n, undesirable_suffix) : 0;
			if (start != 0)
				*start= '\0';
			if (!sa && plt)
				sa = plt->sh_addr + f*plt->sh_addralign;
			if (sa && !mut_exec_symtab_add_fun(&exec->symtab, n, sa))
				goto could_not_add_fun;
			f += 1;
		}
	}
	mut_mem_free(symbols);
	return 1;

could_not_add_fun:
	mut_exec_symtab_close(&exec->symtab);
could_not_read_symbols:
could_not_seek_to_symbols:
	mut_mem_free(symbols);
could_not_allocate_symbols:
incorrect_symtab_size:
	return 0;
}


static int
mut_exec_extract_stabs(mut_exec_reader *reader,
		       struct mut_exec_stab **stabs, size_t *n_stabs)
{
	mut_assert_pre(reader->stabs != 0);

	if (reader->stabs->sh_entsize != sizeof(struct mut_exec_stab)) {
		mut_log_fatal(reader->log, "elf.stab.size", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto incorrect_stabs_size;
	}
	if (reader->stabs->sh_size % sizeof(struct mut_exec_stab) != 0) {
		mut_log_fatal(reader->log, "elf.stab.size", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto incorrect_stabs_size;
	}
	*n_stabs= reader->stabs->sh_size/sizeof(struct mut_exec_stab);
	*stabs= mut_mem_malloc(reader->stabs->sh_size);
	if (*stabs ==  0) {
		mut_log_mem_full(reader->log, errno);
		goto could_not_allocate_stabs;
	}
	if (fseek(reader->source, reader->stabs->sh_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_long(reader->log, reader->stabs->sh_offset);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_stabs;
	}
	if (fread(*stabs, reader->stabs->sh_size, 1, reader->source) != 1){
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_stabs;
	}
	return 1;

could_not_read_stabs:
could_not_seek_to_stabs:
	mut_mem_free(*stabs);
could_not_allocate_stabs:
incorrect_stabs_size:
	return 0;
}



/*
 * Based on the stab section header in reader->stabs, locates the
 * stab strings (.stabstr) and reads it all into exec->stabstr.
 */
static int
mut_exec_stabstr_open(mut_exec * exec, mut_exec_reader * reader)
{
	long strtab_offset;
	mut_elf_shdr strtab;

	if (reader->stabs->sh_link > reader->header.e_shnum) {
		mut_log_fatal(reader->log, "elf.section_index.range", 0);
		mut_log_string(reader->log, reader->file_name);
		mut_log_string(reader->log, ".stabstr");
		(void)mut_log_end(reader->log);
		goto hash_link_range_error;
	}
	strtab_offset= reader->header.e_shoff 
		+ reader->stabs->sh_link*reader->header.e_shentsize;
	if (fseek(reader->source, strtab_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_header;
	}
	if (fread(&strtab, reader->header.e_shentsize, 1, reader->source) != 1) {
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_header;
	}
	if (strtab.sh_type != SHT_STRTAB) {
		mut_log_fatal(reader->log, "elf.stab.stabstr", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_header;
	}
	exec->stabstr.size= strtab.sh_size;
	exec->stabstr.data= mut_mem_malloc(strtab.sh_size);
	if (exec->stabstr.data == 0) {
		mut_log_fatal(reader->log, "mem.full", errno);
		(void)mut_log_end(reader->log);
		goto could_not_allocate_strings;
	}
	if (fseek(reader->source, strtab.sh_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_strings;
	}
	if (fread(exec->stabstr.data, strtab.sh_size, 1, reader->source) != 1) {
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_strings;
	}
	return 1;

could_not_read_strings:
could_not_seek_to_strings:
	mut_mem_free(exec->stabstr.data);
could_not_allocate_strings:
could_not_read_header:
could_not_seek_to_header:
hash_link_range_error:
	return 0;
}


#if 0
/* 19980922 - why is this not used ? */
char const *
mut_exec_stabstr_lookup(mut_exec * exec, mut_elf_word str)
{
     mut_assert_pre(str < exec->stabstr.size);
     return &exec->stabstr.data[str];
}
#endif


static void mut_exec_stabstr_close(mut_exec *exec)
{
	mut_mem_free(exec->stabstr.data);
}



/*
 * Extracts line number information from the stab entries referred to
 * by the header in reader->stabs and adds it to the symbol table 
 * (exec->symtab).
 *
 * If there are any problems with doing this, then an error is logged
 * and 0 is returned.  Otherwise a non-zero value is returned.
 */
static int
mut_exec_extract_debug_info(mut_exec * exec, mut_exec_reader * reader)
{
	struct mut_exec_stab    * stabs;
	size_t                    n_stabs;
	size_t                    i;
	char const              * dn;
	char const              * fn= 0;
	struct mut_exec_stab    * s;
	size_t                    string_index;
	mut_exec_addr             base_addr= 0;

	if (!mut_exec_extract_stabs(reader, &stabs, &n_stabs))
		goto could_not_extract_stabs;

	for (i= 0; i < n_stabs; i += 1) {
		s= &stabs[i];
		switch (mut_exec_stab_type(s)) {
		case mut_exec_stab_type_LINE: 
			if (base_addr == 0) {
				/* XXX: LINE not after a FUN */
			} else {
				size_t offset= mut_exec_stab_offset(s);
				mut_exec_addr a= mut_exec_addr_inc(base_addr, offset);
				unsigned int l= (unsigned int)mut_exec_stab_line(s);
				if (!mut_exec_symtab_add_range(&exec->symtab, a, l, fn))
					goto could_not_add_range;
			}
			break;
		case mut_exec_stab_type_FUN:
			base_addr= mut_exec_stab_addr(s);
			break;
		case mut_exec_stab_type_PSYM:
			break;
		case mut_exec_stab_type_SO:
			string_index= mut_exec_stab_str(s);
			if (string_index < exec->stabstr.size) {
				char const * x= &exec->stabstr.data[string_index];
				size_t l= strlen(x);
				if (x[l-1] == '/') {
					dn= x;
				} else if (fn == 0) {
					fn= x;
				} else {
					fn= 0;
				}
			} else {
				/* XXX: out of range */
			}
			break;
		default:
			break;
		}
	}
	mut_mem_free(stabs);
	return 1;

could_not_add_range:
	mut_mem_free(stabs);
could_not_extract_stabs:
	return 0;
}


/*
 * Uses the initialised reader->header to read in the section header string
 * table (.shstrtab) and store it in reader->header.strings.
 */
static int
mut_exec_shstrtab_open(mut_exec_reader * reader)
{
	size_t byte_shstrndx = reader->header.e_shstrndx*reader->header.e_shentsize;
	long byte_offset= reader->header.e_shoff + byte_shstrndx;
	mut_elf_shdr header;

	if (fseek(reader->source, byte_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	}
	if (fread(&header, reader->header.e_shentsize, 1, reader->source) != 1) {
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	}
	if (header.sh_type != SHT_STRTAB) {
		mut_log_fatal(reader->log, "elf.shstrtab", 0);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	}
	reader->strings.size= header.sh_size;
	reader->strings.data= mut_mem_malloc(header.sh_size);
	if (reader->strings.data == 0) {
		mut_log_fatal(reader->log, "mem.full", errno);
		(void)mut_log_end(reader->log);
		goto could_not_allocate_strings;
	}
	if (fseek(reader->source, header.sh_offset, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_seek_to_strings;
	}
	if (fread(reader->strings.data, header.sh_size, 1, reader->source) != 1) {
		mut_log_fatal(reader->log, "io.read", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		goto could_not_read_strings;
	}
	return 1;

could_not_read_strings:
could_not_seek_to_strings:
	mut_mem_free(reader->strings.data);
could_not_allocate_strings:
	return 0;
}


static const char *
mut_exec_shstrtab_lookup(mut_exec_reader *reader, mut_elf_word index)
{
	mut_assert_pre(index < reader->strings.size);
	return &reader->strings.data[index];
}


static void mut_exec_shstrtab_close(mut_exec_reader * reader)
{
	mut_mem_free(reader->strings.data);
}


/*
 * Assumes that reader->header contains the header for the executable.
 * Searches the section headers looking for a dynamic symbol
 * table section (DYNSYM/.dynsym) a static symbol table section 
 * (SYMTAB/.symtab) or debugging info. (PROGBITS/.stab).
 *
 * If a SYMTAB section is found then that is taken to be the symbol
 * table for the executable and reader->symtab is initialised.
 *
 * If a DYNSYM section is found then reader->dsymtab is initialised
 * and an effort is still made to find a SYMTAB section if there is one.
 * since that will contain more symbol information.
 *
 * If either a SYMTAB or a DYNSYM section is found then a non-zero
 * value is returned.  If neither is found then 0 is returned 
 * and an error is logged.
 /
/*
 *.protect: Just in case the file contains multiple .dynsym, .symtab
 * and .stab sections, only the first of each type is actually used.
 * Probably should log something if this case is actually hit.
 */
static int mut_exec_locate_sections(mut_exec_reader * reader)
{
	size_t n_headers= reader->header.e_shnum;
	size_t n;

	if (fseek(reader->source, reader->header.e_shoff, SEEK_SET) < 0) {
		mut_log_fatal(reader->log, "io.seek", errno);
		mut_log_string(reader->log, reader->file_name);
		(void)mut_log_end(reader->log);
		return 0;
	}
	reader->section.free= 0;
	reader->symtab= 0;
	reader->dsymtab= 0;
	reader->stabs= 0;
	reader->plt = 0;
	for (n= 1;  n < n_headers;  n+=1) {
		mut_elf_shdr *section_header= &reader->section.header[reader->section.free];
		size_t size= reader->header.e_shentsize;
		char const *section_name;

		if (fread(section_header, size, 1, reader->source) != 1) {
			mut_log_fatal(reader->log, "io.read", errno);
			mut_log_string(reader->log, reader->file_name);
			(void)mut_log_end(reader->log);
			return 0;
		}
		switch (section_header->sh_type) {
		case SHT_DYNSYM:
			if (reader->dsymtab != 0) /* .protect */
				break;
			section_name = mut_exec_shstrtab_lookup(reader, section_header->sh_name);
			if (strcmp(section_name, ".dynsym") == 0) {
				reader->dsymtab= section_header;
				reader->section.free += 1;
			}
			break;
		case SHT_SYMTAB:
			if (reader->symtab != 0) /* .protect */
				break;
			section_name = mut_exec_shstrtab_lookup(reader, section_header->sh_name);
			if (strcmp(section_name, ".symtab") == 0) {
				reader->symtab= section_header;
				reader->section.free += 1;
			}
			break;
		case SHT_PROGBITS:
			section_name = mut_exec_shstrtab_lookup(reader, section_header->sh_name);
			if (strcmp(section_name, ".stab") == 0) {
				if (reader->stabs != 0) /* .protect */
					break;
				reader->stabs= section_header;
				reader->section.free += 1;
			} else if (strcmp(section_name, ".plt") == 0) {
				if (reader->plt != 0) /* .protect */
					break;
				reader->plt = section_header;
				reader->section.free += 1;
			}
			break;
		default:
			break;
		}
	}
	if (reader->dsymtab != 0 || reader->symtab != 0)
		return 1;
	mut_log_fatal(reader->log, "elf.nosymbols", 0);
	mut_log_string(reader->log, reader->file_name);
	(void)mut_log_end(reader->log);
	return 0;
}



int mut_exec_open(mut_exec *exec, char const *file_name, mut_log *log,
		  char const * undesirable_suffix)
{
	mut_exec_reader reader;

	reader.log = log;
	reader.file_name = file_name;

	if ((reader.source = fopen(file_name, "r")) == (FILE *)0) {
		mut_log_fatal(log, "io.open", errno);
		mut_log_string(log, file_name);
		(void)mut_log_end(log);
		return 0;
	}

	if (fread(&reader.header, sizeof(mut_elf_ehdr), 1, reader.source) != 1) {
		mut_log_fatal(log, "io.read", errno);
		mut_log_string(log, file_name);
		(void)mut_log_end(log);
		goto could_not_read_header;
	}

	if (!mut_exec_check_id(&reader))
		goto not_ELF;

	if (!mut_exec_check_class(&reader))
		goto incorrect_ELF_class;

	if (!mut_exec_check_type(&reader))
		goto not_executable;

	if (!mut_exec_shstrtab_open(&reader))
		goto could_not_open_section_header_string_table;

	if (!mut_exec_locate_sections(&reader))
		goto could_not_locate_sections;

	mut_exec_symtab_open(&exec->symtab, reader.log);
	exec->strtab.data = 0;
	exec->dynstr.data = 0;
	if (reader.symtab) {
		if (!mut_exec_strtab_open(exec, &reader, reader.symtab, &exec->strtab))
			goto could_not_open_strtab;
		if (!mut_exec_extract_symbols(exec, &reader, reader.symtab, &exec->strtab, undesirable_suffix, 0))
			goto could_not_open_symtab;
		exec->has_full_symbols = 1;
	}
	if (reader.dsymtab) {
		if (!mut_exec_strtab_open(exec, &reader, reader.dsymtab, &exec->dynstr))
			goto could_not_open_dynstr;
		if (!mut_exec_extract_symbols(exec, &reader, reader.dsymtab, &exec->dynstr, undesirable_suffix, reader.plt))
			goto could_not_open_dynsym;
	}
	
	if (reader.stabs != 0) {
		if (!mut_exec_stabstr_open(exec, &reader))
			goto could_not_open_stabstr;
		if (!mut_exec_extract_debug_info(exec, &reader))
			goto could_not_extract_debug_info;
	} else {
		exec->stabstr.data = 0;
	}

	if (fclose(reader.source) != 0) {
		mut_log_warning(log, "io.close");
		mut_log_string(log, file_name);
		(void)mut_log_end(log);	/* XXX */
	}
	mut_exec_shstrtab_close(&reader);
	exec->log = log;
	return 1;

could_not_extract_debug_info:
	if (reader.stabs != 0)
		mut_exec_stabstr_close(exec);
could_not_open_stabstr:
could_not_open_dynsym:
	mut_exec_strtab_close(&exec->dynstr);
could_not_open_dynstr:
could_not_open_symtab:
	mut_exec_strtab_close(&exec->strtab);
could_not_open_strtab:
	mut_exec_symtab_close(&exec->symtab);
could_not_locate_sections:
	mut_exec_shstrtab_close(&reader);
could_not_open_section_header_string_table:
not_executable:
incorrect_ELF_class:
not_ELF:
could_not_read_header:
	(void)fclose(reader.source);
     return 0;
}



void mut_exec_functions_addr(mut_exec *exec, size_t n, mut_exec_function *fs)
{
	size_t i;
	mut_exec_addr sa;

	for (i = 0;  i < n;  i += 1) {
		sa = mut_exec_symtab_lookup_name(&exec->symtab, fs[i].name);
		fs[i].addr = sa;
		fs[i].flags = (sa != 0);
	}
}



int mut_exec_addr_name(mut_exec *e, mut_exec_addr a, mut_exec_addr *fa,
		       char const **n, char const **fn, unsigned int *l)
{
	if (!e->has_full_symbols)
		return 0;
	return mut_exec_symtab_lookup_addr(&e->symtab, a, fa, n, fn, l);
}



int mut_exec_has_backtrace_symbols(mut_exec *exec)
{
	return exec->has_full_symbols;
}



int mut_exec_close(mut_exec *exec)
{
	if (exec->stabstr.data != 0)
		mut_exec_stabstr_close(exec);
	mut_exec_strtab_close(&exec->strtab);
	mut_exec_strtab_close(&exec->dynstr);
	mut_exec_symtab_close(&exec->symtab);
	return 1;
}
