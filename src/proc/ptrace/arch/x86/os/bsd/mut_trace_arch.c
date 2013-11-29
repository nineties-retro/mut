#include <machine/reg.h>
#include "mut_assert.h"		/* errno */
#include "mut_errno.h"		/* errno */
#include "mut_ptrace.h"		/* ptrace */
#include "mut_log.h"		/* mut_log, mut_log_fatal, ... etc. */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_reg.h"		/* mut_reg */
#include "mut_trace.h"

/*
 * OpenBSD does not support PTRACE_PEEKUSER which is the interface
 * that was used under Linux and Solaris.  So as a quick hack,
 * GETREGS is used to grab all the registers and the individual
 * register value is extracted from the group.  This is is obviously
 * not particularly efficient.  It would be better to re-design the
 * interface to allow some sort of object to be cached (the register
 * set) and have lookups go to that.
 */

int mut_trace_read_reg(mut_trace *p, unsigned int reg, mut_reg * value)
{
	struct reg regs;
	int v = ptrace(PT_GETREGS, p->pid, (caddr_t)&regs, 0);
	if (v == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.peek.user", errno);
		mut_log_uint(p->log, reg);
		(void)mut_log_end(p->log);
		return 0;
	}
	switch (reg) {
	case tEAX:
		*value = regs.r_eax;
		break;
	case tEBP:
		*value = regs.r_ebp;
		break;
	case tEIP:
		*value = regs.r_eip;
		break;
	case tESP:
		*value = regs.r_esp;
		break;
	default:
		mut_assert(0);
	}
	return 1;
}


int mut_trace_write_reg(mut_trace *p, unsigned int reg, mut_reg value)
{
	struct reg regs;
	int v = ptrace(PT_GETREGS, p->pid, (caddr_t)&regs, 0);
	if (v == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.peek.user", errno);
		mut_log_uint(p->log, reg);
		(void)mut_log_end(p->log);
		return 0;
	}
	switch (reg) {
	case tEAX:
		regs.r_eax = value;
		break;
	case tEBP:
		regs.r_ebp = value;
		break;
	case tEIP:
		regs.r_eip = value;
		break;
	case tESP:
		regs.r_esp = value;
		break;
	default:
		mut_assert(0);
	}
	if (ptrace(PT_SETREGS, p->pid, (caddr_t)&regs, 0) == -1 && errno != 0) {
		mut_log_fatal(p->log, "ptrace.poke.reg", errno);
		mut_log_uint(p->log, reg);
		(void)mut_log_end(p->log);
		return 0;
	}
	return 1;
}
