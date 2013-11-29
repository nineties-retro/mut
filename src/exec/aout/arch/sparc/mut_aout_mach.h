#ifndef MUT_AOUT_ARCH_H
#define MUT_AOUT_ARCH_H

#include <a.out.h>

#define mut_aout_mach M_SPARC

/* The following is a cheap hack and should be done in a cleaner manner
 * i.e. the following are not SPARC specific, they are SunOS 4.1.X specific.
 */

#define N_MAGIC(x) ((x).a_magic)
#define N_MACHTYPE(x) ((x).a_machtype)

#endif
