/*
 *.intro: A simple dump program for dynamic ELF executables.
 *
 * This program was thrown together in order to extract some information
 * from ELF executables.  It should not be take as an example of good
 * style.
 */

#include <stdio.h>		/* fprintf, sprintf */
#include <stdlib.h>		/* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/types.h>		/* waitpid, open */
#include <fcntl.h>		/* O_RDONLY */
#include <assert.h>		/* assert */
#include <sys/stat.h>		/* stat */
#include <unistd.h>		/* read, stat */
#include <errno.h>		/* errno */
#include <string.h>		/* strerror */
#include <elf.h>

static int fd;
static char const *program_name;
static char const *elf_file_name;
static char *file_contents;
static char **symbol_names;
static Elf32_Shdr *symbol_table_header;
static Elf32_Shdr *hash_table_header;
static Elf32_Shdr *string_table_header;
static Elf32_Shdr *dynamic_table_header;

static void usage(void)
{
	fprintf(stderr, "%s: executable\n", program_name);
}


static int display_type(Elf32_Half type) 
{
	switch(type) {
	case ET_NONE:
		printf(" NONE");
		break;
	case ET_REL:
		printf(" relocatable");
		break;
	case ET_EXEC:
		printf("executable");
		break;
	case ET_DYN:
		printf("shared object");
		break;
	case ET_CORE:
		printf("core");
		break;
	case ET_LOPROC:
		printf("processor specific LO type");
		break;
	case ET_HIPROC:
		printf("processor specific HI type");
		break;
	default:
		printf("UNKNOWN");
	}
	return EXIT_SUCCESS;
}


static char const *locate_section_header_string(Elf32_Word name)
{
	const Elf32_Ehdr *h = (Elf32_Ehdr *)file_contents;
	const Elf32_Shdr *shs = (Elf32_Shdr *)&file_contents[h->e_shoff];
	const Elf32_Shdr *sh = &shs[h->e_shstrndx];

	if (h->e_shstrndx == 0) {
		return "NONE";
	} else {
		const char *st = &file_contents[sh->sh_offset];
		return &st[name];
	}
}

static void locate_table_headers(void)
{
	Elf32_Ehdr * h = (Elf32_Ehdr *)file_contents;
	size_t max = h->e_shnum;
	size_t n;

	for (n = 1; n < max; n += 1) {
		Elf32_Shdr * sh = (Elf32_Shdr *)&file_contents[h->e_shoff+n*h->e_shentsize];
		const char *name = locate_section_header_string(sh->sh_name);

		printf("TYPE %d NAME %s o %u\n", sh->sh_type, name, sh->sh_offset);
		switch (sh->sh_type) {
		case SHT_HASH:
			// sh->sh_link is index of symbol table.
			assert(sh->sh_link <= h->e_shnum);
			printf("HASHTAB %u\n", n);
			symbol_table_header = (Elf32_Shdr *)&file_contents[h->e_shoff + sh->sh_link*h->e_shentsize];
			printf("SYMTAB %d\n", sh->sh_link);
			hash_table_header = sh;
			assert(symbol_table_header->sh_link <= h->e_shnum);
			string_table_header = (Elf32_Shdr *)&file_contents[h->e_shoff+symbol_table_header->sh_link*h->e_shentsize];
			printf("STRTAB %d\n", symbol_table_header->sh_link);
			return;
		case SHT_SYMTAB:
			// sh->sh_link is index of string table.
			if (!symbol_table_header) {
				symbol_table_header = sh;
				string_table_header = (Elf32_Shdr *)&file_contents[h->e_shoff+symbol_table_header->sh_link*h->e_shentsize];
			}
			break;
		case SHT_DYNSYM:
			// sh->sh_link is index of string table.
			symbol_table_header = sh;
			string_table_header = (Elf32_Shdr *)&file_contents[h->e_shoff+symbol_table_header->sh_link*h->e_shentsize];
			break;
		case SHT_STRTAB:
			if (!string_table_header)
				string_table_header = sh;
			break;
		case SHT_PROGBITS:
			if (strcmp(name, ".plt") == 0) {
			} else  {
			}
			break;
		case SHT_DYNAMIC:
			dynamic_table_header = sh;
			break;
		}
	}
}

#if 0

static Elf32_Shdr * locate_symbol_table_header(void)
{
	Elf32_Ehdr * header = (Elf32_Ehdr *)file_contents;
	size_t max = header->e_shnum;
	size_t n;;

	for (n= 1; n < max; n += 1) {
		Elf32_Shdr * dh= (Elf32_Shdr *)&file_contents[header->e_shoff+n*header->e_shentsize];
		if (sh->sh_type == SHT_SYMTAB)
			return sh;
	}
	return 0;
}
#endif

