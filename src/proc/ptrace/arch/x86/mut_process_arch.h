#ifndef MUT_PROCESS_ARCH_H
#define MUT_PROCESS_ARCH_H

/*
 * `mut_process_read_pc' and `mut_process_write_pc' are only being
 * exported to mut_process.c rather than to anything that uses
 * mut_process.h.  Consequently they really should be in a separate
 * header file, but since they don't require anything that isn't
 * already being imported for use by other routines, they are being
 * piggy-backed on this file.
 */
int mut_process_read_pc(mut_process *, mut_exec_addr *);

int mut_process_write_pc(mut_process *, mut_exec_addr);

int mut_process_read_arg(mut_process *, size_t, mut_arg *);

int mut_process_write_arg(mut_process *, size_t, mut_arg);

int mut_process_function_return_addr(mut_process *, mut_exec_addr *);

int mut_process_function_backtrace(mut_process *, mut_backtrace *);

int mut_process_read_result(mut_process *, mut_arg *);

int mut_process_write_result(mut_process *, mut_arg);

int mut_process_skip_to(mut_process *, mut_exec_addr);

#endif
