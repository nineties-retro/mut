#ifndef MUT_PROCESS_TYPE_H
#define MUT_PROCESS_TYPE_H

/*
 *.intro: definition of mut_process
 */

typedef enum {
	mut_process_status_running,
	mut_process_status_stopped,
	mut_process_status_exited
} mut_process_status;


struct mut_process {
	pid_t child;
	mut_process_status status;
	mut_trace trace;
	mut_log * log;
	mut_exec_addr pc;
};

typedef struct mut_process mut_process;

#endif
