#ifndef MUT_INSTR_H
#define MUT_INSTR_H

#include "mut_reg.h"

typedef mut_reg mut_instr;

#define mut_instr_to_reg(xx) (xx)
#define mut_instr_from_reg(xx) (xx)
#define mut_instr_to_int(xx) (xx)

/*
** <URI:mut/bib/pentium#breakpoint.instr>
*/
#define mut_instr_bp 0xcc

#endif
