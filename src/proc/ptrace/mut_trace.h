#ifndef MUT_TRACE_H
#define MUT_TRACE_H

/*
 *.intro: interface to mut_trace.
 */

struct mut_trace {
	pid_t pid;
	mut_log *log;
};

typedef struct mut_trace mut_trace;

void mut_trace_open(mut_trace *, pid_t, mut_log *);

int mut_trace_trace_me(mut_log *);

int mut_trace_read_text(mut_trace *, mut_exec_addr, mut_reg *);

int mut_trace_read_data(mut_trace *, mut_exec_addr, mut_reg *);

int mut_trace_read_reg(mut_trace *, unsigned int, mut_reg *);

int mut_trace_write_text(mut_trace *, mut_exec_addr, mut_reg);

int mut_trace_write_data(mut_trace *, mut_exec_addr, mut_reg);

int mut_trace_write_reg(mut_trace *, unsigned int, mut_reg);

int mut_trace_continue(mut_trace *, unsigned int);

void mut_trace_kill(mut_trace *);

void mut_trace_close(mut_trace *);

#endif
