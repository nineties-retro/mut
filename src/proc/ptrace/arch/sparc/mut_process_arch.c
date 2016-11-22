/*
 *.intro: architecture specific ptrace stuff.
 *
 */

#include <sys/types.h>
#include "mut_stddef.h"		/* size_t */
#include "mut_assert.h"		/* mut_assert_pre */
#include "mut_log.h"		/* mut_log */
#include "mut_arg.h"		/* mut_arg */
#include "mut_exec_addr.h"	/* mut_exec_addr */
#include "mut_log_exec_addr.h"	/* mut_log_exec_addr */
#include "mut_instr.h"		/* mut_instr */
#include "mut_trace.h"		/* mut_trace */
#include "mut_backtrace_enter.h" /* mut_backtrace_enter */
#include "mut_process_ref.h"
#include "mut_process_arch.h"
#include "mut_process_arch_regs.h"

/*
 * See <URI:mut/log/19970315>
 *
 * The test isn't as accurate as it could be.  It only checks :-
 *
 * a) the preceeding instruction
 * b) only partially decodes the instruction enough to determine that
 *    it is a jmp with %g1
 *
 * XXX: This should be made more robust if the program is going to
 * be used seriously.
 */
static int mut_process_arch_is_dynamic_link(mut_process *p, int pc)
{
	mut_exec_addr prev_addr = mut_exec_addr_from_int(pc-mut_instr_bytes);
	int prev_instr;

	if (!mut_trace_read_text(&p->trace, prev_addr, &prev_instr))
		return -1;
	return ((prev_instr & 0xfffff000) == 0x81c06000);
}


int mut_process_read_pc(mut_process * p, mut_exec_addr * pc)
{
	int pc_reg;
	int npc_reg;

	mut_assert_pre(p->status == mut_process_status_stopped);
	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_pc, &pc_reg))
		return 0;
	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_npc, &npc_reg))
		return 0;
	if (npc_reg != pc_reg + mut_instr_bytes) {
		int xxx= mut_process_arch_is_dynamic_link(p, pc_reg);
		if (xxx < 0) {
			return 0;
		} else if (xxx > 0) {
			pc_reg = npc_reg;
			npc_reg += mut_instr_bytes;
			if (!mut_trace_write_reg(&p->trace, mut_process_arch_regs_pc, pc_reg))
				return 0;
			if (!mut_trace_write_reg(&p->trace, mut_process_arch_regs_npc, npc_reg))
				return 0;
			return -1;
		} else {
			mut_log_info(p->log, "sparc.pc");
			mut_log_uint_hex(p->log, (unsigned int)pc_reg);
			mut_log_uint_hex(p->log, (unsigned int)npc_reg);
			(void)mut_log_end(p->log);
		}
	}
	*pc= mut_exec_addr_from_int(pc_reg);
	return 1;
}



int
mut_process_write_pc(mut_process * p, mut_exec_addr addr)
{
	mut_reg pc= mut_exec_addr_to_long(addr);
	mut_reg npc= pc+4;

	mut_assert_pre(p->status == mut_process_status_stopped);

	return mut_trace_write_reg(&p->trace, mut_process_arch_regs_pc, pc)
		&& mut_trace_write_reg(&p->trace, mut_process_arch_regs_npc, npc);
}


/*
 * <URI:mut/bib/sparc9#arg>
 */
#define mut_process_n_arg_regs 6

int mut_process_read_arg(mut_process *p, size_t n, mut_arg *arg)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(n < mut_process_n_arg_regs);
	return mut_trace_read_reg(&p->trace, mut_process_arch_regs_o0+n, arg);
}


int mut_process_write_arg(mut_process *p, size_t n, mut_arg arg)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	mut_assert_pre(n < mut_process_n_arg_regs);

	return mut_trace_write_reg(&p->trace, mut_process_arch_regs_o0+n, arg);
}



int mut_process_function_return_addr(mut_process *p, mut_exec_addr *addr)
{
	unsigned int o7;

	mut_assert_pre(p->status == mut_process_status_stopped);

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_o7, &o7))
		return 0;
	*addr = mut_exec_addr_from_int(o7+8); /* <URI:mut/bib/sparc9#call/return> */
	return 1;
}



int mut_process_read_result(mut_process *p, mut_arg *result)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	return mut_trace_read_reg(&p->trace, mut_process_arch_regs_o0, result);
}


int mut_process_write_result(mut_process *p, mut_arg result)
{
	mut_assert_pre(p->status == mut_process_status_stopped);
	
	return mut_trace_write_reg(&p->trace, mut_process_arch_regs_o0, result);
}



/*
** Fold this in to mut_process_function_backtrace -- the only reason it
** is separate is historical.
**
*/
static int mut_process_function_xxx_backtrace(mut_process *p, mut_backtrace *bt)
{
	mut_exec_addr fp_addr, link_addr;
	int fp, link;
	int status;

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_o6, &fp))
		return 0;
	for (; fp != 0; ) {
		link_addr = mut_exec_addr_from_int(fp + 15*mut_reg_size);
		if (!mut_trace_read_data(&p->trace, link_addr, &link))
			return 0;
		status = mut_backtrace_enter(bt, mut_exec_addr_from_int(link));
		if (status < 0)  return 1;
		if (status == 0) return 0;
		fp_addr= mut_exec_addr_from_int(fp + 14*mut_reg_size);
		if (!mut_trace_read_data(&p->trace, fp_addr, &fp))
			return 0;
	}
	return 1;
}



int mut_process_function_backtrace(mut_process * p, mut_backtrace * bt)
{
	int link, status;

	if (!mut_trace_read_reg(&p->trace, mut_process_arch_regs_o7, &link))
		return 0;
	status = mut_backtrace_enter(bt, mut_exec_addr_from_int(link));
	if (status < 0)  return 1;
	if (status == 0) return 0;
	return mut_process_function_xxx_backtrace(p, bt);
}



/*
 * On a SPARC using standard calling conventions just need to set the pc
 * to ensure that the function is skipped.
 */
int mut_process_skip_to(mut_process *p, mut_exec_addr pc)
{
	mut_assert_pre(p->status == mut_process_status_stopped);

	p->pc = pc;
	return 1;
}
