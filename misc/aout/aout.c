/*
 *.intro: Dumps out the string and symbol tables for a a.out file.
 *
 */

#include <a.out.h>		/* struct exec */
#include <errno.h>		/* errno */
#include <string.h>		/* strerror */
#include <stdio.h>		/* fprintf, sprintf */
#include <stdlib.h>		/* EXIT_SUCCESS, EXIT_FAILURE */
#include <assert.h>		/* assert */

static void usage(char const * program_name)
{
	fprintf(stderr, "%s: executable\n", program_name);
}



static int debug(char const * program_name, char * aout_file_name)
{
	FILE *aout_file;
	struct exec header;
	struct nlist * symbol;
	char         * string;
	size_t i;
	size_t str_size;

	if ((aout_file= fopen(aout_file_name, "r")) == (FILE *)0) {
		fprintf(stderr, "%s: could not open %s because %s\n", program_name, aout_file_name, strerror(errno));
		goto could_not_open_aout_file_name;
	}
  
	if ((fread(&header, sizeof(struct exec), 1, aout_file)) != 1) {
		fprintf(stderr, "%s: could not read header from %s because %s\n", program_name, aout_file_name, strerror(errno));
		goto could_not_read_aout_header;
	}

	printf("NSYMS= %u, SYMS= %u, STRS= %u\n", header.a_syms/sizeof(struct nlist), N_SYMOFF(header), N_STROFF(header));

	assert(header.a_syms%sizeof(struct nlist) == 0);

	if ((symbol= malloc(header.a_syms)) == 0) {
		fprintf(stderr, "%s: could not allocate space for symbols because %s\n", program_name, strerror(errno));
		goto could_not_allocate_space_for_symbols;
	}

	if (fseek(aout_file, N_SYMOFF(header), SEEK_SET) < 0) {
		fprintf(stderr, "%s: could not seek to symbols because %s\n", program_name, strerror(errno));
		goto could_not_seek_to_symbols;
	}

	if (fread(symbol, 1, header.a_syms, aout_file) != header.a_syms) {
		fprintf(stderr, "%s: could not read symbols because %s\n", program_name, strerror(errno));
		goto could_not_read_symbols;
	}


	if (fseek(aout_file, N_STROFF(header), SEEK_SET) < 0) {
		fprintf(stderr, "%s: could not seek to strings because %s\n", program_name, strerror(errno));
		goto could_not_seek_to_string_table;
	}

	if (fread(&str_size, sizeof(size_t), 1, aout_file) != 1) {
		fprintf(stderr, "%s: could not read string table size because %s\n", program_name, strerror(errno));
		goto could_not_read_string_table_size;
	}

	printf("STR_SIZE= %d\n", str_size);

	if ((string= malloc(str_size)) == 0) {
		fprintf(stderr, "%s: could not allocate space for strings because %s\n", program_name, strerror(errno));
		goto could_not_allocate_space_for_strings;
	}

	if (fread(&string[sizeof(size_t)], 1, str_size-sizeof(size_t), aout_file) != str_size-sizeof(size_t)) {
		fprintf(stderr, "%s: could not read strings because %s\n", program_name, strerror(errno));
		goto could_not_read_strings;
	}

	for (i= 0; i< header.a_syms/sizeof(struct nlist); i++) {
		struct nlist *s= &symbol[i];
		int type= s->n_type;
		assert(s->n_un.n_strx < str_size);
		printf("%u x= %x (%s), type %d/%x/%o, value %lu/%ld/%x\n", (unsigned)i, s->n_un.n_strx, &string[s->n_un.n_strx], type, type, type, s->n_value, s->n_value, s->n_value);
		if (type == N_UNDF)
			printf("UNDEF");
		if ((type & N_ABS) == N_ABS)
			printf("ABS");
		if ((type & N_TEXT) == N_TEXT)
			printf(" TEXT");
		if ((type & N_DATA) == N_DATA)
			printf(" DATA");
		if ((type & N_BSS) == N_BSS)
			printf(" BSS");
		if ((type & N_FN) == N_FN)
			printf(" FN");

		if ((type & N_EXT) == N_EXT)
			printf(" EXT");
		if ((type & N_TYPE) == N_TYPE)
			printf(" TYPE");
		if ((type & N_STAB) == N_STAB)
			printf(" STAB");
		printf("\n");
	}

	free(symbol);
	if (fclose(aout_file) != 0) {
		fprintf(stderr, "%s: could not open %s because %s\n", program_name, aout_file_name, strerror(errno));
		goto could_not_close_aout_file_name;
	}
	return EXIT_SUCCESS;

could_not_read_strings:
	free(string);
could_not_allocate_space_for_strings:
could_not_read_string_table_size:
could_not_seek_to_string_table:
could_not_read_symbols:
could_not_seek_to_symbols:
	free(symbol);
could_not_allocate_space_for_symbols:
could_not_read_aout_header:
	(void)fclose(aout_file);
could_not_open_aout_file_name:
could_not_close_aout_file_name:
	return EXIT_FAILURE;
}



int main(int argc, char **argv)
{
	char const * program_name= argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-') {
		switch(argv[0][1]) {
		default:
			usage(program_name);
			return EXIT_FAILURE;
		}
	}
	if (argc != 1) {
		usage(program_name);
		return EXIT_FAILURE;
	}
	return debug(program_name, argv[0]);
}
