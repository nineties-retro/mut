#ifndef MUT_ELF_TYPES_H
#define MUT_ELF_TYPES_H

#include <mut_elf.h>

typedef Elf64_Word mut_elf_word;
typedef Elf64_Half mut_elf_half;
typedef Elf64_Ehdr mut_elf_ehdr;
typedef Elf64_Shdr mut_elf_shdr;
typedef Elf64_Sym mut_elf_sym;

typedef Elf64_Addr mut_elf_addr;

#define mut_elf_st_type(t) (ELF64_ST_TYPE(t))

#define mut_elf_class ELFCLASS64

#endif