#if 0
static void locate_string_table(char const ** table, size_t *size)
{
	Elf32_Ehdr * header= (Elf32_Ehdr *)file_contents;
	size_t max= header->e_shnum;
	size_t n= 1;

	for (n= 1; n < max; n += 1) {
		Elf32_Shdr * section_header= (Elf32_Shdr *)&file_contents[header->e_shoff+n*header->e_shentsize];
		if (section_header->sh_type == SHT_STRTAB && n != header->e_shstrndx && strcmp(".strtab", locate_section_header_string(section_header->sh_name)) == 0) {
			assert(section_header->sh_offset);
			*table= &file_contents[section_header->sh_offset];
			*size= section_header->sh_size;
			return;
		}
	}
	assert(0);
}

#endif

#if 0
static char const * locate_string(Elf32_Word name)
{
	char const * string;
	static char const * string_table= 0;
	static size_t       string_table_size;

	if (string_table == 0) {
		locate_string_table(&string_table, &string_table_size);
		assert(string_table != (char const *)0);
		assert(string_table_size > 0);
	}

	assert(name < string_table_size);
	string= &string_table[name];
	assert(string);
	return string;
}
#endif


static char const * locate_string(Elf32_Word name)
{
	char const * string;
	static char const * string_table;

	assert(string_table_header);
	string_table= &file_contents[string_table_header->sh_offset];
	assert(name < string_table_header->sh_size);
	string= &string_table[name];
	assert(string);
	return string;
}


static unsigned long elf_hash(const char *name)
{
	const unsigned char *n = (const unsigned char *)name;
	unsigned long h = 0, g;
	while (*n != 0) {
		h = (h<<4) + *n++;
		if ((g= h&0xf0000000) != 0) {
			h ^= g>>24;
			h ^= g;
		}
	}
	return h;
}



static int display_symbols(void)
{
	for (; *symbol_names; symbol_names+=1) {
		printf("XXX %s=%lu\n", *symbol_names, elf_hash(*symbol_names));
	}
	return EXIT_SUCCESS;
}


static int display_symbol_table(void)
{
	Elf32_Sym * symbol;
	size_t n;
	size_t i;

	if (!symbol_table_header)
		return EXIT_SUCCESS;
	symbol= (Elf32_Sym *)&file_contents[symbol_table_header->sh_offset];
	n= symbol_table_header->sh_size;
	assert(n%sizeof(Elf32_Sym) == 0);
	printf("SYMSIZE= %d, %x\n", sizeof(Elf32_Sym), sizeof(Elf32_Sym));
	for (i= 1; n != 0; n-=sizeof(Elf32_Sym), i+=1, symbol+=1) {
		printf("SYM%u (%p) name=%u (%s), addr=%x, size=%u, bind=%u, type=%u, other=%x, shndx=%u\n",
		       i, symbol, symbol->st_name,
		       locate_string(symbol->st_name),
		       symbol->st_value,
		       symbol->st_size,
		       (unsigned int)ELF32_ST_BIND(symbol->st_info),
		       (unsigned int)ELF32_ST_TYPE(symbol->st_info),
		       (unsigned int)symbol->st_other,
		       symbol->st_shndx);
	}
	return EXIT_SUCCESS;
}


static int display_dynamic_table(void)
{
	Elf32_Dyn *d;
	size_t n;
	size_t i;

	if (!dynamic_table_header)
		return EXIT_SUCCESS;
	d = (Elf32_Dyn *)&file_contents[dynamic_table_header->sh_offset];
	n = dynamic_table_header->sh_size;
	assert(sizeof(*d) == dynamic_table_header->sh_entsize);
	assert(n%sizeof(*d) == 0);
	
	for (i = 1; n != 0; n -= sizeof(*d), i += 1, d += 1) {
		printf("DYN%u tag %u val %u %x\n", i, d->d_tag, d->d_un.d_val, d->d_un.d_val);
	}
	return EXIT_SUCCESS;
}


