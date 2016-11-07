/*
 *.intro: ptrace version of mut_process
 *
 * See <URI:mut/src/proc/mut_process.h> for a description of what each
 * public routine in this file does.
 */


#include <unistd.h>		/* fork, waitpid */
#include <signal.h>		/* SIGTRAP */
#include <limits.h>		/* CHAR_BIT */
#include <sys/wait.h>		/* waitpid */
#include "mut_stdlib.h"		/* EXIT_FAILURE */
#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_errno.h"		/* errno */
#include "mut_log.h"		/* mut_log */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_instr.h"		/* mut_instr, mut_instr_bp */
#include "mut_pc.h"		/* mut_pc_offset, mut_pc_adjust */
#include "mut_backtrace_sharer.h" /* mut_backtrace */
#include "mut_process.h"

int mut_process_open(mut_process *p, char **argv, mut_log *log)
{
	pid_t pid = fork();
	if (pid < 0) {
		mut_log_fatal(log, "fork", errno);
		mut_log_string(log, argv[0]);
		(void)mut_log_end(log);
		return 0;
	} if (pid == 0) {
		if (!mut_trace_trace_me(log))
			_exit(EXIT_FAILURE);
		execv(argv[0], argv);
		mut_log_fatal(log, "execv", errno);
		mut_log_string(log, argv[0]);
		(void)mut_log_end(log);
		_exit(EXIT_FAILURE);
	} else {
		int status;
		p->status = mut_process_status_running;
		p->log = log;
		p->child = pid;
		mut_trace_open(&p->trace, pid, log);
		if (waitpid(pid, &status, 0) < 0) {
			mut_log_fatal(p->log, "waitpid", errno);
			mut_log_uint(p->log, (unsigned int)pid);
			mut_log_uint_hex(p->log, (unsigned int)status);
			(void)mut_log_end(p->log);
			return 0;
		}
		if (WIFEXITED(status)) {
			p->status = mut_process_status_exited;
			mut_log_fatal(p->log, "process.exit", 0);
			(void)mut_log_end(p->log);
			return 0;
		} else if (WIFSIGNALED(status)) {
			mut_log_fatal(p->log, "process.signal", 0);
			(void)mut_log_end(p->log);
			return 0;
		} else if (WIFSTOPPED(status)) {
			p->status = mut_process_status_stopped;
			if (mut_process_read_pc(p, &p->pc) <= 0)
				return 0;
			return 1;
		} else {
			mut_log_fatal(p->log, "process.unkown", 0);
			(void)mut_log_end(p->log);
			return 0;
		}
	}
	return 0;		/* keep dumb compilers happy */
}



int mut_process_wait(mut_process *p, mut_exec_addr *pc)
{
	int status;
	mut_assert_pre(p->status == mut_process_status_running);

loop:
	if (waitpid(p->child, &status, 0) < 0) {
		mut_log_fatal(p->log, "waitpid", errno);
		mut_log_int(p->log, status);
		(void)mut_log_end(p->log);
		return 0;
	}
	if (WIFEXITED(status)) {
		mut_log_info(p->log, "process.exited");
		mut_log_int(p->log, WEXITSTATUS(status));
		mut_log_end(p->log);
		p->status = mut_process_status_exited;
		return -1;
	} else if (WIFSIGNALED(status)) {
		mut_log_info(p->log, "process.signaled");
		mut_log_int(p->log, WTERMSIG(status));
		mut_log_end(p->log);
		p->status = mut_process_status_exited;
		return -1;
	} else if (WIFSTOPPED(status)) {
		if (WSTOPSIG(status) == SIGTRAP) {
			int pc_status;
			p->status = mut_process_status_stopped;
			pc_status = mut_process_read_pc(p, &p->pc);
			if (pc_status < 0) {
				if (!mut_trace_continue(&p->trace, 0))
					return 0;
				goto loop;
			} else if (pc_status == 0) {
				return 0;
			} else {
				*pc = p->pc = mut_pc_adjust(p->pc);
				return 1;
			}
		} else {
			if (!mut_trace_continue(&p->trace, WSTOPSIG(status)))
				return 0;
			goto loop;
		}
	} else {
		mut_log_info(p->log, "process.unknown");
		mut_log_uint_hex(p->log, (unsigned int)status);
		(void)mut_log_end(p->log);
		if (!mut_trace_continue(&p->trace, 0))
			return 0;
		goto loop;
	}
}



