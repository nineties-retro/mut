#ifndef MUT_ELF_TYPES_H
#define MUT_ELF_TYPES_H

#include <mut_elf.h>

typedef Elf32_Word mut_elf_word;
typedef Elf32_Half mut_elf_half;
typedef Elf32_Ehdr mut_elf_ehdr;
typedef Elf32_Shdr mut_elf_shdr;
typedef Elf32_Sym mut_elf_sym;
typedef Elf32_Addr mut_elf_addr;

#define mut_elf_st_type(t) (ELF32_ST_TYPE(t))

#define mut_elf_class ELFCLASS64

#endif
