#include "mut_assert.h"		/* errno */
#include "mut_errno.h"		/* errno */
#include "mut_ptrace.h"		/* ptrace */
#include "mut_log.h"		/* mut_log, mut_log_fatal, ... etc. */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_reg.h"		/* mut_reg */
#include "mut_trace.h"


int mut_trace_read_reg(mut_trace *p, unsigned int reg, mut_reg * value)
{
	int v = ptrace(3, p->pid, reg*mut_reg_size, 0);
	if (v == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.peek.user", errno);
		mut_log_uint(p->log, reg);
		(void)mut_log_end(p->log);
		return 0;
	}
	*value = v;
	return 1;
}


int mut_trace_write_reg(mut_trace *p, unsigned int reg, mut_reg value)
{
	if (ptrace(6, p->pid, reg*mut_reg_size, value) == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.poke.reg", errno);
		mut_log_uint(p->log, reg);
		(void)mut_log_end(p->log);
		return 0;
	}
	return 1;
}
