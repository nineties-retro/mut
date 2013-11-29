/*
 * $Id$
 *
 *.intro: a wrapper for ptrace to make it more typesafe.
 */

#include <sys/types.h>		/* pid_t */
#include "mut_errno.h"		/* errno */
#include "mut_ptrace.h"		/* ptrace */
#include "mut_log.h"		/* mut_log, mut_log_fatal, ... etc. */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_reg.h"		/* mut_reg */
#include "mut_trace.h"


void mut_trace_open(mut_trace *p, pid_t pid, mut_log * log)
{
	p->pid = pid;
	p->log = log;
}


int mut_trace_trace_me(mut_log *log)
{
	if (ptrace(PT_TRACE_ME, (pid_t)0, 0, 0) < 0) {
		mut_log_fatal(log, "ptrace.traceme", errno);
		(void)mut_log_end(log);
		return 0;
	}
	return 1;
}



int mut_trace_read_text(mut_trace *p, mut_exec_addr addr, int *data)
{
	void *a = mut_exec_addr_to_voidp(addr);
	int d = ptrace(PT_READ_I, p->pid, a, 0);

	if (d == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.peek.text", errno);
		mut_log_exec_addr(p->log, addr);
		(void)mut_log_end(p->log);
		return 0;
	}
	*data = d;
	return 1;
}



int mut_trace_read_data(mut_trace *p, mut_exec_addr addr, int *data)
{
	void * a = mut_exec_addr_to_voidp(addr);
	int d = ptrace(PT_READ_D, p->pid, a, 0);

	if (d == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.peek.data", errno);
		mut_log_exec_addr(p->log, addr);
		(void)mut_log_end(p->log);
		return 0;
	}
	*data = d;
	return 1;
}



int mut_trace_write_text(mut_trace *p, mut_exec_addr addr, int value)
{
	void * a = mut_exec_addr_to_voidp(addr);

	if (ptrace(PT_WRITE_I, p->pid, a, value) == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.poke.text", errno);
		mut_log_exec_addr(p->log, addr);
		(void)mut_log_end(p->log);
		return 0;
	}
	return 1;
}


int mut_trace_write_data(mut_trace *p, mut_exec_addr addr, int value)
{
	void * a = mut_exec_addr_to_voidp(addr);
	if (ptrace(PT_WRITE_D, p->pid, a, value) == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.poke.data", errno);
		mut_log_exec_addr(p->log, addr);
		(void)mut_log_end(p->log);
		return 0;
	}
	return 1;
}



int mut_trace_continue(mut_trace *p, unsigned int signal_number)
{
	if (ptrace(PT_CONTINUE, p->pid, (void*)1, signal_number) < 0) {
		mut_log_fatal(p->log, "ptrace.cont", errno);
		mut_log_uint(p->log, signal_number);
		(void)mut_log_end(p->log);
		return 0;
	}
	return 1;
}


void mut_trace_kill(mut_trace *p)
{
	if (ptrace(PT_KILL, p->pid, 0, 0) < 0) {
		mut_log_info(p->log, "ptrace.kill");
		mut_log_uint(p->log, (unsigned int)p->pid);
		(void)mut_log_end(p->log);
	}
}


void mut_trace_close(mut_trace *p)
{
}
