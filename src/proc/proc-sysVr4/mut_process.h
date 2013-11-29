#ifndef MUT_PROCESS_H
#define MUT_PROCESS_H

int mut_process_open(mut_process *, char **argv, mut_log *);

int mut_process_wait(mut_process *, mut_exec_addr);

int mut_process_breakpoint(mut_process *, mut_exec_addr, mut_instr *);

int mut_process_restore(mut_process *, mut_exec_addr, mut_instr);

int mut_process_resume(mut_process *);

int mut_process_read_word(mut_process *, mut_exec_addr, int *);

int mut_process_write_word(mut_process *, mut_exec_addr, int);

int mut_process_memcpy(mut_process *, mut_exec_addr, mut_exec_addr, size_t);

int mut_process_memset(mut_process *, mut_exec_addr, int, size_t);

void mut_process_close(mut_process *);

#endif
