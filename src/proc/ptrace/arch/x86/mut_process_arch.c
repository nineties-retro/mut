/*
 *.intro: architecture specific ptrace stuff.
 */

#include "mut_stddef.h"		/* size_t */
#include "mut_assert.h"		/* mut_assert_pre */
#include <sys/types.h>		/* pid_t */
#include <sys/ptrace.h>		/* ptrace, PTRACE_TRACEME */
#include "mut_process_arch_regs.h"
#include "mut_log.h"		/* mut_log */
#include "mut_arg.h"		/* mut_arg */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_instr.h"		/* mut_instr */
#include <sys/types.h>		/* pid_t */
#include "mut_trace.h"		/* mut_trace */
#include "mut_backtrace_enter.h" /* mut_backtrace_enter */
#include "mut_process_ref.h"
#include "mut_process_arch.h"


int mut_process_read_pc(mut_process *p, mut_exec_addr *pc)
{
	int eip;
	mut_assert_pre(p->status == mut_process_status_stopped);
	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_eip, &eip))
		return 0;
	*pc= mut_exec_addr_from_int(eip);
	return 1;
}



int mut_process_write_pc(mut_process *p, mut_exec_addr pc)
{
	int eip = mut_exec_addr_to_int(pc);
	mut_assert_pre(p->status == mut_process_status_stopped);
	return mut_trace_write_reg(&p->trace, mut_process_arch_regs_eip, eip);
}



int mut_process_read_arg(mut_process *p, size_t n, mut_arg *arg)
{
	int esp;
	mut_exec_addr arg_addr;

	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_esp, &esp))
		return 0;
	arg_addr = mut_exec_addr_from_int(esp + mut_arg_size + n*mut_arg_size);
	return mut_trace_read_data(&p->trace, arg_addr, arg);
}


int mut_process_write_arg(mut_process *p, size_t n, mut_arg arg)
{
	int esp;
	mut_exec_addr arg_addr;

	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_esp, &esp))
		return 0;
	arg_addr = mut_exec_addr_from_int(esp + mut_arg_size + n*mut_arg_size);
	return mut_trace_write_data(&p->trace, arg_addr, arg);
}



int mut_process_function_return_addr(mut_process *p, mut_exec_addr *addr)
{
	int esp;  mut_exec_addr esp_addr;
	int a;

	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_esp, &esp))
		return 0;
	esp_addr = mut_exec_addr_from_int(esp);
	if (!mut_trace_read_data(&p->trace, esp_addr, &a))
		return 0;
	*addr = mut_exec_addr_from_int(a);
	return 1;
}



int  mut_process_read_result(mut_process *p, mut_arg *result)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	return mut_trace_read_reg(&p->trace, mut_process_arch_regs_eax, result);
}


int mut_process_write_result(mut_process *p, mut_arg result)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	return mut_trace_write_reg(&p->trace, mut_process_arch_regs_eax, result);
}


/*
 * Implements the solution described in <URI:mut/README#sol.backtrace.x86>
 * with the added twist of also terminating the backtrace if the return
 * address is 0.
 *
 * Also implements <URI:mut/README#sol.backtrace.x86.no-fp.inc> to deal with
 * EBP not being used as a frame pointer.
 */
int mut_process_function_backtrace(mut_process * p, mut_backtrace * bt)
{
	int return_addr;
	mut_exec_addr return_addr_addr;
	int esp, ebp, base;
	int status;

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_esp, &esp))
		return 0;
	return_addr_addr = mut_exec_addr_from_int(esp);
	if (!mut_trace_read_data(&p->trace, return_addr_addr, &return_addr))
		return 0;
	return_addr -= 5;		/* size of a call instruction */
	status = mut_backtrace_enter(bt, mut_exec_addr_from_int(return_addr));
	if (status < 0)  return 1;
	if (status == 0) return 0;

	base = esp;
	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_ebp, &ebp))
		return 0;
	for (; ebp != 0 && (unsigned int)ebp > (unsigned int)base;) {
		return_addr_addr= mut_exec_addr_from_int(ebp+mut_arg_size);
		if (!mut_trace_read_data(&p->trace, return_addr_addr, &return_addr))
			return 0;
		if (return_addr == 0)
			return 1;
		return_addr -= 5;		/* size of a call instruction */
		status= mut_backtrace_enter(bt, mut_exec_addr_from_int(return_addr));
		if (status < 0)  return 1;
		if (status == 0) return 0;
		base = ebp;
		if (!mut_trace_read_data(&p->trace, mut_exec_addr_from_int(ebp), &ebp))
			return 0;
	}
	return 1;
}




/*
 * On a x86 using the x86 standard ABI just need to set the pc
 * and pop the return address from the stack to ensure that the
 * function is not called.
 */
int mut_process_skip_to(mut_process *p, mut_exec_addr pc)
{
	int esp;
	mut_assert_pre(p->status == mut_process_status_stopped);

	p->pc = pc;
	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_esp, &esp))
		return 0;
	if (!mut_trace_write_reg(&p->trace, mut_process_arch_regs_esp,
				 esp+mut_arg_size))
		return 0;
	return 1;
}
