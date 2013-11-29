#ifndef MUT_ELF_H
#define MUT_ELF_H

/*
 *.intro: A Linux 1.3.4 specific version of elf.h
 *
 * <elf.h> appears to be missing STN_UNDEF, so just define it.
 */

#include <elf.h>

#define STN_UNDEF 0

#endif