static int display_section_headers(void)
{
	Elf32_Ehdr * h= (Elf32_Ehdr *)file_contents;

	if (h->e_shoff == 0) {
	} else {
		size_t max= h->e_shnum;
		size_t n;

		for (n= 1; n < max; n += 1) {
			Elf32_Shdr * sh= (Elf32_Shdr *)&file_contents[h->e_shoff+n*h->e_shentsize];
			printf("S%u sh_name= %u (%s)\n", n, sh->sh_name, locate_section_header_string(sh->sh_name));
			printf("S%u sh_type= %u\n", n, sh->sh_type);
			printf("S%u sh_flags= %x\n", n, sh->sh_flags);
			printf("S%u sh_addr= %x\n", n, sh->sh_addr);
			printf("S%u sh_offset= %u\n", n, sh->sh_offset);
			printf("S%u sh_size= %u\n", n, sh->sh_size);
			printf("S%u sh_link= %u\n", n, sh->sh_link);
			printf("S%u sh_info= %u\n", n, sh->sh_info);
			printf("S%u sh_addralign= %x\n", n, sh->sh_addralign);
			printf("S%u sh_entsize= %u\n", n, sh->sh_entsize);
		}
	}
	return EXIT_SUCCESS;
}


static int display_program_headers(void)
{
	Elf32_Ehdr * h= (Elf32_Ehdr *)file_contents;

	if (h->e_phoff == 0) {
	} else {
		size_t max= h->e_phnum;
		size_t n;

		for (n= 0; n < max; n += 1) {
			Elf32_Phdr * ph= (Elf32_Phdr *)&file_contents[h->e_phoff+n*h->e_phentsize];
			printf("P%u type %u ", n, ph->p_type);
			printf("offset %u ", ph->p_offset);
			printf("vaddr %x ", ph->p_vaddr);
			printf("filesz %u ", ph->p_filesz);
			printf("memsz %u ", ph->p_memsz);
			printf("flags %x ", ph->p_flags);
			printf("align %x\n", ph->p_align);
		}
	}
	return EXIT_SUCCESS;
}

static int display_header(void)
{
	Elf32_Ehdr * h= (Elf32_Ehdr *)file_contents;

	printf("E type= %d (", h->e_type);
	display_type(h->e_type);  printf(")\n");
	printf("E phentsize= %u\n", h->e_phentsize);
	printf("E phnum= %u\n", h->e_phnum);
	printf("E shentsize= %u\n", h->e_shentsize);
	printf("E shnum= %u\n", h->e_shnum);
	printf("E shstrndx= %u\n", h->e_shstrndx);
	return EXIT_SUCCESS;
}


static int debug(void)
{
	struct stat st;
	if ((fd= open(elf_file_name, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: could not open %s due to %s\n", program_name, elf_file_name, strerror(errno));
		goto could_not_open_executable;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "%s: could not determine size of %s due to %s\n", program_name, elf_file_name, strerror(errno));
		goto could_not_stat_file;
	}

	if ((file_contents= malloc(st.st_size)) == (void *)0) {
		fprintf(stderr, "%s: could not malloc due to %s\n", program_name, strerror(errno));
		goto could_not_malloc;
	}

	if (read(fd, file_contents, st.st_size) != st.st_size) {
		fprintf(stderr, "%s: could not read %s header due to %s\n", program_name, elf_file_name, strerror(errno));
		goto could_not_read_file;
	}

	if (display_header () == EXIT_FAILURE)
		goto could_not_display_header;

	locate_table_headers();

	if (display_program_headers() == EXIT_FAILURE)
		goto could_not_display_program_headers;

	if (display_section_headers() == EXIT_FAILURE)
		goto could_not_display_section_headers;

	if (display_symbol_table() == EXIT_FAILURE)
		goto could_not_display_symbol_table;

	if (display_dynamic_table() == EXIT_FAILURE)
		goto could_not_display_dynamic_table;

	if (display_symbols() == EXIT_FAILURE)
		goto could_not_display_symbols;

	free(file_contents);

	if ((fd= close(fd)) < 0) {
		fprintf(stderr, "%s: could not close %s due to %s\n", program_name, elf_file_name, strerror(errno));
		goto could_not_close_executable;
	}
	return EXIT_SUCCESS;

could_not_display_symbols:
could_not_display_dynamic_table:
could_not_display_symbol_table:
could_not_display_section_headers:
could_not_display_program_headers:
could_not_display_header:
could_not_read_file:
could_not_malloc:
	free(file_contents);
could_not_stat_file:
	(void)close(fd);
could_not_open_executable:
could_not_close_executable:
	return EXIT_FAILURE;
}


int main(int argc, char **argv)
{
	program_name= argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-') {
		switch(argv[0][1]) {
		default:
			usage();
			return EXIT_FAILURE;
		}
	}
	if (argc == 0) {
		usage();
		return EXIT_FAILURE;
	}
	elf_file_name = argv[0];
	symbol_names = argv+1;
	return debug();
}
