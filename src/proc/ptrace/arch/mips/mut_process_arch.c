/*
 *.intro: architecture specific ptrace stuff.
 *
 */

#include "mut_stddef.h"		/* size_t */
#include "mut_assert.h"		/* mut_assert_pre */
#include <sys/types.h>		/* caddr_t (needed by sys/reg.h) */
#include <sys/user.h>		/* PCB_PC, PCB_SP */
#include "mut_log.h"		/* mut_log */
#include "mut_arg.h"		/* mut_arg */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_instr.h"		/* mut_instr */
#include "mut_trace.h"		/* mut_trace */
#include "mut_backtrace_enter.h" /* mut_backtrace_enter */
#include "mut_process_ref.h"
#include "mut_process_arch.h"

int mut_process_read_pc(mut_process * p, mut_exec_addr * pc)
{
	int pc_reg;
	mut_assert_pre(p->status == mut_process_status_stopped);
	if (!mut_trace_read_reg(&p->trace, PCB_PC, &pc_reg))
		return 0;
	*pc= mut_exec_addr_from_int(pc_reg);
	return 1;
}


int mut_process_write_pc(mut_process * p, mut_exec_addr addr)
{
	int pc= mut_exec_addr_to_int(addr);
	mut_assert_pre(p->status == mut_process_status_stopped);
	return mut_trace_write_reg(&p->trace, PC, pc);
}

/*
 * <URI:mut/bib/mips#arg>
 */
#define mut_process_n_arg_regs 4


/*
 * <URI:mut/bib/mips#arg>
 */
int mut_process_read_arg(mut_process * p, size_t n, mut_arg * arg)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(n < mut_process_n_arg_regs);
	return mut_trace_read_reg(&p->trace, a0+n, arg);
}



/*
 * <URI:mut/bib/mips#arg>
 */
int mut_process_write_arg(mut_process * p, size_t n, mut_arg arg)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(n < mut_process_n_arg_regs);
	return mut_trace_write_reg(&p->trace, a0+n, arg);
}



/*
 * <URI:mut/bib/mips#call/return>
 */
int mut_process_function_return_addr(mut_process *p, mut_exec_addr *addr)
{
	int a;

	mut_assert_pre(p->status == mut_process_status_stopped);
	if (!mut_trace_read_reg(&p->trace, ra, &a))
		return 0;
	*addr = mut_exec_addr_from_int(a);
	return 1;
}



/*
 * <URI:mut/bib/mips#result>
 */
int mut_process_function_result(mut_process *p, mut_arg *result)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	return mut_trace_read_reg(&p->trace, v0, result);
}


/*
 * XXX: fix this.
 */
static int mut_process_function_xxx_backtrace(mut_process *p, mut_backtrace *bt)
{
	mut_exec_addr fp_addr, link_addr;
	int           fp,      link;
	int status;

	if (!mut_trace_read_reg(&p->trace, PCB_SP, &fp))
		return 0;
	for (; fp != 0; ) {
		link_addr= mut_exec_addr_from_int(fp + 15*mut_reg_size);
		if (!mut_trace_read_data(&p->trace, link_addr, &link))
			return 0;
		status= mut_backtrace_enter(bt, mut_exec_addr_from_int(link));
		if (status < 0)  return 1;
		if (status == 0) return 0;
		fp_addr= mut_exec_addr_from_int(fp + 14*mut_reg_size);
		if (!mut_trace_read_data(&p->trace, fp_addr, &fp))
			return 0;
	}
	return 1;
}



int mut_process_function_backtrace(mut_process *p, mut_backtrace *bt)
{
	int sp, status;
	int ra;

	if (!mut_trace_read_reg(&p->trace, PCB_SP, &sp))
		return 0;
	_addr = mut_exec_addr_from_int(fp + 15*mut_reg_size);
	if (!mut_trace_read_data(&p->trace, , &ra))
		return 0;
	status = mut_backtrace_enter(bt, mut_exec_addr_from_int(link));
	if (status < 0)  return 1;
	if (status == 0) return 0;
	return mut_process_function_xxx_backtrace(p, bt);
}



/*
 * XXX: fix this.
 */
int mut_process_skip_to(mut_process *p, mut_exec_addr pc)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	p->pc= pc;
	return 1;
}