int mut_process_breakpoint(mut_process *p, mut_exec_addr pc, mut_instr *instr)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_trace_read_text(&p->trace, pc, instr))
		return 0;
	return mut_trace_write_text(&p->trace, pc, mut_instr_bp);
}


int mut_process_restore(mut_process * p, mut_exec_addr pc, mut_instr instr)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	return mut_trace_write_text(&p->trace, pc, instr);
}


int mut_process_resume(mut_process *p)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_process_write_pc(p, p->pc))
		return 0;
	if (!mut_trace_continue(&p->trace, 0))
		return 0;
	p->status = mut_process_status_running;
	return 1;
}


int mut_process_read_word(mut_process *p, mut_exec_addr addr, mut_reg *n)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(mut_exec_addr_aligned(addr, sizeof(*n)));

	return mut_trace_read_data(&p->trace, addr, n);
}


int mut_process_write_word(mut_process *p, mut_exec_addr addr, mut_reg n)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(mut_exec_addr_aligned(addr, sizeof(n)));

	return mut_trace_write_data(&p->trace, addr, n);
}


/*
 * 0 -> 11111111111111111111111111111111
 * 1 -> 00000000111111111111111111111111
 * 2 -> 00000000000000001111111111111111
 * 3 -> 00000000000000000000000011111111
 */
#define mut_process_byte_mask(nbytes) \
	((((mut_reg)1)<<((sizeof(mut_reg)-(nbytes))*CHAR_BIT))-1)


/*
 * This type of routine highlights one of the problems with the ptrace
 * interface -- reading/writing n bytes of data requires O(n) context
 * switches.
 */
int mut_process_memcpy(mut_process *p, mut_exec_addr dest, mut_exec_addr src, size_t n)
{
	mut_reg data;
	size_t nwords = n/sizeof(mut_reg);
	size_t nbytes = n%sizeof(mut_reg);

	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(mut_exec_addr_aligned(dest, sizeof(mut_reg)));
	mut_assert_pre(mut_exec_addr_aligned(src, sizeof(mut_reg)));

	while (nwords != 0) {
		if (!mut_trace_read_data(&p->trace, src, &data))
			return 0;
		if (!mut_trace_write_data(&p->trace, dest, data))
			return 0;
		dest = mut_exec_addr_inc(dest, sizeof(mut_reg));
		src = mut_exec_addr_inc(src, sizeof(mut_reg));
		nwords -= 1;
	}
	if (nbytes != 0) {
		mut_reg orig;
		if (!mut_trace_read_data(&p->trace, dest, &orig))
			return 0;
		if (!mut_trace_read_data(&p->trace, src, &data))
			return 0;
		data |= orig&mut_process_byte_mask(nbytes);
		if (!mut_trace_write_data(&p->trace, dest, data))
			return 0;
	}
	return 1;
}



/*
 * This type of routine highlights one of the problems with the ptrace
 * interface -- reading/writing n bytes of data requires O(n) context
 * switches.
 */
int mut_process_memset(mut_process *p, mut_exec_addr addr, int c, size_t n)
{
	size_t nwords = n/sizeof(mut_reg);
	size_t nbytes = n%sizeof(mut_reg);
	unsigned int pattern = c | (c<<8) | (c<<16) | (c<<24);
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(mut_exec_addr_aligned(addr, sizeof(mut_reg)));

	while (nwords != 0) {
		if (!mut_trace_write_data(&p->trace, addr, pattern))
			return 0;
		addr = mut_exec_addr_inc(addr, sizeof(int));
		nwords -= 1;
	}
	if (nbytes != 0) {
		mut_reg orig;
		if (!mut_trace_read_data(&p->trace, addr, &orig))
			return 0;
		pattern |= orig&mut_process_byte_mask(nbytes);
		if (!mut_trace_write_data(&p->trace, addr, pattern))
			return 0;
	}
	return 1;
}



void mut_process_close(mut_process *p)
{
	mut_assert_pre(p->status != mut_process_status_running);
	if (p->status != mut_process_status_exited)
		mut_trace_kill(&p->trace);
	mut_trace_close(&p->trace);
}
