#ifndef MUT_PROCESS_ARCH_H
#define MUT_PROCESS_ARCH_H

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
